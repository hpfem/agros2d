class GeneticInfo:
    @staticmethod
    def population_from(genom):
        return int(genom.info["_population_from"])

    @staticmethod
    def set_population_from(genom, value):
        genom.info["_population_from"] = value

    @staticmethod
    def population_to(genom):
        return int(genom.info["_population_to"])

    @staticmethod
    def set_population_to(genom, value):
        genom.info["_population_to"] = value

    @staticmethod
    def priority(genom):
        return int(genom.info["_priority"])

    @staticmethod
    def set_priority(genom, value):
        genom.info["_priority"] = value
