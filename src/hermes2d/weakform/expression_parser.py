from __future__ import division
from pyparsing import (Literal,CaselessLiteral, Word, Keyword, Combine,Group,Optional,
                       ZeroOrMore,Forward,nums,alphas,oneOf)

class NumericStringParser(object):

    def pushFirst(self, strg, loc, toks ):        
        self.exprStack.append( toks[0] )

    def pushUMinus(self, strg, loc, toks ):
        if toks and toks[0]=='-': 
            self.exprStack.append( 'unary -' )

    def __init__(self, symbols, replaces, variables, without_variables):
        
        self.greek_letters = ('alpha', 'beta', 'gamma', 'delta', 'epsilon', 'zeta', 
                         'eta', 'theta', 'iota', 'kappa', 'lambda', 'mu', 'nu', 
                         'xi', 'omicron', 'pi', 'rho', 'sigma', 'tau', 
                         'upsilon', 'phi', 'chi', 'psi', 'omega')
        
        self.functions = ('cos', 'sin')
        
        self.string = ''            
        self.replaces = replaces
        self.variables = variables
        self.without_variables = without_variables         
        point = Literal( "." )
        e = CaselessLiteral( "E" )
        fnumber = Combine( Word( "+-"+nums, nums ) + 
                           Optional( point + Optional( Word( nums ) ) ) +
                           Optional( e + Word( "+-"+nums, nums ) ) )
        ident = Word(alphas, alphas+nums+"_$")       
        plus  = Literal( "+" )
        minus = Literal( "-" )
        mult  = Literal( "*" )
        div   = Literal( "/" )
        lpar  = Literal( "(" ).suppress()
        rpar  = Literal( ")" ).suppress()
        addop  = plus | minus
        multop = mult | div
        expop = Literal( "^" )
        expr = Forward()
        literals = Keyword("value")
        symbols.append("value")        
        for symbol in symbols:
            literals |= Keyword(symbol)
        
        atom = ((Optional(oneOf("- +")) +
                 (literals|fnumber|ident+lpar+expr+rpar).setParseAction(self.pushFirst))
                | Optional(oneOf("- +")) + Group(lpar+expr+rpar)
                ).setParseAction(self.pushUMinus)       

#        atom = atom + vdx.setParseAction(self.pushFirst)        
        
        factor = Forward()
        factor << atom + ZeroOrMore( ( expop + factor ).setParseAction( self.pushFirst ) )
        term = factor + ZeroOrMore( ( multop + factor ).setParseAction( self.pushFirst ) )
        expr << term + ZeroOrMore( ( addop + term ).setParseAction( self.pushFirst ) )
        self.bnf = expr
    
    def parse(self,num_string):
        self.exprStack=[]        
        return self.bnf.parseString(num_string,True)
        
#    def translate_to_cpp(self, expression_list):        
#        string = ''                        
#        for item in expression_list:                                            
#            if (type(item) is list):                    
#                string += '(' + self.translate_to_cpp(item) + ')'                
#            else:                                                
#                if item in self.variables:                   
#                    if self.without_variables:
#                        string += '1'
#                    else:
#                        string += item + '.value()'                    
#                else:    
#                    if item in self.replaces.iterkeys(): 
#                        string += self.replaces[item]
#                    else:
#                        string += item                                  
#        return string

    def translate_to_cpp(self, expression_list):        
        string = ''                        
        expression_list.reverse()
        while not(len(expression_list) == 0):
            item = expression_list.pop()                                                       
            if (type(item) is list):                    
                string += '(' + self.translate_to_cpp(item) + ')'                
            elif item in self.functions:
                string += item + '(' + self.translate_to_cpp(expression_list.pop()) + ')'                 
            else:                                                
                if item in self.variables:                   
                    if self.without_variables:
                        string += '1'
                    else:
                        string += item + '.value()'                    
                else:    
                    if item in self.replaces.iterkeys(): 
                        string += self.replaces[item]
                    else:
                        string += item                                  
        return string
    
    def translate_to_latex(self, expression_list):
        string = ''                        
        for item in expression_list:                                            
            if (type(item) is list):                    
                string += '(' + self.translate_to_latex(item) + ')'                
            else:                                                
                if item in self.replaces.iterkeys(): 
                    string += self.replaces[item]                    
                elif item in self.greek_letters:
                    string += '\\' + item
                elif item in self.variables:
                    string += item[0] + '_{' + item[1:] + '}'
                else:
                    string += item                                  
        return string