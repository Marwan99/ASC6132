#ifndef MODEL
#define MODEL

#include <boost/mpi.hpp>
#include "repast_hpc/Schedule.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedDiscreteSpace.h"
#include "repast_hpc/GridComponents.h"
#include "repast_hpc/Random.h"
#include <math.h>

#include "Household.h"

#define NUMBER_OF_YEARS 551

class AnasaziModel{
private:
	int year;
	int stopAt;
	int boardSizeX, boardSizeY, procX, procY, bufferSize;
	int randomSeed;
	int houseID = 0;

	std::ofstream out;
	std::string data_dir; // path to the data directory

	struct Parameters
	{
		int startYear;
		int endYear;
		int maxStorageYear;
		int maxStorage;
		int householdNeed;
		int minFissionAge;
		int maxFissionAge;
		int minDeathAge;
		int maxDeathAge;
		int maxDistance;
		int initMinCorn;
		int initMaxCorn;
		double annualVariance;
		double spatialVariance;
		double fertilityProbability;
		double harvestAdjustment;
		double maizeStorageRatio;
	} param;

	struct PDSI
	{
		int year;
		double pdsiGeneral;
		double pdsiNorth;
		double pdsiMid;
		double pdsiNatural;
		double pdsiUpland;
		double pdsiKinbiko;
	} pdsi[NUMBER_OF_YEARS];

	struct Hydro
	{
		int year;
		double hydroGeneral;
		double hydroNorth;
		double hydroMid;
		double hydroNatural;
		double hydroUpland;
		double hydroKinbiko;
	} hydro[NUMBER_OF_YEARS];

	const int yieldLevels[5][4] = { {617, 514, 411, 642},
									{719, 599, 479, 749},
									{821, 684, 547, 855},
									{988, 824, 659, 1030},
									{1153, 961, 769, 1201}};

	bool existStreams;
	bool existAlluvium;
	repast::Properties* props;
	repast::SharedContext<Household> context;
	repast::SharedContext<Location> locationContext;	//Need to confirm this line
	repast::SharedDiscreteSpace<Household, repast::StrictBorders, repast::SimpleAdder<Household> >* householdSpace;
	repast::SharedDiscreteSpace<Location, repast::StrictBorders, repast::SimpleAdder<Location> >* locationSpace;
	repast::DoubleUniformGenerator* fissionGen;// = repast::Random::instance()->createUniDoubleGenerator(0,1);
	repast::IntUniformGenerator* deathAgeGen;// = repast::Random::instance()->createNormalGenerator(25,5);
	repast::NormalGenerator* yieldGen;// = repast::Random::instance()->createNormalGenerator(0,sqrt(0.1));
	repast::NormalGenerator* soilGen;// = repast::Random::instance()->createNormalGenerator(0,sqrt(0.1));
	repast::IntUniformGenerator* initAgeGen;// = repast::Random::instance()->createUniIntGenerator(0,29);
	repast::IntUniformGenerator* initMaizeGen;// = repast::Random::instance()->createUniIntGenerator(1000,1600);


public:
	int population[NUMBER_OF_YEARS];

	AnasaziModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm);
	AnasaziModel(std::string propsFile, boost::mpi::communicator* comm, std::string data_dir);
	~AnasaziModel();
	void initAgents();
	void initSchedule(repast::ScheduleRunner& runner);
	void doPerTick();
	void readCsvMap();
	void readCsvWater();
	void readCsvPdsi();
	void readCsvHydro();
	int yieldFromPdsi(int zone, int maizeZone);
	double hydroLevel(int zone);
	void checkWaterConditions();
	void writeOutputToFile();
	void updateLocationProperties();
	void updateHouseholdProperties();
	bool fieldSearch(Household* household);
	void removeHousehold(Household* household);
	bool relocateHousehold(Household* household);

	// Test 3 methods
	void testDeathAge(int deathAge, std::ofstream* log_file);

	// Test 6 methods
	bool testRelocateHousehold(std::ofstream* log_file);
	void getClosestToWater(std::vector<Location*> & locations, std::vector<Location*> & waterSources, std::vector<int> & closest_location_coordinates);

	//Test 5 methods
	void FieldTest(std::ofstream* log_file);

	// Test 1 methods
	void testInitAgent(std::ofstream* log_file);

	// Test 4 method
	void testOutputFile(std::ofstream* log_file);
};

#endif
