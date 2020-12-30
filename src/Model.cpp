#include <stdio.h>
#include <vector>
#include <unordered_set>
#include <limits>
#include <boost/mpi.hpp>
#include "repast_hpc/AgentId.h"
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Utilities.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/initialize_random.h"
#include "repast_hpc/SVDataSetBuilder.h"
#include "repast_hpc/Point.h"
#include "repast_hpc/Random.h"
#include "repast_hpc/Schedule.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedDiscreteSpace.h"
#include "repast_hpc/GridComponents.h"
#include <string>
#include <fstream>
#include <stdlib.h>
#include "repast_hpc/Moore2DGridQuery.h"
#include "Logger.h"

#include "Model.h"

// substracts b<T> to a<T>
template <typename T>
void substract_vector(std::vector<T>& a, const std::vector<T>& b)
{
	typename std::vector<T>::iterator       it = a.begin();
	typename std::vector<T>::const_iterator it2 = b.begin();

	while (it != a.end())
	{
		while (it2 != b.end() && it != a.end())
		{
			if (*it == *it2)
			{
				it = a.erase(it);
				it2 = b.begin();
			}

			else
				++it2;
		}
		if (it != a.end())
			++it;

		it2 = b.begin();
	}
}

AnasaziModel::AnasaziModel(int* int_params, double* double_params, boost::mpi::communicator* comm, std::string data_directory): context(comm) , locationContext(comm)
{
	data_dir = data_directory;

	// repast::initializeSeedMarwan();
	boardSizeX = 80;
	boardSizeY = 120;

	repast::Point<double> origin(0,0);
	repast::Point<double> extent(boardSizeX, boardSizeY);
	repast::GridDimensions gd (origin, extent);

	int procX = 1;
	int procY = 1;
	int bufferSize = 0;

	std::vector<int> processDims;
	processDims.push_back(procX);
	processDims.push_back(procY);
	householdSpace = new repast::SharedDiscreteSpace<Household, repast::StrictBorders, repast::SimpleAdder<Household> >("AgentDiscreteSpace",gd,processDims,bufferSize, comm);
	locationSpace = new repast::SharedDiscreteSpace<Location, repast::StrictBorders, repast::SimpleAdder<Location> >("LocationDiscreteSpace",gd,processDims,bufferSize, comm);

	context.addProjection(householdSpace);
	locationContext.addProjection(locationSpace);

	param.startYear = 800;
	param.endYear = 850;
	param.maxStorageYear = int_params[0];
	param.maxStorage = int_params[1];
	param.householdNeed = int_params[2];
	param.minFissionAge = int_params[3];
	param.maxFissionAge = int_params[4];
	param.minDeathAge = int_params[5];
	param.maxDeathAge = int_params[6];
	param.maxDistance = int_params[7];
	param.initMinCorn = int_params[8];
	param.initMaxCorn = int_params[9];
	
	// new int parameters
	param.Migrationyear = int_params[10];
    param.influenceRadius = int_params[11];
    param.excessMaizeThreshold = int_params[12];
	// end of new int parameters

	param.annualVariance = double_params[0];
	param.spatialVariance = double_params[1];
	param.fertilityProbability = double_params[2];
	param.harvestAdjustment = double_params[3];
	param.maizeStorageRatio = double_params[4];

	// new double parameters
	param.biasVariance = double_params[5];
	param.newbiesFactor = double_params[6];
	param.biasMu = double_params[7];
    param.immigrationVarience = double_params[8];
    param.deltaNeighboursWeight = double_params[9];
    param.expectationsWeight = double_params[10];
    param.fissionsWeight = double_params[11];
    param.deathWeight = double_params[12];
    param.migrationHappinessVariance = double_params[13];
	// end of new double parameters

	year = param.startYear;
	stopAt = param.endYear - param.startYear + 1;
	fissionGen = new repast::DoubleUniformGenerator(repast::Random::instance()->createUniDoubleGenerator(0,1));
	deathAgeGen = new repast::IntUniformGenerator(repast::Random::instance()->createUniIntGenerator(param.minDeathAge,param.maxDeathAge));
	yieldGen = new repast::NormalGenerator(repast::Random::instance()->createNormalGenerator(0,param.annualVariance));
	soilGen = new repast::NormalGenerator(repast::Random::instance()->createNormalGenerator(0,param.spatialVariance));
	initAgeGen = new repast::IntUniformGenerator(repast::Random::instance()->createUniIntGenerator(0,param.minDeathAge));
	initMaizeGen = new repast::IntUniformGenerator(repast::Random::instance()->createUniIntGenerator(param.initMinCorn,param.initMaxCorn));

	Immigration = true;
	YieldPrediction = true;
	Happiness = true;

	if (YieldPrediction){
		biasGen = new repast::NormalGenerator(repast::Random::instance()->createNormalGenerator(1, param.biasVariance));
	}else{
		biasGen = new repast::NormalGenerator(repast::Random::instance()->createNormalGenerator(1, 0));
	}
	
	happinessGen = new repast::NormalGenerator(repast::Random::instance()->createNormalGenerator(0, param.biasVariance));

	string resultFile = "NumberOfHousehold.csv";
	out.open(resultFile);
	out << "Year,Number-of-Households" << endl;
}


