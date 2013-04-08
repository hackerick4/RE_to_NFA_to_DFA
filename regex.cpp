#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string.h>
#include <algorithm>
#include "regex.h"
using namespace std;

#define EPSILON       -1 
#define CLOSURE       '*'
#define PLUS_CLOSURE  '+'
#define OPTIONAL      '?'
#define OPEN_PAREN    '('
#define CLOSE_PAREN   ')'
#define OR            '|'
#define FALSE_CONCAT  '.'


RegEx::RegEx() {
}
RegEx::~RegEx(){
	CleanUp();
}
bool RegEx::Is_Meta( char inputCh ) {
	return(( inputCh == CLOSURE )      || 
		   ( inputCh == OR )           || 
		   ( inputCh == OPEN_PAREN )   || 
		   ( inputCh == CLOSE_PAREN )  || 
		   ( inputCh == PLUS_CLOSURE ) ||
		   ( inputCh == FALSE_CONCAT ) ||
		   ( inputCh == OPTIONAL ));
}
bool RegEx::Is_inputCH(char inputCh)      { return(!Is_Meta(inputCh));   }
bool RegEx::Is_Left_Brackets(char inputCh)  { return(inputCh == OPEN_PAREN);  }
bool RegEx::Is_Right_Brackets(char inputCh) { return(inputCh == CLOSE_PAREN); }

void RegEx::CleanUp() {
	group.clear();
	state_set.clear();
	for(int i = 0; i < (int)NFA_Table.size(); ++i) {
		delete NFA_Table[i];
	}
	NFA_Table.clear();

	for(int i = 0; i < (int)DFA_Table.size(); ++i)
		delete DFA_Table[i];
	DFA_Table.clear();

	NextState_ID = 0;

	while(!character_stack.empty())
		character_stack.pop();
    while(!expression_stack.empty())
		expression_stack.pop();
    InputCh_SET.clear();

}

bool RegEx::pop_table(Table  &NFATable) {
	if(character_stack.size()>0) {
		NFATable = character_stack.top();
		character_stack.pop();
		return true;
	}
	return false;
}

bool RegEx::Connect() { //a.b

    Table  LeftTable, RightTable;
    if(!pop_table(RightTable) || !pop_table(LeftTable))
		return false;

	(*(LeftTable.rbegin()))->Add_Transition(EPSILON, (*(RightTable.begin())));
	LeftTable.insert(LeftTable.end(), RightTable.begin(), RightTable.end());
	character_stack.push(LeftTable);

	return true;
}

void RegEx::Push_character_stack(char chInput) {
    RegExState *s0 = new RegExState(NextState_ID++);
	RegExState *s1 = new RegExState(NextState_ID++);

	s0->Add_Transition(chInput, s1); //1--a-->2

	Table NewSubTable;
	NewSubTable.push_back(s0);
	NewSubTable.push_back(s1);
	character_stack.push(NewSubTable);

	InputCh_SET.insert(chInput);
}

bool RegEx::ClosureOptional() {//?

	Table PrevTable;
	if(!pop_table(PrevTable))
		return false;

	RegExState *LeftTable  = new RegExState(NextState_ID++);
	RegExState *RightTable = new RegExState(NextState_ID++);

	LeftTable->Add_Transition(EPSILON, RightTable);
	LeftTable->Add_Transition(EPSILON, ((*(PrevTable.begin()))));
	((*(PrevTable.rbegin())))->Add_Transition(EPSILON, RightTable);

	PrevTable.insert( PrevTable.begin(), LeftTable );
	PrevTable.push_back ( RightTable);

	character_stack.push(PrevTable);

	return true;
}

