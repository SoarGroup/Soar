typedef struct agent_struct agent;
/*************************************************************************
 *************************************************************************/
extern agent * glbAgent;
#define current_agent(x) (glbAgent->x)
/************************************************************************/
/********************************************************************
created:	2001/09/04
created:	4:9:2001   22:50
filename: 	c:\dev\soar-84\port\lore_interface.cpp
file path:	c:\dev\soar-84\port
file base:	lore_interface
file ext:	cpp
author:		Jens Wessling

  purpose:	
*********************************************************************/

#ifdef _WIN32
#pragma warning (disable : 4786)
#endif

#ifdef ATTENTION_LAPSE
//void start_attention_lapse (long duration);
//void wake_from_attention_lapse ();
//void init_attention_lapse ();

#endif
#include "Loar_interface.h"

#ifdef _WIN32
#define CDECL __cdecl
#else
#define CDECL
#endif

#include "assert.h"

#  include "kernel.h"
#  include "init_soar.h"
#  include "print.h"
#  include "agent.h"
#  include "symtab.h"
#  include "init_soar.h"
#  include "gdatastructs.h"
#  include "production.h"
#  include "wmem.h"
#  include "io.h"

#include "soarapi.h"
#include "soar_core_api.h"
#include "soar_ecore_api.h"
#include "utilities.h"
#include "new_api.h"
#include "new_soar.h"

#include <map>
#include <string>
#include <utility>
#include <fstream>
#include <iostream>
#include "definitions.h"
#include "source_cmd.h"
#include "diff.h"

namespace Sym{
	class Symbol;
	class Sym_float_const;
}

typedef void * psoar_wme;

typedef struct {
	std::string fn;
	lua_State   *S;
} CallbackInfo;

typedef std::multimap<agent *, CallbackInfo> MappAgentLuaFn;


MappAgentLuaFn CallBackRemap;

#define assertSS(S,x)   assert(lua_gettop(S) == x);
#define assertTopStr(S) assert(lua_isstring(S, -1));
#define assertTopNum(S) assert(lua_isnumber(S, -1)); 
#define assertTopUD(S)  assert(lua_isuserdata(S, -1)); 
#define assertBool(S)   assert(lua_isnumber(S,-1) || lua_isnil(S,-1)); // any number is true, nil is false.
#define pop_top(S) lua_pop(S,1);


#define CREATE_LUA_SOAR_FUNC_STRING(NAME)  \
	int lua_##NAME(lua_State *S){                   \
	assertSS(S,1);                               \
	char *S1 = getString(S);                     \
        NAME(S1);                                  \
	delete[](S1);                                  \
	return 0;                                    \
}

#define CREATE_LUA_SOAR_FUNC_STRING_RET_INT(NAME)  \
	int lua_##NAME(lua_State *S){                   \
	assertSS(S,1);                               \
	char *S1 = getString(S);                     \
	int rv = NAME(S1);                         \
	delete[](S1);                                  \
	lua_pushnumber(S, rv);                       \
	return 1;                                    \
}


/*
===============================
getNumber
===============================
*/
double getNumber(lua_State *S){
	assertTopNum(S);
	double tmp = lua_tonumber(S, -1);
	pop_top(S);
	return tmp;
}

/*
===============================
print_cb
===============================
*/
//extern "C"void __cdecl print_cb(  soar_callback_agent a, 
//                                  soar_callback_data cb_data, 
//                                  soar_call_data call_data){
//   int x =5;
//}
//
/*
===============================
getBool
===============================
*/
Bool getBool(lua_State *S){
	Bool rv;
	assert(lua_isnumber(S,-1) || lua_isnil(S,-1));
	if(lua_isnil(S,-1)) rv =  false;
	else rv = true;
	pop_top(S);
	return rv;
}

/*
===============================
getUserdata
===============================
*/
template <class T>
T getUserdata(lua_State *S){
	T tmp;
	if(lua_isuserdata(S, -1)){
      
		tmp = /*lint -e(611) */(T)(lua_touserdata(S, -1)); 
		pop_top(S);
	} else if(lua_isnil(S, -1)){
		tmp = NULL; 
		pop_top(S);
	} else {
		assert(!"Not User Data!");
	}
	return tmp;
}

/*
===============================
lua_soar_cInitializeSoar
===============================
*/
int lua_soar_cInitializeSoar(lua_State *){  
	soar_cInitializeSoar();
	return 0;
}

/*
===============================
lua_soar_cReInitSoar
===============================
*/
int lua_soar_cReInitSoar(lua_State *){
	
    return soar_cReInitSoar();
}


/*
===============================
getString
===============================
*/
char *getString(lua_State *S){
	assertTopStr(S);
	const char *lua_tmp = lua_tostring(S, -1);
	char *tmp = new char[1+strlen(lua_tmp)];
	strcpy(tmp, lua_tmp);
	pop_top(S);
	return tmp;
}