AnasaziModel::AnasaziModel(bool* Selector, std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm): context(comm) , locationContext(comm)
{
	data_dir = "data/";
	props = new repast::Properties(propsFile, argc, argv, comm);
	boardSizeX = repast::strToInt(props->getProperty("board.size.x"));
	boardSizeY = repast::strToInt(props->getProperty("board.size.y"));

	initializeRandom(*props, comm);
	repast::Point<double> origin(0,0);
	repast::Point<double> extent(boardSizeX, boardSizeY);
	repast::GridDimensions gd (origin, extent);

	int procX = repast::strToInt(props->getProperty("proc.per.x"));
	int procY = repast::strToInt(props->getProperty("proc.per.y"));
	int bufferSize = repast::strToInt(props->getProperty("grid.buffer"));

	std::vector<int> processDims;
	processDims.push_back(procX);
	processDims.push_back(procY);
	householdSpace = new repast::SharedDiscreteSpace<Household, repast::StrictBorders, repast::SimpleAdder<Household> >("AgentDiscreteSpace",gd,processDims,bufferSize, comm);
	locationSpace = new repast::SharedDiscreteSpace<Location, repast::StrictBorders, repast::SimpleAdder<Location> >("LocationDiscreteSpace",gd,processDims,bufferSize, comm);

	context.addProjection(householdSpace);
	locationContext.addProjection(locationSpace);

//Existing Parameters 
	param.startYear = repast::strToInt(props->getProperty("start.year"));
	param.endYear = repast::strToInt(props->getProperty("end.year"));
	param.maxStorageYear = repast::strToInt(props->getProperty("max.store.year"));
	param.maxStorage = repast::strToInt(props->getProperty("max.storage"));
	param.householdNeed = repast::strToInt(props->getProperty("household.need"));
	param.minFissionAge = repast::strToInt(props->getProperty("min.fission.age"));
	param.maxFissionAge = repast::strToInt(props->getProperty("max.fission.age"));
	param.minDeathAge = repast::strToInt(props->getProperty("min.death.age"));
	param.maxDeathAge = repast::strToInt(props->getProperty("max.death.age"));
	param.maxDistance = repast::strToInt(props->getProperty("max.distance"));
	param.initMinCorn = repast::strToInt(props->getProperty("initial.min.corn"));
	param.initMaxCorn = repast::strToInt(props->getProperty("initial.max.corn"));

// Parameters added by team for migration and yield prediction 
	param.Migrationyear = repast::strToInt(props->getProperty("migration.year"));
	param.biasVariance = repast::strToDouble(props->getProperty("bias.Variance"));
    param.influenceRadius = repast::strToInt(props->getProperty("Influence.Radius"));
    param.excessMaizeThreshold = repast::strToInt(props->getProperty("excess.Maize.Threshold"));
	param.newbiesFactor = repast::strToDouble(props->getProperty("newbies.factor"));
	
	param.biasMu = repast::strToDouble(props->getProperty("bias.Mu"));
    param.immigrationVarience = repast::strToDouble(props->getProperty("Immigration.variance"));
    param.deltaNeighboursWeight = repast::strToDouble(props->getProperty("delta.neighbours.weight"));
    param.expectationsWeight = repast::strToDouble(props->getProperty("expectations.weight"));
    param.fissionsWeight = repast::strToDouble(props->getProperty("fission.weight"));
    param.deathWeight = repast::strToDouble(props->getProperty("death.weight"));
    param.migrationHappinessVariance = repast::strToDouble(props->getProperty("migration.happiness.variance"));
	
// Existing Parameters 
	param.annualVariance = repast::strToDouble(props->getProperty("annual.variance"));
	param.spatialVariance = repast::strToDouble(props->getProperty("spatial.variance"));
	param.fertilityProbability = repast::strToDouble(props->getProperty("fertility.prop"));
	param.harvestAdjustment = repast::strToDouble(props->getProperty("harvest.adj"));
	param.maizeStorageRatio = repast::strToDouble(props->getProperty("new.household.ini.maize"));

	year = param.startYear;
	stopAt = param.endYear - param.startYear + 1;
	fissionGen = new repast::DoubleUniformGenerator(repast::Random::instance()->createUniDoubleGenerator(0,1));
	deathAgeGen = new repast::IntUniformGenerator(repast::Random::instance()->createUniIntGenerator(param.minDeathAge,param.maxDeathAge));
	yieldGen = new repast::NormalGenerator(repast::Random::instance()->createNormalGenerator(0,param.annualVariance));
	soilGen = new repast::NormalGenerator(repast::Random::instance()->createNormalGenerator(0,param.spatialVariance));
	initAgeGen = new repast::IntUniformGenerator(repast::Random::instance()->createUniIntGenerator(0,param.minDeathAge));
	initMaizeGen = new repast::IntUniformGenerator(repast::Random::instance()->createUniIntGenerator(param.initMinCorn,param.initMaxCorn));

	Immigration = Selector[0];
	YieldPrediction = Selector[2];
	Happiness = Selector[3];

	if (YieldPrediction){
		biasGen = new repast::NormalGenerator(repast::Random::instance()->createNormalGenerator(1, param.biasVariance));
	}else{
		biasGen = new repast::NormalGenerator(repast::Random::instance()->createNormalGenerator(1, 0));
	}
	
	happinessGen = new repast::NormalGenerator(repast::Random::instance()->createNormalGenerator(0, param.biasVariance));
	

	string resultFile = props->getProperty("result.file");
	out.open(resultFile);
	out << "Year,Number-of-Households" << endl;
}

// AnasaziModel::~AnasaziModel()
// {
// 	delete props;
// 	out.close();
// }

void AnasaziModel::initAgents()
{
	int rank = repast::RepastProcess::instance()->rank();

	int LocationID = 0;
	for(int i=0; i<boardSizeX; i++ )
	{
		for(int j=0; j<boardSizeY; j++)
		{
			repast::AgentId id(LocationID, rank, 1);
			Location* agent = new Location(id, soilGen->next());
			locationContext.addAgent(agent);
			locationSpace->moveTo(id, repast::Point<int>(i, j));
			LocationID++;
		}
	}

	readCsvMap();
	readCsvWater();
	readCsvPdsi();
	readCsvHydro();
	int noOfAgents  = 14;
	
	// IntUniformGenerator outputs values that are inclusive of the limits, therfore a -1 is used to avoid segfaults. 
	repast::IntUniformGenerator xGen = repast::IntUniformGenerator(repast::Random::instance()->createUniIntGenerator(0,boardSizeX-1));
	repast::IntUniformGenerator yGen = repast::IntUniformGenerator(repast::Random::instance()->createUniIntGenerator(0,boardSizeY-1));
	for(int i =0; i< noOfAgents;i++)
	{
		repast::AgentId id(houseID, rank, 2); //Agent type 1 = field, agent type 2 = household, agent type 3 = dead house 
		int initAge = initAgeGen->next();
		int mStorage = initMaizeGen->next();
	
		Household* agent = new Household(id, initAge, deathAgeGen->next(), mStorage);
		context.addAgent(agent);
		std::vector<Location*> locationList;

		newLocation:
		int x = xGen.next();
		int y = yGen.next();
		locationSpace->getObjectsAt(repast::Point<int>(x, y), locationList);

		if(locationList[0]->getState()==2)
		{
			locationList.clear();
			goto newLocation;
		}
		else
		{
			householdSpace->moveTo(id, repast::Point<int>(x, y));
			locationList[0]->setState(1);
		}


		houseID++;
	}

	updateLocationProperties();

	repast::SharedContext<Household>::const_iterator local_agents_iter = context.begin();
	repast::SharedContext<Household>::const_iterator local_agents_end = context.end();

	while(local_agents_iter != local_agents_end)
	{
		Household* household = (&**local_agents_iter);
		if(household->death())
		{
			repast::AgentId id = household->getId();
			local_agents_iter++;

			std::vector<int> loc;
			householdSpace->getLocation(id, loc);

			std::vector<Location*> locationList;
			if(!loc.empty())
			{
				locationSpace->getObjectsAt(repast::Point<int>(loc[0], loc[1]), locationList);
				locationList[0]->setState(0);
			}
			context.removeAgent(id);
		}
		else
		{	
			double initBias = biasGen->next();
			local_agents_iter++;
			if (fieldSearch(household)){
				network(household);
				household->initVariables(Neighbours, param.biasMu, (180*happinessGen->next()), initBias, param.deltaNeighboursWeight, param.expectationsWeight, param.fissionsWeight, param.deathWeight);
				updateBias(household,Neighbours);
			}

		}
	}
}

