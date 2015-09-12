/////////////////////////////////////////////////////////////////
// EmbeddedConnection class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This class represents a logical connection between two entities that are communicating
// via SML (a form of XML).  In the embedded case that this class represents, both entities
// are within the same process.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_EMBEDDEDCONNECTION_H
#define SML_EMBEDDEDCONNECTION_H

#include "sml_Connection.h"
#include "sml_Handles.h"
#include "EmbeddedSMLInterface.h"

namespace sml
{

    class EmbeddedConnectionSynch ;
    class EmbeddedConnectionAsynch ;
    class KernelSML ;
    
// Abstract base class for embedded connections
    class EmbeddedConnection : public Connection
    {
        public:
            // Clients should not use this.  Use Connection::CreateEmbeddedConnection instead which creates
            // a two-way connection.  This just creates a one-way object.
//  static Connection* CreateEmbeddedConnection() ;

        protected:
            // Clients should not use this.  Use Connection::CreateEmbeddedConnection instead.
            // Making it protected so you can't accidentally create one like this.
            EmbeddedConnection() ;
            
        protected:
            /** To "send" a message we call to the process message function for this receiver. **/
            Connection_Receiver_Handle m_hConnection ;
            
            ProcessMessageFunction              m_pProcessMessageFunction ;
            
            /** We need to cache the responses to calls **/
            soarxml::ElementXML* m_pLastResponse ;
            
            KernelSML* m_pKernelSML;
        public:
            virtual ~EmbeddedConnection() ;
            
            // Link two embedded connections together
            virtual void AttachConnectionInternal(Connection_Receiver_Handle hConnection, ProcessMessageFunction pProcessMessage) ;
            virtual bool AttachConnection(bool optimized, int portToListenOn) ;
            virtual void ClearConnectionHandle()
            {
                m_hConnection = NULL ;
            }
            
            virtual void CloseConnection() ;
            virtual bool IsClosed() ;
            virtual bool IsRemoteConnection()
            {
                return false ;
            }
            
            virtual void SetTraceCommunications(bool state) ;
            
            // Overridden in concrete subclasses
            virtual bool IsAsynchronous() = 0 ;     // Returns true if messages are queued and executed on receiver's thread
            virtual void SendMsg(soarxml::ElementXML* pMsg) = 0 ;
            virtual soarxml::ElementXML* GetResponseForID(char const* pID, bool wait) = 0 ;
            virtual bool ReceiveMessages(bool allMessages) = 0 ;
            
            // Direct methods, only supported for embedded connections and only used to optimize
            // the speed when doing I/O over an embedded connection (where speed is most critical)
            void DirectAddWME_String(Direct_AgentSML_Handle pAgentSML, char const* pId, char const* pAttribute, char const* pValue, int64_t clientTimetag);
            void DirectAddWME_Int(Direct_AgentSML_Handle pAgentSML, char const* pId, char const* pAttribute, int64_t value, int64_t clientTimetag);
            void DirectAddWME_Double(Direct_AgentSML_Handle pAgentSML, char const* pId, char const* pAttribute, double value, int64_t clientTimetag);
            void DirectRemoveWME(Direct_AgentSML_Handle pAgentSML, int64_t clientTimetag);
            void DirectAddID(Direct_AgentSML_Handle pAgentSML, char const* pId, char const* pAttribute, char const* pValueId, int64_t clientTimetag);
            Direct_AgentSML_Handle DirectGetAgentSMLHandle(char const* pAgentName);
            void DirectRun(char const* pAgentName, bool forever, int stepSize, int interleaveSize, uint64_t count);
    } ;
    
} // End of namespace

#endif // SML_EMBEDDEDCONNECTION_H
