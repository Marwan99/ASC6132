from abcpy.probabilisticmodels import ProbabilisticModel, Discrete, InputConnector
from anasazi_cpp.anasazi_model import anasazi_model
import numpy as np
from mpi4py import MPI

# creating a class that inherits from ProbabilisticModel (API base class)
class Anasazi(ProbabilisticModel):
    def __init__(self, parameters, name='Anasazi'):
        # We expect input of type parameters ....
        if not isinstance(parameters, list):
            raise TypeError('Input of Normal model is of type list')

        # TODO: set number of paramters 
        # if len(parameters) != 2:
        #     raise RuntimeError('Input list must be of length 2, containing [mu, sigma].')

        input_connector = InputConnector.from_list(parameters)
        super().__init__(input_connector, name)

    def _check_input(self, input_values):
        # TODO: implement if needed
        return True

    def forward_simulate(self, input_values, k, rng=np.random.RandomState(), mpi_comm=None):
        print("--- forward sim started ---")

        if (mpi_comm == None):
            print ("NO MPI")
        else:
            print ("Using MPI")
        
        modelProps = [
            "random.seed",
            "count.of.agents",
            "board.size.x",
            "board.size.y",
            "proc.per.x",
            "proc.per.y",
            "grid.buffer",
            "start.year",
            "end.year",
            "max.store.year",
            "max.storage",
            "household.need",
            "min.fission.age",
            "max.fission.age",
            "min.death.age",
            "max.death.age",
            "max.distance",
            "initial.min.corn",
            "initial.max.corn",
            "annual.variance",
            "spatial.variance",
            "fertility.prop",
            "harvest.adj",
            "result.file",
            "new.household.ini.maize"
        ]

        assert len(modelProps) == len(input_values)

        modelPropsDict = dict(zip(modelProps, input_values))

        params_file = "model.props"
        with open(params_file, 'w') as writer:
            for key, val in modelPropsDict.items():
                writer.write(key + " = " + str(val) + "\n")

        config_file_path = "../props/config.props"

        vector_of_k_samples = anasazi_model(551, config_file_path, params_file)

        # Format the output to obey API
        result = [np.array([x]) for x in vector_of_k_samples]
        
        # print(result)

        print("*** forward sim completed ***")
        return result

    def _check_output(self, values):
        # TODO: implement if needed
        # At this point values is a number (int, float); full domain for Normal is allowed

        # check output

        # write output
        return True
    
    def get_output_dimension(self):
        return 1