void AnasaziModel::doPerTick()
{
	// std::cout << year << ".";
	updateLocationProperties();
	writeOutputToFile();
	year++;
	
	updateHouseholdProperties();
	
	migration();
}

void AnasaziModel::initSchedule(repast::ScheduleRunner& runner)
{
	runner.scheduleEvent(1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<AnasaziModel> (this, &AnasaziModel::doPerTick)));
	runner.scheduleStop(stopAt);
}

void AnasaziModel::readCsvMap()
{
	int x,y,z , mz;
	string zone, maizeZone, temp;

	std::ifstream file (data_dir + "map.csv");//define file object and open map.csv
	file.ignore(500,'\n');//Ignore first line

	while(1)//read until end of file
	{
		getline(file,temp,',');
		if(!temp.empty())
		{
			x = repast::strToInt(temp); //Read until ',' and convert to int & store in x
			getline(file,temp,',');
			y = repast::strToInt(temp); //Read until ',' and convert to int & store in y
			getline(file,temp,','); //colour
			getline(file,zone,',');// read until ',' and store into zone
			getline(file,maizeZone,'\n');// read until next line and store into maizeZone
			if(zone == "\"Empty\"")
			{
				z = 0;
			}
			else if(zone == "\"Natural\"")
			{
				z = 1;
			}
			else if(zone == "\"Kinbiko\"")
			{
				z = 2;
			}
			else if(zone == "\"Uplands\"")
			{
				z = 3;
			}
			else if(zone == "\"North\"")
			{
				z = 4;
			}
			else if(zone == "\"General\"")
			{
				z = 5;
			}
			else if(zone == "\"North Dunes\"")
			{
				z = 6;
			}
			else if(zone == "\"Mid Dunes\"")
			{
				z = 7;
			}
			else if(zone == "\"Mid\"")
			{
				z = 8;
			}
			else
			{
				z = 99;
			}

			if(maizeZone.find("Empty") != std::string::npos)
			{
				mz = 0;
			}
			else if(maizeZone.find("No_Yield") != std::string::npos)
			{
				mz = 1;
			}
			else if(maizeZone.find("Yield_1") != std::string::npos)
			{
				mz = 2;
			}
			else if(maizeZone.find("Yield_2") != std::string::npos)
			{
				mz = 3;
			}
			else if(maizeZone.find("Yield_3") != std::string::npos)
			{
				mz = 4;
			}
			else if(maizeZone.find("Sand_dune") != std::string::npos)
			{
				mz = 5;
			}
			else
			{
				mz = 99;
			}
			std::vector<Location*> locationList;
			locationSpace->getObjectsAt(repast::Point<int>(x, y), locationList);
			locationList[0]->setZones(z,mz);
		}
		else{
			goto endloop;
		}
	}
	endloop: ;
}

void AnasaziModel::readCsvWater()
{
	//read "type","start date","end date","x","y"
	int type, startYear, endYear, x, y;
	string temp;

	std::ifstream file (data_dir + "water.csv");//define file object and open water.csv
	file.ignore(500,'\n');//Ignore first line
	while(1)//read until end of file
	{
		getline(file,temp,',');
		if(!temp.empty())
		{
			getline(file,temp,',');
			getline(file,temp,',');
			getline(file,temp,',');
			type = repast::strToInt(temp); //Read until ',' and convert to int
			getline(file,temp,',');
			startYear = repast::strToInt(temp); //Read until ',' and convert to int
			getline(file,temp,',');
			endYear = repast::strToInt(temp); //Read until ',' and convert to int
			getline(file,temp,',');
			x = repast::strToInt(temp); //Read until ',' and convert to int
			getline(file,temp,'\n');
			y = repast::strToInt(temp); //Read until ',' and convert to int

			std::vector<Location*> locationList;
			locationSpace->getObjectsAt(repast::Point<int>(x, y), locationList);
			locationList[0]->addWaterSource(type,startYear, endYear);
			//locationList[0]->checkWater(existStreams, existAlluvium, x, y, year);
		}
		else
		{
			goto endloop;
		}
	}
	endloop: ;
}

void AnasaziModel::readCsvPdsi()
{
	//read "year","general","north","mid","natural","upland","kinbiko"
	int i=0;
	string temp;

	std::ifstream file (data_dir + "pdsi.csv");//define file object and open pdsi.csv
	file.ignore(500,'\n');//Ignore first line

	while(1)//read until end of file
	{
		getline(file,temp,',');
		if(!temp.empty())
		{
			pdsi[i].year = repast::strToInt(temp); //Read until ',' and convert to int
			getline(file,temp,',');
			pdsi[i].pdsiGeneral = repast::strToDouble(temp); //Read until ',' and convert to double
			getline(file,temp,',');
			pdsi[i].pdsiNorth = repast::strToDouble(temp); //Read until ',' and convert to double
			getline(file,temp,',');
			pdsi[i].pdsiMid = repast::strToDouble(temp); //Read until ',' and convert to double
			getline(file,temp,',');
			pdsi[i].pdsiNatural = repast::strToDouble(temp); //Read until ',' and convert to int
			getline(file,temp,',');
			pdsi[i].pdsiUpland = repast::strToDouble(temp); //Read until ',' and convert to int
			getline(file,temp,'\n');
			pdsi[i].pdsiKinbiko = repast::strToDouble(temp); //Read until ',' and convert to double
			i++;
		}
		else{
			goto endloop;
		}
	}
	endloop: ;
}

void AnasaziModel::readCsvHydro()
{
	//read "year","general","north","mid","natural","upland","kinbiko"
	string temp;
	int i =0;

	std::ifstream file (data_dir + "hydro.csv");//define file object and open hydro.csv
	file.ignore(500,'\n');//Ignore first line

	while(1)//read until end of file
	{
		getline(file,temp,',');
		if(!temp.empty())
		{
			hydro[i].year = repast::strToInt(temp); //Read until ',' and convert to int
			getline(file,temp,',');
			hydro[i].hydroGeneral = repast::strToDouble(temp); //Read until ',' and convert to double
			getline(file,temp,',');
			hydro[i].hydroNorth = repast::strToDouble(temp); //Read until ',' and convert to double
			getline(file,temp,',');
			hydro[i].hydroMid = repast::strToDouble(temp); //Read until ',' and convert to double
			getline(file,temp,',');
			hydro[i].hydroNatural = repast::strToDouble(temp); //Read until ',' and convert to int
			getline(file,temp,',');
			hydro[i].hydroUpland = repast::strToDouble(temp); //Read until ',' and convert to int
			getline(file,temp,'\n');
			hydro[i].hydroKinbiko = repast::strToDouble(temp); //Read until ',' and convert to double
			i++;
		}
		else
		{
			goto endloop;
		}
	}
	endloop: ;
}

