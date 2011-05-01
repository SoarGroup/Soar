%module PHP_sml_ClientInterface

%apply int { long long };

%newobject sml::Identifier::CreateIntWME(const char*,long long);
%newobject sml::Identifier::CreateFloatWME(const char*,double);
%newobject sml::Identifier::CreateStringWME(const char*,const char*);
%newobject sml::Identifier::CreateIdWME(const char*);
%newobject sml::Identifier::CreateSharedIdWME(const char*,Identifier*);

%newobject sml::Identifier::ConvertToIdentifier();
%newobject sml::Identifier::FindFromTimeTag(long long) const;
%newobject sml::Identifier::FindByAttribute(const char*, int) const;
%newobject sml::Identifier::GetChild(int);

%newobject sml::StringElement::ConvertToStringElement();
%newobject sml::IntElement::ConvertToIntElement();
%newobject sml::FloatElement::ConvertToFloatElement();

%{
	#include <list>
	#include <algorithm>
	
	#include "sml_ClientAgent.h"
	#include "sml_ClientEvents.h"
	
	struct PHPUserData {
		zval* func;
		zval* userdata;
		int callbackid;
		
		~PHPUserData()
		{
			FREE_ZVAL( func );
			FREE_ZVAL( userdata );
		}
	};
	
	std::list< PHPUserData* > callbackdatas;
	
	// calls func( id, userdata, agent_name, phase )
	void PHPRunEventCallback( sml::smlRunEventId id, void* pUserData, sml::Agent* pAgent, sml::smlPhase phase )
	{
		PHPUserData* pud = static_cast< PHPUserData* >( pUserData );
		
		zval** params[4];
		zval* retval;
		zval* event_id;
		zval* agent_name;
		zval* phase_id;
		
		MAKE_STD_ZVAL( event_id );
		ZVAL_LONG( event_id, id );
		params[0] = &event_id;
		
		params[1] = &pud->userdata;
		
		MAKE_STD_ZVAL( agent_name );
		ZVAL_STRING( agent_name, pAgent->GetAgentName(), 0 );
		params[2] = &agent_name;
		
		MAKE_STD_ZVAL( phase_id );
		ZVAL_LONG( phase_id, phase );
		params[3] = &phase_id;
		
		call_user_function_ex( CG(function_table), NULL, pud->func, &retval, 4, params, 0, NULL TSRMLS_CC );
		
		FREE_ZVAL( event_id );
		FREE_ZVAL( agent_name );
		FREE_ZVAL( phase_id );
	}
	
	// calls func( id, userdata, agent_name, prod, inst )
	void PHPProductionEventCallback( sml::smlProductionEventId id, void* pUserData, sml::Agent* pAgent, char const* pProdName, char const* pInstantiation )
	{
		PHPUserData* pud = static_cast< PHPUserData* >( pUserData );
		
		zval** params[5];
		zval* retval;
		zval* event_id;
		zval* agent_name;
		zval* prod_name;
		zval* inst_name;
		
		MAKE_STD_ZVAL( event_id );
		ZVAL_LONG( event_id, id );
		params[0] = &event_id;
		
		params[1] = &pud->userdata;
		
		MAKE_STD_ZVAL( agent_name );
		ZVAL_STRING( agent_name, pAgent->GetAgentName(), 0 );
		params[2] = &agent_name;
		
		MAKE_STD_ZVAL( prod_name );
		ZVAL_STRING( prod_name, pProdName, 0 );
		params[3] = &prod_name;
		
		MAKE_STD_ZVAL( inst_name );
		if ( pInstantiation != NULL )
		{
			ZVAL_STRING( inst_name, pInstantiation, 0 );
		}
		else 
		{
			ZVAL_NULL( inst_name );
		}
		params[4] = &inst_name;
		
		call_user_function_ex( CG(function_table), NULL, pud->func, &retval, 5, params, 0, NULL TSRMLS_CC );
		
		FREE_ZVAL( event_id );
		FREE_ZVAL( agent_name );
		FREE_ZVAL( prod_name );
		FREE_ZVAL( inst_name );
	}
	
	// calls func( id, userdata, agent_name, message )
	void PHPPrintEventCallback( sml::smlPrintEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessage )
	{
		PHPUserData* pud = static_cast< PHPUserData* >( pUserData );
		
		zval** params[4];
		zval* retval;
		zval* event_id;
		zval* agent_name;
		zval* msg;
		
		MAKE_STD_ZVAL( event_id );
		ZVAL_LONG( event_id, id );
		params[0] = &event_id;
		
		params[1] = &pud->userdata;
		
		MAKE_STD_ZVAL( agent_name );
		ZVAL_STRING( agent_name, pAgent->GetAgentName(), 0 );
		params[2] = &agent_name;
		
		MAKE_STD_ZVAL( msg );
		ZVAL_STRING( msg, pMessage, 0 );
		params[3] = &msg;

		call_user_function_ex( CG(function_table), NULL, pud->func, &retval, 4, params, 0, NULL TSRMLS_CC );

		FREE_ZVAL( event_id );
		FREE_ZVAL( agent_name );
		FREE_ZVAL( msg );
	}
	
	PHPUserData* CreatePHPUserData( zval* func, zval* userData ) 
	{
		PHPUserData* pud = new PHPUserData();
	    pud->func = func;
	    pud->userdata = userData;
	    
	    // Save the callback data so we can free it later
		callbackdatas.push_back(pud);
	    
	    return pud;
	}
	
	void ReleaseCallbackData( PHPUserData* pud ) 
	{
		// Release callback data and remove from collection of those we need to release at shutdown
		std::list< PHPUserData* >::iterator itr = find( callbackdatas.begin(), callbackdatas.end(), pud );
		if( itr != callbackdatas.end() ) 
		{
			callbackdatas.erase( itr );
			delete pud;
		}
    }
    
    bool IsValidCallbackData( PHPUserData* pud ) 
	{
		std::list< PHPUserData* >::iterator itr = find( callbackdatas.begin(), callbackdatas.end(), pud );
		
		if ( itr == callbackdatas.end() ) 
		{
			return false;
		} 
		else 
		{
			return true;
		}
    }
%}