bool RegEx::ClosurePlus() {//+
	Table PrevTable;
	if(!pop_table(PrevTable))
		return false;

	RegExState *LeftTable  = new RegExState(NextState_ID++);
	RegExState *RightTable = new RegExState(NextState_ID++);

	LeftTable->Add_Transition(EPSILON, ((*(PrevTable.begin()))));
	((*(PrevTable.rbegin())))->Add_Transition(EPSILON, RightTable);
	((*(PrevTable.rbegin())))->Add_Transition(EPSILON, ((*(PrevTable.begin()))));

	PrevTable.insert( PrevTable.begin(), LeftTable );
	PrevTable.push_back ( RightTable);

	character_stack.push(PrevTable);

	return true;
}
bool RegEx::Closure() {//*

	Table PrevTable;
	if(!pop_table(PrevTable))
		return false;

	RegExState *LeftTable  = new RegExState(NextState_ID++);
	RegExState *RightTable = new RegExState(NextState_ID++);

	LeftTable->Add_Transition(EPSILON, RightTable);
	LeftTable->Add_Transition(EPSILON, ((*(PrevTable.begin()))));
	((*(PrevTable.rbegin())))->Add_Transition(EPSILON, RightTable);
	((*(PrevTable.rbegin())))->Add_Transition(EPSILON, ((*(PrevTable.begin()))));

	PrevTable.insert( PrevTable.begin(), LeftTable );
	PrevTable.push_back ( RightTable);

	character_stack.push(PrevTable);

	return true;
}

bool RegEx::Or() { // |

	Table  UpperTable, LowerTable;

	if(!pop_table(LowerTable) || !pop_table(UpperTable))
		return false;

	RegExState *LeftTable	= new RegExState(NextState_ID++);
	RegExState *RightTable	= new RegExState(NextState_ID++);
	
	LeftTable->Add_Transition(EPSILON, ((*(UpperTable.begin()))));
	LeftTable->Add_Transition(EPSILON, ((*(LowerTable.begin()))));
	((*(UpperTable.rbegin())))->Add_Transition(EPSILON, RightTable);
	((*(LowerTable.rbegin())))->Add_Transition(EPSILON, RightTable);

	LowerTable.push_back(RightTable);
	UpperTable.insert(UpperTable.begin(),LeftTable);
	UpperTable.insert(UpperTable.end(), LowerTable.begin(), LowerTable.end());

	character_stack.push(UpperTable);
	return true;
}
void RegEx::EpsilonClosure(std::set<RegExState*> startSet, std::set<RegExState*> &result) { //Epsilon
	std::stack<RegExState*> UnVisitedStates;	
	result.clear();	
	result = startSet; 
	state_iter iter;
	for(iter = startSet.begin(); iter != startSet.end(); ++iter) {
		UnVisitedStates.push(*iter);
	}

	while(!UnVisitedStates.empty()) {
		RegExState* curState = UnVisitedStates.top();
		UnVisitedStates.pop();

		Table epsilonStates;
		curState->Get_Transition(EPSILON, epsilonStates);

		table_iter epsilonItr;

		for ( epsilonItr = epsilonStates.begin(); epsilonItr != epsilonStates.end(); ++epsilonItr ){
			if(result.find((*epsilonItr)) == result.end()) {
				result.insert((*epsilonItr));
				UnVisitedStates.push((*epsilonItr));
			}
		}
	}
}

void RegEx::Move(char chInput, std::set<RegExState*> NFAState, std::set<RegExState*> &Result) {
    Result.clear();
	state_iter iter;
	for(iter = NFAState.begin(); iter != NFAState.end(); ++iter) {
		Table States;
		(*iter)->Get_Transition(chInput, States);
		for(int index = 0; index < (int)States.size(); ++index){
			Result.insert(States[index]);
		}
	}
}

