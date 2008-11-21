#ifndef APPRAISAL_FRAME_H
#define APPRAISAL_FRAME_H

#include <string>
#include <vector>
#include <math.h>
#include <boost/lexical_cast.hpp>

#include "symtab.h"

using std::string;
using std::vector;
using std::ostringstream;
using boost::lexical_cast;

const double fErrorValue = -246.0;
const double fInvalidValue = -123.0;
const string sInvalidValue = "no value";
const double fVeryLow = 0.0;
const double fLow = 0.25;
const double fMedium = 0.5;
const double fHigh = 0.75;
const double fVeryHigh = 1.0;

const double fFullVeryLow = -1.0;
const double fFullLow = -0.5;
const double fFullMedium = 0.0;
const double fFullHigh = 0.5;
const double fFullVeryHigh = 1.0;

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

	void Init() { Reset(0, fInvalidValue); }

	void Reset(Symbol* newid, double op) {
		id_sym = newid;
		suddenness = fInvalidValue;
		unpredictability = fInvalidValue;
		intrinsic_pleasantness = fInvalidValue;
		goal_relevance = fInvalidValue;
		causal_agent_self = fInvalidValue;
		causal_agent_other = fInvalidValue;
		causal_agent_nature = fInvalidValue;
		causal_motive_intentional = fInvalidValue;
		causal_motive_chance = fInvalidValue;
		causal_motive_negligence = fInvalidValue;
		outcome_probability = op;
		discrepancy = fInvalidValue;
		conduciveness = fInvalidValue;
		control = fInvalidValue;
		power = fInvalidValue;
	}

	string SetAppraisalValue(string appraisal, Symbol* value) {
		string result = "";
		double numericValue = fInvalidValue;
		string strValue = "";
		bool categorical = false;

		switch(value->fc.common_symbol_info.symbol_type)
		{
		case FLOAT_CONSTANT_SYMBOL_TYPE:
			numericValue = value->fc.value;
			break;
		case INT_CONSTANT_SYMBOL_TYPE:
			numericValue = value->ic.value;
			break;
		case SYM_CONSTANT_SYMBOL_TYPE:
			categorical = true;
			strValue = value->sc.name;
			break;
		default:
			// ignore
			break;
		}

		// BADBAD: all of the constants below should be Symbols in common symbols
		if(!categorical)
		{
			if(appraisal == "suddenness") { suddenness = numericValue; }
			else if(appraisal == "unpredictability") { unpredictability = numericValue; }
			else if(appraisal == "intrinsic-pleasantness") { intrinsic_pleasantness = numericValue; }
			else if(appraisal == "goal-relevance") { goal_relevance = numericValue; }
			else if(appraisal == "outcome-probability") { outcome_probability = numericValue; }
			else if(appraisal == "discrepancy") { discrepancy = numericValue; }
			else if(appraisal == "conduciveness") { conduciveness = numericValue; }
			else if(appraisal == "control") { control = numericValue; }
			else if(appraisal == "power") { power = numericValue; }
			else { result = "+++Invalid appraisal '" + appraisal + "'"; }
		} else {
			if(appraisal == "causal-agent") {
				if(strValue == "nature") { causal_agent_nature = 1.0; }
				else if(strValue == "other") { causal_agent_other = 1.0; }
				else if(strValue == "self") { causal_agent_self = 1.0; }
				else { result = "+++Invalid value '" + strValue + "' for appraisal 'causal-agent'"; }
			}
			else if(appraisal == "causal-motive") {
				if(strValue == "intentional") { causal_motive_intentional = 1.0; }
				else if(strValue == "chance") { causal_motive_chance = 1.0; }
				else if(strValue == "negligence") { causal_motive_negligence = 1.0; }
				else { result = "+++Invalid value '" + strValue + "' for appraisal 'causal-motive'"; }
			}
			else { result = "+++Invalid appraisal '" + appraisal + "'"; }
		}

		ostringstream oss;
		oss << "Registered " << id_sym->id.name_letter << id_sym->id.name_number << ": " << appraisal << " = ";
		if(categorical) oss << strValue;
		else oss << numericValue;
		result = oss.str();

		return result;
	}

	string GetAppraisalValue(string appraisal)
	{
		if(appraisal == "suddenness") { return lexical_cast<string>(suddenness); }
		else if(appraisal == "unpredictability") { return lexical_cast<string>(unpredictability); }
		else if(appraisal == "intrinsic-pleasantness") { return lexical_cast<string>(intrinsic_pleasantness); }
		else if(appraisal == "goal-relevance") { return lexical_cast<string>(goal_relevance); }
		else if(appraisal == "outcome-probability") { return lexical_cast<string>(outcome_probability); }
		else if(appraisal == "discrepancy") { return lexical_cast<string>(discrepancy); }
		else if(appraisal == "conduciveness") { return lexical_cast<string>(conduciveness); }
		else if(appraisal == "control") { return lexical_cast<string>(control); }
		else if(appraisal == "power") { return lexical_cast<string>(power); }
		else if(appraisal == "causal-agent-nature") { return lexical_cast<string>(causal_agent_nature); }
		else if(appraisal == "causal-agent-other") { return lexical_cast<string>(causal_agent_other); }
		else if(appraisal == "causal-agent-self") { return lexical_cast<string>(causal_agent_self); }
		else if(appraisal == "causal-motive-intentional") { return lexical_cast<string>(causal_motive_intentional); }
		else if(appraisal == "causal-motive-chance") { return lexical_cast<string>(causal_motive_chance); }
		else if(appraisal == "causal-motive-negligence") { return lexical_cast<string>(causal_motive_negligence); }
		else if(appraisal == "causal-agent") {
			return GetCategoricalValue("causal-agent");
		}
		else if(appraisal == "causal-motive") {
			return GetCategoricalValue("causal-motive");
		} else {
			return "+++Invalid appraisal '" + appraisal + "'";
		}
	}

	string ResetAppraisalValue(string appraisal) {
		string result = "";

		double resetVal = fInvalidValue;

		if(appraisal == "suddenness") { suddenness = resetVal; }
		else if(appraisal == "unpredictability") { unpredictability = resetVal; }
		else if(appraisal == "intrinsic-pleasantness") { intrinsic_pleasantness = resetVal; }
		else if(appraisal == "goal-relevance") { goal_relevance = resetVal; }
		else if(appraisal == "outcome-probability") { outcome_probability = fInvalidValue; }
		else if(appraisal == "discrepancy") { discrepancy = fInvalidValue; }
		else if(appraisal == "conduciveness") { conduciveness = resetVal; }
		else if(appraisal == "control") { control = resetVal; }
		else if(appraisal == "power") { power = resetVal; }
		else if(appraisal == "causal-agent") {
			causal_agent_nature = resetVal;
			causal_agent_other = resetVal;
			causal_agent_self = resetVal;
		}
		else if(appraisal == "causal-motive") {
			causal_motive_intentional = resetVal;
			causal_motive_chance = resetVal;
			causal_motive_negligence = resetVal;
		} else {
			result = "+++Invalid appraisal '" + appraisal + "'";
			return result;
		}

		result = "Reset: " + appraisal;

		return result;
	}

	string GetCategoricalValue(string appraisal) {
		if(appraisal == "causal-agent") {
			double canVal = causal_agent_nature;
			double casVal = causal_agent_self;
			double caoVal = causal_agent_other;
			if(canVal == fInvalidValue && casVal == fInvalidValue && caoVal == fInvalidValue) { return sInvalidValue; }
			// BUGBUG: for a tie, the more recent value should be used
			if(canVal >= casVal && canVal >= caoVal) {
				return "nature"; }
			else if(casVal >= canVal && casVal >= caoVal) {
				return "self"; }
			else { return "other"; }
		} else if(appraisal == "causal-motive") {
			double cmcVal = causal_motive_chance;
			double cmiVal = causal_motive_intentional;
			double cmnVal = causal_motive_negligence;
			if(cmcVal == fInvalidValue && cmiVal == fInvalidValue && cmnVal == fInvalidValue) { return sInvalidValue; }
			// BUGBUG: for a tie, the more recent value should be used
			if(cmcVal >= cmiVal && cmcVal >= cmnVal) {
				return "chance"; }
			else if(cmiVal >= cmcVal && cmiVal >= cmnVal) {
				return "intentional"; }
			else { return "negligence"; }	
		} else { return sInvalidValue; }
	}

	double CalculateIntensity() {
		
		double total = 0.0;
		long num = 0;

		
		if(suddenness != fInvalidValue) { total += suddenness; num += 1; }
		if(unpredictability != fInvalidValue) { total += unpredictability; num += 1; }
		if(intrinsic_pleasantness != fInvalidValue) { total += fabs(intrinsic_pleasantness); num += 1; } // normalize: on [-1,1] scale
		if(goal_relevance != fInvalidValue) { total += goal_relevance; num += 1; }
		if(conduciveness != fInvalidValue) { total += fabs(conduciveness); num += 1; } // normalize: on [-1,1] scale
		if(control != fInvalidValue) { total += fabs(control); num += 1; } // normalize: on [-1,1] scale
		if(power != fInvalidValue) { total += fabs(power); num += 1; } // normalize: on [-1,1] scale
		
		double average = 0.0;
		if(num != 0) { average = (total/(double)num); }

		double factor = 0.0;
		// BUGBUG: an alternative is to say if either is invalid, then neither influences intensity (since there's no context for the known value)
		if(outcome_probability == fInvalidValue && discrepancy == fInvalidValue) {
			factor = 1.0; // if both are invalid, factor will not influence the intensity
		} else if(outcome_probability == fInvalidValue && discrepancy != fInvalidValue) {
			factor = discrepancy; // if op is invalid, then it will not influence the intensity
		} else if(outcome_probability != fInvalidValue && discrepancy == fInvalidValue) {
			factor = outcome_probability; // if de is invalid, then it will not influence the intensity
		} else {
			factor = (1.0 - outcome_probability)*(1.0 - discrepancy) + outcome_probability*discrepancy;
		}

		double intensity = factor*average;

		return intensity;
	}

   double CalculateValence() {
      double total = 0.0;
		long num = 0;

      if(intrinsic_pleasantness != fInvalidValue) { total += intrinsic_pleasantness; num += 1; }
      if(conduciveness != fInvalidValue) { total += conduciveness; num += 1; }
      if(control != fInvalidValue) { total += control; num += 1; }
		if(power != fInvalidValue) { total += power; num += 1; }

      double valence = 0.0;
		if(num != 0) { valence = (total/(double)num); }

      return valence;
   }

	string ToString() {
		string result = "";
		result += "Suddenness = " + lexical_cast<string>(suddenness) + "\n";
		result += "Unpredictability = " + lexical_cast<string>(unpredictability) + "\n";
		result += "Intrinsic-pleasantness = " + lexical_cast<string>(intrinsic_pleasantness) + "\n";
		result += "Goal-relevance = " + lexical_cast<string>(goal_relevance) + "\n";
		result += "Causal-agent (self) = " + lexical_cast<string>(causal_agent_self) + "\n";
		result += "Causal-agent (other) = " + lexical_cast<string>(causal_agent_other) + "\n";
		result += "Causal-agent (nature) = " + lexical_cast<string>(causal_agent_nature) + "\n";
		result += "Causal-motive (intentional) = " + lexical_cast<string>(causal_motive_intentional) + "\n";
		result += "Causal-motive (chance) = " + lexical_cast<string>(causal_motive_chance) + "\n";
		result += "Causal-motive (negligence) = " + lexical_cast<string>(causal_motive_negligence) + "\n";
		result += "Outcome-probability = " + lexical_cast<string>(outcome_probability) + "\n";
		result += "Discrepancy = " + lexical_cast<string>(discrepancy) + "\n";
		result += "Conduciveness = " + lexical_cast<string>(conduciveness) + "\n";
		result += "Control = " + lexical_cast<string>(control) + "\n";
		result += "Power = " + lexical_cast<string>(power);

		return result;
	}
};

#endif // APPRAISAL_FRAME_H