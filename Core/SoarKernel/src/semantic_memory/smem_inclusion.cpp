/*
 * smem_inclusion.cpp
 *
 * Structural inclusion check for semantic memory (Kilpeläinen-Mannila 1995).
 * Experimental -- detection only, no eviction.
 *
 * An LTI A "includes" LTI B if B's augmentation graph embeds injectively
 * into A's: every slot (attribute -> values) in B has a matching slot in A
 * with a superset of values. For child LTI values, recurse with
 * backtracking to ensure correct injective matching. Constants match exactly.
 *
 * SMem graphs may contain cycles and shared substructure. Cycle handling
 * uses a separate recursion stack (active_pairs) from the memoization
 * table (memo) to avoid conflating "currently exploring" with "proven".
 */

#include "semantic_memory.h"
#include "smem_db.h"
#include "smem_settings.h"

#include "agent.h"
#include "output_manager.h"

#include <map>
#include <set>
#include <vector>
#include <string>
#include <sstream>

/* ----------------------------------------------------------------
 * Lightweight representation of an LTI's direct augmentations,
 * keyed by raw hash IDs so we avoid Symbol allocation/deallocation.
 * ---------------------------------------------------------------- */

struct smem_aug_const
{
    int64_t value_type;
    int64_t value_hash;

    bool operator<(const smem_aug_const& o) const
    {
        if (value_type != o.value_type) return value_type < o.value_type;
        return value_hash < o.value_hash;
    }
    bool operator==(const smem_aug_const& o) const
    {
        return value_type == o.value_type && value_hash == o.value_hash;
    }
};

typedef std::pair<int64_t, int64_t> attr_key; // (attr_type, attr_hash)

struct smem_lti_augmentations
{
    uint64_t lti_id;

    // For each attribute, the set of constant values
    std::map<attr_key, std::set<smem_aug_const>> const_values;

    // For each attribute, the multiset of child LTI ids (stored as vector for injective matching)
    std::map<attr_key, std::vector<uint64_t>> lti_values;
};

/* ----------------------------------------------------------------
 * Load an LTI's direct augmentations from the database.
 * Uses web_expand: columns are attr_type(0), attr_hash(1),
 *     value_type(2), value_hash(3), value_lti(4).
 * ---------------------------------------------------------------- */
static smem_lti_augmentations load_lti_augs(SMem_Manager* smem, soar_module::sqlite_statement* expand_q, uint64_t lti_id)
{
    smem_lti_augmentations result;
    result.lti_id = lti_id;

    expand_q->bind_int(1, lti_id);
    while (expand_q->execute() == soar_module::row)
    {
        int64_t attr_type = expand_q->column_int(0);
        int64_t attr_hash = expand_q->column_int(1);
        attr_key ak = std::make_pair(attr_type, attr_hash);

        int64_t value_lti = expand_q->column_int(4);
        if (value_lti != SMEM_AUGMENTATIONS_NULL)
        {
            result.lti_values[ak].push_back(static_cast<uint64_t>(value_lti));
        }
        else
        {
            smem_aug_const c;
            c.value_type = expand_q->column_int(2);
            c.value_hash = expand_q->column_int(3);
            result.const_values[ak].insert(c);
        }
    }
    expand_q->reinitialize();

    return result;
}

/* ----------------------------------------------------------------
 * Check whether LTI A includes LTI B (i.e. B is dominated by A).
 *
 * For every attribute in B:
 *   - A must have the same attribute
 *   - Every constant value under that attribute in B must appear in A
 *   - Every child LTI under that attribute in B must be injectively
 *     matched to a child LTI in A that recursively includes it
 *
 * Uses separate structures for cycle detection vs. memoization:
 *   active_pairs: recursion stack — pair is currently being explored
 *   memo: proven results — pair has been fully evaluated
 *
 * Child matching uses backtracking (not greedy) to ensure correct
 * injective assignment when first-fit would block later matches.
 * ---------------------------------------------------------------- */

typedef std::pair<uint64_t, uint64_t> lti_pair;

