
/////////////////////////////////////////////////////////
//
// Soar2Soar
//
// Author: Nate Derbinsky, nlderbin@umich.edu
// Date  : 2009
//
// Soar2Soar facilitates fast Soar environment
// prototyping by allowing a Soar agent to serve
// as the environment to one or more other agents.
//
/////////////////////////////////////////////////////////

#include <portability.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cassert>
#include <ctime>

#include "sml_Client.h"
#include "sml_Names.h"

using namespace std;
using namespace sml;

typedef map< string, string > sym2symMap;
typedef map< string, set<Identifier*> > symWMEMap;
typedef map< long, long > timetagMap;

class AgentState
{
public:	
	AgentState():
		id( 0 ), pAgent( NULL ), pEnvInRoot( NULL ), pEnvInCommands( NULL ), pEnvOutRoot( NULL ), pEnvOutInput( NULL ), pEnvOutFeedback( NULL )
	{
		name.assign( "" );
	}
	
	long id;
	string name;
	
	Agent* pAgent;
	Identifier* pEnvInRoot;
	Identifier* pEnvInCommands;
	Identifier* pEnvOutRoot;
	Identifier* pEnvOutInput;
	Identifier* pEnvOutFeedback;

	sym2symMap symFromEnv;
	symWMEMap wmeIn;
	timetagMap ttFromEnv;

	sym2symMap symToEnv;
	symWMEMap wmeEnv;
	timetagMap ttToEnv;

	sym2symMap symToEnvCheck;
	sym2symMap symToEnvRev;
};

typedef map< string, AgentState* > sym2agentMap;
typedef map< long, Identifier** > timetag2idMap;

class EnvState
{
public:
	Agent* pEnv;

	Identifier* console;
	Identifier* agentsIn;
	Identifier* agentsOut;
	IntElement* time;

	timetag2idMap tt2Id;
	sym2agentMap out2Agent;
	sym2agentMap feedback2Agent;

	long runningAgents;
};

typedef vector< AgentState > AgentList;

class Soar2SoarState
{
public:
	EnvState env;
	AgentList agents;
};

// Conversion from string to value
template <class T> bool fromString( T& val, const string& str )
{
	istringstream i( str );
	i >> val;
	return !i.fail();
}

// Conversion of value to string
template<class T> string& toString( const T& x, string& dest )
{
	static ostringstream o;
	
	// get value into stream
	o << setprecision( 16 ) << x;
	
	dest.assign( o.str() );
	o.str("");
	return dest;
}

// usage statement
void usage( const char* pName )
{
	cout << "usage: " << pName << " <# agents> [environment source] [a1 source] [a2 source] ..." << endl;
}

