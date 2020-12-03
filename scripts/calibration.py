import csv
from pathlib import Path
from abcpy.distances import Euclidean
from abcpy.statistics import Identity
from abcpy.inferences import RejectionABC
from abcpy.backends import BackendDummy as Backend
# from abcpy.backends import BackendMPI as Backend
from abcpy.output import Journal
from abcpy.continuousmodels import Uniform, Normal as Gaussian
from abcpy.discretemodels import DiscreteUniform
from anasazi import Anasazi
import numpy as np


modelParameters = [
    DiscreteUniform([1, 3], name="max.store.year"),         # "max.store.year",
    DiscreteUniform([1000, 2000], name="max.storage"),      # "max.storage",
    DiscreteUniform([600, 1000], name="household.need"),    # "household.need",
    DiscreteUniform([12, 21], name="min.fission.age",),     # "min.fission.age",
    DiscreteUniform([24, 30], name="max.fission.age",),     # "max.fission.age",
    DiscreteUniform([20, 30], name="min.death.age"),        # "min.death.age",
    DiscreteUniform([30, 40], name="max.death.age"),        # "max.death.age",
    DiscreteUniform([500, 1500], name="max.distance"),      # "max.distance",
    DiscreteUniform([150, 200], name="initial.min.corn"),   # "initial.min.corn",
    DiscreteUniform([150, 200], name="initial.max.corn"),   # "initial.max.corn",
    
    Uniform([[150], [200]], name="annual.variance"),    # "annual.variance",
    Uniform([[150], [200]], name="spatial.variance",),  # "spatial.variance",
    Uniform([[150], [200]], name="fertility.prop"),     # "fertility.prop",
    Uniform([[150], [200]], name="harvest.adj"),        # "harvest.adj",
    Uniform([[0], [1]], name="new.household.ini.maize") # "new.household.ini.maize"
]

backend = Backend()

anasaziModel = Anasazi(modelParameters, name='anasazi')

statistics_calculator = Identity(degree=2, cross=False)
distance_calculator = Euclidean(statistics_calculator)
sampler = RejectionABC([anasaziModel], [distance_calculator], backend, seed=1)

n_sample, n_samples_per_param = 2, 1  # For some reason we get a seg fault with n_samples_per_param = 10
epsilon = 5000

target_data_file = ("../data/target_data.csv")
target_data = []
with open(target_data_file) as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    for row in csv_reader:
        target_data.append(np.array([int(row[1])]))

# print(target_data)

journal = sampler.sample([target_data], n_sample, n_samples_per_param, epsilon)

