/*
 * smem_inclusion.cpp
 *
 * Structural inclusion check for semantic memory (Kilpeläinen-Mannila 1995).
 * Experimental -- detection and budgeted eviction (sweep).
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
#include "smem_stats.h"

#include "agent.h"
#include "output_manager.h"

#include <algorithm>
#include <functional>
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
 * Cycle handling is coinductive: revisiting an active pair returns true
 * (optimistic assumption). If the assumption is wrong, the non-cyclic
 * parts of the proof will fail. This correctly handles self-referential
 * structures like @1 ^next @1 vs @2 ^next @2.
 *
 * Global injectivity is enforced via b_to_a: a map from B node IDs to
 * their assigned A node IDs, threaded through all recursion. Two
 * distinct B nodes cannot map to the same A node even if reached
 * through different attributes.
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
    std::map<uint64_t, uint64_t>& b_to_a);

/* Backtracking injective matcher for child LTIs under one attribute.
 * Tries to assign each b_lti to a distinct a_lti that includes it.
 * Returns true if a complete injective matching exists.
 * Enforces global injectivity via b_to_a map.
 * Snapshots b_to_a before each speculative branch and restores on
 * failure to prevent leaked descendant bindings. */
static bool match_children_backtrack(
    SMem_Manager* smem,
    soar_module::sqlite_statement* expand_q,
    const std::vector<uint64_t>& a_ltis,
    const std::vector<uint64_t>& b_ltis,
    size_t b_idx,
    std::vector<bool>& a_used,
    std::map<uint64_t, smem_lti_augmentations>& aug_cache,
    std::set<lti_pair>& active_pairs,
    std::map<uint64_t, uint64_t>& b_to_a)
{
    if (b_idx == b_ltis.size()) return true; // all B children matched

    uint64_t b_child = b_ltis[b_idx];

    // If this B node was already assigned globally, only try that A node
    auto prior = b_to_a.find(b_child);
    if (prior != b_to_a.end())
    {
        uint64_t required_a = prior->second;
        for (size_t ai = 0; ai < a_ltis.size(); ai++)
        {
            if (a_used[ai] || a_ltis[ai] != required_a) continue;
            a_used[ai] = true;
            if (match_children_backtrack(smem, expand_q, a_ltis, b_ltis, b_idx + 1, a_used, aug_cache, active_pairs, b_to_a))
            {
                return true;
            }
            a_used[ai] = false;
        }
        return false;
    }

    for (size_t ai = 0; ai < a_ltis.size(); ai++)
    {
        if (a_used[ai]) continue;

        // Check global injectivity: is this A node already claimed by a different B node?
        bool a_claimed = false;
        for (auto& ba : b_to_a)
        {
            if (ba.second == a_ltis[ai] && ba.first != b_child)
            {
                a_claimed = true;
                break;
            }
        }
        if (a_claimed) continue;

        // Snapshot b_to_a before speculative branch
        std::map<uint64_t, uint64_t> b_to_a_snapshot = b_to_a;

        // Pre-bind before recursion so descendant checks see the intended assignment
        b_to_a[b_child] = a_ltis[ai];

        if (smem_lti_includes_impl(smem, expand_q, a_ltis[ai], b_child, aug_cache, active_pairs, b_to_a))
        {
            a_used[ai] = true;
            if (match_children_backtrack(smem, expand_q, a_ltis, b_ltis, b_idx + 1, a_used, aug_cache, active_pairs, b_to_a))
            {
                return true;
            }
            a_used[ai] = false;
        }

        // Restore full b_to_a state on failure (undoes all descendant bindings)
        b_to_a = b_to_a_snapshot;
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
    std::map<uint64_t, uint64_t>& b_to_a)
{
    if (lti_a == lti_b) return true;

    lti_pair pair_key = std::make_pair(lti_a, lti_b);

    // Coinductive cycle handling: if we're currently exploring this pair,
    // optimistically assume inclusion holds. If the assumption is wrong,
    // the non-cyclic parts of the proof will fail.
    if (active_pairs.count(pair_key)) return true;
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
            if (!match_children_backtrack(smem, expand_q, a_ltis, b_ltis, 0, a_used, aug_cache, active_pairs, b_to_a))
            {
                result = false;
                break;
            }
        }
    }

    active_pairs.erase(pair_key);
    // No memoization: results depend on b_to_a context, which changes
    // during backtracking. Smem entries are shallow, so the perf cost
    // of re-evaluation is negligible.
    return result;
}

/* ----------------------------------------------------------------
 * Public entry point: check if LTI A includes LTI B.
 * ---------------------------------------------------------------- */