int AnasaziModel::yieldFromPdsi(int zone, int maizeZone)
{
	int pdsiValue, row, col;
	switch(zone)
	{
		case 1:
			pdsiValue = pdsi[year-param.startYear].pdsiNatural;
			break;
		case 2:
			pdsiValue = pdsi[year-param.startYear].pdsiKinbiko;
			break;
		case 3:
			pdsiValue = pdsi[year-param.startYear].pdsiUpland;
			break;
		case 4:
		case 6:
			pdsiValue = pdsi[year-param.startYear].pdsiNorth;
			break;
		case 5:
			pdsiValue = pdsi[year-param.startYear].pdsiGeneral;
			break;
		case 7:
		case 8:
			pdsiValue = pdsi[year-param.startYear].pdsiMid;
			break;
		default:
			return 0;
	}

	/* Rows of pdsi table*/
	if(pdsiValue < -3)
	{
		row = 0;
	}
	else if(pdsiValue >= -3 && pdsiValue < -1)
	{
		row = 1;
	}
	else if(pdsiValue >= -1 && pdsiValue < 1)
	{
		row = 2;
	}
	else if(pdsiValue >= 1 && pdsiValue < 3)
	{
		row = 3;
	}
	else if(pdsiValue >= 3)
	{
		row = 4;
	}
	else
	{
		return 0;
	}

	/* Col of pdsi table*/
	if(maizeZone >= 2)
	{
		col = maizeZone - 2;
	}
	else
	{
		return 0;
	}

	return yieldLevels[row][col];
}

double AnasaziModel::hydroLevel(int zone)
{
	switch(zone)
	{
		case 1:
			return hydro[year-param.startYear].hydroNatural;
		case 2:
			return hydro[year-param.startYear].hydroKinbiko;
		case 3:
			return hydro[year-param.startYear].hydroUpland;
		case 4:
		case 6:
			return hydro[year-param.startYear].hydroNorth;
		case 5:
			return hydro[year-param.startYear].hydroGeneral;
		case 7:
		case 8:
			return hydro[year-param.startYear].hydroMid;
		default:
			return 0;
	}
}

void AnasaziModel::checkWaterConditions()
{
	if ((year >= 280 && year < 360) or (year >= 800 && year < 930) or (year >= 1300 && year < 1450))
	{
		existStreams = true;
	}
	else
	{
		existStreams = false;
	}

	if (((year >= 420) && (year < 560)) or ((year >= 630) && (year < 680)) or	((year >= 980) && (year < 1120)) or ((year >= 1180) && (year < 1230)))
	{
		existAlluvium = true;
	}
	else
	{
		existAlluvium = false;
	}
}

void AnasaziModel::writeOutputToFile()
{
	population[year-param.startYear] = context.size();
	out << year << "," <<  context.size() << std::endl;
}

void  AnasaziModel::updateLocationProperties()
{
	checkWaterConditions();
	int x = 0;
	for(int i=0; i<boardSizeX; i++ )
	{
		for(int j=0; j<boardSizeY; j++)
		{
			std::vector<Location*> locationList;
			locationSpace->getObjectsAt(repast::Point<int>(i, j), locationList);
			locationList[0]->checkWater(existStreams,existAlluvium, i, j, year);
			int mz = locationList[0]->getMaizeZone();
			int z = locationList[0]->getZone();
			int y = yieldFromPdsi(z,mz);
			locationList[0]->calculateYield(y, param.harvestAdjustment, yieldGen->next());
			
		}
	}
}

void AnasaziModel::updateHouseholdProperties()
{
	repast::SharedContext<Household>::const_iterator local_agents_iter = context.begin();
	repast::SharedContext<Household>::const_iterator local_agents_end = context.end();
	std::unordered_set<int> DeadAgents;
	

	while(local_agents_iter != local_agents_end)
	{	
		Household* household = (&**local_agents_iter);
		if(household->death())
		{
			local_agents_iter++;
			repast::AgentId temp = household->getId();
			DeadAgents.insert(temp.id());
			//std::cout << "Agent" << household->getId().id() << "has died" << std::endl; 
			removeHousehold(household);
		}
		else
		{
			local_agents_iter++;
			if(household->fission(param.minFissionAge,param.maxFissionAge, fissionGen->next(), param.fertilityProbability))
			{
				int rank = repast::RepastProcess::instance()->rank();
				repast::AgentId id_f(houseID, rank, 2);

				int mStorage = household->splitMaizeStored(param.maizeStorageRatio);
				Household* newAgent = new Household(id_f, 0, deathAgeGen->next(), mStorage);
				context.addAgent(newAgent);	
				std::vector<int> loc;
				householdSpace->getLocation(household->getId(), loc);
				householdSpace->moveTo(id_f, repast::Point<int>(loc[0], loc[1]));
				if(fieldSearch(newAgent)){
					network(newAgent);
					newAgent->initVariables(Neighbours, param.biasMu, 180*happinessGen->next(), biasGen->next(), param.deltaNeighboursWeight, param.expectationsWeight, param.fissionsWeight, param.deathWeight);
					updateBias(newAgent,Neighbours);
				}
				houseID++;
			}


			bool fieldFound = true;
			if(!(household->checkMaize(param.householdNeed)))
			{
				fieldFound = fieldSearch(household);
			}
			if(fieldFound)
			{
				household->nextYear(param.householdNeed);
			}
		}
	}
	//std::cout << "----------------" << std::endl;
	local_agents_iter = context.begin();
	local_agents_end = context.end();
	while(local_agents_iter != local_agents_end){ // add as method in household class 
		Household* household = (&**local_agents_iter);
		// std::vector<int> loc;

		local_agents_iter++;
		network(household);
		updateBias(household,Neighbours);
		household->calculateHappiness(Neighbours, DeadAgents);
		// repast::AgentId temp = household->getId();
		// householdSpace->getLocation(temp, loc);
		// std::cout << "Agent " << temp << " location X-" << loc[0] <<" Y-"<< loc[1] << std::endl;
	}
}

bool AnasaziModel::fieldSearch(Household* household)
{
	/******** Choose Field ********/
	std::vector<int> loc;
	householdSpace->getLocation(household->getId(), loc);
	repast::Point<int> center(loc);

	std::vector<Location*> neighbouringLocations;
	std::vector<Location*> checkedLocations;
	repast::Moore2DGridQuery<Location> moore2DQuery(locationSpace);
	int range = 1;
	while(1)
	{
		moore2DQuery.query(loc, range, false, neighbouringLocations);

		for (std::vector<Location*>::iterator it = neighbouringLocations.begin() ; it != neighbouringLocations.end(); ++it)
		{
			Location* tempLoc = (&**it);
			if(tempLoc->getState() == 0)
			{
				if(tempLoc->getExpectedYield() >= 800)
				{
					std::vector<int> loc;
					locationSpace->getLocation(tempLoc->getId(), loc);
					tempLoc->setState(2);
					household->chooseField(tempLoc);
					goto EndOfLoop;
				}
			}
		}
		range++;
		if(range > boardSizeY)
		{
			removeHousehold(household);
			return false;
		}
	}
	EndOfLoop:
	if(range >= 10)
	{
		return relocateHousehold(household);
	}
	else
	{
		return true;
	}
}