void RegEx::ConvertNFAtoDFA() {
	for(int i = 0; i < (int)DFA_Table.size(); ++i)
		delete DFA_Table[i];
	DFA_Table.clear();

	if(NFA_Table.size() == 0)
		return;
	
	std::set<RegExState*> NFAStartStateSet;
	NFAStartStateSet.insert(NFA_Table[0]);
	std::set<RegExState*> DFAStartStateSet;
    EpsilonClosure(NFAStartStateSet, DFAStartStateSet);
	NextState_ID=0;
	RegExState *DFAStartState = new RegExState(DFAStartStateSet, NextState_ID++);

	DFA_Table.push_back(DFAStartState);

	Table UnVisitedStates;
	UnVisitedStates.push_back(DFAStartState);
	while(!UnVisitedStates.empty()) {

		RegExState* CurDFAState = UnVisitedStates[UnVisitedStates.size()-1];
		UnVisitedStates.pop_back(); //have been visited

		std::set<char>::iterator iter;
		for(iter = InputCh_SET.begin(); iter != InputCh_SET.end(); ++iter) {
			std::set<RegExState*> MoveRes, EpsilonClosureRes;

			Move(*iter, CurDFAState->Get_NFA_state(), MoveRes);
			EpsilonClosure(MoveRes, EpsilonClosureRes);

			state_iter MoveResItr;
			state_iter EpsilonClosureResItr;

			bool bFound = false;
			RegExState *s   = NULL;
			for(int i = 0; i < (int)DFA_Table.size(); ++i) {
				s = DFA_Table[i];
				if(s->Get_NFA_state() == EpsilonClosureRes) {
					bFound = true;
					break;
				}
			}
			if(!bFound) {
				RegExState* U = new RegExState(EpsilonClosureRes, NextState_ID++);
				UnVisitedStates.push_back(U);
				DFA_Table.push_back(U);                                
				CurDFAState->Add_Transition(*iter, U);
			} else {
				CurDFAState->Add_Transition(*iter, s);
			}
		}
	}	
	ReduceDFA();
}
void RegEx::ReduceDFA() {
	std::set<RegExState*> DeadEndSet;
	for(int i = 0; i < (int)DFA_Table.size(); ++i) {
		if(DFA_Table[i]->IsDeadEnd()) {
			DeadEndSet.insert(DFA_Table[i]);
		}
	}
	if(DeadEndSet.empty())
		return;
	state_iter iter;
	for(iter = DeadEndSet.begin(); iter != DeadEndSet.end(); ++iter) {
		for(int i = 0; i < (int)DFA_Table.size(); ++i)
			DFA_Table[i]->Remove_Transition(*iter);

		table_iter pos;
		for(pos = DFA_Table.begin(); pos != DFA_Table.end(); ++pos) {
			if(*pos == *iter) {
				break;
			}
		}
		DFA_Table.erase(pos);
		delete *iter;
	}
}


bool RegEx::Should_be_otherGroup(string s){
	int check = atoi(s.c_str());
	for (vector <std::set<int>> ::iterator it=group.begin(); it!= group.end();++it){
		bool is_in = (*it).find(check) != (*it).end();
	    if (is_in) return true;
	}
   return false;
}
void RegEx::MinimizeDFA () {
	
	set<int> newParty;
	for(int i = 0; i < (int)DFA_Table.size(); i++) state_set.insert(i);
	for(int i = 0; i < (int)DFA_Table.size(); i++) {
		if(DFA_Table[i]->be_acceptingState) { //depart the end state
			newParty.insert(  atoi(DFA_Table[i]->getStringID().c_str()) );
			state_set.erase( atoi(DFA_Table[i]->getStringID().c_str()));
		}
	}
	group.push_back(newParty);
	newParty.clear();
	for( int i = 0; i < (int)state_set.size(); ++i) {
		std::vector<RegExState*> State;
		RegExState *pState = DFA_Table[i];
	    std::set<char>::iterator iter;
		for(iter = InputCh_SET.begin(); iter != InputCh_SET.end(); ++iter) {
			pState->Get_Transition(*iter, State);
			for(int j = 0; j < (int)State.size(); ++j) {
				if (Should_be_otherGroup(State[j]->getStringID())){
					newParty.insert(  atoi(DFA_Table[i]->getStringID().c_str()) );
					group.push_back(newParty);
					newParty.clear();
					if (!(state_set.find(i)!=state_set.end())) 
						break;
					state_set.erase( atoi(DFA_Table[i]->getStringID().c_str()));
					i=0;
				}
			}
		}
	}
	if (group.size()>1)group.push_back(newParty);
	vector <std::set<int>> ::iterator it=group.end()-1;
	(*it).insert( state_set.begin(),state_set.end());
	 sort(group.begin(), group.end());
	 group.erase( unique( group.begin(), group.end() ), group.end());
}