void syncWME( WMElement* delta, bool wasAdded, string deltaIdName, sym2symMap* ssMap, symWMEMap* swMap, timetagMap* ttMap, sym2symMap* ssMapC = NULL )
{
	// find the corresponding identifier in the agent's namespace
	sym2symMap::iterator s_p = ssMap->find( deltaIdName );
	if ( s_p == ssMap->end() )
	{
		return;
	}

	symWMEMap::iterator w_p = swMap->find( s_p->second );
	assert( w_p != swMap->end() );
	assert( !w_p->second.empty() );
	Identifier* destId = (*w_p->second.begin());

	if ( wasAdded )
	{
		WMElement* newbie = NULL;

		//

		if ( delta->GetValueType() == sml_Names::kTypeInt )
		{
			newbie = destId->CreateIntWME( delta->GetAttribute(), delta->ConvertToIntElement()->GetValue() );
		}
		else if ( delta->GetValueType() == sml_Names::kTypeDouble )
		{
			newbie = destId->CreateFloatWME( delta->GetAttribute(), delta->ConvertToFloatElement()->GetValue() );
		}
		else if ( delta->GetValueType() == sml_Names::kTypeString )
		{
			newbie = destId->CreateStringWME( delta->GetAttribute(), delta->ConvertToStringElement()->GetValue() );
		}
		else if ( delta->GetValueType() == sml_Names::kTypeID )
		{
			string valueName( delta->GetValueAsString() );
			sym2symMap::iterator p = ssMap->find( valueName );

			string valueEquivName;

			// new value vs. old
			if ( p == ssMap->end() )
			{
				newbie = destId->CreateIdWME( delta->GetAttribute() );
				valueEquivName.assign( newbie->GetValueAsString() );

				(*ssMap)[ valueName ] = valueEquivName;

				if ( ssMapC )
				{
					(*ssMapC)[ valueName ] = valueEquivName;
				}
			}
			else
			{					
				valueEquivName = p->second;
				
				symWMEMap::iterator p = swMap->find( valueEquivName );
				assert( p != swMap->end() );
				assert( !p->second.empty() );

				newbie = destId->CreateSharedIdWME( delta->GetAttribute(), (*p->second.begin()) );
			}

			(*swMap)[ valueEquivName ].insert( newbie->ConvertToIdentifier() );
		}

		//

		assert( newbie );

		(*ttMap)[ delta->GetTimeTag() ] = newbie->GetTimeTag();
	}
	else
	{
		timetagMap::iterator t = ttMap->find( delta->GetTimeTag() );
		assert( t != ttMap->end() );

		WMElement* oldie = destId->FindFromTimeTag( t->second );
		assert( oldie );			

		// if identifier, deal with mappings
		if ( delta->GetValueType() == sml_Names::kTypeID )
		{
			string valueName( delta->GetValueAsString() );
			string valueEquivName;

			{
				// find corresponding symbol
				sym2symMap::iterator s = ssMap->find( valueName );
				assert( s != ssMap->end() );
				valueEquivName.assign( s->second );

				// find symbol example set
				symWMEMap::iterator p = swMap->find( valueEquivName );
				assert( p != swMap->end() );

				// find wme in example set, remove
				set<Identifier*>::iterator wp = p->second.find( oldie->ConvertToIdentifier() );
				assert( wp != p->second.end() );
				p->second.erase( wp );

				// if no more examples, symbol not in use
				if ( p->second.empty() )
				{
					swMap->erase( p );
					ssMap->erase( s );
				}
			}
		}

		// remove timetag
		ttMap->erase( t );

		// remove wme
		oldie->DestroyWME();
	}
}

void onHalt( smlRunEventId id, void* pUserData, Agent* pAgent, smlPhase phase )
{
	EnvState* env = (EnvState*) pUserData;

	env->runningAgents--;
	if ( !env->runningAgents )
	{
		env->pEnv->StopSelf();
	}
}

