#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <conio.h>
#include "regex.h"
#include <iostream>
#include <fstream>
using namespace std;

int main() {

	 ifstream fin;
	 string File_Name,readIn;
	 cin >> File_Name;
	 fin.open(File_Name);
	 RegEx re;
     while (getline(fin,readIn)){
		 if (readIn.substr(0,4)=="REG:"){
			 cout<<endl;
			re.Compile(readIn.substr(4,std::string::npos));
		 }
		 else if (readIn.substr(0,1)=="#" ||readIn.substr(0,1)=="\n" || readIn.substr(0,1)=="") 
			 continue;
		 else {
			 if (re.Match(readIn) ) cout <<" matches the RE"<<endl;
			 else cout <<" does not match the RE"<<endl;
		 }
	 }
	getch();
	return 0;
}