char *_FN_=NULL;
/*
===============================
lua_soar_cCreateAgent
===============================
*/
int lua_soar_cCreateAgent(lua_State *S){ 
	assertSS(S,3);
	
	assertBool(S);
	
	bool op2Mode = getBool(S) != 0;
	
	char *logfile = getString(S);

	if(_FN_ != NULL) delete[](_FN_);
	_FN_ = logfile;
	
	char *S1 = getString(S);
	soar_cCreateAgent(S1, op2Mode);
	delete[](S1);
		
	return 0;
}

/*
===============================
toEnum
===============================
*/
template <class T1> 
T1 toEnum(double v){
	return(static_cast<T1>(static_cast<int>(v)));
}

/*
===============================
lua_soar_cRun
===============================
*/
int lua_soar_cRun(lua_State *S){
	assertSS(S,4);
	
	double slot_ = getNumber(S);
	double type = getNumber(S);
	Bool all = getBool(S);
	long n = static_cast<long>(getNumber(S));
	
	int rv = soar_cRun(n, all, toEnum<go_type_enum>(type), toEnum<soar_apiSlotType>(slot_));
	
	lua_pushnumber(S, static_cast<long>(rv));
	return 1;
}


/*
===============================
lua_soar_OSupportMode
===============================
*/
int lua_soar_OSupportMode(lua_State *S){
	char **tmp_string = new char*[2];
	
	assertSS(S,1);
	
	int n = static_cast<int>(getNumber(S));
	
	assert((n >= 0) && (n < 4));
	
	char x[11];
   sprintf(x,"%d",n);
	//itoa(n, x, 10);
	tmp_string[0] = x; // meaningless, this string is not used, so I set it here
	// to a known valid string.
	tmp_string[1] = x; // I should not be passing a local string to a function, but "what-the-hell"
	// I am going to do it anyway.  If you don't like it. Fix it. 8)
	
	soarResult result;
	soar_OSupportMode(2, tmp_string, &result);
   delete[](tmp_string);
	return 1;
}

/*
===============================
lua_soar_SaveBacktraces
===============================
*/
int lua_soar_SaveBacktraces(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);
	char * str = getString(S);

	if (!strcmp(str, "-on"))
	{
		printf("\nsave_backtraces on\n");
		set_sysparam(glbAgent, EXPLAIN_SYSPARAM, TRUE);
	}
	else if (!strcmp(str, "-off"))
	{
		printf("\nsave_backtraces off\n");
		set_sysparam(glbAgent, EXPLAIN_SYSPARAM, FALSE);
	}
	else
	{
		printf("\nBad value for save_backtraces\n");
	}

        delete[](str);

	return 0;
}

/*
===============================
lua_soar_ExplainBacktraces
===============================
*/
int lua_soar_ExplainBacktraces(lua_State *S)
{
	assertSS(S,1);
	assertTopStr(S);
	char * argv[40];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	
	soarResult result;

	lua_pushnumber(S, soar_ExplainBacktraces(argc, argv, &result));

        delete[](str);
	return 1;
}

/*
===============================
lua_soar_Memories
===============================
*/
int lua_soar_Memories(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);
	char * argv[20];
	int argc = 1;
	
	char *str = getString(S);
	parse_command_string(str, argv, argc);
	
	lua_pushnumber(S, soar_Memories(argv, argc));
        delete[](str);
	return 1;
}

/*
===============================
lua_soar_Stats
===============================
*/
int lua_soar_Stats(lua_State *S)
{ 
	assertSS(S,1);
	assertTopStr(S);
	char * argv[20];
	int argc = 1;
	
	char *str = getString(S);
	parse_command_string(str, argv, argc);
	
	soarResult result;
	lua_pushnumber(S, soar_Stats(argc, argv, &result));
	return 1;
}

/*
===============================
lua_soar_Pwatch
===============================
*/
int lua_soar_Pwatch(lua_State *S){
	
	char **tmp_string = new char*[40];
	
	assertSS(S,2);
	
	char *prod = getString(S);
	char *state = getString(S);
	
	
	
	tmp_string[0] = "";        // meaningless, this string is not used, so I set it here
	// to a known valid string.
	tmp_string[1] = state; // I should not be passing a local string to a function, but "what-the-hell"
	
	tmp_string[2] = prod; 
	
    // I am going to do it anyway.  If you don't like it. Fix it. 8)
	
    soarResult result;
	
	//Pwatch in all possible formats
	
	if (tmp_string[2]=="") {
		printf("\n Pwatch with no production\n");
		soar_PWatch(2, tmp_string, &result);
	}
	else {
		printf("\n Pwatch with production\n");
		soar_PWatch(3, tmp_string, &result);
	}
	
	delete[](tmp_string);
        delete[](state);
        delete[](prod);


	return 1;
}

/*
===============================
lua_soar_Watch
===============================
*/
int lua_soar_Watch(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);
	
	char * argv[20];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	
	int rv;
	soarResult res;
	rv = soar_Watch(argc, argv, &res);
	lua_pushnumber(S, rv);

        delete[](str);
	return 1;
}