// on update
void onUpdate( smlUpdateEventId eventId, void* userData, Kernel* pKernel, smlRunFlags )
{	
	Soar2SoarState* s2s = (Soar2SoarState*) userData;

	// we do direct manipulation of the output-link,
	// so we need it to exist before doing ANYTHING
	if ( s2s->env.pEnv->GetOutputLink() )
	{		
		// first time, setup all structures
		// note: due to weirdness in SML/timing,
		// at this stage we get internal timetags,
		// once the changes propogate, we get pointers
		// to "true" WMEs (output-link only)
		if ( !s2s->env.agentsIn )
		{			
			Identifier* agentsOut;
			
			s2s->env.console = s2s->env.pEnv->GetInputLink()->CreateIdWME( "console" );
			s2s->env.agentsIn = s2s->env.pEnv->GetInputLink()->CreateIdWME( "agents" );

			s2s->env.time = s2s->env.console->CreateIntWME( "time", 0 );

			agentsOut = s2s->env.pEnv->GetOutputLink()->CreateIdWME( "agents" );
			s2s->env.tt2Id[ agentsOut->GetTimeTag() ] = &( s2s->env.agentsOut );

			// create structures for each agent
			Identifier* pEnvOutRoot;			
			Identifier* pEnvOutInput;
			Identifier* pEnvOutFeedback;
			{				
				for ( AgentList::iterator p=s2s->agents.begin(); p!=s2s->agents.end(); p++ )
				{
					// agent (in)
					(*p).pEnvInRoot = s2s->env.agentsIn->CreateIdWME( "agent" );					

					// agent (out)
					pEnvOutRoot = agentsOut->CreateIdWME( "agent" );
					s2s->env.tt2Id[ pEnvOutRoot->GetTimeTag() ] = &( (*p).pEnvOutRoot );

					// agent id (in/out)
					(*p).pEnvInRoot->CreateIntWME( "id", (*p).id );
					pEnvOutRoot->CreateIntWME( "id", (*p).id );

					// agent name (in/out)
					(*p).pEnvInRoot->CreateStringWME( "name", (*p).name.c_str() );
					pEnvOutRoot->CreateStringWME( "name", (*p).name.c_str() );

					// agent commands (in)
					(*p).pEnvInCommands = (*p).pEnvInRoot->CreateIdWME( "commands" );
					//s2s->env.tt2Id[ pEnvInCommands->GetTimeTag() ] = &( (*p).pEnvInCommands );

					// agent input (out)
					pEnvOutInput = pEnvOutRoot->CreateIdWME( "input" );
					s2s->env.tt2Id[ pEnvOutInput->GetTimeTag() ] = &( (*p).pEnvOutInput );

					// agent feedback (out)
					pEnvOutFeedback = pEnvOutRoot->CreateIdWME( "feedback" );
					s2s->env.tt2Id[ pEnvOutFeedback->GetTimeTag() ] = &( (*p).pEnvOutFeedback );
				}
			}
		}
		else
		{
			// if still need to capture correct ids from above
			if ( !s2s->env.agentsOut )
			{
				// step 1: get all flagged identifiers
				{
					WMElement* delta;
					bool wasAdded;
					timetag2idMap::iterator tt_p;
					
					for ( int i=0; i<s2s->env.pEnv->GetNumberOutputLinkChanges(); i++ )
					{
						delta = s2s->env.pEnv->GetOutputLinkChange( i );
						wasAdded = s2s->env.pEnv->IsOutputLinkChangeAdd( i );

						if ( wasAdded )
						{
							tt_p = s2s->env.tt2Id.find( delta->GetTimeTag() );
							if ( tt_p != s2s->env.tt2Id.end() )
							{
								(*tt_p->second) = delta->ConvertToIdentifier();
							}
						}
					}					

					s2s->env.tt2Id.clear();
				}

				// step 2: establish id->agent mapping for out.agents.agent.input->input-link 
				// (at this point we are guaranteed the environment has an output-link, agents wait)
				{
					string temp, temp2;
					
					for ( AgentList::iterator p=s2s->agents.begin(); p!=s2s->agents.end(); p++ )
					{
						temp.assign( (*p).pEnvOutInput->GetValueAsString() );
						temp2.assign( (*p).pAgent->GetInputLink()->GetValueAsString() );
						
						s2s->env.out2Agent[ temp ] = &( (*p) );
						(*p).symFromEnv[ temp ] = temp2;
						(*p).wmeIn[ temp2 ].insert( (*p).pAgent->GetInputLink() );												
					}
				}

				// step 3: establish id->agent mapping for out.agents.agent.feedback				
				{
					string temp;
					
					for ( AgentList::iterator p=s2s->agents.begin(); p!=s2s->agents.end(); p++ )
					{
						temp.assign( (*p).pEnvOutFeedback->GetValueAsString() );						
						
						s2s->env.feedback2Agent[ temp ] = &( (*p) );						
					}
				}
			}

			// bring relevant identifiers up to date
			{
				AgentList::iterator p;
				sym2symMap::iterator s;
				symWMEMap::iterator w;

				const char* temp;
				string temp2;
				set<Identifier*> temp3;
				
				for ( p=s2s->agents.begin(); p!=s2s->agents.end(); p++ )
				{
					for ( s=(*p).symToEnvCheck.begin(); s!=(*p).symToEnvCheck.end(); s++ )
					{
						temp = s2s->env.pEnv->ConvertIdentifier( s->second.c_str() );						
						assert( temp );

						temp2.assign( temp );
						(*p).symToEnv[ s->first ] = temp2;

						(*p).symToEnvRev[ temp2 ] = s->first;

						w = (*p).wmeEnv.find( s->second );
						assert( w != (*p).wmeEnv.end() );
						temp3 = w->second;
						(*p).wmeEnv.erase( w );
						(*p).wmeEnv[ temp2 ] = temp3;
					}

					(*p).symToEnvCheck.clear();
				}
			}

			// set env time
			{
				s2s->env.time->Update( static_cast<int>( clock() / CLOCKS_PER_SEC ) );
			}

			// let's sync!
			{
				WMElement* delta;
				bool wasAdded;
				
				sym2agentMap::iterator a_p;
				string deltaIdName;
				string deltaValueName;
				
				// env->agents
				{
					for ( int i=0; i<s2s->env.pEnv->GetNumberOutputLinkChanges(); i++ )
					{
						delta = s2s->env.pEnv->GetOutputLinkChange( i );
						wasAdded = s2s->env.pEnv->IsOutputLinkChangeAdd( i );
						deltaIdName.assign( delta->GetIdentifierName() );

						// determine if the change affects an agent directly via sync (or feedback)
						{
							a_p = s2s->env.out2Agent.find( deltaIdName );
							if ( a_p != s2s->env.out2Agent.end() )
							{
								if ( delta->GetValueType() == sml_Names::kTypeID )
								{
									deltaValueName.assign( delta->ConvertToIdentifier()->GetValueAsString() );
									s2s->env.out2Agent[ deltaValueName ] = a_p->second;
								}
								
								syncWME( delta, wasAdded, deltaIdName, &( a_p->second->symFromEnv ), &( a_p->second->wmeIn ), &( a_p->second->ttFromEnv ) );
							}
							else if ( wasAdded && delta->IsIdentifier() )
							{								
								a_p = s2s->env.feedback2Agent.find( deltaIdName );
								if ( a_p != s2s->env.feedback2Agent.end() )
								{
									bool goodFeedback = false;

									if ( ( !strcmp( "add-wme", delta->GetAttribute() ) ) &&
										( delta->ConvertToIdentifier()->FindByAttribute( "attr", 0 ) ) &&
										( delta->ConvertToIdentifier()->FindByAttribute( "value", 0 ) ) &&
										( delta->ConvertToIdentifier()->FindByAttribute( "id", 0 ) ) &&
										( delta->ConvertToIdentifier()->FindByAttribute( "id", 0 )->IsIdentifier() ) )
									{
										string idValueName( delta->ConvertToIdentifier()->FindByAttribute( "id", 0 )->ConvertToIdentifier()->GetValueAsString() );
										sym2symMap::iterator s = a_p->second->symToEnvRev.find( idValueName );
										if ( s != a_p->second->symToEnvRev.end() )
										{
											string cmd( "add-wme " );
											cmd.append( s->second );
											cmd.append( " ^" );
											cmd.append( delta->ConvertToIdentifier()->FindByAttribute( "attr", 0 )->GetValueAsString() );
											cmd.append( " " );
											cmd.append( delta->ConvertToIdentifier()->FindByAttribute( "value", 0 )->GetValueAsString() );

											a_p->second->pAgent->ExecuteCommandLine( cmd.c_str() );
											goodFeedback = true;
										}
									}								

									if ( goodFeedback )
									{
										delta->ConvertToIdentifier()->CreateStringWME( "status", "complete" );
									}
									else
									{
										delta->ConvertToIdentifier()->CreateStringWME( "status", "failure" );
									}
								}								
							}
						}
					}
				}				

				// agents -> env
				{
					string temp, temp2;
					
					for ( AgentList::iterator p=s2s->agents.begin(); p!=s2s->agents.end(); p++ )
					{
						// check up on initial agent mappings (occurs on agent's first change to output-link)
						{							
							if ( (*p).symToEnv.empty() && (*p).pAgent->GetOutputLink() )
							{
								temp.assign( (*p).pEnvInCommands->GetValueAsString() );
								temp2.assign( (*p).pAgent->GetOutputLink()->GetValueAsString() );

								(*p).symToEnv[ temp2 ] = temp;
								(*p).symToEnvCheck[ temp2 ] = temp;
								(*p).wmeEnv[ temp ].insert( (*p).pEnvInCommands );
							}
						}
						
						for ( int i=0; i<(*p).pAgent->GetNumberOutputLinkChanges(); i++ )
						{
							delta = (*p).pAgent->GetOutputLinkChange( i );
							wasAdded = (*p).pAgent->IsOutputLinkChangeAdd( i );
							deltaIdName.assign( delta->GetIdentifierName() );

							syncWME( delta, wasAdded, deltaIdName, &( (*p).symToEnv ), &( (*p).wmeEnv ), &( (*p).ttToEnv ), &( (*p).symToEnvCheck ) );
						}

						(*p).pAgent->ClearOutputLinkChanges();
						(*p).pAgent->Commit();
					}
				}
			}
		}

		s2s->env.pEnv->ClearOutputLinkChanges();
		s2s->env.pEnv->Commit();
	}
}