void AnasaziModel::removeHousehold(Household* household)
{
	repast::AgentId id = household->getId();
	//std::cout << "Agent " << id << " removed" <<std::endl;
	std::vector<int> loc;

	householdSpace->getLocation(id, loc);

	std::vector<Location*> locationList;
	std::vector<Household*> householdList;
	if(!loc.empty())
	{
		locationSpace->getObjectsAt(repast::Point<int>(loc[0], loc[1]), locationList);
		householdSpace->getObjectsAt(repast::Point<int>(loc[0], loc[1]), householdList);
		if(householdList.size() == 1)
		{
			//std::cout << "house location set to zero" << loc[0] << "," << loc[1] << std::endl;
			locationList[0]->setState(0);
		}
		if(household->getAssignedField()!= NULL)
		{
			//std::cout << "field set to zero" << loc[0] << "," << loc[1] << std::endl;
			std::vector<int> loc2;
			locationSpace->getLocation(household->getAssignedField()->getId(), loc2);
			locationSpace->getObjectsAt(repast::Point<int>(loc2[0], loc2[1]), locationList);
			locationList[0]->setState(0);
		}
	}
	context.removeAgent(id);
}

bool AnasaziModel::relocateHousehold(Household* household)
{
	std::vector<Location*> neighbouringLocations;
	std::vector<Location*> suitableLocations;
	std::vector<Location*> waterSources;
	std::vector<Location*> checkedLocations;

	std::vector<int> loc, loc2;
	locationSpace->getLocation(household->getAssignedField()->getId(), loc);
	householdSpace->getLocation(household->getId(),loc2);

	locationSpace->getObjectsAt(repast::Point<int>(loc2[0], loc2[1]), neighbouringLocations);
	Location* householdLocation = neighbouringLocations[0];

	repast::Point<int> center(loc);
	repast::Moore2DGridQuery<Location> moore2DQuery(locationSpace);
	int range = floor(param.maxDistance/100);
	int i = 1;
	bool conditionC = true;

	//get all !Field with 1km
	LocationSearch:
		moore2DQuery.query(loc, range*i, false, neighbouringLocations);
		for (std::vector<Location*>::iterator it = neighbouringLocations.begin() ; it != neighbouringLocations.end(); ++it)
		{
			Location* tempLoc = (&**it);
			if(tempLoc->getState() != 2)
			{
				if((householdLocation->getExpectedYield() * household->getBias()) < (tempLoc->getExpectedYield() * household->getBias()) && conditionC == true)
				{
					suitableLocations.push_back(tempLoc);
				}
				if(tempLoc->getWater())
				{
					waterSources.push_back(tempLoc);
				}
			}
		}
		if(suitableLocations.size() == 0 || waterSources.size() == 0)
		{
			if(conditionC == true)
			{
				conditionC = false;
			}
			else
			{
				conditionC = true;
				i++;
				if(range*i > boardSizeY)
				{
					removeHousehold(household);
					return false;
				}
			}
			goto LocationSearch;
		}
		else if(suitableLocations.size() == 1)
		{
			std::vector<int> loc2;
			locationSpace->getLocation(suitableLocations[0]->getId(),loc2);
			householdSpace->moveTo(household->getId(),repast::Point<int>(loc2[0], loc2[1]));
			return true;
		}
		else
		{
			std::vector<int> point1, point2;
			std::vector<double> distances;
			for (std::vector<Location*>::iterator it1 = suitableLocations.begin() ; it1 != suitableLocations.end(); ++it1)
			{
				locationSpace->getLocation((&**it1)->getId(),point1);
				for (std::vector<Location*>::iterator it2 = waterSources.begin() ; it2 != waterSources.end(); ++it2)
				{
					locationSpace->getLocation((&**it2)->getId(),point2);
					double distance = sqrt(pow((point1[0]-point2[0]),2) + pow((point1[1]-point2[1]),2));
					distances.push_back(distance);
				}
			}
			int minElementIndex = std::min_element(distances.begin(),distances.end()) - distances.begin();
			minElementIndex = minElementIndex / waterSources.size();
			std::vector<int> loc2;
			locationSpace->getLocation(suitableLocations[minElementIndex]->getId(),loc2);
			householdSpace->moveTo(household->getId(),repast::Point<int>(loc2[0], loc2[1]));
			return true;
		}
}

bool AnasaziModel::testRelocateHousehold(std::ofstream* log_file)
{
	customPrint logger(log_file);

	std::vector<int> initial_location, excpected_new_location, actual_new_location, field_location;

	std::vector<Location*> neighbouringLocations;
	std::vector<Location*> suitableLocations;
	std::vector<Location*> subOptimalLocations;
	std::vector<Location*> waterSources;
	std::unordered_set<Location*> checkedLocations;

	// Get household agent
	repast::SharedContext<Household>::const_iterator local_agents_iter = context.begin();
	Household* household = (&**local_agents_iter);

	// Moving household to (0, 0).
	householdSpace->moveTo(household->getId(), repast::Point<int>(0, 0));

	// Getting current household location on the grid
	householdSpace->getLocation(household->getId(), initial_location);
	logger.print("Initial household location: (" +
		std::to_string(initial_location[0]) + ", " +
		std::to_string(initial_location[1]) + ").");

	// Getting field location
	locationSpace->getLocation(household->getAssignedField()->getId(), field_location);
	logger.print("Field location: (" + std::to_string(field_location[0]) +
		", " + std::to_string(field_location[1]) + ").");

	Location* householdLocation = locationSpace->getObjectAt(repast::Point<int>(initial_location[0], initial_location[1]));

	bool new_location_found = false;
	int range = floor(param.maxDistance/100);
	repast::Moore2DGridQuery<Location> moore2DQuery(locationSpace);

	for (int i=1; range*i < boardSizeY && !new_location_found; i++)
	{
		moore2DQuery.query(field_location, range*i, false, neighbouringLocations);

		for (auto neighbouringLocation: neighbouringLocations)
		{
			if(checkedLocations.find(neighbouringLocation) == checkedLocations.end()) // checking if neighbour has been visited before
			{
				checkedLocations.insert(neighbouringLocation);
				if(neighbouringLocation->getState() != 2)  // Skipping fields
				{
					if(householdLocation->getExpectedYield() < neighbouringLocation->getExpectedYield())
						suitableLocations.push_back(neighbouringLocation);
					else
						subOptimalLocations.push_back(neighbouringLocation);
					
					if(neighbouringLocation->getWater())
						waterSources.push_back(neighbouringLocation);
				}
			}
		}

		if(suitableLocations.size()) // If a location with less yield is found
		{
			new_location_found = true;

			if(suitableLocations.size() > 1 && waterSources.size())
				getClosestToWater(suitableLocations, waterSources, excpected_new_location);
			else
				locationSpace->getLocation(suitableLocations[0]->getId(), excpected_new_location);
		}
		else if(subOptimalLocations.size()) // If a location with more yield is found
		{
			new_location_found = true;

			if(subOptimalLocations.size() > 1 && waterSources.size())
				getClosestToWater(subOptimalLocations, waterSources, excpected_new_location);
			else
				locationSpace->getLocation(subOptimalLocations[0]->getId(), excpected_new_location);
		}
	}

	logger.print("Excpected new household location: (" +
		std::to_string(excpected_new_location[0]) + ", " +
		std::to_string(excpected_new_location[1]) + ").");

	// triggerign household to move
	relocateHousehold(household);

	// Getting final agent location
	householdSpace->getLocation(household->getId(), actual_new_location);
	logger.print("Calculated new household location: (" + std::to_string(actual_new_location[0])
		+ ", " + std::to_string(actual_new_location[1]) + ").");

	if(actual_new_location[0] == excpected_new_location[0] && actual_new_location[1] == excpected_new_location[1])
		logger.print("Calculated location matches excpected location.\nTest passed.");
	else
		logger.print("Calculated location does not match excpected location.\nTest failed.");
}

