#ifndef COMMANDLINEINTERFACEDLL_H
#define COMMANDLINEINTERFACEDLL_H

#ifndef unused
#define unused(x) (void)(x)
#endif

#ifdef _WIN32
#ifdef _USRDLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif
#else
#define EXPORT
#endif

#endif // COMMANDLINEINTERFACEDLL_H
