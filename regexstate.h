#ifndef _REGEXSTATE_H_
#define _REGEXSTATE_H_

#include <map>
#include <set>
#include <vector>
#include <string>

class RegExState{
private:	
	std::set<RegExState*> m_NFAStates;	
public:
	typedef std::vector<RegExState*> Table;
	typedef std::set<RegExState*>::iterator StateIterator;
	std::multimap<char, RegExState*> m_Transition;	
	int m_nStateID;
	bool m_bAcceptingState;
	bool m_Marked;
	int  m_GroupID;

	RegExState() : m_nStateID(-1), m_bAcceptingState(false) {};
	RegExState(int nID) : m_nStateID(nID), m_bAcceptingState(false), m_GroupID(0) {};
	RegExState(std::set<RegExState*> NFAState, int nID);
	RegExState(const RegExState &other);

	virtual ~RegExState();

	void AddTransition(char inputCh, RegExState *pState);
	void RemoveTransition(RegExState* pState);
	void GetTransition(char inputCh, Table &States);
	std::set<RegExState*>& GetNFAState();

	bool IsDeadEnd();
	RegExState& operator=(const RegExState& other);
	bool operator==(const RegExState& other);

	std::string getStringID();
protected:
};
#endif
