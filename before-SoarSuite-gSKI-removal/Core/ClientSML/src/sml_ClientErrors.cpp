#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

/////////////////////////////////////////////////////////////////
// Base class for setting and reporting errors.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
/////////////////////////////////////////////////////////////////

#include "sml_Errors.h"
#include "sml_ClientErrors.h"

using namespace sml ;

/*************************************************************
* @brief Resets the last error to no error
*************************************************************/
void ClientErrors::ClearError()
{
	m_LastError = Error::kNoError ;
	m_LastErrorDetail.clear() ;
}

/*************************************************************
* @brief Records that an error has occurred
*************************************************************/
void ClientErrors::SetError(sml::ErrorCode error)
{
	m_LastError = error ;
}

/*************************************************************
* @brief Records that an error has occurred and we are overriding
*		 the default message to go with it.
*************************************************************/
void ClientErrors::SetDetailedError(sml::ErrorCode error, char const* pDetailedError)
{
	m_LastError = error ;
	m_LastErrorDetail = pDetailedError ;
}

/*************************************************************
* @brief Returns true if an error occurred in the last call.
*************************************************************/
bool ClientErrors::HadError()
{ 
	return m_LastError != Error::kNoError ;
}

/*************************************************************
* @brief Returns a description of the error that occurred in the last call.
*************************************************************/
char const* ClientErrors::GetLastErrorDescription()
{
	if (m_LastErrorDetail.empty())
		return Error::GetErrorDescription(m_LastError) ;
	else
		return m_LastErrorDetail.c_str() ;
}
