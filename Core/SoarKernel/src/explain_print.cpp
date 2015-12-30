#include "ebc_explain.h"

#include "agent.h"
#include "condition.h"
#include "debug.h"
#include "instantiations.h"
#include "prefmem.h"
#include "production.h"
#include "output_manager.h"
#include "wmem.h"

void Explanation_Logger::explain_chunking_summary() {
    outputManager->printa_sf(thisAgent, "%fEBC Explainer Summary\n\n");
}

void Explanation_Logger::explain_stats() {
    outputManager->printa_sf(thisAgent, "%fEBC Explainer Statistics\n\n");
    outputManager->printa_sf(thisAgent, "Chunks recorded                %u\n", total_recorded.chunks);
    outputManager->printa_sf(thisAgent, "Actions recorded               %u\n", total_recorded.actions);
    outputManager->printa_sf(thisAgent, "Conditions                     %u\n", total_recorded.conditions);
    outputManager->printa_sf(thisAgent, "Instantiations                 %u\n", total_recorded.instantiations);
    outputManager->printa_sf(thisAgent, "Instantiations_referenced      %u\n", total_recorded.instantiations_referenced);
    outputManager->printa_sf(thisAgent, "Instantiations_skipped         %u\n\n", total_recorded.instantiations_skipped);
    outputManager->printa_sf(thisAgent, "%fEBC History Statistics\n\n");
    outputManager->printa_sf(thisAgent, "chunks_attempted               %u\n", stats.chunks_attempted);
    outputManager->printa_sf(thisAgent, "succeeded                      %u\n", stats.succeeded);
    outputManager->printa_sf(thisAgent, "justifications_attempted       %u\n", stats.justifications_attempted);
    outputManager->printa_sf(thisAgent, "justifications                 %u\n", stats.justifications);

    outputManager->printa_sf(thisAgent, "no_grounds                     %u\n", stats.no_grounds);
    outputManager->printa_sf(thisAgent, "max_chunks                     %u\n", stats.max_chunks);
    outputManager->printa_sf(thisAgent, "unorderable                    %u\n", stats.unorderable);
    outputManager->printa_sf(thisAgent, "duplicates                     %u\n", stats.duplicates);
    outputManager->printa_sf(thisAgent, "chunk_did_not_match            %u\n", stats.chunk_did_not_match);
    outputManager->printa_sf(thisAgent, "justification_did_not_match    %u\n", stats.justification_did_not_match);
    outputManager->printa_sf(thisAgent, "tested_local_negation          %u\n", stats.tested_local_negation);

    outputManager->printa_sf(thisAgent, "instantations_backtraced       %u\n", stats.instantations_backtraced);
    outputManager->printa_sf(thisAgent, "seen_instantations_backtraced  %u\n", stats.seen_instantations_backtraced);

    outputManager->printa_sf(thisAgent, "merged_conditions              %u\n", stats.merged_conditions);
    outputManager->printa_sf(thisAgent, "constraints_attached           %u\n", stats.constraints_attached);
    outputManager->printa_sf(thisAgent, "constraints_collected          %u\n", stats.constraints_collected);
}

void Explanation_Logger::print_chunk_list() {}
bool Explanation_Logger::explain_rule(const std::string* pStringParameter) {return false;}
bool Explanation_Logger::explain_item(const std::string* pStringParameter, const std::string* pStringParameter2) {return false;}
bool Explanation_Logger::current_discussed_chunk_exists() {return false;}
void Explanation_Logger::explain_instantiation() {}
void Explanation_Logger::explain_dependency_analysis() {}
void Explanation_Logger::explain_identity_sets() {}
void Explanation_Logger::explain_constraints_enforced() {}
void Explanation_Logger::should_explain_rule(bool pEnable) {}
void Explanation_Logger::should_explain_all(bool pEnable) {}

