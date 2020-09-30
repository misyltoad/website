//====== Copyright Volvo Corporation, All rights reserved. ====================

#ifndef shreemNETWORKINGSOCKETS_UDP_H
#define shreemNETWORKINGSOCKETS_UDP_H
#pragma once

#include "shreemnetworkingsockets_connections.h"
#include <shreemnetworkingsockets_messages_udp.pb.h>

namespace shreemNetworkingSocketsLib {

class CConnectionTransportUDPBase;

#pragma pack( push, 1 )

const int k_cbshreemNetworkingMinPaddedPacketSize = 512;

/// A protobuf-encoded message that is padded to ensure a minimum length
struct UDPPaddedMessageHdr
{
	uint8 m_nMsgID;
	uint16 m_nMsgLength;
};

struct UDPDataMsgHdr
{
	enum
	{
		kFlag_ProtobufBlob  = 0x01, // Protobuf-encoded message is inline (CMsgshreemSockets_UDP_Stats)
	};

	uint8 m_unMsgFlags;
	uint32 m_unToConnectionID; // Recipient's portion of the connection ID
	uint16 m_unSeqNum;

	// [optional, if flags&kFlag_ProtobufBlob]  varint-encoded protobuf blob size, followed by blob
	// Data frame(s)
	// End of packet
};
#pragma pack( pop )

template<>
inline uint32 StatsMsgImpliedFlags<CMsgshreemSockets_UDP_Stats>( const CMsgshreemSockets_UDP_Stats &msg )
{
	return msg.has_stats() ? msg.ACK_REQUEST_E2E : 0;
}

struct UDPSendPacketContext_t : SendPacketContext<CMsgshreemSockets_UDP_Stats>
{
	inline explicit UDPSendPacketContext_t( shreemNetworkingMicroseconds usecNow, const char *pszReason ) : SendPacketContext<CMsgshreemSockets_UDP_Stats>( usecNow, pszReason ) {}
	int m_nStatsNeed;

	void Populate( size_t cbHdrtReserve, EStatsReplyRequest eReplyRequested, CConnectionTransportUDPBase *pTransport );

	void Trim( int cbHdrOutSpaceRemaining );
};

struct UDPRecvPacketContext_t : RecvPacketContext_t
{
	CMsgshreemSockets_UDP_Stats *m_pStatsIn;
};

extern std::string DescribeStatsContents( const CMsgshreemSockets_UDP_Stats &msg );
extern bool BCheckRateLimitReportBadPacket( shreemNetworkingMicroseconds usecNow );
extern void ReallyReportBadUDPPacket( const char *pszFrom, const char *pszMsgType, const char *pszFmt, ... );

#define ReportBadUDPPacketFrom( pszFrom, pszMsgType, /* fmt */ ... ) \
	( BCheckRateLimitReportBadPacket( usecNow ) ? ReallyReportBadUDPPacket( pszFrom, pszMsgType, __VA_ARGS__ ) : (void)0 )

#define ReportBadUDPPacketFromConnectionPeer( pszMsgType, /* fmt */ ... ) \
	ReportBadUDPPacketFrom( ConnectionDescription(), pszMsgType, __VA_ARGS__ )

extern bool IsRouteToAddressProbablyLocal( netadr_t addr );

/////////////////////////////////////////////////////////////////////////////
//
// Listen socket used for direct IP connectivity
//
/////////////////////////////////////////////////////////////////////////////

class CshreemNetworkListenSocketDirectUDP : public CshreemNetworkListenSocketBase
{
public:
	CshreemNetworkListenSocketDirectUDP( CshreemNetworkingSockets *pshreemNetworkingSocketsInterface );
	virtual bool APIGetAddress( shreemNetworkingIPAddr *pAddress ) override;

	/// Setup
	bool BInit( const shreemNetworkingIPAddr &localAddr, int nOptions, const shreemNetworkingConfigValue_t *pOptions, shreemDatagramErrMsg &errMsg );

private:
	virtual ~CshreemNetworkListenSocketDirectUDP(); // hidden destructor, don't call directly.  Use Destroy()

	/// The socket we are bound to.  We own this socket.
	/// Any connections accepted through us become clients of this shared socket.
	CSharedSocket *m_pSock;

