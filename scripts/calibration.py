import csv
from pathlib import Path
from abcpy.distances import Euclidean
from abcpy.statistics import Identity
from abcpy.inferences import RejectionABC
from abcpy.backends import BackendMPI as Backend
from abcpy.output import Journal
from abcpy.continuousmodels import Uniform, Normal as Gaussian
from anasazi import Anasazi


modelParameters = [
    Uniform([[1], [1]], name="random.seed"),            # "random.seed",
    Uniform([[14], [14]], name="count.of.agents"),      # "count.of.agents,
    Uniform([[80], [80]], name="board.size.x"),         # "board.size.x",
    Uniform([[120], [120]], name="board.size.y"),       # "board.size.y",
    Uniform([[1], [1]], name="proc.per.x"),             # "proc.per.x",
    Uniform([[1], [1]], name="proc.per.y"),             # "proc.per.y",
    Uniform([[0], [0]], name="grid.buffer"),            # "grid.buffer",
    Uniform([[800], [800]], name="start.year"),         # "start.year",
    Uniform([[1000], [1000]], name="end.year"),         # "end.year",
    Uniform([[1], [3]], name="max.store.year"),         # "max.store.year",
    Uniform([[1000], [2000]], name="max.storage"),      # "max.storage",
    Uniform([[600], [1000]], name="household.need"),    # "household.need",
    Uniform([[12], [21]], name="min.fission.age",),     # "min.fission.age",
    Uniform([[24], [30]], name="max.fission.age",),     # "max.fission.age",
    Uniform([[20], [30]], name="min.death.age"),        # "min.death.age",
    Uniform([[30], [40]], name="max.death.age"),        # "max.death.age",
    Uniform([[500], [1500]], name="max.distance"),      # "max.distance",
    Uniform([[150], [200]], name="initial.min.corn"),   # "initial.min.corn",
    Uniform([[150], [200]], name="initial.max.corn"),   # "initial.max.corn",
    Uniform([[150], [200]], name="annual.variance"),    # "annual.variance",
    Uniform([[150], [200]], name="spatial.variance",),  # "spatial.variance",
    Uniform([[150], [200]], name="fertility.prop"),     # "fertility.prop",
    Uniform([[150], [200]], name="harvest.adj"),        # "harvest.adj",
    Uniform([[150], [200]], name="result.file"),        # "result.file",
    Uniform([[0], [1]], name="new.household.ini.maize") # "new.household.ini.maize"
]

backend = Backend()

anasaziModel = Anasazi(modelParameters, name='anasazi')

statistics_calculator = Identity(degree=2, cross=False)
distance_calculator = Euclidean(statistics_calculator)
sampler = RejectionABC([anasaziModel], [distance_calculator], backend, seed=1)

n_sample, n_samples_per_param = 2, 10
epsilon = 5000

target_data_file = ("../data/target_data.csv")
target_data = []
with open(target_data_file) as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    for row in csv_reader:
        target_data.append(row[1])

# print(target_data)

journal = sampler.sample(target_data, n_sample, n_samples_per_param, epsilon)

