/////////////////////////////////////////////////////////////////
// load-memory command file.
//
// Author: Yongjia Wang
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

//#ifdef SEMANTIC_MEMORY


#include "cli_CommandLineInterface.h"

#include "sml_Names.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "IgSKI_Agent.h"
//#include "agent.h"
//#include "print.h"
#include "IgSKI_DoNotTouch.h"
#include "IgSKI_Kernel.h"

//#include "../../gSKI/src/gSKI_Agent.h"
//#include "../../SoarKernel/include/agent.h"
//#include "../../SoarKernel/include/print.h"
//#include "gSKI_DoNotTouch.h"
//#include "gSKI_Kernel.h"



using namespace cli;
using namespace sml;
using namespace std;
using namespace gSKI;

// Preload semantic memory from file
bool CommandLineInterface::ParseLoadMemory(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	
/* In IgSKI now
	Agent* pAgent2 = (Agent*)(pAgent);
	agent* thisAgent = pAgent2->GetSoarAgent();
*/
	for(std::vector<string>::iterator itr = argv.begin(); itr != argv.end(); ++itr){
		cout << *itr << endl;
	}
	
	if(argv.size() < 2){
		cout << "no file name specified\n";
		return false;
	}

	string filename = argv[1];

	StripQuotes(filename);

    // Separate the path out of the filename if any
	// Code Copied from cli_source.cpp
	std::string path;
	unsigned int separator1 = filename.rfind('/');
	if (separator1 != std::string::npos) {
		++separator1;
		if (separator1 < filename.length()) {
			path = filename.substr(0, separator1);
			filename = filename.substr(separator1, filename.length() - separator1);
			if (!DoPushD(path)) return false;
		}
	}
	unsigned int separator2 = filename.rfind('\\');
	if (separator2 != std::string::npos) {
		++separator2;
		if (separator2 < filename.length()) {
			path = filename.substr(0, separator2);
			filename = filename.substr(separator2, filename.length() - separator2);
			if (!DoPushD(path)) return false;
		}
	}

	// Open the file
	std::ifstream soarFile(filename.c_str());
	if (!soarFile) {
		if (path.length()) DoPopD();
		return SetError(CLIError::kOpenFileFail);
	}
	
	//cout << "Path: " << path << endl;
	//cout << "Filename: " << filename << endl;
	

 	m_Result << "Loading Memory\nPath: " << path << "\nFilename: " << filename << std::endl;
	//AddListenerAndDisableCallbacks(pAgent);
	//print(thisAgent, "Loading Memory\nPath: %s\nFilename: %s\n", path.c_str(), filename.c_str());
	//RemoveListenerAndEnableCallbacks(pAgent);

	std::string line;
	int lineCount = 0;
	
	

	while (getline(soarFile, line)) {
		
		// Increment line count
		++lineCount;

		// Trim whitespace and comments
		if (!Trim(line)) {
		//	HandleSourceError(lineCount, filename); Interface changed
			if (path.length()) DoPopD();
			return false;
		}
	//	mResult << line << std::endl;
		//cout << line << endl;
		//AddListenerAndDisableCallbacks(pAgent);
		//print(thisAgent, "%s\n", line.c_str());
		//RemoveListenerAndEnableCallbacks(pAgent);
		if(line.size() == 0 || line[0] == '#'){
			continue;
		}
		istringstream isstr(line);
		string id, attr, value;
		int type;

		isstr >> id >> attr >> value >> type;

		// Attain the evil back door of doom, even though we aren't the TgD
		gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
		pKernelHack->load_semantic_memory_data(pAgent, id, attr, value, type);
		
		/* This part in (I)gSKI_DoNotTouch, 
		char id_letter = id[0];
		unsigned long id_number = StringToUnsignedLong(id.substr(1));
		//AddListenerAndDisableCallbacks(pAgent);
		//print(thisAgent, "%c counter %d, number%d\n", id_letter, 
		//	 thisAgent->id_counter[id_letter-'A'], id_number);
		//RemoveListenerAndEnableCallbacks(pAgent);
		
		if(id_number >= thisAgent->id_counter[id_letter-'A']){
			thisAgent->id_counter[id_letter-'A'] = id_number+1;//start from the next number
			//AddListenerAndDisableCallbacks(pAgent);
			//print(thisAgent, "%c counter %d\n", id_letter, id_number);
			//RemoveListenerAndEnableCallbacks(pAgent);

		}

		// get rid of leading zeros of the id number, kind of preprocessing
		char new_id[32];
		sprintf(new_id, "%c%d", id_letter, id_number);
		id = string(new_id);
		thisAgent->semantic_memory->insert_LME(id, attr, value, type);
		*/
	}


	soarFile.close();
	if (path.length()) DoPopD();

	// Quit needs no help
	return true;
}


