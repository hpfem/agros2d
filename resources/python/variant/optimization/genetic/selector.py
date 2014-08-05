from variant.optimization.genetic.info import GeneticInfo
from copy import deepcopy

class SurvivorsSelector:
    """General class for selection of genoms that should be kept into the new population."""

    def __init__(self, functionals):
        self.functionals = functionals
        self.recomended_population_size = 0

    def select(population):
        pass

class SingleCriteriaSelector(SurvivorsSelector):
    """Selector of genoms, that should be kept to next generation in the case of single criteria optimization."""

    def select(self, population):
        assert not self.functionals.multicriteria()
        direction = self.functionals.functional().direction_sign()

        scores = []
        for genom in population:
            scores.append(self.functionals.evaluate(genom))
        scores.sort(reverse=bool(direction != 1))

        survivors_number = min(len(population), int(self.recomended_population_size/2))
        priority_tresholds = [scores[survivors_number-1],
                              scores[int(survivors_number*0.8)-1],
                              scores[int(survivors_number*0.5)-1]]

        survivors = []
        for genom in population:
            new_genom = deepcopy(genom)
            score = self.functionals.evaluate(genom)

            priority = 0
            for index in range(len(priority_tresholds)):
                if direction * score <= direction * priority_tresholds[index]:
                    priority = index + 1

            if priority > 0:
                GeneticInfo.set_priority(new_genom, priority)
                survivors.append(new_genom)

        return survivors

class MultiCriteriaSelector(SurvivorsSelector):

    def is_front(self, index, population, direction):
        [sign_f, sign_g] = direction
        member = population[index]
        [val_f, val_g] = self.functionals.evaluate(member)
        
        for i in range(len(population)):
            test_member = population[i]
            [test_f, test_g] = self.functionals.evaluate(test_member)
            
            if sign_f * test_f <  sign_f * val_f and sign_g * test_g <= sign_g * val_g:
                return False
            
            if sign_f * test_f <= sign_f * val_f and sign_g * test_g <  sign_g * val_g:
                return False
                
        return True

    def set_priority(self, member, priority):
        if (priority > member.priority):
            member.priority = priority

    def select(self, population):
        assert self.functionals.isMulticriterial()
        sign_f = self.functionals.functional(0).directionSign()
        sign_g = self.functionals.functional(1).directionSign()
        direction = [sign_f, sign_g]
        
        #create deep copy of population and assign implicit priority (with how big probability we should select this element to mutations or crossings 1..3, 3 highest)
        population_copy = []
        for member in population:
            newMember = deepcopy(member)
            newMember.priority = 1            
            population_copy.append(newMember)
        
        # what should remain to next step? Indices to the population array
        include = set()
        last_pop_size = len(population)
        
        #calculate average values of each functional for norming
        avg_f = 0
        avg_g = 0
        for member in population_copy:
            [val_f, val_g] = self.functionals.evaluate(member)
            avg_f += val_f
            avg_g += val_g
        avg_f /= last_pop_size
        avg_g /= last_pop_size
        
        # contains triples [fval, gval, index], to be sorted
        front = []
        for i in range(last_pop_size):
            if self.is_front(i, population_copy, direction):
                [val_f, val_g] = self.functionals.evaluate(population[i])
                front.append([val_f, val_g, i])
        
        #sort front according to first functional value
        front.sort()
        
        #find largest distances between neighbouring individuals on the front
        distances = []
        for i in range(len(front) - 1):
            distances.append([((front[i+1][0] - front[i][0])**2 + (front[i+1][1] - front[i][1])**2)**0.5, i])
        distances.sort()
        
        for i in range(len(front)):
            include.add(front[i][2])
            
        front_len = len(include)
        
        #on both sides of the front increase priorities
        front_ends_indices = [0, 1, 2, 3, len(front)-4, len(front)-3, len(front)-2, len(front)-1]
        front_ends_priorities = [3, 3, 2, 2, 2, 2, 3, 3]
        
        i = 0
        while (i < len(front_ends_indices)):
            if (front_ends_indices[i] not in range(front_len)):
                front_ends_indices.pop(i)
                front_ends_priorities.pop(i)
            else:
                i += 1
                
        for i in range(len(front_ends_indices)):
            front_member_idx = front[front_ends_indices[i]][2]            
            self.set_priority(population_copy[front_member_idx], front_ends_priorities[i])
                
        #high priority for several largest holes        
        if (front_len > 3):
            for num_hole in range(3):
                index_front = distances[-1-num_hole][1]
                self.set_priority(population_copy[index_front-1], 2)
                self.set_priority(population_copy[index_front], 3)
                self.set_priority(population_copy[index_front+1], 3)
                self.set_priority(population_copy[index_front+2], 2)
        
            
        ##debug print:
        #print front
        #print include
        #print distances
        #priority_print = []
        #for i in range(len(front)):
            #priority_print.append(priority[front[i][2]])
        #print priority_print
        
            
        # for each linear combination of two functionals keep several best 
        ratios = [0.01, 0.03, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.95, 0.97, 0.99]
        
        # to store pairs [ratio, values], where values is list of [value, index] pair related to given ratio
        values_ratios = []
        for ratio in ratios:
            values = []
            # for each ratio go over all population and determine the distance from line given by the ratio
            for i in range(last_pop_size):
                member = population_copy[i]
                [val_f, val_g] = self.functionals.evaluate(member)

                # this is the quality with respect to the ratio (line direction)
                value = ratio * sign_f * val_f / avg_f + (1-ratio) * sign_g * val_g / avg_g
                
                # add value and index, so that the array may be sorted according to value (quality)
                values.append([value, i])
            values.sort()
            values_ratios.append(values)
        
        level = 0
        while (len(include) < 0.6 * self.recomended_population_size or len(include) - front_len < 0.25 * self.recomended_population_size):
            for ratio_idx in range(len(ratios)):
                candidate_idx = values_ratios[ratio_idx][level][1]
                
                # it will often hapen, that candidate_idx is allready there. It is ok, since include is set
                include.add(candidate_idx)
                
                # the best with higher priority    
                if (level < 3):
                    priority = 2
                else:
                    priority = 1
                    
                # set priority (if allready set, this function makes maximum)
                self.set_priority(population_copy[candidate_idx], priority)
            level += 1
            if (level > 0.9*last_pop_size):
                break
                
        survivors = []
        for i in range(last_pop_size):
            if i in include:
                survivors.append(population_copy[i])        
                
        return survivors

if __name__ == '__main__':
    from variant import ModelDict
    from variant.optimization import Functionals, Functional

    from test_suite.optilab.examples import quadratic_function

    md = ModelDict()
    variants = range(1000)
    for x in variants:
        model = quadratic_function.QuadraticFunction()
        model.parameters['x'] = x
        GeneticInfo.set_population_from(model, 0)
        GeneticInfo.set_population_to(model, 0)
        md.add_model(model)

    md.solve(save=False)

    functionals = Functionals(Functional('F', 'min'))
    selector = SingleCriteriaSelector(functionals)
    selector.recomended_population_size = len(variants)

    selected = selector.select(md.models)
    print(len(selected))
