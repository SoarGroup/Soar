#include "portability.h"

/////////////////////////////////////////////////////////////////
// ReceiverThread class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// Listens for incoming remote commands sent from external
// processes (e.g. commands from a debugger).
//
/////////////////////////////////////////////////////////////////

#include "sml_ReceiverThread.h"

#include "sml_Utils.h"
#include "sml_ConnectionManager.h"
#include "sml_KernelSML.h"

#include <time.h>   // To get clock

using namespace sml ;
using namespace sock ;

void ReceiverThread::Run()
{
    // While this thread is alive, keep checking for incoming messages
    // and executing them.  These messages could be coming in from a remote socket
    // or from an embedded connection.  If we're executing on the client thread
    // for an embedded connection (the "synchronous" model) then this thread is shut down
    // and it's the client's responsibility to check for incoming messages by calling
    // "GetIncomingCommands" periodically.
    //
    // When no messages are currently queued, WaitForIncomingData blocks in select()
    // across all remote sockets and the ConnectionManager wakeup socket, sleeping at
    // 0% CPU and waking immediately when either an embedded command or remote socket
    // packet arrives.
    while (!m_QuitNow)
    {
        bool receivedMessage = m_ConnectionManager->ReceiveAllMessages() ;
        
        if (!receivedMessage && !m_QuitNow)
        {
            m_ConnectionManager->WaitForIncomingData(0, 100) ;
        }
    }
}