std::string RegEx::PreProcessBracket( std::string strRegEx) {
	std::string::size_type startPos, endPos, separatorPos;
	std::string ReplacedStrRegEx;

	startPos = strRegEx.find_first_of("[");
	endPos   = strRegEx.find_first_of("]");
	separatorPos = strRegEx.find_first_of("-");

	if ( startPos == std::string::npos || endPos == std::string::npos )
		return strRegEx;

	ReplacedStrRegEx += strRegEx.substr( 0, startPos );
	ReplacedStrRegEx.push_back('(');
	std::string result = strRegEx.substr( startPos + 1, endPos - startPos - 1);
	char first = result[0];
	char last  = result[result.size() - 1 ];

	if ( separatorPos != std::string::npos ) {		
		while ( first != last ) {
			ReplacedStrRegEx.push_back(first);
			ReplacedStrRegEx += "|";
			first++;
		}
		ReplacedStrRegEx.push_back(first);				
	} else {
		startPos++;
		while ( startPos != endPos - 1) {
			ReplacedStrRegEx.push_back(strRegEx[startPos]);
			ReplacedStrRegEx += "|";
			startPos++;
		}
		ReplacedStrRegEx.push_back(strRegEx[endPos - 1]);
	}
	ReplacedStrRegEx += strRegEx.substr( endPos + 1, strRegEx.size() - endPos );
	ReplacedStrRegEx += ")";

	return ReplacedStrRegEx;
}

bool RegEx::RE_compile(std::string strRegEx) {
	cout << "REG : "<<strRegEx<<endl;
	while ( strRegEx.find( "[" ) != std::string::npos ) { //ÀË¬d¬O§_¦³[]
		strRegEx = PreProcessBracket(strRegEx);
	}

	infix_order_RE = const_cast<char*>(strRegEx.c_str());
	CleanUp();
	
	if(!Construct_NFA()) 
		return false;	

	pop_table( NFA_Table);
	NFA_Table[NFA_Table.size() - 1 ]->be_acceptingState = true;
	//cout << "\n=======NFA_transition table======="<<endl;
	//PrintTable(NFA_Table);
	ConvertNFAtoDFA();
	cout << "\n=======DFA_transition table======="<<endl;
	PrintTable(DFA_Table);
	MinimizeDFA();
	PrintMinimizeDFA_Group();
	return true;
}

bool RegEx::Dose_Match(std::string strText) {
	 cout<<strText;
	stringText = strText;
	RegExState *pState = DFA_Table[0];
	std::vector<RegExState*>  Transition;
	for(int i = 0; i < (int)stringText.size(); ++i) {
		char CurrChar = stringText[i];		
		pState->Get_Transition(CurrChar, Transition);
		if ( Transition.empty()){
			return false;
		}
		pState = Transition[0];
		if ( pState->be_acceptingState && i==stringText.size()-1){
			return true;
		}
	}
	return false;
}