%extend sml::Agent {
	
	long RegisterForRunEvent( sml::smlRunEventId id, const char* func_name, const char* userData, bool addToBack = true ) 
	{
		zval* func;
		zval* udata;
		
		MAKE_STD_ZVAL( func );
		ZVAL_STRING( func, func_name, 1 );
		
		MAKE_STD_ZVAL( udata );
		ZVAL_STRING( udata, userData, 1 );
		
		PHPUserData* pud = CreatePHPUserData( func, udata );
		pud->callbackid = self->RegisterForRunEvent( id, PHPRunEventCallback, (void*)pud, addToBack );
		
		return (long) pud;
	}
	
	bool UnregisterForRunEvent( long id ) 
	{
		PHPUserData* pud = (PHPUserData *) id;
		
		if( !IsValidCallbackData(pud) ) return false;
		
		self->UnregisterForRunEvent( pud->callbackid );
		ReleaseCallbackData(pud);
		
		return true;
    }
	
	//
	
	long RegisterForProductionEvent( sml::smlProductionEventId id, const char* func_name, const char* userData, bool addToBack = true ) 
	{
		zval* func;
		zval* udata;
		
		MAKE_STD_ZVAL( func );
		ZVAL_STRING( func, func_name, 1 );
		
		MAKE_STD_ZVAL( udata );
		ZVAL_STRING( udata, userData, 1 );
		
		PHPUserData* pud = CreatePHPUserData( func, udata );
		pud->callbackid = self->RegisterForProductionEvent( id, PHPProductionEventCallback, (void*)pud, addToBack );
		
		return (long) pud;
    }
	
	bool UnregisterForProductionEvent( long id ) 
	{
		PHPUserData* pud = (PHPUserData *) id;
		
		if( !IsValidCallbackData(pud) ) return false;
		
		self->UnregisterForProductionEvent( pud->callbackid );
		ReleaseCallbackData(pud);
		
		return true;
    }
	
	//
	
	long RegisterForPrintEvent( sml::smlPrintEventId id, const char* func_name, const char* userData, bool ignoreOwnEchos = true, bool addToBack = true)
	{	    
		zval* func;
		zval* udata;
		
		MAKE_STD_ZVAL( func );
		ZVAL_STRING( func, func_name, 1 );

		MAKE_STD_ZVAL( udata );
		ZVAL_STRING( udata, userData, 1 );

		PHPUserData* pud = CreatePHPUserData( func, udata );
		pud->callbackid = self->RegisterForPrintEvent( id, PHPPrintEventCallback, (void*)pud, ignoreOwnEchos, addToBack );
		
		return (long) pud;
	}
	
	bool UnregisterForPrintEvent( long id ) 
	{
		PHPUserData* pud = (PHPUserData *) id;
		
		if( !IsValidCallbackData(pud) ) return false;
		
		self->UnregisterForPrintEvent( pud->callbackid );
		ReleaseCallbackData(pud);
		
		return true;
    }
	
};

%exception Shutdown {
	$action
	
	// Release remaining PHPUserData's
	std::list< PHPUserData* >::iterator itr;
	for( itr=callbackdatas.begin(); itr!=callbackdatas.end(); itr++ )
	{
		delete (*itr);
	}
	callbackdatas.clear();
}


%include "../sml_ClientInterface.i"
