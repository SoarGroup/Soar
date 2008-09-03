/********************************************************************
* @file gski_matchset.h 
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/20/2002   9:20
*
* purpose: 
*********************************************************************/
#ifndef GSKI_MATCHSET_H
#define GSKI_MATCHSET_H

namespace gSKI {

   struct Error;
   class tIMatchIterator;
   class tIMatchSetIterator;
   class IConditionSet;

   /**
    * @brief Handles sets of matches.
    *
    * Each Match is associated with a Condition, so the Match structure
    * needs to be represented in much the same way as the Condition
    * structure.
    *
    * @see IConditionSet
    *
    * The recursive nature of this definition of IMatchSet is due to
    * negated Match sets which can contain negated Match sets which
    * can contain negated Match sets etc. etc. etc.  <p>
    *
    * At each "level" of the condition, you can retrieve the list of 
    * single matches at that "level", or the list of negated matchSets
    * for that same "level".
    *
    * The structure used for handling Matches works very similarly
    * to the way that the ConditionSet structure works.  The main
    * difference revolves around the fact that there can be many 
    * matches to a single Condition.
    *
    * @see ConditionSet
    *
    * A MatchSet is a single match against a Condition.  Each top-level
    * MatchSet represents a single match!
    *
    * @code
    * Example:
    *  >A---------------------------------------------------------<
    *   >B--------------------------------------------------------<
    *     *  *  >C----<  >D----------------------------------<   *
    *             *  *     *  *  >E----<  >%-< >%----------<
    *                              *  *     *     *   *   *
    *  -{c1 c2 -{c3 c4} -{c5 c6 -{c7 c8} -{c9} {c10 c11 c12} } c13}
    *
    *      >-...-<   ==> MatchSet
    *      *         ==> Individual Condition Match
    *      [A-Z]     ==> MatchSet Identifier
    *      %         ==> Simplification to non-Set.
    *
    * becomes:
    *
    *    MatchSet
    *       Matches   -> none
    *       MatchSets -> 1
    *       ---MatchSet[A]
    *          Matches   -> none
    *          MatchSets -> 1
    *          ---MatchSet[B]
    *             Matches   -> 3
    *             --- c1
    *             --- c2
    *             --- c13
    *             MatchSets -> 2
    *             ---MatchSet[C] 
    *                Matches   -> 2
    *                --- c3
    *                --- c4
    *                MatchSets -> 0
    *             ---MatchSet[D]
    *                Matches   -> 6
    *                --- c5
    *                --- c6
    *                --- c9
    *                --- c10
    *                --- c11
    *                --- c12
    *                MatchSets -> 3
    *                ---MatchSet[E]
    *                   Matches   -> 2
    *                   --- C7
    *                   --- C8
    *                   MatchSets -> 0
    * @endcode
    */
  class MatchSet {
   public:
      
     /**
      * @brief Destructor for the IMatchSet
      *
      * This function insures that the destructor in the most derived
      * class is called when it is destroyed.  This will always be 
      * neccesary because this is a purebase class.
      */
     ~MatchSet();

      /**
      * @brief Returns the single (non-set) matches for the 
      *        current match.
      *
      * The matches that are returned here are all part of the same match.
      *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *              is not NULL this structure is filled with extended error
      *              information.  If it is NULL (the default) extended error
      *              information is not returned.
      *
      * @returns An Iterator to a set of condition matches for a specific
      *          match.  A 0 is returned if there is an error.
      */
     tIMatchIterator *GetMatches(Error* err = 0);


      /**
      * @brief Returns the set of matched negated sets of conditions.
      *
      * @see ConditionSet
      *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *              is not NULL this structure is filled with extended error
      *              information.  If it is NULL (the default) extended error
      *              information is not returned.
      *
      * @returns An iterator to a set of MatchSets representing the negated
      *          sets of Conditions associated with the MatchSet.  A 0 is 
      *          returned if there is an error.
      *
      */
     tIMatchSetIterator *GetMatchSets(Error* err = 0);

      /**
      * @brief Returns the ConditionSet that This Match matched against.
      *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *              is not NULL this structure is filled with extended error
      *              information.  If it is NULL (the default) extended error
      *              information is not returned.
      * 
      * @returns A ConditionSet that was matched by this object.  A 0 is 
      *          returned if there is an error.
      *
      */
     IConditionSet *GetConditionSet(Error* err = 0); \
   };
}
#endif
