#ifndef EMOTION_H
#define EMOTION_H

#include <string>
#include <vector>

using std::string;
using std::vector;

class AppraisalStatus {
private:
	bool suddenness;
	bool unpredictability;
	bool intrinsic_pleasantness;
	bool goal_relevance;
	bool causal_agent_self;
	bool causal_agent_other;
	bool causal_agent_nature;
	bool causal_motive_intentional;
	bool causal_motive_chance;
	bool causal_motive_negligence;
	bool outcome_probability;
	bool discrepancy;
	bool conduciveness;
	bool control;
	bool power;

public:
	AppraisalStatus();

   string SetStatus(string appraisal, bool status);
   bool GetStatus(string appraisal) const;
};


//////////////////////////////////////////////////////////
// Emotion Types
//////////////////////////////////////////////////////////

typedef struct agent_struct agent;
typedef struct wme_struct wme;
typedef union symbol_union Symbol;

struct AppraisalFrame {
	Symbol* id_sym;
	double suddenness;
	double unpredictability;
	double intrinsic_pleasantness;
	double goal_relevance;
	double causal_agent_self;
	double causal_agent_other;
	double causal_agent_nature;
	double causal_motive_intentional;
	double causal_motive_chance;
	double causal_motive_negligence;
	double outcome_probability;
	double discrepancy;
	double conduciveness;
	double control;
	double power;

	AppraisalFrame();

	void Reset(Symbol* newid, double op);
	string SetAppraisalValue(string appraisal, Symbol* value);
	string GetAppraisalValue(string appraisal);
	string ResetAppraisalValue(string appraisal);
	string GetCategoricalValue(string appraisal);
	double CalculateIntensity();
	double CalculateValence();
	string ToString();
};

struct Bounds {
	double lower_bound;
	double upper_bound;
	bool open;
	void SetValue(double v) { lower_bound = v; upper_bound = v; open = false; }
	void SetRange(double lb, double ub) { lower_bound = lb; upper_bound = ub; open = false; }
};

struct ModalEmotion {
	string name;
	Bounds suddenness;
	Bounds unpredictability;
	Bounds intrinsic_pleasantness;
	Bounds goal_relevance;
	vector<string> causal_agent; // if empty then "open"
	vector<string> causal_motive;  // if empty then "open"
	Bounds outcome_probability;
	Bounds discrepancy;
	Bounds conduciveness;
	Bounds control;
	Bounds power;

	ModalEmotion(string name) : name(name) {
		suddenness.open = true;
		unpredictability.open = true;
		intrinsic_pleasantness.open = true;
		goal_relevance.open = true;
		outcome_probability.open = true;
		discrepancy.open = true;
		conduciveness.open = true;
		control.open = true;
		power.open = true;
	}
};

void InitModalEmotions();

struct Mood {
public:
	AppraisalFrame af; // TODO: to make private, need way to easily access all members (i.e., AppraisalFrame needs to provide key/value interface)
private:
	// this stuff could be static, but making it non-static allows for the possibility of different parameters for each state
	double decay_rate;
	double move_rate;

	double MoveTowardEmotion(double val, double target);
	double Decay(double val);
public:
	Mood();

	string SetParameters(vector<string>& params);
	void Decay();
	void MoveTowardEmotion(AppraisalFrame& emotion);
	string GetDimension(string& dim);  // for debugging, trace, etc.
};

class Feeling {
private:
	AppraisalFrame af;

	// helper functions
	static double logbase(double a, double base);
	static double MySign(double v);
	static double LogComb(double v1, double v2, double base);

	double GetValue(double emotion_val, double mood_val, bool status);
	string SetDimension(const string& dim, const AppraisalFrame& emotion, const AppraisalFrame& mood, bool status);
public:
	double GetNumericDimension(const string& dim, const AppraisalFrame& emotion, const AppraisalFrame& mood, bool status);
	string GetCategoricalDimension(const string& dim, const AppraisalFrame& emotion, const AppraisalFrame& mood, bool status);
	inline double CalculateIntensity() { return af.CalculateIntensity(); }
	inline double CalculateValence() { return af.CalculateValence(); }
	
	// these for debugging 
	AppraisalFrame GenerateAppraisalFrame(const AppraisalFrame& emotion, const AppraisalFrame& mood, const AppraisalStatus& as);
	// TODO: may not be necessary
	string GetStringValue(double emotion_val, double mood_val, bool status); 
	string GetDimensionAsString(const string& dim, const AppraisalFrame& emotion, const AppraisalFrame& mood, bool status);
};

struct emotion_data
{
	// emotion stuff
	AppraisalStatus appraisalStatus;
	AppraisalFrame currentEmotion;
	Mood currentMood;
	Feeling currentFeeling;
	wme* feeling_frame;
};

void cleanup_emotion_data(agent* thisAgent, emotion_data* ed);
void emotion_reset_data( agent *thisAgent );
void emotion_update(agent* thisAgent, Symbol* goal);

#endif // EMOTION_H