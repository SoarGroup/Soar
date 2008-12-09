#include <portability.h>

/*
* TODO:
* create commands
*   enable/disable various appraisals
*   set mood parameters
*   turn entire system on/off
* print current emotion/mood/feeling table
* refactor headers so CLI and KernelSML aren't dependent on boost
* fix BADBADs
* use strings for invalid/none/error values in wm
* use Symbols internally instead of raw types?  May allow appraisals to be treated more uniformly
* consider replacing if/elseif blocks with map lookup? (probably requires Symbols since types aren't uniform otherwise)
* can drop lexical_cast and other functions?
* prefix all functions with "emotion"?
* emotion_reset should clean up stuff currently handled in do_input_phase, and be called from there
* consider generating feeling frame all at once, instead of one dimension at a time. Also, have feeling generation directly access emotion/mood from agent, instead of being passed in.
*/

// TODO:
// -- change appraisal registration to detect when appraisal has already been registered for that frame (and print a warning)
// -- Figure out how to get highest ela/joy when one step away from goal (must involve changing appraisals)
// -- Should dimensions with no values report a default value (e.g. 0) or should they report no value (e.g. fInvalidValue)?
// ---- currently reporting no value
// -- Should some dimensions (e.g. discrepancy & outcome_probability) not boost and decay? (e.g. something shouldn't seem more likely just because something else was likely recently)

#include "emotion.h"
#include "agent.h"
#include "symtab.h"
#include "wmem.h"
#include "io_soar.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <algorithm>
#include <map>

using boost::lexical_cast;
using std::string;
using std::ostringstream;
using std::vector;
using std::pair;
using std::multimap;

/////////////////////////////
// AppraisalStatus stuff
/////////////////////////////

AppraisalStatus::AppraisalStatus() {
	suddenness = true;
	unpredictability = true;
	intrinsic_pleasantness = true;
	goal_relevance = true;
	causal_agent_self = true;
	causal_agent_other = true;
	causal_agent_nature = true;
	causal_motive_intentional = true;
	causal_motive_chance = true;
	causal_motive_negligence = true;
	outcome_probability = true;
	discrepancy = true;
	conduciveness = true;
	control = true;
	power = true;
}

string AppraisalStatus::SetStatus(string appraisal, bool status) {
	string result = "";

	if(appraisal == "suddenness") { suddenness = status; }
	else if(appraisal == "unpredictability") { unpredictability = status; }
	else if(appraisal == "intrinsic-pleasantness") { intrinsic_pleasantness = status; }
	else if(appraisal == "goal-relevance") { goal_relevance = status; }
	else if(appraisal == "outcome-probability") { outcome_probability = status; }
	else if(appraisal == "discrepancy") { discrepancy = status; }
	else if(appraisal == "conduciveness") { conduciveness = status; }
	else if(appraisal == "control") { control = status; }
	else if(appraisal == "power") { power = status; }
	else if(appraisal == "causal-agent") {
		causal_agent_nature = status;
		causal_agent_other = status;
		causal_agent_self = status;
	}
	else if(appraisal == "causal-motive") {
		causal_motive_intentional = status;
		causal_motive_chance = status;
		causal_motive_negligence = status;
	} else {
		result = "+++Invalid appraisal '" + appraisal + "'";
		return result;
	}


	result = "Set " + appraisal + " status = " + lexical_cast<string>(status);

	return result;
}

bool AppraisalStatus::GetStatus(string appraisal) const {
	bool status;
	if(appraisal == "suddenness") { status = suddenness; }
	else if(appraisal == "unpredictability") { status = unpredictability; }
	else if(appraisal == "intrinsic-pleasantness") { status = intrinsic_pleasantness; }
	else if(appraisal == "goal-relevance") { status = goal_relevance; }
	else if(appraisal == "outcome-probability") { status = outcome_probability; }
	else if(appraisal == "discrepancy") { status = discrepancy; }
	else if(appraisal == "conduciveness") { status = conduciveness; }
	else if(appraisal == "control") { status = control; }
	else if(appraisal == "power") { status = power; }
	else if(appraisal == "causal-agent") { status = causal_agent_nature; } // all the same, so pick one
	else if(appraisal == "causal-motive") { status = causal_motive_intentional; } // all the same, so pick one
	else {
		status = false;
	}

	return status;
}

