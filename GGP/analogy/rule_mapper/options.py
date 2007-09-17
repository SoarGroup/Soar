# Rule mapper options

# multiplier for contribution of the head predicate match score to the rule
# match score
HEAD_SCORE_FACTOR = 1.5

# should rule matches be allowed even if some of the body predicates can't
# match up? Setting to False enforces structural consistency
#ALLOW_PARTIAL_MAPS = False
ALLOW_PARTIAL_MAPS = False 

# number of times to unroll the rule mapper after having found a full mapping
NUM_RETRIES = 1

# calculate the effects 
USE_MOVE_EFFECTS = True

# score given to a predicate match that is already committed to
MATCHED_PRED_SCORE = 1
#MATCHED_PRED_SCORE = 0

# predicate match score when a predicate already mapped to something else tries
# to match again. Keep this at 0 to enforce one-to-oneness
MISMATCHED_PRED_SCORE = 0

# the score multiplier for a predicate match on predicates in the same bin
BIN_MATCH_MULT = 2

# the score multiplier for a predicate match on predicates in different bins
BIN_MISMATCH_MULT = 0.1

# multiplier for when a rule match contains an already matched predicate
MATCHED_RULE_MULT = 1