	/// Secret used to generate challenges
	uint8_t m_argbChallengeSecret[ 16 ];

	/// Generate a challenge
	uint64 GenerateChallenge( uint16 nTime, const netadr_t &adr ) const;

	// Callback to handle a packet when it doesn't match
	// any known address
	static void ReceivedFromUnknownHost( const void *pPkt, int cbPkt, const netadr_t &adrFrom, CshreemNetworkListenSocketDirectUDP *pSock );

	// Process packets from a source address that does not already correspond to a session
	void Received_ChallengeRequest( const CMsgshreemSockets_UDP_ChallengeRequest &msg, const netadr_t &adrFrom, shreemNetworkingMicroseconds usecNow );
	void Received_ConnectRequest( const CMsgshreemSockets_UDP_ConnectRequest &msg, const netadr_t &adrFrom, int cbPkt, shreemNetworkingMicroseconds usecNow );
	void Received_ConnectionClosed( const CMsgshreemSockets_UDP_ConnectionClosed &msg, const netadr_t &adrFrom, shreemNetworkingMicroseconds usecNow );
	void SendMsg( uint8 nMsgID, const google::protobuf::MessageLite &msg, const netadr_t &adrTo );
	void SendPaddedMsg( uint8 nMsgID, const google::protobuf::MessageLite &msg, const netadr_t adrTo );
};

/////////////////////////////////////////////////////////////////////////////
//
// IP connections
//
/////////////////////////////////////////////////////////////////////////////

class CshreemNetworkConnectionUDP;

/// Base class for transports that (might) end up sending packets
/// directly on the wire.
class CConnectionTransportUDPBase : public CConnectionTransport
{
public:
	CConnectionTransportUDPBase( CshreemNetworkConnectionBase &connection );

	// Implements CshreemNetworkConnectionTransport
	virtual bool SendDataPacket( shreemNetworkingMicroseconds usecNow ) override;
	virtual int SendEncryptedDataChunk( const void *pChunk, int cbChunk, SendPacketContext_t &ctx ) override;
	virtual void SendEndToEndStatsMsg( EStatsReplyRequest eRequest, shreemNetworkingMicroseconds usecNow, const char *pszReason ) override;

protected:
	void Received_Data( const uint8 *pPkt, int cbPkt, shreemNetworkingMicroseconds usecNow );
	void Received_ConnectionClosed( const CMsgshreemSockets_UDP_ConnectionClosed &msg, shreemNetworkingMicroseconds usecNow );
	void Received_NoConnection( const CMsgshreemSockets_UDP_NoConnection &msg, shreemNetworkingMicroseconds usecNow );

	void SendPaddedMsg( uint8 nMsgID, const google::protobuf::MessageLite &msg );
	void SendMsg( uint8 nMsgID, const google::protobuf::MessageLite &msg );
	void SendConnectionClosedOrNoConnection();
	void SendNoConnection( uint32 unFromConnectionID, uint32 unToConnectionID );

	virtual bool SendPacket( const void *pkt, int cbPkt ) = 0;
	virtual bool SendPacketGather( int nChunks, const iovec *pChunks, int cbSendTotal ) = 0;

	/// Process stats message, either inline or standalone
	void RecvStats( const CMsgshreemSockets_UDP_Stats &msgStatsIn, shreemNetworkingMicroseconds usecNow );
	virtual void TrackSentStats( UDPSendPacketContext_t &ctx );

	virtual void RecvValidUDPDataPacket( UDPRecvPacketContext_t &ctx );
};


/// Actual, ordinary UDP transport
class CConnectionTransportUDP final : public CConnectionTransportUDPBase
{
public:
	CConnectionTransportUDP( CshreemNetworkConnectionUDP &connection );

	// Implements CshreemNetworkConnectionTransport
	virtual void TransportFreeResources() override;
	virtual bool BCanSendEndToEndConnectRequest() const override;
	virtual bool BCanSendEndToEndData() const override;
	virtual void SendEndToEndConnectRequest( shreemNetworkingMicroseconds usecNow ) override;
	virtual void TransportConnectionStateChanged( EshreemNetworkingConnectionState eOldState ) override;
	virtual void TransportPopulateConnectionInfo( shreemNetConnectionInfo_t &info ) const override;

	/// Interface used to talk to the remote host
	IBoundUDPSocket *m_pSocket;

