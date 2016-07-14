/*
 * production_record.h
 *
 *  Created on: Apr 22, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_EXPLAIN_PRODUCTION_RECORD_H_
#define CORE_SOARKERNEL_SRC_EXPLAIN_PRODUCTION_RECORD_H_

#include "kernel.h"

class production_record
{
        friend class Explanation_Memory;

    public:

        production_record(agent* pAgent, production* pProd);
        ~production_record();

        condition*  get_lhs() { return lhs_conds; }
        action*     get_rhs() { return rhs_actions; }

    private:
        agent*      thisAgent;
        condition*  lhs_conds;
        action*     rhs_actions;
};

#endif /* CORE_SOARKERNEL_SRC_EXPLAIN_PRODUCTION_RECORD_H_ */
