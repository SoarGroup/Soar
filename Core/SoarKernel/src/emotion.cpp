#include <portability.h>

// TODO:
// -- change appraisal registration to detect when appraisal has already been registered for that frame (and print a warning)
// -- Figure out how to get highest ela/joy when one step away from goal (must involve changing appraisals)
// -- Should dimensions with no values report a default value (e.g. 0) or should they report no value (e.g. fInvalidValue)?
// ---- currently reporting no value
// -- Should some dimensions (e.g. discrepancy & outcome_probability) not boost and decay? (e.g. something shouldn't seem more likely just because something else was likely recently)


#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <time.h>


#include "AppraisalFrame.h"
#include "Mood.h"
#include "ModalEmotion.h"
#include "Feeling.h"
#include "AppraisalStatus.h"

using namespace std;

AppraisalStatus appraisalStatus;

AppraisalFrame currentEmotion;

/*
string RHSSetParameters(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
   vector<string> tokens;
	Tokenize(pArgument, &tokens);

	string result = currentMood.SetParameters(tokens);

	PrintMessage("\n" + result);
	return result;
}

string RHSSetAppraisalStatus(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
   vector<string> tokens;
	Tokenize(pArgument, &tokens);

   bool status = atoi(tokens[1].c_str())!=0; // !=0 silences a warning
   string result = appraisalStatus.SetStatus(tokens[0], status);
   if(!status) currentMood.DisableAppraisal(tokens[0]);

	PrintMessage("\n" + result);
	return result;
}

string RHSRegisterAppraisal(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
   vector<string> tokens;
	Tokenize(pArgument, &tokens);

	if(tokens.size() != 3) {
		string error = "+++Expected 3 arguments, got " + tokens.size();
		cerr << "\n" + error;
		return error;
	}

	if(tokens[0] != currentEmotion.id) {
		currentEmotion.Reset(tokens[0], currentEmotion.outcome_probability);
	}

	string result = currentEmotion.SetAppraisalValue(tokens[1], tokens[2]);

	PrintMessage("\n" + result);
	return result;
}

string RHSResetAppraisal(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
   vector<string> tokens;
	Tokenize(pArgument, &tokens);

	if(tokens.size() != 1) {
		string error = "+++Expected 1 argument, got " + tokens.size();
		cerr << "\n" + error;
		return error;
	}

	string result = currentEmotion.ResetAppraisalValue(tokens[0]);

	PrintMessage("\n" + result);
	return result;
}

string RHSGenerateFeelingLabel(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
	return GenerateLabel(currentFeeling.af);
}

string RHSGenerateFeelingIntensity(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
	double intensity = currentFeeling.af.CalculateIntensity();
	string s = ToString(intensity);
	PrintMessage("\nIntensity" + s);
	return s;
}

string RHSGenerateFeelingValence(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
	double valence = currentFeeling.af.CalculateValence();
	string s = ToString(valence);
	PrintMessage("\nValence" + s);
	return s;
}

string RHSUpdateMood(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
	currentMood.Decay();
	currentMood.MoveTowardEmotion(currentEmotion);

	string result = "Mood updated";

	PrintMessage("\n" + result);

	return result;
}

string RHSGenerateFeelingDimension(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
	vector<string> tokens;
	Tokenize(pArgument, &tokens);

	if(tokens.size() != 2) {
		string error = "+++Expected 2 arguments, got " + tokens.size();
		cerr << "\n" + error;
		return error;
	}
   
   return currentFeeling.GetDimension(tokens[1], currentEmotion, currentMood.af, appraisalStatus.GetStatus(tokens[1]));
}

string RHSGenerateMoodDimension(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
	vector<string> tokens;
	Tokenize(pArgument, &tokens);

	if(tokens.size() != 1) {
		string error = "+++Expected 1 argument, got " + tokens.size();
		cerr << "\n" + error;
		return error;
	}
   
   return currentMood.GetDimension(tokens[0]);
}

string RHSGenerateEmotionLabel(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
	string s = GenerateLabel(currentEmotion);
	PrintMessage("\nEmotion Label" + s);
	return s;
}

string RHSGenerateEmotionIntensity(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
	double intensity = currentEmotion.CalculateIntensity();
	string s = ToString(intensity);
	PrintMessage("\nEmotion Intensity" + s);
	return s;
}

string RHSGenerateEmotionValence(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
	double valence = currentEmotion.CalculateValence();
	string s = ToString(valence);
	PrintMessage("\nEmotion Valence" + s);
	return s;
}

string RHSGenerateMoodLabel(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
	string s = GenerateLabel(currentMood.af);
	PrintMessage("\nMood Label" + s);
	return s;
}

string RHSGenerateMoodIntensity(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
	double intensity = currentMood.af.CalculateIntensity();
	string s = ToString(intensity);
	PrintMessage("\nMood Intensity" + s);
	return s;
}

string RHSGenerateMoodValence(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
	double valence = currentMood.af.CalculateValence();
	string s = ToString(valence);
	PrintMessage("\nMood Valence" + s);
	return s;
}

string GenerateFrameLine(string col1, string col2, string col3, string col4) {
	ostringstream temp;
	temp << setw(34) << col1 << setw(18) << col2 << setw(18) << col3 << setw(18) << col4;
	return temp.str();
}

string GenerateFrameLine(string col1, double col2, double col3, double col4) {
	return GenerateFrameLine(col1, ToString(col2), ToString(col3), ToString(col4));
}

string RHSGenerateFrames1(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
	// this generates a table whose rows are the various appraisals and whose columns are the mood, emotion and feeling, respectively
	// the last two rows will display the label and intensity for each column

	AppraisalFrame maf = currentMood.af;
	AppraisalFrame eaf = currentEmotion;
	AppraisalFrame faf = currentFeeling.GenerateAppraisalFrame(eaf, maf, appraisalStatus);

	string table = "";
	table += GenerateFrameLine("", "Mood", "Emotion", "Feeling") + "\n";
	table += GenerateFrameLine("Suddenness [0,1]", maf.suddenness, eaf.suddenness, faf.suddenness) + "\n";
	table += GenerateFrameLine("Unpredictability [0,1]", maf.unpredictability, eaf.unpredictability, faf.unpredictability) + "\n";
	table += GenerateFrameLine("Intrinsic-pleasantness [-1,1]", maf.intrinsic_pleasantness, eaf.intrinsic_pleasantness, faf.intrinsic_pleasantness) + "\n";
	table += GenerateFrameLine("Goal-relevance [0,1]", maf.goal_relevance, eaf.goal_relevance, faf.goal_relevance) + "\n";
	table += GenerateFrameLine("Causal-agent (self) [0,1]", maf.causal_agent_self, eaf.causal_agent_self, faf.causal_agent_self) + "\n";
	table += GenerateFrameLine("Causal-agent (other) [0,1]", maf.causal_agent_other, eaf.causal_agent_other, faf.causal_agent_other) + "\n";
	table += GenerateFrameLine("Causal-agent (nature) [0,1]", maf.causal_agent_nature, eaf.causal_agent_nature, faf.causal_agent_nature);

	return table;
}

string RHSGenerateFrames2(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
	// this generates a table whose rows are the various appraisals and whose columns are the mood, emotion and feeling, respectively
	// the last two rows will display the label and intensity for each column

	AppraisalFrame maf = currentMood.af;
	AppraisalFrame eaf = currentEmotion;
	AppraisalFrame faf = currentFeeling.GenerateAppraisalFrame(eaf, maf, appraisalStatus);

	string table = "";
	table += GenerateFrameLine("Causal-motive (intentional) [0,1]", maf.causal_motive_intentional, eaf.causal_motive_intentional, faf.causal_motive_intentional) + "\n";
	table += GenerateFrameLine("Causal-motive (chance) [0,1]", maf.causal_motive_chance, eaf.causal_motive_chance, faf.causal_motive_chance) + "\n";
	table += GenerateFrameLine("Causal-motive (negligence) [0,1]", maf.causal_motive_negligence, eaf.causal_motive_negligence, faf.causal_motive_negligence) + "\n";
	table += GenerateFrameLine("Outcome-probability [0,1]", maf.outcome_probability, eaf.outcome_probability, faf.outcome_probability) + "\n";
	table += GenerateFrameLine("Discrepancy [0,1]", maf.discrepancy, eaf.discrepancy, faf.discrepancy) + "\n";
	table += GenerateFrameLine("Conduciveness [-1,1]", maf.conduciveness, eaf.conduciveness, faf.conduciveness) + "\n";
	table += GenerateFrameLine("Control [-1,1]", maf.control, eaf.control, faf.control) + "\n";
	table += GenerateFrameLine("Power [-1,1]", maf.power, eaf.power, faf.power) + "\n";
	table += GenerateFrameLine("Label", GenerateLabel(maf), GenerateLabel(eaf), GenerateLabel(faf)) + "\n";
	table += GenerateFrameLine("Intensity", maf.CalculateIntensity(), eaf.CalculateIntensity(), faf.CalculateIntensity()) + "\n";
   table += GenerateFrameLine("Valence", maf.CalculateValence(), eaf.CalculateValence(), faf.CalculateValence());

	return table;
}

string RHSGenerateIndiff(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
	double intensity = atof(pArgument);
	//double result = 3.0*pow(2, 10.0*intensity);
	double result = exp(2.0*10.0*intensity);

	return ToString(result);
}

string RHSGenerateRawData(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument) {
	
	vector<string> tokens;
	Tokenize(pArgument, &tokens);

	if(tokens.size() != 2) {
		string error = "+++Expected 2 arguments, got " + tokens.size();
		cerr << "\n" + error;
		return error;
	}
	
   AppraisalFrame af;
	if(tokens[0] == "emotion") {
		af = currentEmotion;
	} else if(tokens[0] == "mood") {
		af = currentMood.af;
	} else if(tokens[0] == "feeling") {
		af = currentFeeling.GenerateAppraisalFrame(currentEmotion, currentMood.af, appraisalStatus);
	} else {
		return "Expected one of 'emotion','mood','feeling', got " + tokens[0];
	}

	return af.GetAppraisalValue(tokens[1]);
}
*/