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
	bias = NewBias;
	if(((assignedField->getExpectedYield() * bias) + maizeStorage) > needs)
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
		fissioned = 1;
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

void Household::calculateHappiness(std::unordered_set<int> currentNeighbours, std::unordered_set<int> deadAgents)
{
	int noPreviousNeighbours = prevNeighbours.size();
	int noNeighbours = currentNeighbours.size(); 
	int noDeadNeighbours = 0;	
	double actualYield = assignedField->getExpectedYield();
	double predictedYield = actualYield * bias; 
	
	for (auto N: deadAgents)
	{
		if (prevNeighbours.find(N) != prevNeighbours.end())
		{
			noDeadNeighbours++;
		}
	}
	// std::cout << "Total dead neighbours " << noDeadNeighbours << std::endl; 

	double temp = deltaNeighboursWeight * (noNeighbours- noPreviousNeighbours - noDeadNeighbours) + expectationsWeight * (actualYield - predictedYield) + fissionWeight * (fissioned) - deathWeight * (noDeadNeighbours) + Happiness.back();

	if (temp > 200){
		temp = 200;
	}else if (temp < -200){
		temp = -200;
	}

	Happiness.push(temp);

	if (Happiness.size() > 5){
		Happiness.pop();
	}
	
	// std::cout << "Happiness: " << Happiness.front() << std::endl; 
	// std::cout << "---------------------------" << std::endl;
	fissioned = 0;
	prevNeighbours = currentNeighbours; 
}

void Household::setBias(double biasTemp)
{
	NewBias = bias - (biasMu * biasTemp);
	if (NewBias > 2)
	{
		NewBias = 2;
	}else if (NewBias < 0)
	{
		NewBias = 0;
	}
	//std::cout << "Agent " << householdId << " Bias: " << NewBias << std::endl;
}

double Household::getBias()
{
	return bias;
}

void Household::initVariables(std::unordered_set<int> currentNeighbours, double Mu, double happiness, double Bias, double W1, double W2, double W3, double W4)
{
	prevNeighbours = currentNeighbours; 
	biasMu = Mu;
	bias = Bias;
	Happiness.push(happiness);
	deltaNeighboursWeight = W1;
	expectationsWeight = W2;
	fissionWeight = W3;
	deathWeight = W4;
}

double Household::AverageHappiness(){
	double temp = 0;
	int N =0;

	while(!Happiness.empty()){
		temp = Happiness.front() + temp;
		Happiness.pop();
		N++;
	}
	return (temp/N); 
}

int Household::getExcessMaize()
{
	
}
