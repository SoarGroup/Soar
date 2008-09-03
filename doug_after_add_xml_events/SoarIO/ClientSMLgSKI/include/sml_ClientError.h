#ifndef SML_CLIENTERROR_H
#define SML_CLIENTERROR_H

/////////////////////////////////////////////////////////////////////////////
// ClientError class
//
// Author: Bob Marinier, Cory Dunham, Devvan Stokes, University of Michigan
// Date  : August 2004
//
// This class implements error flagging for all 
// classes on the client side of the SML stream
//
/////////////////////////////////////////////////////////////////////////////

#include <string>

namespace sml
{
typedef int smlErrorCode_t; //this could also be an enum

class sml_ClientError
{
public:
	sml_ClientError() : smlErrorCode(0) {}

	virtual smlErrorCode_t GetLastError() { return smlErrorCode ; }

	virtual char const* GetLastErrorDescription() { return smlErrorDescription.c_str() ; }

	virtual void ClearError() { smlErrorCode = 0 ; smlErrorDescription.clear() ;}

protected:
	smlErrorCode_t smlErrorCode;

	std::string smlErrorDescription;
};


}//closes namespace

#endif //SML_CLIENTERROR_H