/*
===============================
lua_soar_cExciseAllProductions
===============================
*/
int lua_soar_cExciseAllProductions(lua_State *S){
	
	char **tmp_string = new char*[40];
	
	assertSS(S,1);
	
	char *chunkname = getString(S);
	char *all = "-all";
	char *cond_num = "1";
	
	tmp_string[0] = "-all";        // meaningless, this string is not used, so I set it here
	// to a known valid string.
	tmp_string[1] = "-all";        // I should not be passing a local string to a function, but "what-the-hell"
	
	
	// I am going to do it anyway.  If you don't like it. Fix it. 8)
	
	soarResult result;
	printf("\nExcising \n");
	soar_Excise(2, tmp_string, &result);

   delete[](tmp_string);
        delete[](chunkname);
	
	return 0;
}


/*
===============================
lua_soar_cSaveReteNet
===============================
*/
int lua_soar_cSaveReteNet(lua_State *S){ 
	assertSS(S,1);
	
	char *retefile = getString(S);
	printf("\nSaving retenet \n");
	soar_cSaveReteNet(retefile);
	return 0;
	
}

/*
===============================
lua_soar_cLoadReteNet
===============================
*/
int lua_soar_cLoadReteNet(lua_State *S){ 
	assertSS(S,1);
	
	char *retefile = getString(S);
	
	printf("\nLoading retenet \n");
	soar_cLoadReteNet(retefile);
        delete[](retefile);
	return 0;   
}
/*
===============================
initEnums
===============================
*/
void initEnums(lua_State *S){
	int rv;
	rv=lua_dostring(S, " go_type_enum = { GO_PHASE=0, \
														GO_ELABORATION=1, \
														GO_DECISION=2, \
														GO_STATE=3, \
														GO_OPERATOR=4, \
														GO_SLOT=5, \
														GO_OUTPUT=6}");
	assert(rv == 0);
	
	rv=lua_dostring(S, " soar_apiSlotType = { NO_SLOT=0, \
														   STATE_SLOT=1,   \
														   OPERATOR_SLOT=2, \
														   SUPERSTATE_SLOT=3, \
														   SUPEROPERATOR_SLOT=4, \
														   SUPERSUPERSTATE_SLOT=5,  \
														   SUPERSUPEROPERATOR_SLOT=6 }");
	assert(rv == 0);
	
	rv=lua_dostring(S, "Bool = { TRUE=1, FALSE=0 }");
	assert(rv == 0);
	
	rv=lua_dostring(S, "TRUE=1 \n FALSE=0\n");
	assert(rv == 0);
}

/*
===============================
lua_soar_cStopAllAgents
===============================
*/
int lua_soar_cStopAllAgents(lua_State *S){
	assertSS(S,0);
	soar_cStopAllAgents();
	return 0;
}

/*
===============================
lua_soar_cDestroyAgentById
===============================
*/
int lua_soar_cDestroyAgentById(lua_State *S){
	int rv = soar_cDestroyAgentById(static_cast<int>(getNumber(S)));
	lua_pushnumber(S, static_cast<double>(rv));
	return 1;
}


static Bool show_output=true;

/*
===============================
print_to_screen
===============================
*/
int print_to_screen(lua_State *S)
{
	show_output = getBool(S);
	return 0;
}

/*
===============================
myprint
===============================
*/
extern "C" void CDECL myprint( soar_callback_agent a, 
								soar_callback_data cb_data, 
								soar_call_data call_data)
{ 
	if(show_output) printf("%s", call_data);
}

int COMPRESS=true;

/*
===============================
setCompression
===============================
*/
int setCompression(lua_State *S)
{
	Bool b = getBool(S);
	COMPRESS = b;
	return 0;
}

/*
===============================
mkfn
===============================
*/
std::string mkfn(std::string s1, const char *dir, Bool new_test=false){
	static int test_num=0;
	
	if(new_test) test_num++;
	
	char num[15];
   sprintf(num, "%04d", test_num);
   s1 = std::string(num) + "_" + s1;

   std::cout << s1 << std::endl;
	
#ifdef WIN32 
   std::string dir_separator("\\");
#else
   std::string dir_separator("/");
#endif

   s1 = "TestResults" + dir_separator + std::string(dir) + dir_separator + s1;
	return s1;
}


std::ofstream *fout = NULL;
/*
===============================
compress_print
===============================
*/
extern "C" void CDECL compress_print( soar_callback_agent a, 
				      soar_callback_data cb_data, 
				      soar_call_data call_data)
{ 
  std::string s1 = _FN_;
	
   if (!fout)
   {
     fout = new std::ofstream(mkfn(s1, "current", true).c_str());
   }
	
   //if(show_output) printf("%s", call_data);
   //char* tempstring = reinterpret_cast<char *>(call_data);
   // 
   // Removed the C++ style cast because it was making g++ freak out
   //
   *fout << (char*)call_data;
   fout->flush();
}

/*
===============================
closeFile
===============================
*/
int closeFile(lua_State *S)
{
  if (fout != NULL) { 
    fout->close();
    delete(fout);
    fout = NULL;
  }
  return 0;
}  


