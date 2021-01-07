from abcpy.probabilisticmodels import ProbabilisticModel, Discrete, InputConnector
from anasazi_cpp.anasazi_model import *
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

        # Check min.fission.age is smaller than max.fission.age
        if(input_values[3] > input_values[4]):
            return False

        # Check min.death.age is smaller than max.death.age
        if(input_values[5] > input_values[6]):
            return False

        # Check initial.min.corn is smaller than initial.max.corn
        if(input_values[8] > input_values[9]):
            return False

        return True

    def forward_simulate(self, input_values, k, rng=np.random.RandomState(), mpi_comm=None):
        print("--- forward sim started ---")

        if (mpi_comm == None):
            print ("NO MPI")
        else:
            print ("Using MPI")

        int_params = []
        double_params = []

        for value in input_values:
            if isinstance(value, np.int64):
                int_params.append(value.astype('int32'))
            elif isinstance(value, float):
                double_params.append(value)

        print("int params len: " + str(len(int_params)))
        print("double params len: " + str(len(double_params)))

        vector_of_k_samples = anasazi_model(551, int_params, double_params)

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