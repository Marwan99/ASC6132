#include <boost/mpi.hpp>
#include "repast_hpc/RepastProcess.h"

#include "Model.h"
#include "Location.h"
#include "Household.h"
#include <iomanip>

void anasazi_model(int* result, unsigned int k, int * int_params, unsigned int int_array_len, double * double_params, unsigned int double_array_len)
{
	std::cout << "Inside cpp now, pid:" << getpid() << std::endl;
		
	// char *argv_mock[3] = {"lol", config_file, parameters_file};
	// int lol =3;
	// boost::mpi::environment env;
	boost::mpi::communicator* world;
	world = new boost::mpi::communicator;
	std::cout << "world created... rank " << world->rank() << " of " << world->size() << std::endl;

	repast::RepastProcess::init("../props/config.props");
	// std::cout << "Repast initialised\n";

	AnasaziModel* model = new AnasaziModel(int_params, double_params, world, "../data/");
	repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
	// std::cout << "Runner created\n";
	model->initAgents();
	// std::cout << "Agent initialized\n";
	model->initSchedule(runner);

	// std::cout << "Sim running ";
	runner.run();
	std::cout << std::endl;

	for(int i = 0; i<551; i++)
		result[i] = model->population[i];

	delete model;
	repast::RepastProcess::instance()->done();

	std::cout << "cpp simulation complete\n";
}

int main(int argc, char** argv){
	return 0;
}