void AnasaziModel::getClosestToWater(std::vector<Location*> & locations, std::vector<Location*> & waterSources, std::vector<int> & closest_location_coordinates)
{ 
	std::vector<int> point1, point2;
	Location* closest_location;
	double shortest_dist=std::numeric_limits<double>::max();

	for (auto location : locations)
	{
		locationSpace->getLocation(location->getId(),point1);
		for (auto waterSource : waterSources)
		{			
			locationSpace->getLocation(waterSource->getId(), point2);
			double distance = sqrt(pow((point1[0]-point2[0]),2) + pow((point1[1]-point2[1]),2));
			
			if(distance < shortest_dist)
			{
				shortest_dist = distance;
				closest_location = location;
			}
		}
	}

	locationSpace->getLocation(closest_location->getId(), closest_location_coordinates);
}

void AnasaziModel::FieldTest(std::ofstream* log_file)
{
	customPrint logger(log_file);

	doPerTick(); // ????
	int i;
	std::vector<int> loc;
	std::vector<int> loc2;
	std::vector<int> loc3;
	std::vector<int> loc4;
	repast::SharedContext<Household>::const_iterator local_agents_iter = context.begin();
	
	Household* household = (&**local_agents_iter);
	repast::AgentId id = household->getId();
	locationSpace->getLocation(household->getAssignedField()->getId(), loc);
	householdSpace->getLocation(id,loc2);

	logger.print("Retrieving AgentId... ");
	logger.print("AgentId: " + std::to_string(id.id()));
	logger.print("Retrieving field Position of agent Before..." );
	logger.print("Field Position(X,Y): " + std::to_string(loc[0]) + "," + std::to_string(loc[1]));
	logger.print("Retrieving House Position of agent Before..." );
	logger.print("House Position(X,Y)" + std::to_string(loc2[0])+ ","+std::to_string(loc2[1]));

	updateLocationProperties(); 

	std::vector<Location*> locationList;
	locationSpace->getObjectsAt(repast::Point<int>(loc[0], loc[1]), locationList);
	int mz = locationList[0]->getMaizeZone();
	int z = locationList[0]->getZone();
	int y = yieldFromPdsi(z,mz);
	locationList[0]->calculateYield(y, 0, yieldGen->next());// change yield to 0 for assigned field
 	
 	logger.print("Changing expected yield of field (X,Y)"+std::to_string(loc[0]) +","+ std::to_string(loc[1])+" to zero");

	writeOutputToFile();
	year++;
	updateHouseholdProperties();

	id = household->getId();
	locationSpace->getLocation(household->getAssignedField()->getId(), loc3);
	householdSpace->getLocation(id,loc4);

	logger.print("Retrieving AgentId... ");
	logger.print("AgentId: " +std::to_string(id.id()));
	logger.print("Retrieving field Position of agent after making decision...");
	logger.print("Field Position(X,Y): " + std::to_string(loc3[0]) + "," + std::to_string(loc3[1]));
	logger.print("Retrieving House Position of agent after making decision...");
	logger.print("House Position(X,Y)" + std::to_string(loc4[0])+ ","+std::to_string(loc4[1]));

	if ((loc[0] != loc3[0]) || (loc[1] != loc3[1])){
		if ((loc2[0]==loc4[0])&&(loc2[1]==loc4[1])){
			logger.print("Agent"+std::to_string(id.id())+
				" has correctly moved their field due to providing insufficient maize and remained at the same household");
		}
	}
}


void AnasaziModel::testDeathAge(int deathAge, std::ofstream* log_file)
{
	customPrint logger(log_file);

	int mStorage = 1200;
	int yearsFromDeath = 2;

	int rank = repast::RepastProcess::instance()->rank();
	repast::AgentId id1(1001, rank, 1);
	repast::AgentId id2(1002, rank, 1);
	repast::AgentId id3(1003, rank, 1);

	Household* deadAgent = new Household(id1, deathAge+yearsFromDeath, deathAge, mStorage);
	context.addAgent(deadAgent);
	householdSpace->moveTo(id1, repast::Point<int>(0, 0));
	fieldSearch(deadAgent);
	logger.print("Created Agent 1 with age " + std::to_string(deathAge+yearsFromDeath)
		+ " and death age " + std::to_string(deathAge) + ".");;

	Household* oldAgent = new Household(id2, deathAge, deathAge, mStorage);
	context.addAgent(oldAgent);
	householdSpace->moveTo(id2, repast::Point<int>(0, 0));
	fieldSearch(oldAgent);
	logger.print("Created Agent 3 with age " + std::to_string(deathAge)
		+ " and death age " + std::to_string(deathAge) + ".");

	Household* youngAgent = new Household(id3, deathAge-yearsFromDeath, deathAge, mStorage);
	context.addAgent(youngAgent);
	householdSpace->moveTo(id3, repast::Point<int>(0, 0));
	fieldSearch(youngAgent);
	logger.print("Created Agent 3 with age " + std::to_string(deathAge-yearsFromDeath) +
		" and death age " + std::to_string(deathAge) + ".");

	logger.print("\nAgent 1 should be dead immediately.");
	logger.print("Agent 2 should be dead immediately.");
	logger.print("Agent 3 should die after " + std::to_string(yearsFromDeath) + " years.");

	int year = 0;
	logger.print("\nYear: " + std::to_string(year));

	if(deadAgent->death()) {

		logger.print("Agent 1 is dead.");
		if(oldAgent->death()) {

			logger.print("Agent 2 is dead.");
			while( !(youngAgent->death()) ) { // while young agent is not dead

				logger.print("Agent 3 is not dead.");
				youngAgent->nextYear(1); // random number given as 'needs' parameter
				year++;
				yearsFromDeath--;
				logger.print("\nYear: " + std::to_string(year));

				if(yearsFromDeath < 0){ break; }
			}
			if(yearsFromDeath == 0) { // young agent died at reaching death age
				logger.print("Agent 3 is dead.");
				logger.print("\nTest 3 successful.");
				// deleteTestModel(testModel);
				// return true;
			}
			else { // young agent died before or after death age
				logger.print("Agent 3 did not die at the right time.");
				// return false;
			}
		}
		else { // old agent didnt die
			logger.print("Agent 2 did not die.");
			// return false;
		}
	}
	else { // dead agent didnt die
		logger.print("Agent 2 did not die.");
		// return false;
	}
}