bool CommandLineInterface::ParsePrintMemory(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
	AddListenerAndDisableCallbacks(pAgent);
	
	string attr = "", value = "";
	if(argv.size() >= 3){
		attr = argv[1];
		value = argv[2];
	}
	// print the chunks with the specified attr-value
	// Only support one pair, should be able to support more complex regex in the future.
	pKernelHack->print_semantic_memory(pAgent, attr, value);
	
	int count_size = pKernelHack->semantic_memory_chunk_count(pAgent);
	int lme_size = pKernelHack->semantic_memory_lme_count(pAgent);
	m_Result << count_size << " Chunks" << endl;
	m_Result << lme_size << " Elements" << endl;

	RemoveListenerAndEnableCallbacks(pAgent);

/*
	Agent* pAgent2 = (Agent*)(pAgent);
	agent* thisAgent = pAgent2->GetSoarAgent();
		
	vector<LME> content;
	thisAgent->semantic_memory->dump(content);
	for(vector<LME>::iterator itr = content.begin(); itr != content.end(); ++itr){
		AddListenerAndDisableCallbacks(pAgent);
		print(thisAgent, "<%s, %s, %s, %d>\n",itr->id.c_str(), itr->attr.c_str(), itr->value.c_str(), itr->value_type);
		RemoveListenerAndEnableCallbacks(pAgent);
	}

*/
	// Quit needs no help
	return true;
}

bool CommandLineInterface::ParseClearMemory(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
	RemoveListenerAndEnableCallbacks(pAgent);
	int size = pKernelHack->clear_semantic_memory(pAgent);
	m_Result << size << " elements cleared" << endl;
	
	RemoveListenerAndEnableCallbacks(pAgent);
/*
	Agent* pAgent2 = (Agent*)(pAgent);
	agent* thisAgent = pAgent2->GetSoarAgent();
	//thisAgent->semantic_memory->insert_LME(id, attr, value, type);
		
	int size = thisAgent->semantic_memory->clear();
	AddListenerAndDisableCallbacks(pAgent);
	print(thisAgent, "%d LMEs cleared\n",size);
	RemoveListenerAndEnableCallbacks(pAgent);
*/
	return true;
}

bool CommandLineInterface::ParseSummarizeMemory(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
	RemoveListenerAndEnableCallbacks(pAgent);
	int count_size = pKernelHack->semantic_memory_chunk_count(pAgent);
	int lme_size = pKernelHack->semantic_memory_lme_count(pAgent);
	m_Result << count_size << " Chunks" << endl;
	m_Result << lme_size << " Elements" << endl;
	
	RemoveListenerAndEnableCallbacks(pAgent);

	return true;
}

