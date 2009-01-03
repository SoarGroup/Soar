#ifndef EMOTION_H
#define EMOTION_H

#include "symtab.h"

#include <string>
#include <vector>
#include <map>
#include <boost/foreach.hpp>

using std::string;
using std::vector;
using std::map;

typedef struct agent_struct agent;
typedef struct wme_struct wme;
typedef union symbol_union Symbol;

class Mood;

class Appraisal {
protected:

	static double logbase(double a, double base)
	{
		return log(a) / log(base);
	}

	static double MySign(double v) {
		if(v>=0) return 1.0;
		else return -1.0;
	}

	static double LogComb(double v1, double v2, double base) {
		double s1 = MySign(v1) * ( pow(base, 10.0*abs(v1)) - 1.0);
		double s2 = MySign(v2) * ( pow(base, 10.0*abs(v2)) - 1.0);
		double sumpart = s1 + s2;

		double res = 0.1 * MySign(sumpart) * logbase(abs(sumpart+MySign(sumpart)), base);

		return res;
	}

	enum AppraisalType{ NUMERIC, CATEGORICAL, };

	agent* thisAgent;
	string name;
	bool enabled;
	AppraisalType type;
	bool valenced;

	friend Mood; // for the following functions

	virtual void Decay(double decay_rate)=0;
	virtual void MoveTowardAppraisal(Appraisal* a, double move_rate)=0;
	virtual void Zero()=0;

public:

	Appraisal(agent* thisAgent, string name, bool valenced, bool enabled, AppraisalType type) : thisAgent(thisAgent), name(name), valenced(valenced), enabled(enabled), type(type) {}
	virtual Symbol* GetValue()=0;
	virtual void SetValue(Symbol *val)=0;
	virtual void SetValue(Appraisal* emotion, Appraisal* mood)=0;
	virtual double GetValueAsDouble()=0;
	virtual string GetValueAsString()=0;

	AppraisalType GetType() { return type; }
	string GetName() { return name; }
	bool IsValenced() { return valenced; }
	bool IsEnabled() { return enabled; }

	virtual ~Appraisal() {};

};

class CategoricalAppraisal;

class NumericAppraisal : public Appraisal {
protected:
	Symbol* value;

	friend CategoricalAppraisal;
	void Decay(double decay_rate);
	void MoveTowardAppraisal(Appraisal* a, double move_rate);

	void Zero();

public:
	NumericAppraisal(agent* thisAgent, string name, bool valenced, bool enabled);

	Symbol* GetValue();
	void SetValue(Symbol* val);
	void SetValue(double val);
	void SetValue(Appraisal* emotion, Appraisal* mood);
	double GetValueAsDouble();
	string GetValueAsString();
	~NumericAppraisal();
};

class CategoricalAppraisal : public Appraisal {
protected:
	typedef map<Symbol*, NumericAppraisal*> CategoricalAppraisalMap;
	typedef CategoricalAppraisalMap::iterator CategoricalAppraisalMapItr;
	CategoricalAppraisalMap values;

	void Zero();
	void Decay(double decay_rate);
	void MoveTowardAppraisal(Appraisal* a, double move_rate);

public:
	CategoricalAppraisal(agent* thisAgent, string name, vector<string> valueNames, bool enabled);

	Symbol* GetValue();
	void SetValue(Symbol* val);
	void SetValue(Appraisal* emotion, Appraisal* mood);
	double GetValueAsDouble();
	string GetValueAsString();
	~CategoricalAppraisal();
};

class Feeling;

class AppraisalFrame {
protected:
	agent* thisAgent;
	Symbol* id_sym;
	
	typedef map<string, Appraisal*> AppraisalMap;  // BADBAD: should this be a map from Symbol* to Appraisal*?
	typedef AppraisalMap::iterator AppraisalMapItr;
	typedef AppraisalMap::const_iterator AppraisalMapConstItr;
	
