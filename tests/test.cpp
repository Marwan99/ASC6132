#include <boost/mpi.hpp>
#include "repast_hpc/RepastProcess.h"

#include "Model.h"
#include "Location.h"
#include "Household.h"
#include "Logger.h"
#include <iomanip>

void test1(int argc, char** argv, std::ofstream* output_file)
{
    /* Test setup ---------------------------------------------------*/
    std::string configFile = argv[1]; // The name of the configuration file
    std::string propsFile  = argv[2]; // The name of the properties file
    boost::mpi::communicator* world;


    repast::RepastProcess::init(configFile);
    world = new boost::mpi::communicator; // confirm if this is needed

    /* Test body ------------------------------------------------------*/
    AnasaziModel* testModel = new AnasaziModel(propsFile, argc, argv, world);
    testModel->initAgents();
    testModel->testInitAgent(output_file);

    /* Teardown-----------------------------------------------------*/
    delete testModel;
    repast::RepastProcess::instance()->done();
}


void test2(int argc, char** argv, std::ofstream* output_file)
{
    /* Test setup ---------------------------------------------------*/
    customPrint logger(output_file);
    std::string configFile = argv[1]; // The name of the configuration file
    std::string propsFile  = argv[2]; // The name of the properties file
    // boost::mpi::environment env(argc, argv, file);
    boost::mpi::communicator* world;

    repast::RepastProcess::init(configFile);
    world = new boost::mpi::communicator; // confirm if this is needed

    // logger.print("Running Test 2.");

    double expectedResult = 1.6;
    int zone = 2;
    
    logger.print("Expected hydro level for zone " + std::to_string(zone) + " in year 800 is: " + std::to_string(expectedResult));

    // AnasaziModel* testModel = createTestModel();
    AnasaziModel* testModel = new AnasaziModel(propsFile, argc, argv, world);

    testModel->readCsvHydro();
    double testResult = testModel->hydroLevel(zone);

    logger.print("Value in model environment is: " + std::to_string(testResult));

    // deleteTestModel(testModel);
    delete testModel;
    // delete world;
    repast::RepastProcess::instance()->done();

    if (testResult == expectedResult){
        logger.print("Test result matches expected result.");
        logger.print("Test 2 successful.");
        // return true;
    }
    else{
        logger.print("Test result does not match expected result.");
        logger.print("Test 2 failed.");
        // return false;
    }
}

void test3(int argc, char** argv, std::ofstream* output_file)
{
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
    testModel->testDeathAge(age, output_file);

    /* Teardown-----------------------------------------------------*/
    delete testModel;
    repast::RepastProcess::instance()->done();
}

void test4(int argc, char** argv, std::ofstream* output_file){
    std::string configFile = argv[1]; // The name of the configuration file
    std::string propsFile  = argv[2]; // The name of the properties file

    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator* world;

    repast::RepastProcess::init(configFile);
    world = new boost::mpi::communicator;

    AnasaziModel* model = new AnasaziModel(propsFile, argc, argv, world);
    repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();

    model->initAgents();
    model->testOutputFile(output_file);

    delete model;
    repast::RepastProcess::instance()->done();
}

void test5(int argc, char** argv, std::ofstream* output_file)
{
    std::string configFile = argv[1]; // The name of the configuration file
    std::string propsFile  = argv[2]; // The name of the properties file

    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator* world;

    repast::RepastProcess::init(configFile);
    world = new boost::mpi::communicator;

    AnasaziModel* model = new AnasaziModel(propsFile, argc, argv, world);
    repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();

    model->initAgents();
    model->FieldTest(output_file);

    delete model;
    repast::RepastProcess::instance()->done();
}

void test6(int argc, char** argv, std::ofstream* output_file)
{
	/* Test setup ---------------------------------------------------*/

	std::string configFile = argv[1]; // The name of the configuration file
	std::string propsFile  = argv[2]; // The name of the properties file
	// boost::mpi::environment env(argc, argv, file);
	boost::mpi::communicator* world;

	repast::RepastProcess::init(configFile);
	world = new boost::mpi::communicator; // confirm if this is needed

	/* Test body ------------------------------------------------------*/
	AnasaziModel* model = new AnasaziModel(propsFile, argc, argv, world);
	model->initAgents();
	model->testRelocateHousehold(output_file);

	/* Teardown-----------------------------------------------------*/
	delete model;
	repast::RepastProcess::instance()->done();
}

int main(int argc, char** argv){

    std::ofstream *file = (new std::ofstream);
    file->open("test_log.txt");

    customPrint logger(file);

    boost::mpi::environment env(argc, argv);
	
    logger.print("\nAnasazi Model Testing - Group 2\n");
    logger.print("\nStarting test 1...\n");
    test1(argc, argv, file);
    logger.print("----------------------------------\n");

    logger.print("Starting test 2...\n");
    test2(argc, argv, file);
    logger.print("----------------------------------\n");

    logger.print("Starting test 3...\n");
    test3(argc, argv, file);
    logger.print("----------------------------------\n");

    logger.print("Starting test 4...\n");
    test4(argc, argv, file);
    logger.print("----------------------------------\n");

    logger.print("Starting test 5...\n");
    test5(argc,argv, file);
    logger.print("----------------------------------\n");
    
    logger.print("Starting test 6...\n");
    test6(argc, argv, file);
    logger.print("----------------------------------\n");
    
    file->close();
    return 0;
}
