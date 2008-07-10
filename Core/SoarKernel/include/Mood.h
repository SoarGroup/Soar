#ifndef MOOD_H
#define MOOD_H

#include <string>
#include <math.h>
#include "AppraisalFrame.h"

struct Mood {
	AppraisalFrame af;

	double decay_rate;
	double move_rate;

	void Init() {
		// mood is disabled by default
		decay_rate = 0.0;
		move_rate = 0.0;

		af.suddenness = 0;
		af.unpredictability = 0;
		af.intrinsic_pleasantness = 0;
		af.goal_relevance = 0;
		af.causal_agent_self = 0;
		af.causal_agent_other = 0;
		af.causal_agent_nature = 0;
		af.causal_motive_intentional = 0;
		af.causal_motive_chance = 0;
		af.causal_motive_negligence = 0;
		af.outcome_probability = 0;
		af.discrepancy = 0;
		af.conduciveness = 0;
		af.control = 0;
		af.power = 0;
	}

	string SetParameters(vector<string> params) {

		string result = "";

		for(unsigned int i=0; i<params.size(); i+=2) {
			if((i+1) == params.size()) { return "+++Wrong number of args; Expected an even number"; }

			string name = params[i];
			string value = params[i+1];

			if(name == "decay-rate") {
				decay_rate = atof(value.c_str());
			} else if(name == "move-rate") {
				move_rate = atof(value.c_str());
			} else { return "+++Unrecognized parameter " + name; }
		}
		return "decay-rate = " + lexical_cast<string>(decay_rate) + "\nmove-rate = " + lexical_cast<string>(move_rate);
	}

	double Decay(double val) {
      if(val == fInvalidValue) return fInvalidValue;
		//decay towards zero, but don't pass zero
		else return val * (1.0 - decay_rate);
	}

	void Decay() {
		af.suddenness = Decay(af.suddenness);
		af.unpredictability = Decay(af.unpredictability);
		af.intrinsic_pleasantness = Decay(af.intrinsic_pleasantness);
		af.goal_relevance = Decay(af.goal_relevance);
		af.causal_agent_self = Decay(af.causal_agent_self);
		af.causal_agent_other = Decay(af.causal_agent_other);
		af.causal_agent_nature = Decay(af.causal_agent_nature);
		af.causal_motive_intentional = Decay(af.causal_motive_intentional);
		af.causal_motive_chance = Decay(af.causal_motive_chance);
		af.causal_motive_negligence = Decay(af.causal_motive_negligence);
		af.outcome_probability = Decay(af.outcome_probability);
		af.discrepancy = Decay(af.discrepancy);
		af.conduciveness = Decay(af.conduciveness);
		af.control = Decay(af.control);
		af.power = Decay(af.power);
	}

	double MoveTowardEmotion(double val, double target) {
      if(val == fInvalidValue) { return fInvalidValue; }
      if(target == fInvalidValue) { return val; }

		double distance = (target - val);
		double delta = distance * move_rate;
		return val + delta;
	}

	void MoveTowardEmotion(AppraisalFrame emotion) {
		af.suddenness = MoveTowardEmotion(af.suddenness, emotion.suddenness);
		af.unpredictability = MoveTowardEmotion(af.unpredictability, emotion.unpredictability);
		af.intrinsic_pleasantness = MoveTowardEmotion(af.intrinsic_pleasantness, emotion.intrinsic_pleasantness);
		af.goal_relevance = MoveTowardEmotion(af.goal_relevance, emotion.goal_relevance);
		af.causal_agent_self = MoveTowardEmotion(af.causal_agent_self, emotion.causal_agent_self);
		af.causal_agent_other = MoveTowardEmotion(af.causal_agent_other, emotion.causal_agent_other);
		af.causal_agent_nature = MoveTowardEmotion(af.causal_agent_nature, emotion.causal_agent_nature);
		af.causal_motive_intentional = MoveTowardEmotion(af.causal_motive_intentional, emotion.causal_motive_intentional);
		af.causal_motive_chance = MoveTowardEmotion(af.causal_motive_chance, emotion.causal_motive_chance);
		af.causal_motive_negligence = MoveTowardEmotion(af.causal_motive_negligence, emotion.causal_motive_negligence);
		af.outcome_probability = MoveTowardEmotion(af.outcome_probability, emotion.outcome_probability);
		af.discrepancy = MoveTowardEmotion(af.discrepancy, emotion.discrepancy);
		af.conduciveness = MoveTowardEmotion(af.conduciveness, emotion.conduciveness);
		af.control = MoveTowardEmotion(af.control, emotion.control);
		af.power = MoveTowardEmotion(af.power, emotion.power);
	}

