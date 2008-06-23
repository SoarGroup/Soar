#ifndef FEELING_H
#define FEELING_H

#include "AppraisalFrame.h"
#include "Mood.h"
#include "AppraisalStatus.h"

// helper functions
double logbase(double a, double base)
{
   return log(a) / log(base);
}

double MySign(double v) {
	if(v>=0) return 1.0;
	else return -1.0;
}

struct Feeling {
	AppraisalFrame af;

	double LogComb(double v1, double v2, double base) {
		double s1 = MySign(v1) * ( pow(base, 10.0*abs(v1)) - 1.0);
		double s2 = MySign(v2) * ( pow(base, 10.0*abs(v2)) - 1.0);
		double sumpart = s1 + s2;

		double res = 0.1 * MySign(sumpart) * logbase(abs(sumpart+MySign(sumpart)), base);

		return res;
	}

	double GetValue(double emotion_val, double mood_val, bool status) {
      if(!status) { return fInvalidValue; }
		if(emotion_val == fInvalidValue) { return mood_val; }

		double res;
		if( (emotion_val>=0 && mood_val>=0) || (emotion_val<=0 && mood_val<=0) ) {
			res = LogComb(emotion_val, mood_val, 2.71828183); // base is e
		} else {
			res = LogComb(emotion_val, mood_val, 1.1);
		}

		if(res > 1.0) return 1.0;
		else if(res < -1.0) return -1.0;
		else return res;
	}

	string GetStringValue(double emotion_val, double mood_val, bool status) {
      double val = GetValue(emotion_val, mood_val, status);
		return lexical_cast<string>(val);
	}

	string SetDimension(string dim, AppraisalFrame emotion, AppraisalFrame mood, bool status) {
      if(dim == "suddenness") { af.suddenness = GetValue(emotion.suddenness, mood.suddenness, status); }
		else if(dim == "unpredictability") { af.unpredictability = GetValue(emotion.unpredictability, mood.unpredictability, status); }
		else if(dim == "intrinsic-pleasantness") { af.intrinsic_pleasantness = GetValue(emotion.intrinsic_pleasantness, mood.intrinsic_pleasantness, status); }
		else if(dim == "goal-relevance") { af.goal_relevance = GetValue(emotion.goal_relevance, mood.goal_relevance, status); }
		else if(dim == "outcome-probability") { af.outcome_probability = GetValue(emotion.outcome_probability, mood.outcome_probability, status); }
		else if(dim == "discrepancy") { af.discrepancy = GetValue(emotion.discrepancy, mood.discrepancy, status); }
		else if(dim == "conduciveness") { af.conduciveness = GetValue(emotion.conduciveness, mood.conduciveness, status); }
		else if(dim == "control") { af.control = GetValue(emotion.control, mood.control, status); }
		else if(dim == "power") { af.power = GetValue(emotion.power, mood.power, status); }
		else if(dim == "causal-agent") {
			af.causal_agent_nature = GetValue(emotion.causal_agent_nature, mood.causal_agent_nature, status);
			af.causal_agent_self = GetValue(emotion.causal_agent_self, mood.causal_agent_self, status);
			af.causal_agent_other = GetValue(emotion.causal_agent_other, mood.causal_agent_other, status);
		}
		else if(dim == "causal-motive") {
			af.causal_motive_chance = GetValue(emotion.causal_motive_chance, mood.causal_motive_chance, status);
			af.causal_motive_intentional = GetValue(emotion.causal_motive_intentional, mood.causal_motive_intentional, status);
			af.causal_motive_negligence = GetValue(emotion.causal_motive_negligence, mood.causal_motive_negligence, status);		
		} else {
			return "+++Invalid dimension '" + dim + "'";
		}
		return "";
	}

	string GetDimension(string dim, AppraisalFrame emotion, AppraisalFrame mood, bool status) {
		SetDimension(dim, emotion, mood, status);

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

	AppraisalFrame GenerateAppraisalFrame(const AppraisalFrame& emotion, const AppraisalFrame& mood, const AppraisalStatus& as) {
		AppraisalFrame af;
      af.suddenness = GetValue(emotion.suddenness, mood.suddenness, as.GetStatus("suddenness"));
		af.unpredictability = GetValue(emotion.unpredictability, mood.unpredictability, as.GetStatus("unpredictability"));
		af.intrinsic_pleasantness = GetValue(emotion.intrinsic_pleasantness, mood.intrinsic_pleasantness, as.GetStatus("intrinsic-pleasantness"));
		af.goal_relevance = GetValue(emotion.goal_relevance, mood.goal_relevance, as.GetStatus("goal-relevance"));
		af.causal_agent_nature = GetValue(emotion.causal_agent_nature, mood.causal_agent_nature, as.GetStatus("causal-agent"));
		af.causal_agent_other = GetValue(emotion.causal_agent_other, mood.causal_agent_other, as.GetStatus("causal-agent"));
		af.causal_agent_self = GetValue(emotion.causal_agent_self, mood.causal_agent_self, as.GetStatus("causal-agent"));
		af.causal_motive_chance = GetValue(emotion.causal_motive_chance, mood.causal_motive_chance, as.GetStatus("causal-motive"));
		af.causal_motive_intentional = GetValue(emotion.causal_motive_intentional, mood.causal_motive_intentional, as.GetStatus("causal-motive"));
		af.causal_motive_negligence = GetValue(emotion.causal_motive_negligence, mood.causal_motive_negligence, as.GetStatus("causal-motive"));
		af.outcome_probability = GetValue(emotion.outcome_probability, mood.outcome_probability, as.GetStatus("outcome-probability"));
		af.discrepancy = GetValue(emotion.discrepancy, mood.discrepancy, as.GetStatus("discrepancy"));
		af.conduciveness = GetValue(emotion.conduciveness, mood.conduciveness, as.GetStatus("conduciveness"));
		af.control = GetValue(emotion.control, mood.control, as.GetStatus("control"));
		af.power = GetValue(emotion.power, mood.power, as.GetStatus("power"));
		
		return af;
	}
};

Feeling currentFeeling;

#endif // FEELING_H