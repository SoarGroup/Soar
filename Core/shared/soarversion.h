#ifndef SOARVERSION_H
#define SOARVERSION_H

#define _soar_xstr(s) _soar_str(s)
#define _soar_str(s) #s

/*
Many places to change the version number (this list is shorter than it used to be):

Core/shared/soarversion.h
rename Java/Debugger/src/main/resources/.dlf files
Java/SMLJava/ edu.umich.soar.SoarProperties.VERSION
Inside the root SConstruct file
Inside all Java launchers in Release/ subfolders
soar-vars.bat in root

*/
#define MAJOR_VERSION_NUMBER 9
#define MINOR_VERSION_NUMBER 4
#define MICRO_VERSION_NUMBER 0
#define GREEK_VERSION_NUMBER 0
inline const char* VERSION_STRING()
{
    return _soar_xstr(MAJOR_VERSION_NUMBER) "." _soar_xstr(MINOR_VERSION_NUMBER) "."  _soar_xstr(MICRO_VERSION_NUMBER);
}

#define SML_MAJOR_VERSION_NUMBER 9
#define SML_MINOR_VERSION_NUMBER 4
#define SML_MICRO_VERSION_NUMBER 0
#define SML_GREEK_VERSION_NUMBER 0
inline const char* SML_VERSION_STRING()
{
    return _soar_xstr(SML_MAJOR_VERSION_NUMBER) "." _soar_xstr(SML_MINOR_VERSION_NUMBER) "."  _soar_xstr(SML_MICRO_VERSION_NUMBER);
}

#endif // SOARVERSION_H