	AppraisalMap appraisals;

	// These needed for access to appraisals
	friend Mood;
	friend Feeling;
	friend void register_appraisal(emotion_data* ed, wme* appraisal);
	friend void generate_feeling_frame(agent* thisAgent, Symbol * goal);

public:
	AppraisalFrame(agent* thisAgent);
	void Reset(Symbol* newid, Symbol* op);
	string SetAppraisalValue(string appraisal, Symbol* value);
	//string GetAppraisalValue(string appraisal);
	//string ResetAppraisalValue(string appraisal);
	//string GetCategoricalValue(string appraisal);
	double CalculateIntensity();
	double CalculateValence();
	//string ToString();

	~AppraisalFrame();
};

class Mood : public AppraisalFrame {
protected:
	// this stuff could be static, but making it non-static allows for the possibility of different parameters for each state
	double decay_rate;
	double move_rate;

public:
	Mood(agent* thisAgent);

	void SetDecayRate(double decay_rate) { this->decay_rate = decay_rate; }
	void SetMoveRate(double move_rate) { this->move_rate = move_rate; }
	void Decay();
	void MoveTowardEmotion(AppraisalFrame& emotion);
	//string GetDimension(string& dim);  // for debugging, trace, etc.
};

class Feeling : public AppraisalFrame {
public:
	void Update(const AppraisalFrame& emotion, const AppraisalFrame& mood);
	//double GetNumericDimension(const string& dim, const AppraisalFrame& emotion, const AppraisalFrame& mood, bool status);
	//string GetCategoricalDimension(const string& dim, const AppraisalFrame& emotion, const AppraisalFrame& mood, bool status);
	
	// these for debugging 
	//AppraisalFrame GenerateAppraisalFrame(const AppraisalFrame& emotion, const AppraisalFrame& mood, const AppraisalStatus& as);
	// TODO: may not be necessary
	//string GetStringValue(double emotion_val, double mood_val, bool status); 
	//string GetDimensionAsString(const string& dim, const AppraisalFrame& emotion, const AppraisalFrame& mood, bool status);

	Feeling(agent* thisAgent) : AppraisalFrame(thisAgent) {}
};

struct emotion_data
{
	AppraisalFrame currentEmotion;
	Mood currentMood;
	Feeling currentFeeling;
	Symbol *feeling_frame_header;
	wme* feeling_frame;

	emotion_data(agent* thisAgent) : currentEmotion(thisAgent), currentFeeling(thisAgent), currentMood(thisAgent), feeling_frame_header(0), feeling_frame(0) {}
};

void cleanup_emotion_data(agent* thisAgent, emotion_data* ed);
void emotion_reset_data( agent *thisAgent );
void emotion_update(agent* thisAgent, Symbol* goal);


//struct Bounds {
//	double lower_bound;
//	double upper_bound;
//	bool open;
//	void SetValue(double v) { lower_bound = v; upper_bound = v; open = false; }
//	void SetRange(double lb, double ub) { lower_bound = lb; upper_bound = ub; open = false; }
//};
//
//struct ModalEmotion {
//	string name;
//	Bounds suddenness;
//	Bounds unpredictability;
//	Bounds intrinsic_pleasantness;
//	Bounds goal_relevance;
//	vector<string> causal_agent; // if empty then "open"
//	vector<string> causal_motive;  // if empty then "open"
//	Bounds outcome_probability;
//	Bounds discrepancy;
//	Bounds conduciveness;
//	Bounds control;
//	Bounds power;
//
//	ModalEmotion(string name) : name(name) {
//		suddenness.open = true;
//		unpredictability.open = true;
//		intrinsic_pleasantness.open = true;
//		goal_relevance.open = true;
//		outcome_probability.open = true;
//		discrepancy.open = true;
//		conduciveness.open = true;
//		control.open = true;
//		power.open = true;
//	}
//};
//
//void InitModalEmotions();

#endif // EMOTION_H