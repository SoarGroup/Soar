#ifndef general_h
#define general_h

inline std::string catStrInt(const char* str, int x);

inline std::string int2str(int x);

inline const char* getCommandParameter(sml::Identifier* c, const char *name);

double squaredDistance(double x1, double y1, double x2, double y2);

#endif