void AnasaziModel::testInitAgent(std::ofstream* log_file)
{
	customPrint logger(log_file);

	int mStorage = 1200;
	int bornAge = 0;
	int deathAge = 30;
	int agentNumber = 1001;
	std::vector<int> field_location;

	int rank = repast::RepastProcess::instance()->rank();
	repast::AgentId id1(agentNumber, rank, 1);

	Household* newAgent = new Household(id1, bornAge, deathAge, mStorage);
	context.addAgent(newAgent);
	householdSpace->moveTo(id1, repast::Point<int>(0, 0));
	fieldSearch(newAgent);
	locationSpace->getLocation(newAgent->getAssignedField()->getId(), field_location);
	int checkId = newAgent->getId().id();

	logger.print("Field location: (" + std::to_string(field_location[0]) + ", " + std::to_string(field_location[1]) + ").");

	logger.print("Created an agent with the Id: \n" + std::to_string(agentNumber));

	logger.print("Checking the agent Id: \n" + std::to_string(checkId));

	if (agentNumber == checkId)
	{
		logger.print("Checked Id matched the actual Id. Test passed.");
	}
	else
	{
		logger.print("Checked Id didn't match the actual Id. Test failed.");
	}
}

void AnasaziModel::testOutputFile(std::ofstream* log_file)
{
	customPrint logger(log_file);
	std::ifstream in;
	int i;
	int householdNumbers[param.endYear-param.startYear];
	int time[param.endYear-param.startYear];
	string temp;
	int flag = 0;

	string resultFile = props->getProperty("result.file");

	

	for(i=0;i<stopAt;i++)
	{
		
		householdNumbers[i] = context.size();
		time[i] = year;

		doPerTick();
	}

	in.open(resultFile);

	in.ignore(500,'\n');

	for(i=0;i<stopAt;i++)
	{
		getline(in,temp,',');

		if(repast::strToInt(temp)==time[i])
		{

			getline(in,temp);

			if(repast::strToInt(temp)==householdNumbers[i])
			{
				flag++;
			}
			else
			{
				logger.print("Mismatch found at year: "+std::to_string(time[i]));
				break;
			}
		}
		else
		{
			logger.print("Mismatch found at year: "+std::to_string(time[i]));
			break;
		}
	}

	endloop: ;

	if(flag==(stopAt))
	{
		logger.print("Output file matches the simulation values. Test passed");
		std::ofstream Test4Outputfile;
		Test4Outputfile.open("Test4Outputfile.csv");
		Test4Outputfile << "Year,Number-of-Households" << std::endl;

		for(i=0;i<stopAt;i++)
		{
			Test4Outputfile << time[i] << "," << householdNumbers[i] << std::endl;
		}

		Test4Outputfile.close();
	}
	else
	{
		logger.print("Output file didn't match the simulation values. Test failed");
	}
}

void AnasaziModel::migration(){
	
	double Newbies = calculateNewbiesFromMaize();
	if (param.Migrationyear == yearsSince){
		// std::cout << "break 1" << std::endl;
		// std::cout << context.size() << std::endl;
		if((Happiness) && (context.size()>0)){
		
			repast::SharedContext<Household>::const_iterator local_agents_iter = context.begin();
			repast::SharedContext<Household>::const_iterator local_agents_end = context.end();
			double min = 200;
			double max = -200;
			

			std::vector<std::pair<double, int>> Average;
			int i = 0;
			while(local_agents_iter != local_agents_end){
				Household* household = (&**local_agents_iter);
				double temp = household->AverageHappiness();
				if (temp < min){
					min = temp;
				}
				if (temp > max){
					max = temp;
				}
				
				repast::AgentId id = household->getId();
		
				Average.push_back(std::make_pair(temp, id.id()));
				//std::cout << "Agent: "<<Average[i].second << " Average Happiness: " << Average[i].first << std::endl;
				i++;
				local_agents_iter++;
			}
			std::vector<double> Normalised(context.size());
			double mean=0;
			for (int i = 0; i < context.size(); i++){
				Normalised[i] = (2 * ((Average[i].first - min)/(max - min))) -1;
				mean = Normalised[i] + mean;
				//std::cout << "Agent: "<<Average[i].second << " Normalised: " << Normalised[i] << std::endl;
			}
			//std::cout << "break 1" << std::endl;
			mean = mean / Normalised.size();
			//std::cout << "break 2" << std::endl;
			//std::cout << "Mean Shift value =" << mean << std::endl;
			repast::NormalGenerator popGen = repast::NormalGenerator(repast::Random::instance()->createNormalGenerator(mean, param.migrationHappinessVariance));
			double percentChange = popGen.next();

			//std::cout << percentChange << std::endl;

			int migrationVal = percentChange * Normalised.size();
			//std::cout << "break 3" << std::endl;
			//std::cout<< "migrationVal = " << migrationVal << std::endl;
			//calculateNewbiesFromMaize();
			if (migrationVal > 0){
				// std::cout << "adding agent" << std::endl;
				AddAgent(migrationVal);
				// std::cout << "added agent" << std::endl;

			}else if (migrationVal < 0){
				std::sort(Average.begin(),Average.end());
				migrationVal =migrationVal * -1;
				if (migrationVal > context.size()){
					migrationVal = context.size();
				}
				
					for (int i = 0; i < migrationVal; i++)
					{
						int rank = repast::RepastProcess::instance()->rank();
						repast::AgentId id2(Average[i].second,rank,2);
						//std::cout << "Agent " << Average[i].second << " with happiness " << Average[i].first << " Has been removed" << std::endl;
						Household* household;
						household = context.getAgent(id2);
						removeHousehold(household);
					}
				//std::cout << "break 4" << std::endl;
			}
		}
		// std::cout << "Adding " << Newbies << "newbies"<< std::endl;
		AddAgent(Newbies);
		yearsSince = 0;
	}else{
		yearsSince++;
	}
}

