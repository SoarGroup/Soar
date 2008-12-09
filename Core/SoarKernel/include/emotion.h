#ifndef EMOTION_H
#define EMOTION_H

typedef struct agent_struct agent;
typedef struct wme_struct wme;
typedef union symbol_union Symbol;

#include <string>
#include <vector>

using std::string;
using std::vector;

struct AppraisalStatus {
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

	AppraisalStatus();

   string SetStatus(string appraisal, bool status);
   bool GetStatus(string appraisal) const;
};

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
	AppraisalFrame af;

	double decay_rate;
	double move_rate;

	Mood();

	string SetParameters(vector<string>& params);
	double Decay(double val);
	void Decay();
	double MoveTowardEmotion(double val, double target);
	void MoveTowardEmotion(AppraisalFrame& emotion);
   void DisableAppraisal(string& appraisal);
   string GetDimension(string& dim);
};

struct Feeling {
	AppraisalFrame af;

	// helper functions
	double logbase(double a, double base);
	double MySign(double v);
	double LogComb(double v1, double v2, double base);

	double GetValue(double emotion_val, double mood_val, bool status);
	string GetStringValue(double emotion_val, double mood_val, bool status);
	string SetDimension(const string& dim, const AppraisalFrame& emotion, const AppraisalFrame& mood, bool status);
	double GetNumericDimension(const string& dim, const AppraisalFrame& emotion, const AppraisalFrame& mood, bool status);
	string GetCategoricalDimension(const string& dim, const AppraisalFrame& emotion, const AppraisalFrame& mood, bool status);
	string GetDimensionAsString(const string& dim, const AppraisalFrame& emotion, const AppraisalFrame& mood, bool status);
	AppraisalFrame GenerateAppraisalFrame(const AppraisalFrame& emotion, const AppraisalFrame& mood, const AppraisalStatus& as);
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

void register_appraisal(emotion_data* ed, wme* appraisal);
void get_appraisals(Symbol* goal);
void update_mood(Symbol* goal);
void generate_feeling_frame(agent* thisAgent, Symbol* goal);
void emotion_reset_data( agent *thisAgent );

void emotion_update(agent* thisAgent, Symbol* goal);

#endif // EMOTION_H