/*
===============================
lua_soar_cGetAgentByName
===============================
*/
int lua_soar_cGetAgentByName(lua_State *S)
{
	assertSS(S,1);
	char *c = getString(S);
	void *A = soar_cGetAgentByName(c);
	assert((A != NULL) && "Could not get agent by name.");
	lua_pushuserdata(S, A);
	delete[](c);
	
	//soar_cAddCallback(A, PRINT_CALLBACK, myprint,        NULL, NULL, "myprint");
	//soar_cAddCallback(A, PRINT_CALLBACK, compress_print, NULL, NULL, "compress_print");

	return 1;
}


CREATE_LUA_SOAR_FUNC_STRING(soar_cStopCurrentAgent)
//CREATE_LUA_SOAR_FUNC_STRING(soar_ecCaptureInput);
CREATE_LUA_SOAR_FUNC_STRING(soar_cDestroyAgentByName)
//CREATE_LUA_SOAR_FUNC_STRING(soar_ecReplayInput);
CREATE_LUA_SOAR_FUNC_STRING_RET_INT(soar_cDestroyAllAgentsWithName)


/*
===============================
lua_soar_errorHandler
===============================
*/
int lua_soar_errorHandler(lua_State *S){
	const char *msg = lua_tostring(S,-1);
	printf("Error: %s\n", msg);
	return 0;
}



/*
===============================
lua_soar_cSetCurrentAgent
===============================
*/
int lua_soar_cSetCurrentAgent(lua_State *S){
	assertSS(S, 1);
	
	//psoar_agent c = static_cast<psoar_agent>(lua_touserdata(S,-1));
	psoar_agent c = getUserdata<agent *>(S);
	assert(c != 0);
	soar_cSetCurrentAgent(c);
	return 0;
}




/*
===============================
lua_sourceProductionsFromFile
===============================
*/
int lua_sourceProductionsFromFile(lua_State *S){
	
	FILE *f;
	Bool eof_reached;
	
	const char *c = getString(S);
	
	f = fopen( c, "r" );
	
	if ( !f ) {
		/*lint -e(641)*/
		lua_pushnumber(S, SOAR_ERROR); 
		printf("Could not open file %s.", c);
                delete[](c);
		return 1;
	}
	
	eof_reached = FALSE;
	while( !eof_reached ) {
		char * tmp = getProductionFromFile( fgetc, f,  &eof_reached );
		//if(strstr(tmp, "default*pass-back-success") != 0){
		//	int v = 12312;
		//}
		loadProduction( tmp );
	}
	
	fclose( f );
	
	/*lint -e(641)*/
	lua_pushnumber(S, SOAR_OK); 
        delete[](c);
	return 1;
}

/*
===============================
lua_callback_translator
===============================
*/
extern "C" void CDECL lua_callback_translator( soar_callback_agent a, 
												soar_callback_data cb_data, 
												soar_call_data call_data){
	
	MappAgentLuaFn::iterator p;
	
	p = CallBackRemap.find(reinterpret_cast<agent *>(a));
	lua_State *S = p->second.S;
	lua_getglobal(S, p->second.fn.c_str());
	lua_pushuserdata(S, a);
	lua_pushuserdata(S, cb_data);
	lua_pushuserdata(S, call_data);
	lua_call(S, 3, 0);
	
	//const char *c = soar_cGetAgentOutputLinkId(a, NULL);
}

/*
===============================
void soar_cAddOutputFunction  (  
agent *  a,  
soar_callback_fn  f,  
soar_callback_data  cb_data,  
soar_callback_free_fn  free_fn,  
char *  output_link_name  
)  

  ===============================
*/
int lua_soar_cAddOutputFunction(lua_State *S)
{
	assertSS(S,5);
	
	char *name    = getString(S);
	soar_callback_free_fn free_fn = getUserdata<soar_callback_free_fn>(S);
	soar_callback_data cb_data = getUserdata<soar_callback_data>(S);
	const char *f = getString(S); // The Lua function name.
	agent *a = getUserdata<agent *>(S);
	
	CallbackInfo CBI;
	CBI.fn = f;
	CBI.S  = S;
	
	CallBackRemap.insert(std::make_pair(a, CBI));
	soar_cAddOutputFunction(a, lua_callback_translator, cb_data, free_fn, name);
	
	return 0;
}

/*
===============================
void soar_cAddInputFunction  (  
agent *  a,  
soar_callback_fn  f,  
soar_callback_data  cb_data,  
soar_callback_free_fn  free_fn,  
char *  name  
)  
===============================
*/
int lua_soar_cAddInputFunction(lua_State *S){
	assertSS(S,5);
	
	char *name    = getString(S);
	soar_callback_free_fn free_fn = getUserdata<soar_callback_free_fn>(S);
	soar_callback_data cb_data = getUserdata<soar_callback_data>(S);
	const char *f = getString(S); // The Lua function name.
	agent *a = getUserdata<agent *>(S);
	
	CallbackInfo CBI;
	CBI.fn = f;
	CBI.S  = S;
	
	CallBackRemap.insert(std::make_pair(a, CBI));
	
	soar_cAddInputFunction(a, lua_callback_translator, cb_data, free_fn, name);
	
	return 0;
}