/////////////////////////////
// AppraisalFrame stuff
/////////////////////////////

static const double fErrorValue = -246.0;
static const double fInvalidValue = -123.0;
static const string sInvalidValue = "no value";
static const double fVeryLow = 0.0;
static const double fLow = 0.25;
static const double fMedium = 0.5;
static const double fHigh = 0.75;
static const double fVeryHigh = 1.0;

static const double fFullVeryLow = -1.0;
static const double fFullLow = -0.5;
static const double fFullMedium = 0.0;
static const double fFullHigh = 0.5;
static const double fFullVeryHigh = 1.0;


AppraisalFrame::AppraisalFrame() { Reset(0, fInvalidValue); }

void AppraisalFrame::Reset(Symbol* newid, double op) {
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

string AppraisalFrame::SetAppraisalValue(string appraisal, Symbol* value) {
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

string AppraisalFrame::GetAppraisalValue(string appraisal)
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

string AppraisalFrame::ResetAppraisalValue(string appraisal) {
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

string AppraisalFrame::GetCategoricalValue(string appraisal) {
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

double AppraisalFrame::CalculateIntensity() {

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

double AppraisalFrame::CalculateValence() {
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

string AppraisalFrame::ToString() {
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


/////////////////////////////
// ModalEmotion stuff
/////////////////////////////

ModalEmotion enj_hap("enj-hap");
ModalEmotion ela_joy("ela-joy");
ModalEmotion disp_disg("disp-disg");
ModalEmotion con_sco("con-sco");
ModalEmotion sad_dej("sad-dej");
ModalEmotion despair("despair");
ModalEmotion anx_wor("anx-wor");
ModalEmotion fear("fear");
ModalEmotion irr_coa("irr-coa");
ModalEmotion rag_hoa("rag-hoa");
ModalEmotion bor_ind("bor-ind");
ModalEmotion shame("shame");
ModalEmotion guilt("guilt");
ModalEmotion pride("pride");

//BADBAD: should the below functions be static functions of ModalEmotion?

double CalculateNumericDistance(double value, Bounds b, double range = 1.0) {
	if(value == fInvalidValue && !b.open) { return 1.0; }  // value matters but isn't supplied, so dist is 1.0
	else if(b.open) { return 0.0; }  // value doesn't matter, so dist is 0.0
	else if(value >= b.lower_bound && value <= b.upper_bound) {
		return 0.0;
	} else {
		return min(fabs(value-b.lower_bound) / range, fabs(value-b.upper_bound) / range); // normalize to 1.0 range
	}
}

double CalculateCategoricalDistance(string value, vector<string> b) {
	if(b.size() == 0 || find(b.begin(), b.end(), value) != b.end()) {
		return 0.0;
	} else {
		return 1.0;
	}
}

double CalculateDistance(AppraisalFrame af, ModalEmotion me) {
	double dist = 0;

	dist += CalculateNumericDistance(af.suddenness, me.suddenness);
	dist += CalculateNumericDistance(af.unpredictability, me.unpredictability);
	dist += CalculateNumericDistance(af.intrinsic_pleasantness, me.intrinsic_pleasantness, 2.0);
	dist += CalculateNumericDistance(af.goal_relevance, me.goal_relevance);

	dist += CalculateCategoricalDistance(af.GetCategoricalValue("causal-agent"), me.causal_agent);
	dist += CalculateCategoricalDistance(af.GetCategoricalValue("causal-motive"), me.causal_motive);

	dist += CalculateNumericDistance(af.outcome_probability, me.outcome_probability);
	dist += CalculateNumericDistance(af.discrepancy, me.discrepancy);
	dist += CalculateNumericDistance(af.conduciveness, me.conduciveness, 2.0);
	dist += CalculateNumericDistance(af.control, me.control, 2.0);
	dist += CalculateNumericDistance(af.power, me.power, 2.0);

	//cout << "\nDist to " << me.name << ": " << dist;
	//PrintMessage("\nDist to " + me.name + ": " + lexical_cast<string>(dist));

	return dist;

}

pair<string, double> Min(pair<string, double> one, pair<string, double> two) {
	if(one.second < two.second) {
		return one;
	} else if(one.second > two.second) {
		return two;
	} else {
		//cout << "\nTie between '" << one.first << "' and '" << two.first << "' at distance " << one.second;
		//PrintMessage("\nTie between '" + one.first + "' and '" + two.first + "' at distance " + lexical_cast<string>(one.second));
		if(rand() < RAND_MAX/2)
			return one;
		else
			return two;
	}
}

string GenerateLabel(AppraisalFrame& af) {

	typedef pair<double, ModalEmotion*> Dist_ModalEmotion;

	typedef multimap<double, ModalEmotion*> Dist_Map;

	Dist_Map distances;

	distances.insert(Dist_ModalEmotion(CalculateDistance(af, enj_hap), &enj_hap));
	distances.insert(Dist_ModalEmotion(CalculateDistance(af, ela_joy), &ela_joy));
	distances.insert(Dist_ModalEmotion(CalculateDistance(af, disp_disg), &disp_disg));
	distances.insert(Dist_ModalEmotion(CalculateDistance(af, con_sco), &con_sco));
	distances.insert(Dist_ModalEmotion(CalculateDistance(af, sad_dej), &sad_dej));
	distances.insert(Dist_ModalEmotion(CalculateDistance(af, despair), &despair));
	distances.insert(Dist_ModalEmotion(CalculateDistance(af, anx_wor), &anx_wor));
	distances.insert(Dist_ModalEmotion(CalculateDistance(af, fear), &fear));
	distances.insert(Dist_ModalEmotion(CalculateDistance(af, irr_coa), &irr_coa));
	distances.insert(Dist_ModalEmotion(CalculateDistance(af, rag_hoa), &rag_hoa));
	distances.insert(Dist_ModalEmotion(CalculateDistance(af, bor_ind), &bor_ind));
	distances.insert(Dist_ModalEmotion(CalculateDistance(af, shame), &shame));
	distances.insert(Dist_ModalEmotion(CalculateDistance(af, guilt), &guilt));
	distances.insert(Dist_ModalEmotion(CalculateDistance(af, pride), &pride));

	Dist_Map::iterator itr = distances.begin();
	double shortest_dist = 1000.0;
	bool first = true;
	string label = "";
	for(; itr != distances.end(); itr++) {
		// only accept labels that don't have an opposite conduciveness
		if(af.conduciveness == fInvalidValue || itr->second->conduciveness.open ||
			(af.conduciveness >= 0 && itr->second->conduciveness.upper_bound >= 0) ||
			(af.conduciveness <= 0 && itr->second->conduciveness.lower_bound <= 0)) {
				if(first) {  // record the first dist, which should be the lowest since the dists are sorted
					shortest_dist = itr->first;
					label = itr->second->name;
					first = false;
				}
				else if(itr->first == shortest_dist) {  // append the names of any labels of equal dist
					label += "/" + itr->second->name;
				}
				else if(itr->first > shortest_dist) break; // dists are sorted, so if we get to a higher dist, we are done
		}
	}

	//PrintMessage("\nClosest label is '" + label + "' at distance " + lexical_cast<string>(shortest_dist));

	return label;
}

void InitModalEmotions() {

	enj_hap.suddenness.SetValue(fLow);
	enj_hap.unpredictability.SetValue(fMedium);
	enj_hap.intrinsic_pleasantness.SetValue(fFullHigh);
	enj_hap.goal_relevance.SetValue(fMedium);
	enj_hap.causal_motive.push_back("intentional");
	enj_hap.outcome_probability.SetValue(fVeryHigh);
	enj_hap.discrepancy.SetValue(fLow);
	enj_hap.conduciveness.SetValue(fFullHigh);

	ela_joy.suddenness.SetRange(fMedium, fHigh);
	ela_joy.unpredictability.SetValue(fHigh);
	ela_joy.goal_relevance.SetValue(fHigh);
	ela_joy.causal_motive.push_back("chance");
	ela_joy.causal_motive.push_back("intentional");
	ela_joy.outcome_probability.SetValue(fVeryHigh);
	ela_joy.conduciveness.SetValue(fFullVeryHigh);

	disp_disg.unpredictability.SetValue(fHigh);
	disp_disg.intrinsic_pleasantness.SetValue(fFullVeryLow);
	disp_disg.goal_relevance.SetValue(fLow);
	disp_disg.outcome_probability.SetValue(fVeryHigh);

	con_sco.goal_relevance.SetValue(fLow);
	con_sco.causal_agent.push_back("other");
	con_sco.causal_motive.push_back("intentional");
	con_sco.outcome_probability.SetValue(fHigh);
	con_sco.control.SetValue(fFullHigh);
	con_sco.power.SetValue(fFullLow);

	sad_dej.suddenness.SetValue(fLow);
	sad_dej.goal_relevance.SetValue(fHigh);
	sad_dej.causal_motive.push_back("chance");
	sad_dej.causal_motive.push_back("negligence");
	sad_dej.outcome_probability.SetValue(fVeryHigh);
	sad_dej.conduciveness.SetValue(fFullLow);
	sad_dej.control.SetValue(fFullVeryLow);
	sad_dej.power.SetValue(fFullVeryLow);

	despair.suddenness.SetValue(fHigh);
	despair.unpredictability.SetValue(fHigh);
	despair.goal_relevance.SetValue(fHigh);
	despair.causal_agent.push_back("other");
	despair.causal_agent.push_back("nature");
	despair.causal_motive.push_back("chance");
	despair.causal_motive.push_back("negligence");
	despair.outcome_probability.SetValue(fVeryHigh);
	despair.discrepancy.SetValue(fHigh);
	despair.conduciveness.SetValue(fFullLow);
	despair.control.SetValue(fFullVeryLow);
	despair.power.SetValue(fFullVeryLow);

	anx_wor.suddenness.SetValue(fLow);
	anx_wor.goal_relevance.SetValue(fMedium);
	anx_wor.causal_agent.push_back("other");
	anx_wor.causal_agent.push_back("nature");
	anx_wor.outcome_probability.SetValue(fMedium);
	anx_wor.conduciveness.SetValue(fFullLow);
	anx_wor.power.SetValue(fFullLow);

	fear.suddenness.SetValue(fHigh);
	fear.unpredictability.SetValue(fHigh);
	fear.intrinsic_pleasantness.SetValue(fFullLow);
	fear.goal_relevance.SetValue(fHigh);
	fear.causal_agent.push_back("other");
	fear.causal_agent.push_back("nature");
	fear.outcome_probability.SetValue(fHigh);
	fear.discrepancy.SetValue(fHigh);
	fear.conduciveness.SetValue(fFullLow);
	fear.power.SetValue(fFullVeryLow);

	irr_coa.suddenness.SetValue(fLow);
	irr_coa.unpredictability.SetValue(fMedium);
	irr_coa.goal_relevance.SetValue(fMedium);
	irr_coa.causal_motive.push_back("intentional");
	irr_coa.causal_motive.push_back("negligence");
	irr_coa.outcome_probability.SetValue(fVeryHigh);
	irr_coa.conduciveness.SetValue(fFullLow);
	irr_coa.control.SetValue(fFullHigh);
	irr_coa.power.SetValue(fFullMedium);

	rag_hoa.suddenness.SetValue(fLow);
	rag_hoa.unpredictability.SetValue(fMedium);
	rag_hoa.goal_relevance.SetValue(fMedium);
	rag_hoa.causal_motive.push_back("intentional");
	rag_hoa.outcome_probability.SetValue(fVeryHigh);
	rag_hoa.conduciveness.SetValue(fFullLow);
	rag_hoa.control.SetValue(fFullHigh);
	rag_hoa.power.SetValue(fFullHigh);

	bor_ind.suddenness.SetValue(fVeryLow);
	bor_ind.unpredictability.SetValue(fVeryLow);
	bor_ind.goal_relevance.SetValue(fLow);
	bor_ind.outcome_probability.SetValue(fVeryHigh);
	bor_ind.discrepancy.SetValue(fLow);
	bor_ind.control.SetValue(fFullMedium);
	bor_ind.power.SetValue(fFullMedium);

	shame.suddenness.SetValue(fLow);
	shame.goal_relevance.SetValue(fHigh);
	shame.causal_agent.push_back("self");
	shame.causal_motive.push_back("intentional");
	shame.causal_motive.push_back("negligence");
	shame.outcome_probability.SetValue(fVeryHigh);

	guilt.goal_relevance.SetValue(fHigh);
	guilt.causal_agent.push_back("self");
	guilt.causal_motive.push_back("intentional");
	guilt.outcome_probability.SetValue(fVeryHigh);
	guilt.conduciveness.SetValue(fFullHigh);

	pride.goal_relevance.SetValue(fHigh);
	pride.causal_agent.push_back("self");
	pride.causal_motive.push_back("intentional");
	pride.outcome_probability.SetValue(fVeryHigh);
	pride.conduciveness.SetValue(fFullHigh);
	pride.conduciveness.SetValue(fFullHigh);
}

/////////////////////////////
// Mood stuff
/////////////////////////////

Mood::Mood() {
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

string Mood::SetParameters(vector<string>& params) {

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

double Mood::Decay(double val) {
	if(val == fInvalidValue) return fInvalidValue;
	//decay towards zero, but don't pass zero
	else return val * (1.0 - decay_rate);
}

void Mood::Decay() {
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

double Mood::MoveTowardEmotion(double val, double target) {
	if(val == fInvalidValue) { return fInvalidValue; }
	if(target == fInvalidValue) { return val; }

	double distance = (target - val);
	double delta = distance * move_rate;
	return val + delta;
}

void Mood::MoveTowardEmotion(AppraisalFrame& emotion) {
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

void Mood::DisableAppraisal(string& appraisal) {
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

string Mood::GetDimension(string& dim) {
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

/////////////////////////////
// Feeling stuff
/////////////////////////////


double Feeling::logbase(double a, double base)
{
	return log(a) / log(base);
}

double Feeling::MySign(double v) {
	if(v>=0) return 1.0;
	else return -1.0;
}

double Feeling::LogComb(double v1, double v2, double base) {
	double s1 = MySign(v1) * ( pow(base, 10.0*abs(v1)) - 1.0);
	double s2 = MySign(v2) * ( pow(base, 10.0*abs(v2)) - 1.0);
	double sumpart = s1 + s2;

	double res = 0.1 * MySign(sumpart) * logbase(abs(sumpart+MySign(sumpart)), base);

	return res;
}

double Feeling::GetValue(double emotion_val, double mood_val, bool status) {
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

string Feeling::GetStringValue(double emotion_val, double mood_val, bool status) {
	double val = GetValue(emotion_val, mood_val, status);
	return lexical_cast<string>(val);
}

string Feeling::SetDimension(const string& dim, const AppraisalFrame& emotion, const AppraisalFrame& mood, bool status) {

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

double Feeling::GetNumericDimension(const string& dim, const AppraisalFrame& emotion, const AppraisalFrame& mood, bool status) {
	SetDimension(dim, emotion, mood, status);

	if(dim == "suddenness") { return af.suddenness; }
	else if(dim == "unpredictability") { return af.unpredictability; }
	else if(dim == "intrinsic-pleasantness") { return af.intrinsic_pleasantness; }
	else if(dim == "goal-relevance") { return af.goal_relevance; }
	else if(dim == "outcome-probability") { return af.outcome_probability; }
	else if(dim == "discrepancy") { return af.discrepancy; }
	else if(dim == "conduciveness") { return af.conduciveness; }
	else if(dim == "control") { return af.control; }
	else if(dim == "power") { return af.power; }		
	else { return fErrorValue;
	}
}

string Feeling::GetCategoricalDimension(const string& dim, const AppraisalFrame& emotion, const AppraisalFrame& mood, bool status) {
	SetDimension(dim, emotion, mood, status);

	if(dim == "causal-agent") {
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
		return "+++Invalid categorical dimension '" + dim + "'";
	}
}

string Feeling::GetDimensionAsString(const string& dim, const AppraisalFrame& emotion, const AppraisalFrame& mood, bool status) {
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

AppraisalFrame Feeling::GenerateAppraisalFrame(const AppraisalFrame& emotion, const AppraisalFrame& mood, const AppraisalStatus& as) {
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


/////////////////////////////
// primary emotion functions
/////////////////////////////

void emotion_clear_feeling_frame(agent* thisAgent, emotion_data* ed)
{
	if(!ed->feeling_frame) return;
	for(wme* w = ed->feeling_frame->value->id.input_wmes; w!=NIL; w=w->next)
	{
		release_io_symbol(thisAgent, w->id);  // Not sure why I have to do this explicitly
		//		remove_input_wme(thisAgent, w);
	}
	remove_input_wme(thisAgent, ed->feeling_frame);
}

void cleanup_emotion_data(agent* thisAgent, emotion_data* ed)
{
	emotion_clear_feeling_frame(thisAgent, ed);
}

void register_appraisal(emotion_data* ed, wme* appraisal)
{
	if(appraisal->attr->sc.common_symbol_info.symbol_type == SYM_CONSTANT_SYMBOL_TYPE)
	{
		if(appraisal->id != ed->currentEmotion.id_sym) {
			ed->currentEmotion.Reset(appraisal->id, ed->currentEmotion.outcome_probability);
		}

		string result = ed->currentEmotion.SetAppraisalValue(appraisal->attr->sc.name, appraisal->value);
	}

}

void get_appraisals(Symbol* goal)
{
	if(!goal->id.emotion_header_appraisal) return;

	slot* frame_slot = goal->id.emotion_header_appraisal->id.slots;
	slot* appraisal_slot;
	wme *frame, *appraisal;

	if ( frame_slot )
	{
		for ( ; frame_slot; frame_slot = frame_slot->next )
		{
			if(    frame_slot->attr->sc.common_symbol_info.symbol_type == SYM_CONSTANT_SYMBOL_TYPE
				&& !strcmp(frame_slot->attr->sc.name, "frame")) /* BADBAD: should store "frame" symbol in common symbols so can do direct comparison */
			{
				for ( frame = frame_slot->wmes ; frame; frame = frame->next)
				{
					if (frame->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE)
					{
						for ( appraisal_slot = frame->value->id.slots; appraisal_slot; appraisal_slot = appraisal_slot->next )
						{
							for ( appraisal = appraisal_slot->wmes; appraisal; appraisal = appraisal->next )
							{
								register_appraisal(goal->id.emotion_info, appraisal);
							}
						}
					}
				}
			}
		}
	}
}

void update_mood(Symbol* goal)
{
	emotion_data* ed = goal->id.emotion_info;
	ed->currentMood.Decay();
	ed->currentMood.MoveTowardEmotion(ed->currentEmotion);
}


inline void generate_feeling_appraisal_numeric(agent* thisAgent, const char* const name, emotion_data* ed)
{
	Symbol* tempAtt;
	Symbol* tempVal;

	tempAtt = make_sym_constant(thisAgent, name);
	tempVal = make_float_constant(thisAgent, ed->currentFeeling.GetNumericDimension(name, ed->currentEmotion, ed->currentMood.af, ed->appraisalStatus.GetStatus(name)));
	add_input_wme(thisAgent, ed->feeling_frame->value, tempAtt, tempVal);
	symbol_remove_ref(thisAgent, tempAtt);
	symbol_remove_ref(thisAgent, tempVal);
}

inline void generate_feeling_appraisal_categorical(agent* thisAgent, const char* const name, emotion_data* ed)
{
	Symbol* tempAtt;
	Symbol* tempVal;

	tempAtt = make_sym_constant(thisAgent, name);
	tempVal = make_sym_constant(thisAgent, ed->currentFeeling.GetCategoricalDimension(name, ed->currentEmotion, ed->currentMood.af, ed->appraisalStatus.GetStatus(name)).c_str());
	add_input_wme(thisAgent, ed->feeling_frame->value, tempAtt, tempVal);
	symbol_remove_ref(thisAgent, tempAtt);
	symbol_remove_ref(thisAgent, tempVal);
}

// BADBAD: should have pre-made Symbols for all of these attributes
// Shouldn't have to pass in status, mood, and emotion -- those should be directly available to the called function
void generate_feeling_frame(agent* thisAgent, Symbol * goal)
{
	emotion_data* ed = goal->id.emotion_info;

	// clear previous feeling frame (stored on agent structure)
	if(ed->feeling_frame) { emotion_clear_feeling_frame(thisAgent, ed); }

	// generate new frame
	Symbol* frame_att = make_sym_constant(thisAgent, "frame");
	ed->feeling_frame = add_input_wme(thisAgent, goal->id.emotion_header_feeling, frame_att, make_new_identifier(thisAgent, 'F', goal->id.level));
	symbol_remove_ref(thisAgent, frame_att);

	generate_feeling_appraisal_numeric(thisAgent, "suddenness", ed);
	generate_feeling_appraisal_numeric(thisAgent, "unpredictability", ed);
	generate_feeling_appraisal_numeric(thisAgent, "intrinsic-pleasantness", ed);
	generate_feeling_appraisal_numeric(thisAgent, "goal-relevance", ed);
	generate_feeling_appraisal_numeric(thisAgent, "outcome-probability", ed);
	generate_feeling_appraisal_numeric(thisAgent, "discrepancy", ed);
	generate_feeling_appraisal_numeric(thisAgent, "conduciveness", ed);
	generate_feeling_appraisal_numeric(thisAgent, "control", ed);
	generate_feeling_appraisal_numeric(thisAgent, "power", ed);
	generate_feeling_appraisal_categorical(thisAgent, "causal-agent", ed);
	generate_feeling_appraisal_categorical(thisAgent, "causal-motive", ed);


	// create feeling intensity, valence
	Symbol* tempAtt;
	Symbol* tempVal;

	double intensity = ed->currentFeeling.af.CalculateIntensity();
	double valence = ed->currentFeeling.af.CalculateValence();
	double reward = intensity * valence;

	tempAtt = make_sym_constant(thisAgent, "intensity");  //BADBAD: this should be a predefined symbol
	tempVal = make_float_constant(thisAgent, intensity);
	add_input_wme(thisAgent, ed->feeling_frame->value, tempAtt, tempVal);
	symbol_remove_ref(thisAgent, tempAtt);
	symbol_remove_ref(thisAgent, tempVal);

	tempAtt = make_sym_constant(thisAgent, "valence");  //BADBAD: this should be a predefined symbol
	tempVal = make_float_constant(thisAgent, valence);
	add_input_wme(thisAgent, ed->feeling_frame->value, tempAtt, tempVal);
	symbol_remove_ref(thisAgent, tempAtt);
	symbol_remove_ref(thisAgent, tempVal);

	tempAtt = make_sym_constant(thisAgent, "reward");  //BADBAD: this should be a predefined symbol
	tempVal = make_float_constant(thisAgent, reward);
	add_input_wme(thisAgent, ed->feeling_frame->value, tempAtt, tempVal);
	symbol_remove_ref(thisAgent, tempAtt);
	symbol_remove_ref(thisAgent, tempVal);
}

void emotion_reset_data( agent *thisAgent )
{
	for(Symbol* goal = thisAgent->top_goal; goal; goal=goal->id.lower_goal)
	{
		emotion_clear_feeling_frame(thisAgent, goal->id.emotion_info);
	}
}

void emotion_update(agent* thisAgent, Symbol* goal)
{
	get_appraisals(goal);
	update_mood(goal);
	generate_feeling_frame(thisAgent, goal);
}