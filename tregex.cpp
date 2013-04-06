#include <conio.h>
#include "regex.h"
#include <iostream>
#include <fstream>
using namespace std;

int main() {
ifstream fin;
	 string File_Name,readIn;
	 cout << "Input the resource name :";
	 cin >> File_Name;
	 fin.open(File_Name);
	 RegEx re;
     while (getline(fin,readIn)){
		 if (readIn.substr(0,4)=="REG:"){
			 cout<<"\n#####################"<<endl;
			if (!re.RE_compile(readIn.substr(4,std::string::npos))) 
				cout<<"RE is wrong!";
			cout<<"\n======================"<<endl;
		 }
		 else if (readIn.substr(0,1)=="#" ||readIn.substr(0,1)=="\n" || readIn.substr(0,1)=="") 
			 continue;
		 else {
			 if (re.Dose_Match(readIn) ) cout <<" Dose match the RE"<<endl;
			 else cout <<" does not match the RE"<<endl;
		 }
	 }
	getch();
	return 0;
}