int main( int argc, const char* argv[] )
{		
	// validate inputs
	int numAgents = 0;
	string envSource( "" );
	vector< string > agentSources;
	{
		int i = 0;
		
		if ( argc < 2 )
		{
			usage( argv[0] );
			return 0;
		}
	
		string temp( argv[1] );
		if ( !fromString<int>( numAgents, temp ) || ( numAgents < 1 ) )
		{
			usage( argv[0] );
			return 0;
		}
		
		// initialize sources
		for ( i=0; i<numAgents; i++ )
		{
			agentSources.push_back( "" );
		}
		
		if ( argc >= 3 )
		{
			// means there must be an environmental agent
			envSource.assign( "source " );
			envSource.append( argv[2] );
			
			if ( argc > 3 )
			{
				// three cases:
				// 1) numAgents == agentSources: one-to-one unique correspondance
				// 2) agentSources == 1: everyone gets first
				// 3) error
				
				if ( numAgents == ( argc - 3 ) )
				{
					for ( i=3; i<argc; i++ )
					{
						agentSources[ i - 3 ].assign( "source " );
						agentSources[ i - 3 ].append( argv[ i ] );
					}
				}
				else if ( argc == 4 )
				{
					for ( i=0; i<numAgents; i++ )
					{
						agentSources[ i ].assign( "source " );
						agentSources[ i ].append( argv[3] );
					}
				}
				else
				{
					usage( argv[0] );
					return 0;
				}
			}
		}		
	}
	
	// create kernel
	Kernel* pKernel = Kernel::CreateKernelInNewThread();
	if ( pKernel->HadError() )
	{
		cout << pKernel->GetLastErrorDescription() << endl;
		return 0;
	}

	// create state
	Soar2SoarState s2s;

	// create environment agent
	{
		s2s.env.pEnv = pKernel->CreateAgent( "env" );
		if ( pKernel->HadError() )
		{
			cout << pKernel->GetLastErrorDescription() << endl;
			return 0;
		}

		s2s.env.console = NULL;
		s2s.env.agentsIn = NULL;
		s2s.env.agentsOut = NULL;

		s2s.env.runningAgents = numAgents;

		s2s.env.pEnv->ExecuteCommandLine( "sp { soar2soar*init (state <s> -^soar2soar ready ^io.output-link <out>) --> (<out> ^soar2soar initialized) }" );
		s2s.env.pEnv->ExecuteCommandLine( "sp { soar2soar*init*agent (state <s> -^soar2soar ready ^io.output-link.agents.agent.input <in>) --> (<in> ^soar2soar initialized) }" );
		s2s.env.pEnv->ExecuteCommandLine( "sp { soar2soar*propose*ready (state <s> -^soar2soar ready ^io.input-link.agents <agents>) -{(<agents> ^agent <agent>) (<agent> ^commands <commands>) -(<commands> ^initialization ack)} --> (<s> ^operator <op> +) (<op> ^name soar2soar-init) }" );
		s2s.env.pEnv->ExecuteCommandLine( "sp { soar2soar*apply*ready (state <s> ^operator.name soar2soar-init) --> (<s> ^soar2soar ready) }" );
		
		if ( !envSource.empty() )
		{
			s2s.env.pEnv->ExecuteCommandLine( envSource.c_str() );
		}
	}

	// prep environment for agents	
	{
		string temp, temp2;
		AgentState newState;
		Agent *newAgent;
		
		for ( long i=1; i<=numAgents; i++ )
		{
			// create agent name by appending
			// the letter "a" to the agent id
			{
				temp.clear();
				temp.assign( "a" );

				toString( i, temp2 );
				temp.append( temp2 );
			}

			// create a new soar agent
			{
				newAgent = pKernel->CreateAgent( temp.c_str() );
				if ( pKernel->HadError() )
				{
					cout << pKernel->GetLastErrorDescription() << endl;
					return 0;
				}

				newAgent->ExecuteCommandLine( "sp { soar2soar*init*propose*ack (state <s> -^soar2soar ready ^io.input-link.soar2soar initialized) --> (<s> ^operator <op> +) (<op> ^name soar2soar-ack) }" );
				newAgent->ExecuteCommandLine( "sp { soar2soar*init*apply*ack (state <s> ^operator.name soar2soar-ack ^io.output-link <out>) --> (<out> ^initialization ack) }" );
				newAgent->ExecuteCommandLine( "sp { soar2soar*init*propose*ready (state <s> -^soar2soar ready ^io <io>) (<io> ^output-link.initialization ack -^input-link.soar2soar initialized) --> (<s> ^operator <op> +) (<op> ^name soar2soar-ready) }" );
				newAgent->ExecuteCommandLine( "sp { soar2soar*init*apply*ready (state <s> ^operator.name soar2soar-ready ^io.output-link <out>) --> (<out> ^initialization ack -) (<s> ^soar2soar ready) }" );
				
				if ( !agentSources[ i - 1 ].empty() )
				{
					newAgent->ExecuteCommandLine( agentSources[ i - 1 ].c_str() );
				}

				newAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_HALTED, onHalt, &( s2s.env ) );
			}

			// add state
			{
				newState.id = i;
				newState.name.assign( temp );
				newState.pAgent = newAgent;

				s2s.agents.push_back( newState );
			}
		}
	}

	// register for updates
	pKernel->RegisterForUpdateEvent( sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES, onUpdate, &s2s );

	// provide console
	{
		string input;
		int i;
		bool done = false;
		bool innerDone = false;
		Agent* pCurrent = NULL;

		do
		{
			cout << "root> ";
			
			getline( cin, input );

			done = !input.compare( "quit" );
			if ( !done )
			{
				pCurrent = NULL;
				
				if ( !input.compare( s2s.env.pEnv->GetAgentName() ) )
				{
					pCurrent = s2s.env.pEnv;
				}
				else
				{
					for ( i=0; i<s2s.agents.size(); i++ )
					{
						if ( !input.compare( s2s.agents[i].name ) )
						{
							pCurrent = s2s.agents[i].pAgent;
						}
					}
				}

				if ( pCurrent )
				{
					do
					{
						cout << pCurrent->GetAgentName() << "> ";
						getline( cin, input );

						done = !input.compare( "quit" );
						if ( !done )
						{
							innerDone = !input.compare( "root" );

							if ( !innerDone )
							{
								cout << pCurrent->ExecuteCommandLine( input.c_str() ) << endl;
							}
						}

					} while ( !done && !innerDone );
				}
			}

		} while ( !done );
	}

	// cleanup on quit
	pKernel->Shutdown();
	delete pKernel;
	
	return 0;
}


