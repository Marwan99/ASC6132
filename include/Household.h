#ifndef HOUSEHOLD
#define HOUSEHOLD

#include "repast_hpc/AgentId.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedDiscreteSpace.h"
#include "repast_hpc/Random.h"
#include "Location.h"
#include <unordered_set>

class Household{
private:
	repast::AgentId householdId;
	Location* assignedField;
	int maizeStorage;
	int age;
	int deathAge;
	std::queue<double> Happiness; 
	std::unordered_set<int> prevNeighbours;
	double deltaNeighboursWeight = 1;
	double expectationsWeight = 1;
	double fissionWeight = 1;
	double deathWeight = 1;
	double bias = 1;
	double biasMu;
	double NewBias;
	bool fissioned;
	

public:
	Household(repast::AgentId id,int a, int deathAge, int mStorage);
	~Household();

	/* Required Getters */
	virtual repast::AgentId& getId() { return householdId; }
	virtual const repast::AgentId& getId() const { return householdId; }

	/* Getters specific to this kind of Agent */
	Location* getAssignedField(){return assignedField; }
	int splitMaizeStored(int percentage);
	bool checkMaize(int needs);
	bool death();
	bool fission(int minFissionAge, int maxFissionAge, double gen, double fProb);
	void nextYear(int needs);
	void chooseField(Location* Field);
	void calculateHappiness(std::unordered_set<int> currentNeighbours, std::unordered_set<int> deadAgents);
	int getExcessMaize();
	void setBias(double Bias);
	double getBias(); 
	double AverageHappiness();
	void initVariables(std::unordered_set<int> currentNeighbours, double Mu, double happiness, double Bias, double W1, double W2, double W3, double W4);
};

#endif