/*
===============================
char* soar_cGetAgentOutputLinkId  (  psoar_agent a, char *buff)   
===============================
*/
int lua_soar_cGetAgentOutputLinkId(lua_State *S)
{
	assertSS(S, 1);
	psoar_agent a = getUserdata<psoar_agent>(S);
	char *d = soar_cGetAgentOutputLinkId(a, NULL, 0);
	
	lua_pushstring(S,d);
	
	free(d);
	return(1);
}

/*
===============================
soar_ecCaptureInput
===============================
*/
int lua_soar_ecCaptureInput(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);
	char * filename = getString(S);
	
	int rv = soar_ecCaptureInput(filename);
	lua_pushnumber(S, rv);
	
	return 1;
}

/*
===============================
tprint
===============================
*/
int tprint(lua_State *S){
	assertSS(S,1);
	char *c=getString(S);  
	print(glbAgent, "%s", c);
	delete(c);
	return 0;
}

/*
===============================
lua_soar_cAddWme

  unsigned long soar_cAddWme  (  char *  szId,   
  char *  szAttr,  
  char *  szValue,  
  Bool  accept,  
  psoar_wme *  new_wme)   
  ===============================
*/
int lua_soar_cAddWme(lua_State *S)
{
	Bool accept=false;
	if(lua_isnil(S, -1)){
		accept = false;
	} 
	else if (lua_isnumber(S, -1)) {
		accept = true;
	}
	pop_top(S); 
	char *szValue = getString(S);
	char *szAttr  = getString(S);
	char *szId    = getString(S);
	
	psoar_wme new_wme= new psoar_wme;
	
	int rv = soar_cAddWme( szId, szAttr, szValue, accept, &new_wme);
	
	delete[](szValue);
	delete[](szAttr);
	delete[](szId);
	lua_pushuserdata(S, (void *)(new_wme));
	lua_pushnumber(S, rv);
	
	return 2; 
}

/*
===============================
lua_soar_cRemoveWmeUsingTimetag
===============================
*/
int lua_soar_cRemoveWmeUsingTimetag(lua_State *S)
{
	assertSS(S,1);
	assertTopNum(S);
	
	int num = static_cast<int>(getNumber(S));
	int rv = soar_cRemoveWmeUsingTimetag(num);
	
	lua_pushnumber(S, rv);
	return 1;
}

/*
===============================
lua_soar_cSetOperand2
===============================
*/
int lua_soar_cSetOperand2(lua_State *S)
{
	assertSS(S,1);
	assertBool(S);
	
	int return_value;
	if(lua_isnil(S, -1)) 
		return_value = soar_cSetOperand2(false);
	else 
		return_value = soar_cSetOperand2(true);
	
	
	lua_pushnumber(S, return_value);
	
	return 1;
}

/*
===============================
lua_do_print_for_wme
===============================
*/
int lua_do_print_for_wme(lua_State *S){
	Bool internal = getBool(S);
	int   depth = static_cast<int>(getNumber(S));
	wme* m_wme = getUserdata<wme *>(S);
	
	do_print_for_wme(m_wme, depth, internal);
	return 0;
}

/*
===============================
lua_soar_memdump
===============================
*/
int lua_soar_memdump(lua_State *S)
{
	char str[] = "print -depth 255 S1";
	int   argc = 4;
	
	char **argv = new char*[argc];
	
	argv[0] = str; 
	str[5] = 0;
	argv[1] = &str[6]; 
	str[12] = 0;
	argv[2] = &str[13];
	str[16] = 0;
	argv[3] = &str[17];
	
	soarResult sr;     
	
	//
	// This writes the data to the compressed print function callback.
	soar_cPushCallback(glbAgent, PRINT_CALLBACK, compress_print, NULL, NULL);
	soar_Print(argc, argv, &sr);
	
	char str2[] = "print -stack";
	argc = 2;
	
	argv[0] = str2;
	str[5] = 0;
	argv[1] = &str2[6];
	soar_Print(argc, argv, &sr);
	
	soar_cPopCallback(glbAgent, PRINT_CALLBACK);
	
	
	delete[](argv);
	return(1);
}

/*
===============================
lua_soar_Matches
===============================
*/
int lua_soar_Matches(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);
	char * argv[40];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	lua_pushnumber(S, soar_Matches(argv, argc));

	return 1;
}

/*
===============================
lua_soar_cPrint
===============================
*/
int lua_soar_cPrint(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);
	char * argv[40];
	int argc = 1;
	
	soarResult sr;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	soar_Print(argc, argv, &sr);
	
	return 0;
}

/*
===============================
lua_soar_cQuit
===============================
*/
int lua_soar_cQuit(lua_State *S)
{
	soar_cQuit();
	return 0;
}

#ifdef ATTENTION_LAPSE

/*
===============================
lua_soar_Init_AttentionLapse
===============================
*/
int lua_soar_Init_AttentionLapse(lua_State *S)
{
	assertSS(S, 0);
	init_attention_lapse();
	return 0;
}

