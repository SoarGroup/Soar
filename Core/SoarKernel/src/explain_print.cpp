#include <explain.h>
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

    outputManager->printa_sf(thisAgent, "-------------------------\n");
    outputManager->printa_sf(thisAgent, "%fEBC Executions Statistics\n");
    outputManager->printa_sf(thisAgent, "-------------------------\n");
    outputManager->printa_sf(thisAgent, "Number of chunks                           %u\n", thisAgent->ebChunker->get_chunk_count());
    outputManager->printa_sf(thisAgent, "Chunks attempted                           %u\n", stats.chunks_attempted);
    outputManager->printa_sf(thisAgent, "Chunks successfully built                  %u\n", stats.succeeded);

    outputManager->printa_sf(thisAgent, "\nNumber of justifications                   %u\n", thisAgent->ebChunker->get_justification_count());
    outputManager->printa_sf(thisAgent, "Justifications attempted                   %u\n", stats.justifications_attempted);
    outputManager->printa_sf(thisAgent, "Justifications successfully built          %u\n", stats.justifications);

    outputManager->printa_sf(thisAgent, "\nTotal instantiations built                 %u\n", thisAgent->ebChunker->get_instantiation_count());
    outputManager->printa_sf(thisAgent, "Total instantiations backtraced through    %u\n", stats.instantations_backtraced);
    outputManager->printa_sf(thisAgent, "Total instantiations already visited       %u\n", stats.seen_instantations_backtraced);

    outputManager->printa_sf(thisAgent, "\nNumber of conditions merged                %u\n", stats.merged_conditions);
    outputManager->printa_sf(thisAgent, "Number of constraints attached             %u\n", stats.constraints_attached);
    outputManager->printa_sf(thisAgent, "Number of constraints collected            %u\n", stats.constraints_collected);

    outputManager->printa_sf(thisAgent, "----------------------\n");
    outputManager->printa_sf(thisAgent, "EBC Failure Statistics\n");
    outputManager->printa_sf(thisAgent, "----------------------\n");

    outputManager->printa_sf(thisAgent, "Duplicate chunk already existed            %u\n", stats.duplicates);
    outputManager->printa_sf(thisAgent, "Chunk tested local negation                %u\n", stats.tested_local_negation);
    outputManager->printa_sf(thisAgent, "Could not re-order chunk                   %u\n", stats.unorderable);
    outputManager->printa_sf(thisAgent, "Chunk had no grounds                       %u\n", stats.no_grounds);
    outputManager->printa_sf(thisAgent, "Already reached max-chunks                 %u\n", stats.max_chunks);
    outputManager->printa_sf(thisAgent, "Chunk formed did not match WM              %u\n", stats.chunk_did_not_match);
    outputManager->printa_sf(thisAgent, "Justification formed did not match WM      %u\n", stats.justification_did_not_match);

    outputManager->printa_sf(thisAgent, "------------------------\n");
    outputManager->printa_sf(thisAgent, "EBC Explainer Statistics\n");
    outputManager->printa_sf(thisAgent, "------------------------\n");
    outputManager->printa_sf(thisAgent, "Chunks records                             %u\n", total_recorded.chunks);
    outputManager->printa_sf(thisAgent, "Actions records                            %u\n", total_recorded.actions);
    outputManager->printa_sf(thisAgent, "Condition records                          %u\n", total_recorded.conditions);
    outputManager->printa_sf(thisAgent, "Instantiation records                      %u\n", total_recorded.instantiations);
    outputManager->printa_sf(thisAgent, "Instantiation references in conditions     %u\n", total_recorded.instantiations_referenced);
    outputManager->printa_sf(thisAgent, "Instantiations skipped                     %u\n", total_recorded.instantiations_skipped);
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

