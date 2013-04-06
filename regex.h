#ifndef _REG_EX_H_
#define _REG_EX_H_

#include <stack>
#include <set>
#include <list>
#include <string>
#include <vector>
#include <map>

#include "regexstate.h"


class RegEx {
public:
	RegEx();
	~RegEx();

	bool RE_compile(std::string strRegEx);
	bool Dose_Match(std::string strText);
	typedef std::vector<RegExState*> Table;
	typedef Table::iterator table_iter; 
	typedef std::set<RegExState*>::iterator state_iter;



private:
	Table NFA_Table;
	Table DFA_Table;

	std::vector <std::set<int>>group;

	std::set<int> state_set;
	std::stack<Table >       character_stack;
	std::stack<char>         expression_stack;
	std::set<char>           InputCh_SET;
	std::string              stringText;
	std::string              post_order_RE;
	char*                    infix_order_RE;
	char                     current_prePro_char;
	int                      NextState_ID;

	bool Construct_NFA();
	void Push_character_stack(char chInput);
	bool pop_table(Table  &NFATable);
	bool Connect();
	bool Closure();
	bool ClosureOptional();
	bool ClosurePlus();
	bool Or();
	bool Is_Meta(char inputCh); 
	bool Is_inputCH(char inputCh);
	bool Is_Left_Brackets(char inputCh); 
	bool Is_Right_Brackets(char inputCh); 
	bool Should_be_otherGroup(std::string );
	void EpsilonClosure(std::set<RegExState*> T, std::set<RegExState*> &Res);
	void Move(char chInput, std::set<RegExState*> T, std::set<RegExState*> &Res);
	void ConvertNFAtoDFA();
	void ReduceDFA();
	void CleanUp();
	void PrintMinimizeDFA_Group();

	int CovertToPostfix();
	int PreProcessLiterals();
	int PreProcessClosure();
	int PrePreprocessConcatenation();
	int PreProcessOr();
	std::string PreProcessBracket( std::string strRegEx);
	void MinimizeDFA ();

	void PrintTable(Table &table);

protected:
};
#endif