/* File : sml_ClientInterface.i */
%module sml

%newobject Connection::CreateEmbeddedConnection(char const* pLibraryName, bool clientThread, bool optimized, int portToListenOn = kDefaultSMLPort, ErrorCode* pError = NULL) ;
%include "../sml_ClientInterface.i"




