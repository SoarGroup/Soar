/*
 * production_record.h
 *
 *  Created on: Apr 22, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_EXPLANATION_MEMORY_PRODUCTION_RECORD_H_
#define CORE_SOARKERNEL_SRC_EXPLANATION_MEMORY_PRODUCTION_RECORD_H_

#include "kernel.h"

class production_record
{
        friend class Explanation_Memory;

    public:

        production_record() {};
        ~production_record() {};

        void init(agent* pAgent, production* pProd);
        void clean_up();

        condition*  get_lhs() { return lhs_conds; }
        action*     get_rhs() { return rhs_actions; }
        bool        was_generated() { return (lhs_conds != NULL); }

    private:
        agent*      thisAgent;
        condition*  lhs_conds;
        action*     rhs_actions;
};

#endif /* CORE_SOARKERNEL_SRC_EXPLANATION_MEMORY_PRODUCTION_RECORD_H_ */