double AnasaziModel::calculateNewbiesFromMaize()
{
	if(Immigration){
		double NewbiesMean;
		double landPotential = 0;
		int supportedFields = 0;
		for(int i=1; i<boardSizeX; i++ )
		{
			for(int j=1; j<boardSizeY; j++)
			{
				std::vector<Location*> locationList;
				locationSpace->getObjectsAt(repast::Point<int>(i, j), locationList);
				
				if(locationList[0]->getState() == 0){
					if(locationList[0]->getExpectedYield()> param.householdNeed){
						landPotential = locationList[0]->getExpectedYield() + landPotential;
						supportedFields++;
						//std::cout<<"landPotential" << landPotential <<std::endl;

					}
				}
			}
		}
		double excessMaze = landPotential - context.size()*param.householdNeed;

		double supportedNewbies = excessMaze/ param.householdNeed;
		//supportedFields = supportedFields - context.size();
		//std::cout << "Supported Newbies"<< supportedNewbies << std::endl;
		// std::cout << "Supported Fields"<< supportedFields <<std::endl;
		// std::cout << "Number of Agents"<<context.size() << std::endl;
		if (supportedFields > param.excessMaizeThreshold){
			NewbiesMean = param.newbiesFactor * supportedFields;
			NewbGen = new repast::NormalGenerator(repast::Random::instance()->createNormalGenerator(NewbiesMean, param.immigrationVarience));
			return NewbGen->next();
		}else{
			return 0;
		}
	}else{
		return 0;
	}
}

void AnasaziModel::network(Household* Centre)
{
	Neighbours.erase(Neighbours.begin(),Neighbours.end());
	std::vector<Household*> currentNeighbours;
	int influenceRadius = param.influenceRadius;
	std::vector<int> agentLoc;
	repast::AgentId id_C = Centre->getId();
	householdSpace->getLocation(id_C, agentLoc);
	repast::Point<int> center(agentLoc);
	repast::Moore2DGridQuery<Household> moore2DQuery(householdSpace);
	moore2DQuery.query(center, influenceRadius, false, currentNeighbours);
	std::vector<Household*>::iterator CurrentNeighbour = currentNeighbours.begin();
	if (currentNeighbours.empty() == false){
		while(CurrentNeighbour != currentNeighbours.end()){
			Neighbours.insert((*CurrentNeighbour)->getId().id());
			CurrentNeighbour++;
		}
		// std::cout << "Agent " << id.id() << " neighbours:" << std::endl;
		// for (const int& x: Neighbours) std::cout << " " << x;
		// std::cout <<  std::endl;
	} else
	{
		// std::cout << "Agent " << id.id() << " neighbours:" << std::endl;
		// std::cout << "No neighbours:" << std::endl;
	}
}

void AnasaziModel::updateBias(Household* household, std::unordered_set<int> currentNeighbours){ // issue that when biaas updates would effect 
	if(YieldPrediction){
		repast::AgentId id_b = household->getId();
		std::vector<int> agentLoc1;
		householdSpace->getLocation(id_b,agentLoc1);
		double Weightbias = 0;
		double weightden =0; 
		double Weight = 0;
		Household* curNeighbour;
		int rank = repast::RepastProcess::instance()->rank();

		double X1 = agentLoc1[0];
		double Y1 = agentLoc1[1];
		for (auto N: currentNeighbours){
			std::vector<int> agentLoc2;
			
			repast::AgentId id2(N,rank,2);
			householdSpace->getLocation(id2, agentLoc2);
			double X2 = agentLoc2[0];
			double Y2 = agentLoc2[1];

			weightden = (sqrt(pow((X1-X2),2)+pow((Y1-Y2),2))) + weightden;
		
		}
		// std::cout << "Household " << id << " at X " << agentLoc1[0] << " Y " << agentLoc1[1] << std::endl;
		for (auto N: currentNeighbours){
			std::vector<int> agentLoc2;

			repast::AgentId id3(N,rank,2);
			householdSpace->getLocation(id3, agentLoc2);
			double X2 = agentLoc2[0];
			double Y2 = agentLoc2[1];
			//std::cout << "Neighbour " << id2 << " at X " << agentLoc2[0] << " Y " << agentLoc2[1] << std::endl;
			Weight = 1 - ((sqrt(pow((X1-X2),2)+pow((Y1-Y2),2)))/weightden);
			// std::cout <<"Weight: "<< Weight << std::endl;
			curNeighbour = context.getAgent(id3);
			// std::cout << "Neighbour id: " << curNeighbour->getId() << " Bias :" << curNeighbour->getBias() << std::endl;
			Weightbias = (Weight * (curNeighbour->getBias() - household->getBias())) + Weightbias;
		}
		household->setBias(Weightbias);
	}else{
		household->setBias(0);
	}
}

void AnasaziModel::AddAgent(double NotoAdd){
	for (int k = 0; k < NotoAdd; k++)
	{
		repast::IntUniformGenerator xGen = repast::IntUniformGenerator(repast::Random::instance()->createUniIntGenerator(1,boardSizeX -1));
		repast::IntUniformGenerator yGen = repast::IntUniformGenerator(repast::Random::instance()->createUniIntGenerator(1,boardSizeY -1));
		int rank = repast::RepastProcess::instance()->rank();
		repast::AgentId id_A(houseID, rank, 2);
		Household* newAgent = new Household(id_A, initAgeGen->next(), deathAgeGen->next(), initMaizeGen->next());
		context.addAgent(newAgent);

		std::vector<Location*> locationList;
		//std::cout << "taking a long time 1" << std::endl;
		bool newLocation2 = true;
		while (newLocation2){	
			int x = xGen.next();
			int y = yGen.next();
			locationSpace->getObjectsAt(repast::Point<int>(x, y), locationList);

			if(locationList[0]->getState()==2)
			{
				locationList.clear();
			}
			else
			{
				householdSpace->moveTo(id_A, repast::Point<int>(x, y));
				locationList[0]->setState(1);
				newLocation2 = false;
			}
		}
		//std::cout << "taking a long time 2" << std::endl;
		if(fieldSearch(newAgent)){
			//std::cout << "New Agent "<< houseID << " created" << std::endl;
			//std::cout << "taking a long time 3" << std::endl;
			network(newAgent);
			newAgent->initVariables(Neighbours, param.biasMu, 180*happinessGen->next(), biasGen->next(), param.deltaNeighboursWeight, param.expectationsWeight, param.fissionsWeight, param.deathWeight);
			updateBias(newAgent,Neighbours);
		}
		houseID++;
	}
}
