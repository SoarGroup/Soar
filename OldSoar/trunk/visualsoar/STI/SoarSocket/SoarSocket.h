
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SOARSOCKET_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SOARSOCKET_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef SOARSOCKET_EXPORTS
#define SOARSOCKET_API __declspec(dllexport)
#else
#define SOARSOCKET_API __declspec(dllimport)
#endif

// This class is exported from the SoarSocket.dll
class SOARSOCKET_API CSoarSocket {
public:
	CSoarSocket(void);
	// TODO: add your methods here.
};

extern SOARSOCKET_API int nSoarSocket;

SOARSOCKET_API int fnSoarSocket(void);