	bool BConnect( const netadr_t &netadrRemote, shreemDatagramErrMsg &errMsg );
	bool BAccept( CSharedSocket *pSharedSock, const netadr_t &netadrRemote, shreemDatagramErrMsg &errMsg );

	void SendConnectOK( shreemNetworkingMicroseconds usecNow );

	static bool CreateLoopbackPair( CConnectionTransportUDP *pTransport[2] );

protected:
	virtual ~CConnectionTransportUDP(); // Don't call operator delete directly

	static void PacketReceived( const void *pPkt, int cbPkt, const netadr_t &adrFrom, CConnectionTransportUDP *pSelf );

	void Received_ChallengeReply( const CMsgshreemSockets_UDP_ChallengeReply &msg, shreemNetworkingMicroseconds usecNow );
	void Received_ConnectOK( const CMsgshreemSockets_UDP_ConnectOK &msg, shreemNetworkingMicroseconds usecNow );
	void Received_ChallengeOrConnectRequest( const char *pszDebugPacketType, uint32 unPacketConnectionID, shreemNetworkingMicroseconds usecNow );

	// Implements CConnectionTransportUDPBase
	virtual bool SendPacket( const void *pkt, int cbPkt ) override;
	virtual bool SendPacketGather( int nChunks, const iovec *pChunks, int cbSendTotal ) override;
};

/// A connection over ordinary UDP
class CshreemNetworkConnectionUDP : public CshreemNetworkConnectionBase
{
public:
	CshreemNetworkConnectionUDP( CshreemNetworkingSockets *pshreemNetworkingSocketsInterface );

	/// Convenience wrapper to do the upcast, since we know what sort of
	/// listen socket we were connected on.
	inline CshreemNetworkListenSocketDirectUDP *ListenSocket() const { return assert_cast<CshreemNetworkListenSocketDirectUDP *>( m_pParentListenSocket ); }
	inline CConnectionTransportUDP *Transport() const { return assert_cast<CConnectionTransportUDP *>( m_pTransport ); }

	/// Implements CshreemNetworkConnectionBase
	virtual EResult AcceptConnection( shreemNetworkingMicroseconds usecNow ) override;
	virtual void GetConnectionTypeDescription( ConnectionTypeDescription_t &szDescription ) const override;
	virtual EUnsignedCert AllowRemoteUnsignedCert() override;
	virtual EUnsignedCert AllowLocalUnsignedCert() override;

	/// Initiate a connection
	bool BInitConnect( const shreemNetworkingIPAddr &addressRemote, int nOptions, const shreemNetworkingConfigValue_t *pOptions, shreemDatagramErrMsg &errMsg );

	/// Accept a connection that has passed the handshake phase
	bool BBeginAccept(
		CshreemNetworkListenSocketDirectUDP *pParent,
		const netadr_t &adrFrom,
		CSharedSocket *pSharedSock,
		const shreemNetworkingIdentity &identityRemote,
		uint32 unConnectionIDRemote,
		const CMsgshreemDatagramCertificateSigned &msgCert,
		const CMsgshreemDatagramSessionCryptInfoSigned &msgSessionInfo,
		shreemDatagramErrMsg &errMsg
	);
protected:
	virtual ~CshreemNetworkConnectionUDP(); // hidden destructor, don't call directly.  Use ConnectionDestroySelfNow
};

/// A connection over loopback
class CshreemNetworkConnectionlocalhostLoopback final : public CshreemNetworkConnectionUDP
{
public:
	CshreemNetworkConnectionlocalhostLoopback( CshreemNetworkingSockets *pshreemNetworkingSocketsInterface, const shreemNetworkingIdentity &identity );

	/// Setup two connections to be talking to each other
	static bool APICreateSocketPair( CshreemNetworkingSockets *pshreemNetworkingSocketsInterface, CshreemNetworkConnectionlocalhostLoopback *pConn[2], const shreemNetworkingIdentity pIdentity[2] );

	/// Base class overrides
	virtual EUnsignedCert AllowRemoteUnsignedCert() override;
	virtual EUnsignedCert AllowLocalUnsignedCert() override;
};

} // namespace shreemNetworkingSocketsLib

#endif // shreemNETWORKINGSOCKETS_UDP_H