static bool smem_lti_includes_impl(
    SMem_Manager* smem,
    soar_module::sqlite_statement* expand_q,
    uint64_t lti_a,
    uint64_t lti_b,
    std::map<uint64_t, smem_lti_augmentations>& aug_cache,
    std::set<lti_pair>& active_pairs,
    std::map<lti_pair, bool>& memo);

/* Backtracking injective matcher for child LTIs under one attribute.
 * Tries to assign each b_lti to a distinct a_lti that includes it.
 * Returns true if a complete injective matching exists. */
static bool match_children_backtrack(
    SMem_Manager* smem,
    soar_module::sqlite_statement* expand_q,
    const std::vector<uint64_t>& a_ltis,
    const std::vector<uint64_t>& b_ltis,
    size_t b_idx,
    std::vector<bool>& a_used,
    std::map<uint64_t, smem_lti_augmentations>& aug_cache,
    std::set<lti_pair>& active_pairs,
    std::map<lti_pair, bool>& memo)
{
    if (b_idx == b_ltis.size()) return true; // all B children matched

    for (size_t ai = 0; ai < a_ltis.size(); ai++)
    {
        if (a_used[ai]) continue;

        if (smem_lti_includes_impl(smem, expand_q, a_ltis[ai], b_ltis[b_idx], aug_cache, active_pairs, memo))
        {
            a_used[ai] = true;
            if (match_children_backtrack(smem, expand_q, a_ltis, b_ltis, b_idx + 1, a_used, aug_cache, active_pairs, memo))
            {
                return true;
            }
            a_used[ai] = false; // undo and try next candidate
        }
    }
    return false;
}

static bool smem_lti_includes_impl(
    SMem_Manager* smem,
    soar_module::sqlite_statement* expand_q,
    uint64_t lti_a,
    uint64_t lti_b,
    std::map<uint64_t, smem_lti_augmentations>& aug_cache,
    std::set<lti_pair>& active_pairs,
    std::map<lti_pair, bool>& memo)
{
    if (lti_a == lti_b) return true;

    lti_pair pair_key = std::make_pair(lti_a, lti_b);

    // Check memo first (proven true or proven false)
    auto memo_it = memo.find(pair_key);
    if (memo_it != memo.end()) return memo_it->second;

    // Cycle detection: if we're currently exploring this pair, conservatively
    // return false (don't assume inclusion for cycles)
    if (active_pairs.count(pair_key)) return false;
    active_pairs.insert(pair_key);

    // Load augmentations (with caching)
    if (aug_cache.find(lti_a) == aug_cache.end())
    {
        aug_cache[lti_a] = load_lti_augs(smem, expand_q, lti_a);
    }
    if (aug_cache.find(lti_b) == aug_cache.end())
    {
        aug_cache[lti_b] = load_lti_augs(smem, expand_q, lti_b);
    }

    const smem_lti_augmentations& augs_a = aug_cache[lti_a];
    const smem_lti_augmentations& augs_b = aug_cache[lti_b];

    bool result = true;

    // Check constant values: for every attribute in B, A must have a superset
    for (auto& b_entry : augs_b.const_values)
    {
        const attr_key& ak = b_entry.first;
        const std::set<smem_aug_const>& b_consts = b_entry.second;

        auto a_it = augs_a.const_values.find(ak);
        if (a_it == augs_a.const_values.end())
        {
            result = false;
            break;
        }
        const std::set<smem_aug_const>& a_consts = a_it->second;

        for (auto& bc : b_consts)
        {
            if (a_consts.find(bc) == a_consts.end())
            {
                result = false;
                break;
            }
        }
        if (!result) break;
    }

    // Check LTI children: injective matching with backtracking
    if (result)
    {
        for (auto& b_entry : augs_b.lti_values)
        {
            const attr_key& ak = b_entry.first;
            const std::vector<uint64_t>& b_ltis = b_entry.second;

            auto a_it = augs_a.lti_values.find(ak);
            if (a_it == augs_a.lti_values.end())
            {
                result = false;
                break;
            }
            const std::vector<uint64_t>& a_ltis = a_it->second;

            if (a_ltis.size() < b_ltis.size())
            {
                result = false;
                break;
            }

            std::vector<bool> a_used(a_ltis.size(), false);
            if (!match_children_backtrack(smem, expand_q, a_ltis, b_ltis, 0, a_used, aug_cache, active_pairs, memo))
            {
                result = false;
                break;
            }
        }
    }

    active_pairs.erase(pair_key);
    memo[pair_key] = result;
    return result;
}

