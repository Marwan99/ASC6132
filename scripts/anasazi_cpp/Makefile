SWIG=swig
SWIGFLAGS=-python -c++
WGET=wget -q

CC=g++
CPPFLAGS=-fPIC
INCLUDEPATH=$(shell python3-config --includes)
INCLUDEPATHNUMPY=$(shell python3 -c 'import numpy as np; print(np.get_include())')
PYTHONLINKERSETTINGS=$(shell python3-config --ldflags | cut -d" " -f 1)
PYTHONLIBS=$(shell python3-config --libs)

MPICXX=$(HOME)/sfw/MPICH/bin/mpicxx -std=c++11 -w

MPI_INCLUDE=-I$(HOME)/sfw/MPICH/include/
# MPI_LIB_DIR=-L$(HOME)/sfw/Boost/Boost_1.61/lib/
# MPI_LIBS=-lboost_mpi-mt -lboost_serialization-mt -lboost_system-mt -lboost_filesystem-mt

BOOST_INCLUDE=-I$(HOME)/sfw/Boost/Boost_1.61/include/
BOOST_LIB_DIR=-L$(HOME)/sfw/Boost/Boost_1.61/lib/
BOOST_LIBS=-lboost_mpi-mt -lboost_serialization-mt -lboost_system-mt -lboost_filesystem-mt

REPAST_HPC_INCLUDE=-I$(HOME)/sfw/repast_hpc-2.3.0/include
REPAST_HPC_LIB_DIR=-L$(HOME)/sfw/repast_hpc-2.3.0/lib
REPAST_HPC_LIB=-lrepast_hpc-2.3.0 -lrelogo-2.3.0

MODEL_OBJ_DIR=Model.o Household.o Location.o anasazi_model.o
MODEL_SRC_DIR=../../src
MODEL_INCLUDE_DIR=-I../../include


# all: library.cpp main.cpp
# # In this case:
# $@ evaluates to all
# $< evaluates to library.cpp
# $^ evaluates to library.cpp main.cpp


cpp: build_model _anasazi_model.so anasazi_model.py clean_objects

all: clean_all cpp

build_model:
	$(MPICXX) -O3 $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -fPIC $(MODEL_INCLUDE_DIR) -c $(MODEL_SRC_DIR)/anasazi_model.cpp -o anasazi_model.o
	$(MPICXX) -O3 $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -fPIC $(MODEL_INCLUDE_DIR) -c $(MODEL_SRC_DIR)/Model.cpp -o Model.o
	$(MPICXX) -O3 $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -fPIC $(MODEL_INCLUDE_DIR) -c $(MODEL_SRC_DIR)/Household.cpp -o Household.o
	$(MPICXX) -O3 $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -fPIC $(MODEL_INCLUDE_DIR) -c $(MODEL_SRC_DIR)/Location.cpp -o Location.o

clean_objects:
	rm $(MODEL_OBJ_DIR) mpi4py.i* numpy.i*

clean_all:
	rm _anasazi_model.so anasazi_model.py

%.py: %.i
	$(SWIG) $(SWIGFLAGS) -o $@ $<

%_wrap.cpp: %.i numpy.i
	$(SWIG) $(SWIGFLAGS) -o $@ $<

%.o: %.cpp
	echo "Build 1"
	# $(BOOST_INCLUDE)
	$(CC) -O3 $(CPPFLAGS) $(MPI_INCLUDE) $(BOOST_INCLUDE) -I $(INCLUDEPATHNUMPY) $(INCLUDEPATH)  -fno-lto -c $< -o $@

_%.so: %_wrap.o
	echo "Build 2"
	$(CC) -shared $(MODEL_OBJ_DIR) $^ $(BOOST_LIB_DIR) $(REPAST_HPC_LIB_DIR) $(REPAST_HPC_LIB) $(BOOST_LIBS) $(PYTHONLINKERSETTINGS) $(PYTHONLIBS) -fno-lto -o  $@

%.i:
	$(WGET) "https://raw.githubusercontent.com/numpy/numpy/master/tools/swig/numpy.i"
	$(WGET) "https://raw.githubusercontent.com/mpi4py/mpi4py/master/src/mpi4py/include/mpi4py/mpi4py.i"
