#ifndef _REGEXSTATE_H_
#define _REGEXSTATE_H_

#include <map>
#include <set>
#include <vector>
#include <string>

class RegExState{
private:	
	std::set<RegExState*> NFAStates_SET;	
	typedef std::vector<RegExState*> Table;
	typedef std::set<RegExState*>::iterator state_iter;
	std::multimap<char, RegExState*> transition_table;	
	bool marked;

public:
	
	bool be_acceptingState;
	RegExState() : state_ID(-1), be_acceptingState(false) {};
	RegExState(int nID) : state_ID(nID), be_acceptingState(false), Group_ID(0) {};
	RegExState(std::set<RegExState*> NFAState, int nID);
	RegExState(const RegExState &other);

	virtual ~RegExState();

	void Add_Transition(char inputCh, RegExState *pState);
	void Remove_Transition(RegExState* pState);
	void Get_Transition(char inputCh, Table &States);
	std::set<RegExState*>& Get_NFA_state();

	bool IsDeadEnd();
	RegExState& operator=(const RegExState& other);
	bool operator==(const RegExState& other);
    std::string getStringID();
	int state_ID;
	int  Group_ID;
protected:
};
#endif
