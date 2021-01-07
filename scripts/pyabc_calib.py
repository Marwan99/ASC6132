import os
import csv
import pyabc
import tempfile
import numpy as np
from mpi4py import MPI
from anasazi_cpp.anasazi_model import anasazi_model


def model(parameters):
    int_params = []
    double_params = []

    int_params.append(int(parameters.max_store_year))
    int_params.append(int(parameters.max_storage))
    int_params.append(int(parameters.household_need))
    int_params.append(int(parameters.min_fission))
    int_params.append(int(parameters.max_fission))
    int_params.append(int(parameters.min_death))
    int_params.append(int(parameters.max_death))
    int_params.append(int(parameters.max_distance))
    int_params.append(int(parameters.initial_min))
    int_params.append(int(parameters.initial_max))
    # print(int_params)

    double_params.append(0.10000)
    double_params.append(0.10000)
    double_params.append(0.11489)
    double_params.append(0.97621)
    double_params.append(0.33000)
#     print(double_params)
    
    ret  = anasazi_model(551, int_params, double_params).tolist()
    # print(type(ret))

    return {"population": ret}

def distance(x, y):
    error = np.array([a_i - b_i for a_i, b_i in zip(x["population"], y["population"])])
    rmse = np.sqrt(np.mean(error**2))
    # print("******************" + str(rmse))
    return rmse

target_data = []
with open("../data/target_data.csv") as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    for row in csv_reader:
        target_data.append(int(row[1]))

observed_data = {"population": target_data}

parameter_priors = pyabc.Distribution(
    max_store_year = pyabc.RV("uniform", 1, 2),
    max_storage = pyabc.RV("uniform", 1400, 200),
    household_need = pyabc.RV("uniform", 700, 20),
    min_fission = pyabc.RV("uniform", 16, 2),
    max_fission = pyabc.RV("uniform", 25, 2),
    min_death = pyabc.RV("uniform", 24, 5),
    max_death = pyabc.RV("uniform", 35, 3),
    max_distance = pyabc.RV("uniform", 900, 200),
    initial_min = pyabc.RV("uniform", 800, 400),
    initial_max = pyabc.RV("uniform", 1400, 200))
    # annual_variance = pyabc.RV("uniform", 0, 0.5),
    # spatial_variance = pyabc.RV("uniform", 0, 0.5),
    # fertility_prop = pyabc.RV("uniform", 0, 0.5),
    # harvest_adj = pyabc.RV("uniform", 0, 0.5),
    # new_household = pyabc.RV("uniform", 0, 0.5))

MPI.COMM_WORLD

abc = pyabc.inference.ABCSMC(model, parameter_priors,distance,
    population_size=15,
    # sampler=pyabc.sampler.SingleCoreSampler(check_max_eval=True))
    sampler=pyabc.sampler.MulticoreEvalParallelSampler(check_max_eval=True))

db_path = ("sqlite:///" +
           os.path.join(tempfile.gettempdir(), "test.db"))
history = abc.new(db_path, observed_data)

history = abc.run(minimum_epsilon=30, max_nr_populations=8)
