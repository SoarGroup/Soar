#ifndef MODALEMOTION_H
#define MODALEMOTION_H

#include <string>
#include <map>
#include <algorithm>

using namespace std;

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

string GenerateLabel(AppraisalFrame af) {

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

#endif // MODALEMOTION_H