   void DisableAppraisal(string appraisal) {
		if(appraisal == "suddenness") { af.suddenness = fInvalidValue; }
		else if(appraisal == "unpredictability") { af.unpredictability = fInvalidValue; }
		else if(appraisal == "intrinsic-pleasantness") { af.intrinsic_pleasantness = fInvalidValue; }
		else if(appraisal == "goal-relevance") { af.goal_relevance = fInvalidValue; }
		else if(appraisal == "outcome-probability") { af.outcome_probability = fInvalidValue; }
		else if(appraisal == "discrepancy") { af.discrepancy = fInvalidValue; }
		else if(appraisal == "conduciveness") { af.conduciveness = fInvalidValue; }
		else if(appraisal == "control") { af.control = fInvalidValue; }
		else if(appraisal == "power") { af.power = fInvalidValue; }
		else if(appraisal == "causal-agent") {
			af.causal_agent_nature = fInvalidValue;
			af.causal_agent_other = fInvalidValue;
			af.causal_agent_self = fInvalidValue;
		}
		else if(appraisal == "causal-motive") {
			af.causal_motive_intentional = fInvalidValue;
			af.causal_motive_chance = fInvalidValue;
			af.causal_motive_negligence = fInvalidValue;
		}
   }

   string GetDimension(string dim) {
		if(dim == "suddenness") { return lexical_cast<string>(af.suddenness); }
		else if(dim == "unpredictability") { return lexical_cast<string>(af.unpredictability); }
		else if(dim == "intrinsic-pleasantness") { return lexical_cast<string>(af.intrinsic_pleasantness); }
		else if(dim == "goal-relevance") { return lexical_cast<string>(af.goal_relevance); }
		else if(dim == "outcome-probability") { return lexical_cast<string>(af.outcome_probability); }
		else if(dim == "discrepancy") { return lexical_cast<string>(af.discrepancy); }
		else if(dim == "conduciveness") { return lexical_cast<string>(af.conduciveness); }
		else if(dim == "control") { return lexical_cast<string>(af.control); }
		else if(dim == "power") { return lexical_cast<string>(af.power); }
		else if(dim == "causal-agent") {
			double canVal = af.causal_agent_nature;
			double casVal = af.causal_agent_self;
			double caoVal = af.causal_agent_other;
			// BUGBUG: for a tie, the more recent value should be used
			if(canVal >= casVal && canVal >= caoVal) {
				return "nature"; }
			else if(casVal >= canVal && casVal >= caoVal) {
				return "self"; }
			else { return "other"; }
		}
		else if(dim == "causal-motive") {
			double cmcVal = af.causal_motive_chance;
			double cmiVal = af.causal_motive_intentional;
			double cmnVal = af.causal_motive_negligence;
			// BUGBUG: for a tie, the more recent value should be used
			if(cmcVal >= cmiVal && cmcVal >= cmnVal) {
				return "chance"; }
			else if(cmiVal >= cmcVal && cmiVal >= cmnVal) {
				return "intentional"; }
			else { return "negligence"; }			
		} else {
			return "+++Invalid dimension '" + dim + "'";
		}
	}
};

#endif // MOOD_H