int RegEx::PreProcessLiterals() {
  if (current_prePro_char == '\0') 
	  return -1;
  if (current_prePro_char == '\\') {
    current_prePro_char = *(infix_order_RE++);
    if (current_prePro_char == '\0') 
		return -1;
    post_order_RE += current_prePro_char;
    current_prePro_char = *(infix_order_RE++);
  } else if (strchr("()|*+?", current_prePro_char) == NULL) {  //not ()|*+?
    if (current_prePro_char == '.')
		post_order_RE += ".";
    else 
		post_order_RE += current_prePro_char;

    current_prePro_char = *(infix_order_RE++);
  } else if (current_prePro_char == '(') {
    current_prePro_char = *(infix_order_RE++);
    PreProcessOr();
    if (current_prePro_char != ')') 
		return -1;
    current_prePro_char = *(infix_order_RE++);
  } else
		return -1;
  return 0;
}
int RegEx::PreProcessClosure() {
	PreProcessLiterals();
	while (current_prePro_char != '\0' && strchr("*+?",current_prePro_char) != NULL) {
		post_order_RE += current_prePro_char;
		current_prePro_char = *(infix_order_RE++);
	}
	return 0;
}
int RegEx::PrePreprocessConcatenation(void) {
	PreProcessClosure();
	while (strchr(")|*+?", current_prePro_char) == NULL) { 
		PreProcessClosure();
		post_order_RE += ".";
	}
	return 0;
}
int RegEx::PreProcessOr() {
	PrePreprocessConcatenation(); //ab -> a.b
	while (current_prePro_char == '|') {
		current_prePro_char = *(infix_order_RE++);
		PrePreprocessConcatenation();
		post_order_RE += "|";
	}
	return 0;
}

int RegEx::CovertToPostfix() {
	post_order_RE.clear();
	current_prePro_char = *(infix_order_RE++);
	PreProcessOr();
	cout << "POST ORDER : "<<post_order_RE<<endl;
	return 0;
}
bool RegEx::Construct_NFA() {
	CovertToPostfix();
	for(int i = 0; i < (int)post_order_RE.size(); ++i) {
		char curChar = post_order_RE[i];
		if ( Is_inputCH( curChar )) {
			Push_character_stack( curChar );
		} else if ( Is_Meta( curChar ) ) {
			switch ( curChar ) {
				case CLOSURE:
					Closure();
					break;
				case PLUS_CLOSURE:
					ClosurePlus();
					break;
				case OPTIONAL:
					ClosureOptional();
					break;
				case OR:
					Or();
					break;
				case FALSE_CONCAT:
					Connect();
					break;
			}
		}		
	}
	return true;
}

void RegEx::PrintTable(Table &table) {
	std::string TableString;
	TableString += "Start state: { " + table[0]->getStringID() +" }";
	 TableString +="\nEND State: {";
	for(int i = 0; i < (int)table.size(); ++i) {
		RegExState *pState = table[i];
		if(pState->be_acceptingState) {
			TableString += pState->getStringID() + " ";
		}
	}
	TableString +="}\n"; 
	for( int i = 0; i < (int)table.size(); ++i) {
		RegExState *pState = table[i];
		std::vector<RegExState*> State;
		pState->Get_Transition(EPSILON, State);
		for(int j = 0; j < (int)State.size(); ++j) {
			TableString += "   " + pState->getStringID() + " -> " + State[j]->getStringID();
			TableString += "   [label = \"£` \"]\n";
		}
		std::set<char>::iterator iter;
		for(iter = InputCh_SET.begin(); iter != InputCh_SET.end(); ++iter) {
			pState->Get_Transition(*iter, State);
			for(int j = 0; j < (int)State.size(); ++j) {
				TableString += "   " + pState->getStringID() + " -> " + State[j]->getStringID() ;
				std::stringstream out;
				out << *iter;
				TableString += "   [label = \"" + out.str() + " \"]\n";
			}
		}
	}
	std::cout << TableString;
}
      
void RegEx::PrintMinimizeDFA_Group(){
	cout<<"\n=====MinimizeDFA===== \n£S = { ";
	for (vector <std::set<int>> ::iterator it=group.begin(); it!= group.end();++it){
		for( set<int>::iterator set_it = (*it).begin(); set_it != (*it).end(); set_it++) {
			if (set_it== (*it).begin()) cout << "{ ";
			cout <<*set_it<< " ";
    }
		cout << " } ";
}
	cout << " }";
}