
%module anasazi_model
%{
  #define SWIG_FILE_WITH_INIT
  #include <boost/mpi.hpp>
  
  extern void anasazi_model(int* result, unsigned int k, int * int_params, unsigned int int_array_len, double * double_params, unsigned int double_array_len);
%}

%include "numpy.i"

%init %{
  import_array();
%}

%apply (int* ARGOUT_ARRAY1, int DIM1 ) {(int* result, unsigned int k)};
%apply (int* IN_ARRAY1, int DIM1 ) {(int* int_params, unsigned int int_array_len)};
%apply (double* IN_ARRAY1, int DIM1 ) {(double* double_params, unsigned int double_array_len)};

// %include mpi4py.i
// %mpi4py_typemap(Comm, MPI_Comm);

extern void anasazi_model(int* result, unsigned int k, int * int_params, unsigned int int_array_len, double * double_params, unsigned int double_array_len);
