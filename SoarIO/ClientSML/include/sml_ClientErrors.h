/////////////////////////////////////////////////////////////////
// Base class for setting and reporting errors.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
/////////////////////////////////////////////////////////////////

#ifndef SML_CLIENT_ERRORS_H
#define SML_CLIENT_ERRORS_H

#include <string>

namespace sml {

// Actual error codes are in sml_Errors.h
typedef int ErrorCode ;

class ClientErrors
{
protected:
	ErrorCode			m_LastError ;

	// Usually this will be empty, indicating that the error code is sufficient.
	// If we do set this string, it should be displayed instead of the default message
	// based on just the error code.
	std::string			m_LastErrorDetail ;

	/*************************************************************
	* @brief Resets our "last error" to no error.
	*************************************************************/
	virtual void ClearError() ;

	/*************************************************************
	* @brief Records that an error has occurred
	*************************************************************/
	virtual void SetError(ErrorCode error) ;

	/*************************************************************
	* @brief Records that an error has occurred and we are overriding
	*		 the default message to go with it.
	*************************************************************/
	virtual void SetDetailedError(ErrorCode error, char const* pDetailedError) ;

public:
	/*************************************************************
	* @brief Returns true if an error occurred in the last call.
	*************************************************************/
	virtual bool	HadError() ;

	/*************************************************************
	* @brief Returns a description of the error that occurred in the last call.
	*************************************************************/
	virtual char const* GetLastErrorDescription() ;
	
	/*************************************************************
	* @brief Classes with virtual functions need a virtual destructor (gcc warning).
	*************************************************************/
	virtual ~ClientErrors() {}	
} ;

} ;	// End of namespace

#endif	// Header
