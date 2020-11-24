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
	double bias;
	double biasMu;
	

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
	void calculateHappiness(bool fission, std::unordered_set<int> currentNeighbours, std::unordered_set<int> deadNeighbours);
	int getExcessMaize();
	void updateBias(std::vector<repast::AgentId> currentNeighbours);
	double getBias(); 
};

#endif
