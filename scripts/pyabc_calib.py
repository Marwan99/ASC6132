import os
import csv
import pyabc
import tempfile
import numpy as np
from mpi4py import MPI
from anasazi_cpp.anasazi_model import anasazi_model

model_call_cntr = 0

def model(parameters):
    global model_call_cntr
    model_call_cntr += 1
    # print("calling model")
    # print(parameters)
    # print(parameters['max_storage'])
    int_params = []
    double_params = []
    
    int_params.append(parameters.max_store_year)
    int_params.append(parameters.max_storage)
    int_params.append(parameters.household_need)
    int_params.append(parameters.min_fission)
    int_params.append(parameters.max_fission)
    int_params.append(parameters.min_death)
    int_params.append(parameters.max_death)
    int_params.append(parameters.max_distance)
    int_params.append(parameters.initial_min)
    int_params.append(parameters.initial_max)
    
#     print(int_params)
    
    double_params.append(parameters.annual_variance)
    double_params.append(parameters.spatial_variance)
    double_params.append(parameters.fertility_prop)
    double_params.append(parameters.harvest_adj)
    double_params.append(parameters.new_household)
#     print(double_params)
    
    ret  = anasazi_model(551, int_params, double_params).tolist()
    # print(type(ret))

    return {"population": ret}

def distance(x, y):
    error = np.array([a_i - b_i for a_i, b_i in zip(x["population"], y["population"])])
    rmse = np.sqrt(np.mean(error**2))
    print("******************" + str(rmse))
    return rmse

target_data = []
with open("../data/target_data.csv") as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    for row in csv_reader:
        target_data.append(int(row[1]))

observed_data = {"population": target_data}

parameter_priors = pyabc.Distribution(
    max_store_year = pyabc.RV("randint", 1, 3),
    max_storage = pyabc.RV("randint", 1400, 1600),
    household_need = pyabc.RV("randint", 700, 900),
    min_fission = pyabc.RV("randint", 16, 18),
    max_fission = pyabc.RV("randint", 25, 27),
    min_death = pyabc.RV("randint", 24, 27),
    max_death = pyabc.RV("randint", 35, 38),
    max_distance = pyabc.RV("randint", 900, 1100),
    initial_min = pyabc.RV("randint", 800, 1200),
    initial_max = pyabc.RV("randint", 1400, 1600),
    annual_variance = pyabc.RV("uniform", 0, 0.5),
    spatial_variance = pyabc.RV("uniform", 0, 0.5),
    fertility_prop = pyabc.RV("uniform", 0, 0.5),
    harvest_adj = pyabc.RV("uniform", 0, 0.5),
    new_household = pyabc.RV("uniform", 0, 0.5))  

MPI.COMM_WORLD

abc = pyabc.inference.ABCSMC(model, parameter_priors,distance,
    population_size=30,
    # sampler=pyabc.sampler.SingleCoreSampler(check_max_eval=True))
    sampler=pyabc.sampler.MulticoreEvalParallelSampler(check_max_eval=True))

db_path = ("sqlite:///" +
           os.path.join(tempfile.gettempdir(), "test.db"))
history = abc.new(db_path, observed_data)

history = abc.run(minimum_epsilon=0.2, max_total_nr_simulations=4)
# history = abc.run(minimum_epsilon=0.2, max_nr_populations=2)

print(model_call_cntr)
