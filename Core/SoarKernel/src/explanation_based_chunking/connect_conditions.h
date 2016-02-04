#ifndef CONNECT_CONDITIONS_H_
#define CONNECT_CONDITIONS_H_

#include "kernel.h"
#include "soar_module.h"

class sym_grounding_path
{
    public:
        sym_grounding_path(Symbol* new_root, wme_list* new_path = NULL, wme* new_wme = NULL) {
            topSym = new_root;
            wme_path = new wme_list();
            if (new_path) {
                (*wme_path) = (*new_path);
                if (new_wme) wme_path->push_front(new_wme);
            }
        }
        ~sym_grounding_path() { delete wme_path; }

        Symbol* get_root() {return topSym;}
        wme_list* get_path() {return wme_path;}

    private:

        Symbol*     topSym;
        wme_list*   wme_path;
};

#endif /* CONNECT_CONDITIONS_H_ */
