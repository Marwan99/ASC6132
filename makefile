include ./env

.PHONY: create_folders
create_folders:
	mkdir -p objects
	mkdir -p bin

.PHONY: clean_compiled_files
clean_compiled_files:
	rm -f ./objects/*.o
	rm -f ./bin/*.exe

.PHONY: clean
clean:
	rm -rf objects
	rm -rf bin

.PHONY: compile
compile: clean_compiled_files
	$(MPICXX) -O3 $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Main.cpp -o ./objects/Main.o
	$(MPICXX) -O3 $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./tests/test.cpp -o ./objects/Test.o
	$(MPICXX) -O3 $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Model.cpp -o ./objects/Model.o
	$(MPICXX) -O3 $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Household.cpp -o ./objects/Household.o
	$(MPICXX) -O3 $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Location.cpp -o ./objects/Location.o
	$(MPICXX) -O3 $(BOOST_LIB_DIR) $(REPAST_HPC_LIB_DIR) -o ./bin/main.exe  ./objects/Main.o ./objects/Model.o ./objects/Household.o ./objects/Location.o $(REPAST_HPC_LIB) $(BOOST_LIBS)
	$(MPICXX) -O3 $(BOOST_LIB_DIR) $(REPAST_HPC_LIB_DIR) -o ./bin/test.exe  ./objects/Test.o ./objects/Model.o ./objects/Household.o ./objects/Location.o $(REPAST_HPC_LIB) $(BOOST_LIBS)

.PHONY: all
all: clean create_folders compile