bool CommandLineInterface::ParseSmemOption(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	Options optionsData[] = {
		//{'d', "disable",	0},
		//{'d', "off",		0},
		//{'e', "enable",		0},
		//{'e', "on",			0},
		{'a', "automatic",	0},
		{'d', "deliberate",		0},
		{'h', "help",			0},
		//{'w', "wrong",		0},
		{0, 0, 0}
	};
	SmemBitset options(0);
	
	
	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1){ // display info
			int current_op = pKernelHack->semantic_memory_set_parameter(pAgent, 2);
			if(current_op == 0){
				RemoveListenerAndEnableCallbacks(pAgent);
				m_Result << "Current Option: Deliberate" << endl;
				RemoveListenerAndEnableCallbacks(pAgent);
			}
			else if(current_op == 1){
				RemoveListenerAndEnableCallbacks(pAgent);
				m_Result << "Current Option: Automatic" << endl;
				RemoveListenerAndEnableCallbacks(pAgent);
			}
			break;
		}

		switch (m_Option) {
			case 'd':
				pKernelHack->semantic_memory_set_parameter(pAgent, 0);
				//options.set(0);
				break;
			case 'a':
				pKernelHack->semantic_memory_set_parameter(pAgent, 1);
				//options.set(1);
				break;
			case 'h':
				m_Result << "-a, --automatic: automatic saving, disable deliberate saving" << endl;
				m_Result << "-d, --deliberate: deliberate saving, disable automatic saving" << endl;
				//options.set(1);
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// No non-option arguments
	//if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);
	//RemoveListenerAndEnableCallbacks(pAgent);
	//m_Result << m_Option << endl;
	//RemoveListenerAndEnableCallbacks(pAgent);
	
	
	return true;
}

// Preload Cluster file
bool CommandLineInterface::ParseCluster(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	
	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
	for(std::vector<string>::iterator itr = argv.begin(); itr != argv.end(); ++itr){
		cout << *itr << endl;
	}
	
	if(argv.size() < 2){
		cout << "Printing Cluster Weights\n";
		AddListenerAndDisableCallbacks(pAgent);
		pKernelHack->clustering(pAgent, std::vector<std::vector<double> >(), true, false);
		AddListenerAndDisableCallbacks(pAgent);
		return false;
	}

	if(argv[1] == "clear"){
		pKernelHack->clustering(pAgent, std::vector<std::vector<double> >(), false, true);
	}

	// Quit needs no help
	return true;
}


bool CommandLineInterface::ParseClusterTrain(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	
	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
	for(std::vector<string>::iterator itr = argv.begin(); itr != argv.end(); ++itr){
		cout << *itr << endl;
	}
	
	if(argv.size() < 2){
		cout << "No filename specified\n";
		return false;
	}

	string filename = argv[1];

	StripQuotes(filename);

    // Separate the path out of the filename if any
	// Code Copied from cli_source.cpp
	std::string path;
	unsigned int separator1 = filename.rfind('/');
	if (separator1 != std::string::npos) {
		++separator1;
		if (separator1 < filename.length()) {
			path = filename.substr(0, separator1);
			filename = filename.substr(separator1, filename.length() - separator1);
			if (!DoPushD(path)) return false;
		}
	}
	unsigned int separator2 = filename.rfind('\\');
	if (separator2 != std::string::npos) {
		++separator2;
		if (separator2 < filename.length()) {
			path = filename.substr(0, separator2);
			filename = filename.substr(separator2, filename.length() - separator2);
			if (!DoPushD(path)) return false;
		}
	}

	// Open the file
	std::ifstream soarFile(filename.c_str());
	if (!soarFile) {
		if (path.length()) DoPopD();
		return SetError(CLIError::kOpenFileFail);
	}
	
	//cout << "Path: " << path << endl;
	//cout << "Filename: " << filename << endl;
	

 	m_Result << "Cluster Training File\nPath: " << path << "\nFilename: " << filename << std::endl;
	//AddListenerAndDisableCallbacks(pAgent);
	//print(thisAgent, "Loading Memory\nPath: %s\nFilename: %s\n", path.c_str(), filename.c_str());
	//RemoveListenerAndEnableCallbacks(pAgent);

	std::string line;
	int lineCount = 0;
	
	
	std::vector<std::vector<std::pair<std::string, std::string> > > instances;

	while (getline(soarFile, line)) {
		
		// Increment line count
		++lineCount;

		// Trim whitespace and comments
		if (!Trim(line)) {
		//	HandleSourceError(lineCount, filename); Interface changed
			if (path.length()) DoPopD();
			return false;
		}
	//	mResult << line << std::endl;
		//cout << line << endl;
		//AddListenerAndDisableCallbacks(pAgent);
		//print(thisAgent, "%s\n", line.c_str());
		//RemoveListenerAndEnableCallbacks(pAgent);
		if(line.size() == 0 || line[0] == '#'){
			continue;
		}
		istringstream isstr(line);
		
		vector<pair<string, string> > one_instance;
		string attr, value;
		while(isstr >> attr >> value){
			one_instance.push_back(pair<string, string>(attr, value));
		}
		instances.push_back(one_instance);
	}
	int train_times = 1;
	if(argv.size() >= 3){
		istringstream isstr(argv[2]);
		isstr >> train_times;
		//if(!isstr.good()){
		//	train_times = 1;
		//}
	}
	m_Result << "Training " << train_times << " Times" << endl;
	for(int i=0; i<train_times; ++i){
		pKernelHack->cluster_train(pAgent,instances);
	}
	

	soarFile.close();
	if (path.length()) DoPopD();

	// Quit needs no help
	return true;
}

