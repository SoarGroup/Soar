#include "include/general.h"

inline std::string catStrInt(const char* str, int x) {
    ostringstream sstr;
    sstr << str << x;
    return sstr.str();
}

inline std::string int2str(int x) {
    ostringstream sstr;
    sstr << x;
    return sstr.str();
}

inline const char* getCommandParameter(sml::Identifier* c, const char *name) {
    const char *val = c->GetParameterValue(name);
    if (strlen(val) == 0) {
        cout << "Error: Parameter " << name << " does not exist." << endl;
        exit(1);
    }
    return val;
}

double squaredDistance(double x1, double y1, double x2, double y2) {
  return ((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}