/*
===============================
lua_soar_AttentionLapse
===============================
*/
int lua_soar_AttentionLapse(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);
	char * str = getString(S);
	
	if (!strlen(str))
	{
		print_current_attention_lapse_settings();
	}
	else if (!strcmp(str, "-on"))
	{   
		set_sysparam (glbAgent, ATTENTION_LAPSE_ON_SYSPARAM, TRUE); 
		wake_from_attention_lapse();
	}
	else if (!strcmp(str, "-off"))
	{
		set_sysparam (glbAgent, ATTENTION_LAPSE_ON_SYSPARAM, FALSE); 
	}
	else
	{
		lua_pushnumber(S, SOAR_ERROR);
		return 1;
	}
	
	lua_pushnumber(S, SOAR_OK);
	return 1;
}

/*
===============================
lua_soar_StartAttentionLapse
===============================
*/
int lua_soar_StartAttentionLapse(lua_State *S)
{
	assertSS(S, 1);
	assertTopNum(S);
	
	long duration = getNumber(S);
	start_attention_lapse(duration);
	
	return 0;
}

/*
===============================
lua_soar_WakeFromAttentionLapse
===============================
*/
int lua_soar_WakeFromAttentionLapse(lua_State *S)
{
	assertSS(S, 0);
	wake_from_attention_lapse();
	return 0;
}

#endif

/*
===============================
lua_soar_IndifferentSelection
===============================
*/
int lua_soar_IndifferentSelection(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);

	char * argv[20];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	
	lua_pushnumber(S, soar_IndifferentSelection(argv, argc));
	return 1;
}

/*
===============================
lua_soar_GDS_Print
===============================
*/
int lua_soar_GDS_Print(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);

	char * argv[20];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	
	lua_pushnumber(S, soar_GDS_Print(argv, argc));
	return 1;
}

/*
===============================
lua_soar_InputPeriod
===============================
*/
int lua_soar_InputPeriod(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);

	char * argv[20];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	
	lua_pushnumber(S, soar_InputPeriod(argv, argc));
	return 1;
}

/*
===============================
lua_soar_InternalSymbols
===============================
*/
int lua_soar_InternalSymbols(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);

	char * argv[20];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	
	lua_pushnumber(S, soar_InternalSymbols(argv, argc));
	return 1;
}

/*
===============================
lua_soar_Soarnews
===============================
*/
int lua_soar_Soarnews(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);

	char * argv[20];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	
	lua_pushnumber(S, soar_Soarnews(argv, argc));
	return 1;
}

/*
===============================
lua_soar_Version
===============================
*/
int lua_soar_Version(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);

	char * argv[20];
	int argc = 1;

	char * str = getString(S);
	parse_command_string(str, argv, argc);

	lua_pushnumber(S, soar_Version(argv, argc));
	return 1;
}

/*
===============================
lua_soar_DefWmeDepth
===============================
*/
int lua_soar_DefWmeDepth(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);

	char * argv[20];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	
	lua_pushnumber(S, soar_DefWmeDepth(argv, argc));
	return 1;
}

/*
===============================
lua_soar_ChunkNameFormat
===============================
*/
int lua_soar_ChunkNameFormat(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);

	char * argv[20];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	
	lua_pushnumber(S, soar_ChunkNameFormat(argv, argc));
	return 1;
}

/*
===============================
lua_soar_MaxNilOutputCycles
===============================
*/
int lua_soar_MaxNilOutputCycles(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);

	char * argv[20];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	
	lua_pushnumber(S, soar_MaxNilOutputCycles(argv, argc));
	return 1;
}

/*
===============================
lua_soar_MaxElaborations
===============================
*/
int lua_soar_MaxElaborations(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);

	char * argv[20];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	
	lua_pushnumber(S, soar_MaxElaborations(argv, argc));
	return 1;
}

/*
===============================
lua_soar_MaxChunks
===============================
*/
int lua_soar_MaxChunks(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);

	char * argv[20];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	
	lua_pushnumber(S, soar_MaxChunks(argv, argc));
	return 1;
}

/*
===============================
lua_soar_FiringCounts
===============================
*/
int lua_soar_FiringCounts(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);

	char * argv[20];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	
	lua_pushnumber(S, soar_FiringCounts(argv, argc));
	return 1;
}

/*
===============================
lua_soar_FormatWatch
===============================
*/
int lua_soar_FormatWatch(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);

	char * argv[20];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	
	lua_pushnumber(S, soar_FormatWatch(argv, argc));
	return 1;
}

/*
===============================
lua_soar_Sp
===============================
*/
int lua_soar_Sp(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);
	char * str = getString(S);
	
	int argc = 2;
	char * argv[2];
	argv[1] = str;
		
	soarResult res;
	lua_pushnumber(S, soar_Sp(argc, argv, &res));
	return 1;
}

/*
===============================
lua_soar_ProductionFind
===============================
*/
int lua_soar_ProductionFind(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);

	char * argv[20];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	
	lua_pushnumber(S, soar_ProductionFind(argv, argc));
	return 1;
}

/*
===============================
lua_soar_Echo
===============================
*/
int lua_soar_Echo(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);

	char * argv[20];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	
	lua_pushnumber(S, soar_Echo(argv, argc));
	return 1;
}