bool CommandLineInterface::ParseClusterRecognize(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	
	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
	for(std::vector<string>::iterator itr = argv.begin(); itr != argv.end(); ++itr){
		cout << *itr << endl;
	}
	
	if(argv.size() < 2){
		cout << "No filename specified\n";
		return false;
	}

	string filename = argv[1];

	StripQuotes(filename);

    // Separate the path out of the filename if any
	// Code Copied from cli_source.cpp
	std::string path;
	unsigned int separator1 = filename.rfind('/');
	if (separator1 != std::string::npos) {
		++separator1;
		if (separator1 < filename.length()) {
			path = filename.substr(0, separator1);
			filename = filename.substr(separator1, filename.length() - separator1);
			if (!DoPushD(path)) return false;
		}
	}
	unsigned int separator2 = filename.rfind('\\');
	if (separator2 != std::string::npos) {
		++separator2;
		if (separator2 < filename.length()) {
			path = filename.substr(0, separator2);
			filename = filename.substr(separator2, filename.length() - separator2);
			if (!DoPushD(path)) return false;
		}
	}

	// Open the file
	std::ifstream soarFile(filename.c_str());
	if (!soarFile) {
		if (path.length()) DoPopD();
		return SetError(CLIError::kOpenFileFail);
	}
	
	//cout << "Path: " << path << endl;
	//cout << "Filename: " << filename << endl;
	

 	m_Result << "Cluster Training File\nPath: " << path << "\nFilename: " << filename << std::endl;
	//AddListenerAndDisableCallbacks(pAgent);
	//print(thisAgent, "Loading Memory\nPath: %s\nFilename: %s\n", path.c_str(), filename.c_str());
	//RemoveListenerAndEnableCallbacks(pAgent);

	std::string line;
	int lineCount = 0;
	
	
	std::vector<std::vector<std::pair<std::string, std::string> > > instances;

	while (getline(soarFile, line)) {
		
		// Increment line count
		++lineCount;

		// Trim whitespace and comments
		if (!Trim(line)) {
		//	HandleSourceError(lineCount, filename); Interface changed
			if (path.length()) DoPopD();
			return false;
		}
	//	mResult << line << std::endl;
		//cout << line << endl;
		//AddListenerAndDisableCallbacks(pAgent);
		//print(thisAgent, "%s\n", line.c_str());
		//RemoveListenerAndEnableCallbacks(pAgent);
		if(line.size() == 0 || line[0] == '#'){
			continue;
		}
		istringstream isstr(line);
		
		vector<pair<string, string> > one_instance;
		string attr, value;
		while(isstr >> attr >> value){
			one_instance.push_back(pair<string, string>(attr, value));
		}
		instances.push_back(one_instance);
	}
	
	std::vector<std::vector<int> > clusters = pKernelHack->cluster_recognize(pAgent,instances);
	//m_Result << clusters.size() << endl;
	for(int i=0; i<clusters.size(); ++i){
		//m_Result << clusters[i].size() << endl;
		for(int j=0; j<clusters[i].size(); ++j){
			m_Result << clusters[i][j] << ",";
		}
		m_Result << endl;
	}
	

	soarFile.close();
	if (path.length()) DoPopD();

	// Quit needs no help
	return true;
}
//#endif
