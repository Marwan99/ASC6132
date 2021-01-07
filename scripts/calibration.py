import csv
import pyspark
import logging
import numpy as np
from pathlib import Path
from anasazi import Anasazi
from abcpy.distances import Euclidean
from abcpy.statistics import Identity
from abcpy.inferences import PMCABC
from abcpy.backends import BackendSpark, BackendDummy
from abcpy.perturbationkernel import DefaultKernel
from abcpy.output import Journal
from abcpy.continuousmodels import Uniform
from abcpy.discretemodels import DiscreteUniform


modelParameters = [
    DiscreteUniform([1, 3], name="max.store.year"),         # max.store.year = 2
    DiscreteUniform([1500, 1700], name="max.storage"),      # max.storage = 1600
    DiscreteUniform([700, 900], name="household.need"),     # household.need = 800
    DiscreteUniform([16, 18], name="min.fission.age",),     # min.fission.age = 17
    DiscreteUniform([25, 29], name="max.fission.age",),     # max.fission.age = 27
    DiscreteUniform([24, 26], name="min.death.age"),        # min.death.age = 25
    DiscreteUniform([34, 38], name="max.death.age"),        # max.death.age = 36
    DiscreteUniform([900, 1100], name="max.distance"),      # max.distance = 1000
    DiscreteUniform([800, 1200], name="initial.min.corn"),  # initial.min.corn = 1000
    DiscreteUniform([1400, 1700], name="initial.max.corn"), # initial.max.corn = 1600
    
    Uniform([[0.05], [0.15]], name="annual.variance"),      # annual.variance = 0.10000
    Uniform([[0.05], [0.15]], name="spatial.variance",),    # spatial.variance = 0.10000
    Uniform([[0.05], [0.15]], name="fertility.prop"),       # fertility.prop = 0.11489
    Uniform([[0.5], [1.1]], name="harvest.adj"),            # harvest.adj = 0.97621
    Uniform([[0.1], [0.7]], name="new.household.ini.maize") # new.household.ini.maize = 0.33000
]

target_data = []
with open("../data/target_data.csv") as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    for row in csv_reader:
        target_data.append(np.array([int(row[1])]))

anasaziModel = Anasazi(modelParameters, name='anasazi')

statistics_calculator = Identity(degree=2, cross=False)
distance_calculator = Euclidean(statistics_calculator)

sc = pyspark.SparkContext()
backend = BackendSpark(sc, parallelism=12)
# backend = BackendDummy()

logging.basicConfig(level=logging.DEBUG)

kernel = DefaultKernel(modelParameters)

sampler = PMCABC([anasaziModel], [distance_calculator], backend, kernel=kernel, seed=1)

steps = 2
eps_arr = np.array([1.4e+04])
n_sample = 10
n_samples_per_param = 1
epsilon_percentile = 10

journal = sampler.sample([target_data], steps, eps_arr, n_sample, n_samples_per_param, epsilon_percentile, full_output=1)

print(journal.posterior_mean())

# journal.plot_ESS()
# journal.Wass_convergence_plot()