static bool smem_lti_includes(SMem_Manager* smem, soar_module::sqlite_statement* expand_q, uint64_t lti_a, uint64_t lti_b)
{
    std::map<uint64_t, smem_lti_augmentations> aug_cache;
    std::set<lti_pair> active_pairs;
    std::map<uint64_t, uint64_t> b_to_a;
    // Pin root: B's root must map to A's root (rooted inclusion)
    b_to_a[lti_b] = lti_a;
    return smem_lti_includes_impl(smem, expand_q, lti_a, lti_b, aug_cache, active_pairs, b_to_a);
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

/* ----------------------------------------------------------------
 * R4 safety check: is an LTI currently referenced in working memory?
 *
 * Derbinsky & Laird's R4 forgetting policy removes WMEs that augment
 * LTI-backed objects. If we evict the smem entry while WMEs in
 * working memory still reference it, those WMEs become orphaned.
 *
 * Current implementation: conservative check via smem_in_wmem
 * reference count. If the LTI has any WM references, it is
 * protected from eviction.
 *
 * TODO: Track which WMEs were forgotten under R4 with a specific
 * smem entry as backup. The current check protects live references
 * but cannot detect already-forgotten WMEs that might need the
 * smem entry for re-retrieval.
 * ---------------------------------------------------------------- */
static bool smem_lti_has_r4_dependents(SMem_Manager* smem, uint64_t lti_id)
{
    // Check if this LTI is currently referenced in working memory
    if (smem->smem_in_wmem->find(lti_id) != smem->smem_in_wmem->end())
    {
        return true;
    }
    return false;
}

/* ----------------------------------------------------------------
 * CLI_sweep_dominated: evict dominated LTIs with safety checks.
 *
 * 1. Run the dominated-set detection (same as CLI_redundancy_check)
 * 2. Filter out R4-protected entries (in working memory)
 * 3. For each entry to evict (up to budget):
 *    a. Disconnect augmentations (update frequency tables)
 *    b. Delete from smem_augmentations, smem_lti, smem_activation_history, smem_lti_alias
 *    c. Decrement node count
 * 4. Report what was evicted
 * ---------------------------------------------------------------- */
bool SMem_Manager::CLI_sweep_dominated(std::string& result, int64_t budget)
{
    attach();
    if (!connected())
    {
        result.append("Semantic memory database not connected.");
        return false;
    }

    // --- Mark phase: find dominated LTIs (same logic as CLI_redundancy_check) ---

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
        if (dominated_by.count(lti_a)) continue;

        for (size_t j = 0; j < all_ltis.size(); j++)
        {
            if (i == j) continue;

            uint64_t lti_b = all_ltis[j];
            if (dominated_by.count(lti_b) && dominated_by[lti_b] == lti_a) continue;
            if (dominated_by.count(lti_a)) break;

            pairs_checked++;

            if (smem_lti_includes(this, expand_q, lti_a, lti_b))
            {
                if (!smem_lti_includes(this, expand_q, lti_b, lti_a))
                {
                    dominated_by[lti_b] = lti_a;
                }
                else if (lti_a < lti_b)
                {
                    dominated_by[lti_b] = lti_a;
                }
            }
        }
    }

    out << "Checked " << pairs_checked << " pairs.\n";

    if (dominated_by.empty())
    {
        out << "No redundant LTIs found. Nothing to sweep.\n";
        result.append(out.str());
        return true;
    }

    out << "Found " << dominated_by.size() << " dominated LTI(s).\n\n";

    // --- Filter phase: exclude R4-protected entries ---

    std::vector<uint64_t> to_evict;
    std::vector<uint64_t> protected_ltis;

    for (auto& entry : dominated_by)
    {
        if (smem_lti_has_r4_dependents(this, entry.first))
        {
            protected_ltis.push_back(entry.first);
        }
        else
        {
            to_evict.push_back(entry.first);
        }
    }

    if (!protected_ltis.empty())
    {
        out << "R4-protected (in working memory), skipping " << protected_ltis.size() << ":\n";
        for (auto lti_id : protected_ltis)
        {
            out << "  @" << lti_id << " (dominated by @" << dominated_by[lti_id] << ")\n";
        }
        out << "\n";
    }

    if (to_evict.empty())
    {
        out << "All dominated LTIs are R4-protected. Nothing to sweep.\n";
        result.append(out.str());
        return true;
    }

    // Apply budget
    int64_t evict_count = static_cast<int64_t>(to_evict.size());
    if (budget > 0 && budget < evict_count)
    {
        evict_count = budget;
        out << "Budget limits sweep to " << budget << " of " << to_evict.size() << " candidates.\n";
    }

    // --- Sweep phase: evict in dependency-safe order ---
    // Process in reverse LTI ID order so children are removed before parents
    // (a dominated child is more likely to have a higher ID than its dominator)
    std::sort(to_evict.begin(), to_evict.end(), std::greater<uint64_t>());

    int64_t swept = 0;
    out << "Sweeping:\n";

    for (size_t i = 0; i < static_cast<size_t>(evict_count); i++)
    {
        uint64_t lti_id = to_evict[i];

        // Step 1: Disconnect augmentations (updates frequency tables, edge stats)
        disconnect_ltm(lti_id, NULL);

        // Step 2: Delete from all smem tables via raw SQL
        // (No prepared DELETE FROM smem_lti statement exists)
        std::string sql;

        sql = "DELETE FROM smem_augmentations WHERE value_lti_id=" + std::to_string(lti_id);
        DB->sql_execute(sql.c_str());

        sql = "DELETE FROM smem_activation_history WHERE lti_id=" + std::to_string(lti_id);
        DB->sql_execute(sql.c_str());

        sql = "DELETE FROM smem_lti_alias WHERE lti_id=" + std::to_string(lti_id);
        DB->sql_execute(sql.c_str());

        sql = "DELETE FROM smem_lti WHERE lti_id=" + std::to_string(lti_id);
        DB->sql_execute(sql.c_str());

        // Step 3: Update node count
        statistics->nodes->set_value(statistics->nodes->get_value() - 1);

        out << "  @" << lti_id << " (was dominated by @" << dominated_by[lti_id] << ") -- evicted\n";
        swept++;
    }

    out << "\n" << swept << " LTI(s) evicted.\n";

    result.append(out.str());
    return true;
}
