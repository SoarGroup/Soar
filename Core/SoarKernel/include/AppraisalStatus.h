#ifndef APPRAISAL_STATUS_H
#define APPRAISAL_STATUS_H

#include <string>
#include <boost/lexical_cast.hpp>

using std::string;
using boost::lexical_cast;

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

	AppraisalStatus() {Init();}

   void Init() {
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

   string SetStatus(string appraisal, bool status) {
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

   bool GetStatus(string appraisal) const {
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
};

#endif // APPRAISAL_STATUS_H