/*
===============================
lua_soar_Learn
===============================
*/
int lua_soar_Learn(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);

	char * argv[20];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	
	lua_pushnumber(S, soar_Learn(argv, argc));
	return 1;
}

/*
===============================
lua_soar_Warnings
===============================
*/
int lua_soar_Warnings(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);

	char * argv[20];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	
	lua_pushnumber(S, soar_Warnings(argv, argc));
	return 1;
}

/*
===============================
lua_soar_Preferences
===============================
*/
int lua_soar_Preferences(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);

	char * argv[20];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	
	lua_pushnumber(S, soar_Preferences(argv, argc));
	return 1;
}

/*
===============================
lua_soar_AttributePreferencesMode
===============================
*/
int lua_soar_AttributePreferencesMode(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);
	char * argv[20];
	int argc = 1;
	
	char * str = getString(S);
	parse_command_string(str, argv, argc);
	
	lua_pushnumber(S, soar_AttributePreferencesMode(argv, argc));
	return 1;
}

/*
===============================
lua_soar_MultiAttributes
===============================
*/
int lua_soar_MultiAttributes(lua_State *S)
{
	assertSS(S, 1);
	assertTopStr(S);
	char * argv[20];
	int argc = 1;

	char * str = getString(S);
	parse_command_string(str, argv, argc);

	lua_pushnumber(S, soar_MultiAttributes(argv, argc));
	return 1;
}

/*
===============================
lua_soar_ecBuildInfo
===============================
*/
int lua_soar_ecBuildInfo(lua_State *S)
{
	soar_ecBuildInfo();
	return 0;
}


/*
===============================
pf
===============================
*/
int pf(lua_State *S){
	char *c=getString(S);
	if(show_output) printf("%s",c);
	delete[](c);
	return 0; 
}


/*
===============================
isHalted
===============================
*/
int isHalted(lua_State *S)
{
	assertSS(S, 1);
	agent *a = getUserdata<agent *>(S);
	
	if(a->system_halted){
		lua_pushnumber(S, 1);
	} else {
		lua_pushnil(S);
	}
	return 1;
}


/*
===============================
diff
===============================
*/
int diff(lua_State *S)
{
	std::string c_fn, cpp_fn;
	

   // This must be fixed to compare the proper versions of the kernel.  


	Bool rv = 1;

#ifdef _WIN32
   rv = _diff(mkfn(_FN_, "current").c_str(), mkfn(_FN_, "windows").c_str());
#else
   rv = _diff(mkfn(_FN_, "current").c_str(), mkfn(_FN_, "linux").c_str());
#endif
	
	if(rv){
		lua_pushnil(S);
		/* Commenting this out ensures that the uncompressed file will be preserved
		even if the outputs match. -ajc (5/14/02)
		lua_pushnumber(S,1);*/
	} else {
		lua_pushnil(S);
	}
	return 1;
}


/*
===============================
init_Lua_Soar
===============================
*/
int rm(lua_State *S){
	std::string s = mkfn(lua_tostring(S,-1), "common");
	remove(s.c_str());
	pop_top(S);
	return 0;
}



