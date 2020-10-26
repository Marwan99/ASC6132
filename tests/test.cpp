#include <boost/mpi.hpp>
#include "repast_hpc/RepastProcess.h"

#include "Model.h"
#include "Location.h"
#include "Household.h"
#include <iomanip>

void test1(int argc, char** argv){

    /* Test setup ---------------------------------------------------*/
    std::string configFile = argv[1]; // The name of the configuration file
    std::string propsFile  = argv[2]; // The name of the properties file
    boost::mpi::communicator* world;

    repast::RepastProcess::init(configFile);
    world = new boost::mpi::communicator; // confirm if this is needed

    /* Test body ------------------------------------------------------*/
    AnasaziModel* testModel = new AnasaziModel(propsFile, argc, argv, world);
    testModel->initAgents();
    testModel->testInitAgent();

    /* Teardown-----------------------------------------------------*/
    delete testModel;
    repast::RepastProcess::instance()->done();
}

void test2(int argc, char** argv){

    /* Test setup ---------------------------------------------------*/

    std::string configFile = argv[1]; // The name of the configuration file
    std::string propsFile  = argv[2]; // The name of the properties file
    // boost::mpi::environment env(argc, argv);
    boost::mpi::communicator* world;

    repast::RepastProcess::init(configFile);
    world = new boost::mpi::communicator; // confirm if this is needed

    // std::cout << "Running Test 2." << std::endl;

    double expectedResult = 1.6;
    int zone = 2;

    std::cout << "Expected hydro level for zone " << zone << " in year 800 is: " << expectedResult << std::endl;

    // AnasaziModel* testModel = createTestModel();
    AnasaziModel* testModel = new AnasaziModel(propsFile, argc, argv, world);

    testModel->readCsvHydro();
    double testResult = testModel->hydroLevel(zone);

    std::cout << "Value in model environment is: " << testResult << std::endl;

    // deleteTestModel(testModel);
    delete testModel;
    // delete world;
    repast::RepastProcess::instance()->done();

    if (testResult == expectedResult){
        std::cout << "Test result matches expected result." << std::endl;
        std::cout << "Test 2 successful." << std::endl;
        // return true;
    }
    else{
        std::cout << "Test result does not match expected result." << std::endl;
        std::cout << "Test 2 failed." << std::endl;
        // return false;
    }
}

void test3(int argc, char** argv){

    /* Test setup ---------------------------------------------------*/
    std::string configFile = argv[1]; // The name of the configuration file
    std::string propsFile  = argv[2]; // The name of the properties file
    boost::mpi::communicator* world;

    repast::RepastProcess::init(configFile);
    world = new boost::mpi::communicator; // confirm if this is needed

    int age = 30;

    /* Test body ------------------------------------------------------*/
    AnasaziModel* testModel = new AnasaziModel(propsFile, argc, argv, world);
    testModel->initAgents();
    testModel->testDeathAge(age);

    /* Teardown-----------------------------------------------------*/
    delete testModel;
    repast::RepastProcess::instance()->done();
}

void test5(int argc, char** argv){
    std::string configFile = argv[1]; // The name of the configuration file
    std::string propsFile  = argv[2]; // The name of the properties file

    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator* world;

    repast::RepastProcess::init(configFile);
    world = new boost::mpi::communicator;

    AnasaziModel* model = new AnasaziModel(propsFile, argc, argv, world);
    repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();

    model->initAgents();
    model->FieldTest();

    delete model;
    repast::RepastProcess::instance()->done();
}

void test6(int argc, char** argv)
{
	/* Test setup ---------------------------------------------------*/

	std::string configFile = argv[1]; // The name of the configuration file
	std::string propsFile  = argv[2]; // The name of the properties file
	// boost::mpi::environment env(argc, argv);
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

    boost::mpi::environment env(argc, argv);
	
    std::cout << "\nStarting test 1...\n";
    test1(argc, argv);
    std::cout << "----------------------------------\n\n";

    std::cout << "Starting test 2...\n";
    test2(argc, argv);
    std::cout << "----------------------------------\n\n";

    std::cout << "Starting test 3...\n";
    test3(argc, argv);
    std::cout << "----------------------------------\n\n";

    std::cout << "Starting test 4...\n";
    /*Insert test 4 function here*/
    std::cout << "----------------------------------\n\n";

    std::cout << "Starting test 5...\n";
    test5(argc,argv);
    std::cout << "----------------------------------\n\n";
    
    std::cout << "Starting test 6...\n";
    test6(argc, argv);
    std::cout << "----------------------------------\n\n";
    
    return 0;
}
