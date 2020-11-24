#include "Household.h"
#include "repast_hpc/AgentId.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedDiscreteSpace.h"
#include <stdio.h>
#include "repast_hpc/Random.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/Point.h"
#include <unordered_set>


Household::Household(repast::AgentId id, int a, int deAge, int mStorage)
{
	householdId = id;
	age = a;
	deathAge = deAge;
	maizeStorage = mStorage;
	assignedField = NULL;
}

Household::~Household()
{

}

int Household::splitMaizeStored(int percentage)
{
	int maizeEndowment;
	maizeEndowment = maizeStorage * percentage;
	maizeStorage = maizeStorage - maizeEndowment;
	return maizeEndowment;
}

bool Household::checkMaize(int needs)
{
	if((assignedField->getExpectedYield() + maizeStorage) > needs)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Household::death()
{
	if(age>=deathAge)
	{
		repast::AgentId id(householdId.id(),householdId.currentRank(),3);
		householdId = id;
		return true;
	}
	else
	{
		return false;
	}
}

bool Household::fission(int minFissionAge, int maxFissionAge, double gen, double fProb)
{
	if((age>=minFissionAge && age<=maxFissionAge) && (gen <= fProb))
	{
			return true;
	}
	else
	{
		return false;
	}
}

void Household::nextYear(int needs)
{
	age++;
	maizeStorage = assignedField->getExpectedYield() + maizeStorage - needs;
}

void Household::chooseField(Location* Field)
{
	assignedField = Field;
}

void Household::calculateHappiness(bool fission, std::unordered_set<int> currentNeighbours, std::unordered_set<int> deadNeighbours)
{



	//Happiness = 
	

}

int Household::getExcessMaize(){



}

void Household::updateBias(std::vector<repast::AgentId> currentNeighbours)
{


}

double Household::getBias()
{
	

}
