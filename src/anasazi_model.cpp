#include <boost/mpi.hpp>
#include "repast_hpc/RepastProcess.h"

#include "Model.h"
#include "Location.h"
#include "Household.h"
#include <iomanip>

void anasazi_model(int* result, unsigned int k, char * config_file, char * parameters_file, const MPI_Comm & comm)
{
	std::cout << "Inside cpp now " << getpid() << std::endl;
	
	std::string configFile(config_file); // The name of the configuration file
	std::string propsFile(parameters_file); // The name of the properties file
	// std::cout << "got: " << configFile << " and " << propsFile << std::endl;
	
	// boost::mpi::environment env();
	boost::mpi::communicator* world;
	world = new boost::mpi::communicator(comm, boost::mpi::comm_attach);
	std::cout << "world created, rank " << world->rank() << std::endl;

	repast::RepastProcess::init(configFile);
	std::cout << "Repast initialised\n";
	char *argv_mock[3] = {"lol", config_file, parameters_file};

	AnasaziModel* model = new AnasaziModel(propsFile, 3, argv_mock, world);
	repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
	std::cout << "Runner created\n";
	model->initAgents();
	std::cout << "Agent initialized\n";
	model->initSchedule(runner);

	runner.run();

	for(int i = 0; i<551; i++)
		result[i] = model->population[i];

	std::cout << "Simulation complete";

	delete model;
	repast::RepastProcess::instance()->done();
}

int main(int argc, char** argv){
	return 0;
}
