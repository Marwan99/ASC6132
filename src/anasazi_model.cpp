#include <boost/mpi.hpp>
#include "repast_hpc/RepastProcess.h"

#include "Model.h"
#include "Location.h"
#include "Household.h"
#include <iomanip>

void anasazi_model(int* result, unsigned int k, char * config_file, char * parameters_file, const MPI_Comm & comm)
{
	std::cout << "Inside cpp now.. \n";
	
	std::string configFile(config_file); // The name of the configuration file
	std::string propsFile(parameters_file); // The name of the properties file
	repast::RepastProcess::init(configFile);
	
	std::cout << "Repast initialised\n";
	// boost::mpi::environment env();
	boost::mpi::communicator* world;
	world = new boost::mpi::communicator(comm, boost::mpi::comm_attach);
	std::cout << "world created\n";
	char *argv_mock[2] = {config_file, parameters_file};

	AnasaziModel* model = new AnasaziModel(propsFile, 2, argv_mock, world);
	repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();

	model->initAgents();
	model->initSchedule(runner);

	runner.run();

	k = NUMBER_OF_YEARS;
	result = model->population;

	std::cout << "Simulation complete";

	delete model;
	repast::RepastProcess::instance()->done();
}

int main(int argc, char** argv){
	return 0;
}
