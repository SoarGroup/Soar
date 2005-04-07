#include <fstream>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[])
{

	string inFileName;

	//should have at least 1 argument - the filename for the input link spec	
	if(argc != 2) 
	{
		cout << "Usage: " <<argc<< " <filename>" << endl;
		exit(1);
	}
	else
		inFileName = argv[1];

	if( (int)inFileName.find(".dm/0") != -1)
	{
		//this is a dm file. maybe.
		cout<<"dm"<<endl;
	}
	else if ( (int)inFileName.find(".il") != -1)
	{
		//this is a il file. probably.
	}
	else
	{
		//who knows what this is. 
	}
	





	int foo;
	cin>>foo;
	//inputFile.close();
	return 0;
}

