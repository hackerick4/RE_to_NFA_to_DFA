#include <iostream>
#include <sstream>
#include "regexstate.h"
using namespace std;
RegExState::~RegExState() {
	NFAStates_SET.clear();
	transition_table.clear();
}

RegExState::RegExState(std::set<RegExState*> NFAState, int nID) {
	NFAStates_SET = NFAState;
	state_ID = nID;
	be_acceptingState = false;
	Group_ID = 0;
	state_iter iter;
	for(iter = NFAState.begin(); iter != NFAState.end(); ++iter) {
		if((*iter)->be_acceptingState) {
			be_acceptingState = true;
		}
	}
}
RegExState::RegExState(const RegExState &other){ 
	*this = other;
}


void RegExState::Add_Transition(char inputCh, RegExState *pState){
	transition_table.insert(std::make_pair(inputCh, pState));
}

void RegExState::Remove_Transition(RegExState* pState) {
	std::multimap<char, RegExState*>::iterator iter;
	for(iter = transition_table.begin(); iter != transition_table.end();) {
		RegExState *toState = iter->second;
		if(toState == pState)
			transition_table.erase(iter++);
		else 
			++iter;
	}
}

void RegExState::Get_Transition(char inputCh, Table &States) {
	States.clear();
	std::multimap<char, RegExState*>::iterator iter;
	for(iter = transition_table.lower_bound(inputCh); iter != transition_table.upper_bound(inputCh); ++iter) {
		RegExState *pState = iter->second;
		States.push_back(pState);
	}
}

std::set<RegExState*>& RegExState::Get_NFA_state() { 
	return NFAStates_SET; 
}

bool RegExState::IsDeadEnd() {
	if(be_acceptingState)
		return false;
	if(transition_table.empty())
		return true;
	std::multimap<char, RegExState*>::iterator iter;
	for(iter=transition_table.begin(); iter!=transition_table.end(); ++iter){
		RegExState *toState = iter->second;
		if(toState != this)
			return false;
	}
	return true;
}	

RegExState& RegExState::operator=(const RegExState& other) { 
	this->transition_table	= other.transition_table; 
	this->state_ID	= other.state_ID;
	this->NFAStates_SET	= other.NFAStates_SET;
	return *this;
}

bool RegExState::operator==(const RegExState& other) {
	if(NFAStates_SET.empty())
		return(state_ID == other.state_ID);
	else 
		return(NFAStates_SET == other.NFAStates_SET);
}
std::string RegExState::getStringID(){
	std::stringstream out;
	out << state_ID;
	return out.str();
}



