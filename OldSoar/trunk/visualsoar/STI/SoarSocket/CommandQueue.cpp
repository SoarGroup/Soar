// CommandQueue.cpp: implementation of the CTCommandQueue class.
//
//////////////////////////////////////////////////////////////////////

#include "CommandQueue.h"
#include "Command.h"
#include "Check.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTCommandQueue::CTCommandQueue()
{

}

CTCommandQueue::~CTCommandQueue()
{

}

void CTCommandQueue::DeleteContents()
{
	Lock() ;

	// Empty the CTCommand list
	for (CommandIter i = m_Commands.begin() ; i != m_Commands.end() ; i++)
	{
		CTCommand* pCTCommand = *i ;
		delete pCTCommand ;
	}

	m_Commands.clear() ;

	Unlock() ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CopyCommandAndPushBack
// 
// Return type    : bool 	
// Argument       : CTCommand& command	
// 
// Description	  : Add a Command to the queue.
//
/////////////////////////////////////////////////////////////////////
bool CTCommandQueue::CopyCommandAndPushBack(CTCommand& command)
{
	// Copy the CTCommand when adding to the stack
	// so we own this version.
	CTCommand* pCommand = new CTCommand ;
	pCommand->CopyFromCommand(command) ;

	// Make sure access to queue is thread safe.
	Lock() ;

	// Add the commmand to the queue.  It will be sent later.
	m_Commands.push_back(pCommand) ;

	// Allow others to access the queue
	Unlock() ;

	return true ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : GetFrontCommand
// 
// Return type    : bool 	
// Argument       : CTCommand& command	
// 
// Description	  : Returns the CTCommand at the front of the incoming queue
//					or false if no CTCommand is pending.
//
/////////////////////////////////////////////////////////////////////
bool CTCommandQueue::GetFrontCommand(CTCommand& command)
{
	// Check if the list of CTCommands is empty
	if (m_Commands.begin() == m_Commands.end())
		return false ;

	// Make this thread safe
	Lock() ;

	// Copy the CTCommand at the front of the queue into the return structure
	CTCommand* pCommand = m_Commands.front() ;
	command.CopyFromCommand(*pCommand) ;

	// Release the lock on the queue
	Unlock() ;

	return true ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : GetFrontCommandPointer
// 
// Return type    : CTCommand const*
// 
// Description	  : Returns a pointer to the command at the front
//					of the queue.
//
//					NOTE: This is faster than GetCommand (which copies the data)
//					but to make it thread-safe the caller must call Lock
//					and Unlock themselves while they are using the pointer
//					or another caller could in theory modify the data.
//
/////////////////////////////////////////////////////////////////////
CTCommand const* CTCommandQueue::GetFrontCommandPointer()
{
	// Check if the list of CTCommands is empty
	if (m_Commands.begin() == m_Commands.end())
		return NULL ;

	// Return pointer to the command at the front of the queue
	CTCommand* pCommand = m_Commands.front() ;

	return pCommand ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : PopFrontCommand
// 
// Return type    : void 	
// 
// Description	  : Remove the CTCommand at the front of the queue.
//
/////////////////////////////////////////////////////////////////////
void CTCommandQueue::PopFrontCommand()
{
	// Shouldn't be popping if the list is empty
	CHECK(m_Commands.begin() != m_Commands.end()) ;

	Lock() ;

	// Get a pointer to the CTCommand so we can free it.
	CTCommand* pCommand = m_Commands.front() ;

	// Pop it off the queue
	m_Commands.pop_front() ;

	// Release the memory
	delete pCommand ;
	pCommand=NULL;

	Unlock() ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : IsCommandAvailable
// 
// Return type    : bool 	
// 
// Description	  : Returns true if a CTCommand is waiting to be retrieved from
//					the front of the queue.
//
/////////////////////////////////////////////////////////////////////
bool CTCommandQueue::IsCommandAvailable()
{
	return (m_Commands.begin() != m_Commands.end()) ;
}
