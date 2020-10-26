#include <boost/mpi.hpp>
#include "repast_hpc/RepastProcess.h"

#include "Model.h"
#include "Location.h"
#include "Household.h"
#include <iomanip>


void test6(int argc, char** argv)
{
	/* Test setup ---------------------------------------------------*/

	std::string configFile = argv[1]; // The name of the configuration file
	std::string propsFile  = argv[2]; // The name of the properties file
	boost::mpi::environment env(argc, argv);
	boost::mpi::communicator* world;

	repast::RepastProcess::init(configFile);
	world = new boost::mpi::communicator; // confirm if this is needed

	/* Test body ------------------------------------------------------*/
	AnasaziModel* model = new AnasaziModel(propsFile, argc, argv, world);
	model->initAgents();
	model->testRelocateHousehold();

	/* Teardown-----------------------------------------------------*/
	delete model;
	repast::RepastProcess::instance()->done();
}

int main(int argc, char** argv){
	
    std::cout << "\nStarting test 1...\n";
    /*Insert test 1 function here*/
    std::cout << "----------------------------------\n\n";

    std::cout << "Starting test 2...\n";
    /*Insert test 2 function here*/
    std::cout << "----------------------------------\n\n";

    std::cout << "Starting test 3...\n";
    /*Insert test 3 function here*/
    std::cout << "----------------------------------\n\n";

    std::cout << "Starting test 4...\n";
    /*Insert test 4 function here*/
    std::cout << "----------------------------------\n\n";

    std::cout << "Starting test 5...\n";
    /*Insert test 5 function here*/
    std::cout << "----------------------------------\n\n";
    
    std::cout << "Starting test 6...\n";
    test6(argc, argv);
    std::cout << "----------------------------------\n\n";
    
    return 0;
}