/* ----------------------------------------------------------------
 * Public entry point: check if LTI A includes LTI B.
 * ---------------------------------------------------------------- */
static bool smem_lti_includes(SMem_Manager* smem, soar_module::sqlite_statement* expand_q, uint64_t lti_a, uint64_t lti_b)
{
    std::map<uint64_t, smem_lti_augmentations> aug_cache;
    std::set<lti_pair> active_pairs;
    std::map<lti_pair, bool> memo;
    return smem_lti_includes_impl(smem, expand_q, lti_a, lti_b, aug_cache, active_pairs, memo);
}

/* ----------------------------------------------------------------
 * CLI_redundancy_check: scan all LTI pairs and report domination.
 *
 * Maintains a running non-dominated set. For each new LTI B,
 * checks against existing dominators. Uses transitivity to skip
 * already-dominated entries.
 * ---------------------------------------------------------------- */
bool SMem_Manager::CLI_redundancy_check(std::string& result)
{
    attach();
    if (!connected())
    {
        result.append("Semantic memory database not connected.");
        return false;
    }

    // Collect all LTI IDs
    std::vector<uint64_t> all_ltis;
    soar_module::sqlite_statement* q = SQL->lti_all;
    while (q->execute() == soar_module::row)
    {
        all_ltis.push_back(static_cast<uint64_t>(q->column_int(0)));
    }
    q->reinitialize();

    if (all_ltis.empty())
    {
        result.append("No LTIs in semantic memory.\n");
        return true;
    }

    std::ostringstream out;
    out << "Scanning " << all_ltis.size() << " LTIs for redundancy...\n";

    soar_module::sqlite_statement* expand_q = SQL->web_expand;

    // Track domination relationships: dominated_lti -> dominator_lti
    std::map<uint64_t, uint64_t> dominated_by;
    size_t pairs_checked = 0;

    for (size_t i = 0; i < all_ltis.size(); i++)
    {
        uint64_t lti_a = all_ltis[i];

        // Skip if already dominated
        if (dominated_by.count(lti_a)) continue;

        for (size_t j = 0; j < all_ltis.size(); j++)
        {
            if (i == j) continue;

            uint64_t lti_b = all_ltis[j];

            // Skip if B is already dominated by A (transitivity)
            if (dominated_by.count(lti_b) && dominated_by[lti_b] == lti_a) continue;

            // Skip if A is already dominated
            if (dominated_by.count(lti_a)) break;

            pairs_checked++;

            // Check if A includes B (B is dominated by A)
            if (smem_lti_includes(this, expand_q, lti_a, lti_b))
            {
                // B is dominated by A, but only if B doesn't also include A
                // (which would mean they're equivalent -- report the one with lower ID as dominator)
                if (!smem_lti_includes(this, expand_q, lti_b, lti_a))
                {
                    dominated_by[lti_b] = lti_a;
                }
                else if (lti_a < lti_b)
                {
                    // Equivalent structures -- lower ID dominates
                    dominated_by[lti_b] = lti_a;
                }
            }
        }
    }

    out << "Checked " << pairs_checked << " pairs.\n\n";

    if (dominated_by.empty())
    {
        out << "No redundant LTIs found.\n";
    }
    else
    {
        out << "Dominated LTIs:\n";
        for (auto& entry : dominated_by)
        {
            out << "  @" << entry.first << " is dominated by @" << entry.second << "\n";
        }
        out << "\n" << dominated_by.size() << " redundant LTI(s) found.\n";
    }

    result.append(out.str());
    return true;
}