/*
===============================
init_Lua_Soar
===============================
*/ 
lua_State *init_Lua_Soar(void){
	lua_State *Lua = lua_open(1024);
	lua_baselibopen(Lua);
	lua_mathlibopen(Lua);
	
	initEnums(Lua);

	//
	// Register the Error handler
	lua_register(Lua, "_ERRORMESSAGE", lua_soar_errorHandler);
	lua_register(Lua, "tprint", tprint);
	
	lua_register(Lua, "soar_cInitializeSoar"           , lua_soar_cInitializeSoar);
	lua_register(Lua, "soar_cReInitSoar"               , lua_soar_cReInitSoar);
	lua_register(Lua, "soar_cCreateAgent"              , lua_soar_cCreateAgent);
	lua_register(Lua, "soar_cRun"                      , lua_soar_cRun);
	lua_register(Lua, "soar_cStopAllAgents"            , lua_soar_cStopAllAgents);
	lua_register(Lua, "soar_cStopCurrentAgent"         , lua_soar_cStopCurrentAgent);
	lua_register(Lua, "soar_cDestroyAgentByName"       , lua_soar_cDestroyAgentByName);
	lua_register(Lua, "soar_cDestroyAllAgentsWithName" , lua_soar_cDestroyAllAgentsWithName);
	lua_register(Lua, "soar_cDestroyAgentById"         , lua_soar_cDestroyAgentById);
	lua_register(Lua, "soar_cGetAgentByName"           , lua_soar_cGetAgentByName); 
	
	lua_register(Lua, "soar_cSetOperand2"              , lua_soar_cSetOperand2); 
	lua_register(Lua, "soar_OSupportMode"              , lua_soar_OSupportMode); 
	lua_register(Lua, "soar_SaveBacktraces"            , lua_soar_SaveBacktraces);
	lua_register(Lua, "soar_ExplainBacktraces"         , lua_soar_ExplainBacktraces); 
	lua_register(Lua, "soar_cSaveReteNet"              , lua_soar_cSaveReteNet);
	lua_register(Lua, "soar_cLoadReteNet"              , lua_soar_cLoadReteNet);
	lua_register(Lua, "soar_cExciseAllProductions"     , lua_soar_cExciseAllProductions);
	lua_register(Lua, "soar_Memories"                  , lua_soar_Memories);
	lua_register(Lua, "soar_Stats"                     , lua_soar_Stats); 
	lua_register(Lua, "soar_Pwatch"                    , lua_soar_Pwatch); 
	lua_register(Lua, "soar_Watch"                     , lua_soar_Watch);
	lua_register(Lua, "soar_cSetCurrentAgent"          , lua_soar_cSetCurrentAgent);
	lua_register(Lua, "sourceProductionsFromFile"      , lua_sourceProductionsFromFile);
	lua_register(Lua, "soar_cAddInputFunction"         , lua_soar_cAddInputFunction);
	lua_register(Lua, "soar_cAddOutputFunction"        , lua_soar_cAddOutputFunction);
	lua_register(Lua, "soar_cGetAgentOutputLinkId"     , lua_soar_cGetAgentOutputLinkId);
	lua_register(Lua, "soar_ecCaptureInput"            , lua_soar_ecCaptureInput);
//	lua_register(Lua, "soar_ecReplayInput"             , lua_soar_ecReplayInput);
	lua_register(Lua, "soar_cAddWme"                   , lua_soar_cAddWme);
	lua_register(Lua, "soar_cRemoveWmeUsingTimetag"    , lua_soar_cRemoveWmeUsingTimetag);
	lua_register(Lua, "do_print_for_wme"               , lua_do_print_for_wme);
	lua_register(Lua, "soar_memdump"                   , lua_soar_memdump);
	lua_register(Lua, "printf"                         , pf);
	lua_register(Lua, "soar_Matches"                   , lua_soar_Matches);
	lua_register(Lua, "soar_cPrint"                    , lua_soar_cPrint);
	lua_register(Lua, "soar_cQuit"                     , lua_soar_cQuit);
	
#ifdef ATTENTION_LAPSE
	lua_register(Lua, "soar_InitAttentionLapse"        , lua_soar_Init_AttentionLapse);
	lua_register(Lua, "soar_AttentionLapse"            , lua_soar_AttentionLapse);
	lua_register(Lua, "soar_WakeFromAttentionLapse"    , lua_soar_WakeFromAttentionLapse);
	lua_register(Lua, "soar_StartAttentionLapse"       , lua_soar_StartAttentionLapse);
#endif

	lua_register(Lua, "soar_IndifferentSelection"      , lua_soar_IndifferentSelection);
	lua_register(Lua, "soar_GDS_Print"                 , lua_soar_GDS_Print);
	lua_register(Lua, "soar_InputPeriod"               , lua_soar_InputPeriod);
	lua_register(Lua, "soar_InternalSymbols"           , lua_soar_InternalSymbols);
	lua_register(Lua, "soar_Soarnews"                  , lua_soar_Soarnews);
	lua_register(Lua, "soar_Version"                   , lua_soar_Version);
	lua_register(Lua, "soar_DefWmeDepth"               , lua_soar_DefWmeDepth);
	lua_register(Lua, "soar_ChunkNameFormat"           , lua_soar_ChunkNameFormat);
	lua_register(Lua, "soar_MaxNilOutputCycles"        , lua_soar_MaxNilOutputCycles);
	lua_register(Lua, "soar_MaxElaborations"           , lua_soar_MaxElaborations);
	lua_register(Lua, "soar_MaxChunks"                 , lua_soar_MaxChunks);
	lua_register(Lua, "soar_FormatWatch"               , lua_soar_FormatWatch);
	lua_register(Lua, "soar_FiringCounts"              , lua_soar_FiringCounts);
	lua_register(Lua, "soar_Sp"                        , lua_soar_Sp);
	lua_register(Lua, "soar_ProductionFind"            , lua_soar_ProductionFind);
	lua_register(Lua, "soar_Echo"                      , lua_soar_Echo);
	lua_register(Lua, "soar_Learn"                     , lua_soar_Learn);
	lua_register(Lua, "soar_Warnings"                  , lua_soar_Warnings);
	lua_register(Lua, "soar_Preferences"               , lua_soar_Preferences);
	lua_register(Lua, "soar_AttributePreferencesMode"  , lua_soar_AttributePreferencesMode);
	lua_register(Lua, "soar_MultiAttributes"           , lua_soar_MultiAttributes);
	lua_register(Lua, "soar_ecBuildInfo"               , lua_soar_ecBuildInfo);
	
	lua_register(Lua, "print_to_screen"                , print_to_screen);
	lua_register(Lua, "isHalted"                       , isHalted);
	lua_register(Lua, "setCompression"                 , setCompression);
	lua_register(Lua, "diff"                           , diff);
	lua_register(Lua, "closeFile"                      , closeFile);
	lua_register(Lua, "rm"                             , rm);
	
	return Lua;
}
