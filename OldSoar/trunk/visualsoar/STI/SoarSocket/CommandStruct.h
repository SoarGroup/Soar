#ifndef COMMANDSTRUCT_H
#define COMMANDSTRUCT_H

#ifndef MAX_PARAM_VERSION_1
#define MAX_PARAM_VERSION_1	6
#endif

const long kUserMsg   = 0x0000 ;
const long kSystemMsg = 0x0001 ;

const long kMajorVersion = 1 ;
const long kMinorVersion = 1 ;

struct CommandStruct
{
	// Message header
	long		m_StructSize ;		// Size of this structure
	long		m_TotalMessageSize ;// Size of this structure + size of data buffer
	long		m_ByteOrder ;		// Sender sets to 0x12345678 (so receiver can check byte order)
	long		m_MessageVersion ;	// (Major version << 8 + Minor version).

	// Message data
	long		m_FromPort ;		// Port number of connection sending this msg
	long		m_ToPort ;			// Port number of connection receiving this msg (not currently used)
	long		m_UniqueID ;		// ID that's unique for messages over this connection (never 0)
	long		m_AckID ;			// If this is a message acknowledging another message--this is the other message's ID

	long		m_SystemMsg ;		// 0 for user commands, > 0 for system commands
	long		m_MsgFlags ;		// Not currently used

	// Command data
	long		m_CommandID ;
	long		m_CommandFlags ;	// Not currently used
	long		m_DataSize ;		// If non-0 gives size in bytes of arbitrary data buffer sent after command
	char*		m_pData ;			// Only valid if dataSize != 0.  Data sent after Msg is sent.
	long		m_Params[MAX_PARAM_VERSION_1] ;

	char		m_StringParam1[512] ;

	// For later expansion
	long		m_Unused1 ;			// Fields for later use w/o changing msg struct size
	long		m_Unused2 ;
	long		m_Unused3 ;
	long		m_Unused4 ;
	long		m_Unused5 ;
	long		m_Unused6 ;
	char		m_Unused7[32] ;
	char		m_Unused8[32] ;
} ;

#endif // COMMANDSTRUCT_H
