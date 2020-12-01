
%module anasazi_model
%{
  #define SWIG_FILE_WITH_INIT
  #include <boost/mpi.hpp>
  
  extern void anasazi_model(int* result, unsigned int k, char * config_file, char * parameters_file, const MPI_Comm & comm);
%}

%include "numpy.i"

%init %{
  import_array();
%}

%apply (int* ARGOUT_ARRAY1, int DIM1 ) {(int* result, unsigned int k)};

%include mpi4py.i
%mpi4py_typemap(Comm, MPI_Comm);

extern void anasazi_model(int* result, unsigned int k, char * config_file, char * parameters_file, const MPI_Comm & comm);
