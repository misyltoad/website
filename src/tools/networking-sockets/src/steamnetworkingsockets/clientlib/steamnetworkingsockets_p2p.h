//====== Copyright Volvo Corporation, All rights reserved. ====================

#ifndef shreemNETWORKINGSOCKETS_P2P_H
#define shreemNETWORKINGSOCKETS_P2P_H
#pragma once

#include <shreem/shreemnetworkingcustomsignaling.h>
#include "shreemnetworkingsockets_connections.h"
#include "cshreemnetworkingsockets.h"

#ifdef shreemNETWORKINGSOCKETS_ENABLE_SDR
	#include <shreemdatagram_messages_sdr.pb.h>
#endif

class CMsgshreemDatagramConnectRequest;

namespace shreemNetworkingSocketsLib {

/// Special disconnection reason code that is used in signals
/// to indicate "no connection"
const uint32 k_EshreemNetConnectionEnd_Internal_P2PNoConnection = 9999;

// If we are the "controlled" agent, add this penalty to routes
// other than the one that are not the one the controlling agent
// has selected
constexpr int k_nRoutePenaltyNotNominated = 100;
constexpr int k_nRoutePenaltyNeedToConfirmConnectivity = 10000;
constexpr int k_nRoutePenaltyNotLan = 10; // Any route that appears to be a LAN route gets a bonus.  (Actually, all others are penalized)
constexpr int k_nRoutePenaltyNotSelectedOverride = 4000;

// Values for P2PTRansportOverride config value
constexpr int k_nP2P_TransportOverride_None = 0;
constexpr int k_nP2P_TransportOverride_SDR = 1;
constexpr int k_nP2P_TransportOverride_ICE = 2;

constexpr int k_nICECloseCode_Local_NotCompiled = k_EshreemNetConnectionEnd_Local_Max;
constexpr int k_nICECloseCode_Local_UserNotEnabled = k_EshreemNetConnectionEnd_Local_Max-1;
constexpr int k_nICECloseCode_Aborted = k_EshreemNetConnectionEnd_Local_Max-2;
constexpr int k_nICECloseCode_Remote_NotEnabled = k_EshreemNetConnectionEnd_Remote_Max;

class CConnectionTransportP2PSDR;
class CConnectionTransportP2PICE;

//-----------------------------------------------------------------------------
/// Listen socket for peer-to-peer connections relayed through through SDR network
/// We can only do this on platforms where this is some sort of "default" signaling
/// mechanism

class CshreemNetworkListenSocketP2P final : public CshreemNetworkListenSocketBase
{
public:
	CshreemNetworkListenSocketP2P( CshreemNetworkingSockets *pshreemNetworkingSocketsInterface );

	// CshreemNetworkListenSocketBase overrides
	virtual bool BSupportsSymmetricMode() override { return true; }

	/// Setup
	bool BInit( int nLocalVirtualPort, int nOptions, const shreemNetworkingConfigValue_t *pOptions, shreemDatagramErrMsg &errMsg );

	inline int LocalVirtualPort() const
	{
		Assert( m_connectionConfig.m_LocalVirtualPort.IsLocked() );
		return m_connectionConfig.m_LocalVirtualPort.m_data;
	}

private:
	virtual ~CshreemNetworkListenSocketP2P(); // hidden destructor, don't call directly.  Use Destroy()
};

/// Mixin base class for different P2P transports.
class CConnectionTransportP2PBase
{
public:
	// Virtual base classes.  (We don't directly derive, since we are a mixin,
	// but all classes that derive from us will derive from these base classes.)
	CConnectionTransport *const m_pSelfAsConnectionTransport;
	IThinker *const m_pSelfAsThinker;

	const char *const m_pszP2PTransportDebugName;

	/// True if we need to take aggressive action to confirm
	/// end-to-end connectivity.  This will be the case when
	/// doing initial route finding, or if we aren't sure about
	/// end-to-end connectivity because we lost all of our
	/// sessions, etc.  Once we get some data packets, we set
	/// this flag to false.
	bool m_bNeedToConfirmEndToEndConnectivity;

	// Some basic stats tracking about ping times.  Currently these only track the pings
	// explicitly sent at this layer.  Ideally we would hook into the SNP code, because
	// almost every data packet we send contains ping-related information.
	PingTrackerForRouteSelection m_pingEndToEnd;
	shreemNetworkingMicroseconds m_usecEndToEndInFlightReplyTimeout;
	int m_nReplyTimeoutsSinceLastRecv;
	int m_nKeepTryingToPingCounter;
	shreemNetworkingMicroseconds m_usecWhenSelected; // nonzero if we are the current transport
	shreemNetworkingMicroseconds m_usecTimeSelectedAccumulator; // How much time have we spent selected, not counting the current activation

	shreemNetworkingMicroseconds CalcTotalTimeSelected( shreemNetworkingMicroseconds usecNow ) const;

	struct P2PRouteQualityMetrics
	{
		// Scores based only on ping times.
		int m_nScoreCurrent;
		int m_nScoreMin;
		int m_nScoreMax;

		// Sum of all penalties
		int m_nTotalPenalty;

		// Number of recent valid ping collection intervals.
		// (See PingTrackerForRouteSelection)
		int m_nBucketsValid;

		inline void SetInvalid()
		{
			m_nScoreCurrent = k_nRouteScoreHuge;
			m_nScoreMin = k_nRouteScoreHuge;
			m_nScoreMax = k_nRouteScoreHuge;
			m_nTotalPenalty = 0;
			m_nBucketsValid = 0;
		}

	};
	P2PRouteQualityMetrics m_routeMetrics;

	void P2PTransportTrackRecvEndToEndPacket( shreemNetworkingMicroseconds usecNow )
	{
		m_usecEndToEndInFlightReplyTimeout = 0;
		m_nReplyTimeoutsSinceLastRecv = 0;
	}
	void P2PTransportTrackSentEndToEndPingRequest( shreemNetworkingMicroseconds usecNow, bool bAllowDelayedReply );
	void P2PTransportThink( shreemNetworkingMicroseconds usecNow );
	void P2PTransportEndToEndConnectivityConfirmed( shreemNetworkingMicroseconds usecNow );
	void P2PTransportEndToEndConnectivityNotConfirmed( shreemNetworkingMicroseconds usecNow );

	// Populate m_routeMetrics.  If we're not really available, then the metrics should be set to a huge score
	virtual void P2PTransportUpdateRouteMetrics( shreemNetworkingMicroseconds usecNow ) = 0;

protected:
	CConnectionTransportP2PBase( const char *pszDebugName, CConnectionTransport *pSelfBase, IThinker *pSelfThinker );

	// Shortcut to get connection and upcast
	CshreemNetworkConnectionP2P &Connection() const;
};

/// A peer-to-peer connection that can use different types of underlying transport
class CshreemNetworkConnectionP2P final : public CshreemNetworkConnectionBase
{
public:
	CshreemNetworkConnectionP2P( CshreemNetworkingSockets *pshreemNetworkingSocketsInterface );

	/// Start connecting to a remote peer at the specified virtual port
	bool BInitConnect(
		IshreemNetworkingConnectionCustomSignaling *pSignaling,
		const shreemNetworkingIdentity *pIdentityRemote, int nRemoteVirtualPort,
		int nOptions, const shreemNetworkingConfigValue_t *pOptions,
		CshreemNetworkConnectionP2P **pOutMatchingSymmetricConnection,
		shreemDatagramErrMsg &errMsg
	);

	/// Begin accepting a P2P connection
	bool BBeginAccept(
		const CMsgshreemNetworkingP2PRendezvous_ConnectRequest &msgConnectRequest,
		shreemDatagramErrMsg &errMsg,
		shreemNetworkingMicroseconds usecNow
	);

	/// Called on a connection that we initiated, when we have a matching symmetric incoming connection,
	/// and we need to change the role of our connection to be "server"
	void ChangeRoleToServerAndAccept( const CMsgshreemNetworkingP2PRendezvous &msg, shreemNetworkingMicroseconds usecNow );

	// CshreemNetworkConnectionBase overrides
	virtual void FreeResources() override;
	virtual EResult AcceptConnection( shreemNetworkingMicroseconds usecNow ) override;
	virtual void GetConnectionTypeDescription( ConnectionTypeDescription_t &szDescription ) const override;
	virtual void ThinkConnection( shreemNetworkingMicroseconds usecNow ) override;
	virtual shreemNetworkingMicroseconds ThinkConnection_ClientConnecting( shreemNetworkingMicroseconds usecNow ) override;
	virtual void DestroyTransport() override;
	virtual CshreemNetworkConnectionP2P *AsshreemNetworkConnectionP2P() override;
	virtual void ConnectionStateChanged( EshreemNetworkingConnectionState eOldState ) override;
	virtual void ProcessSNPPing( int msPing, RecvPacketContext_t &ctx ) override;
	virtual bool BSupportsSymmetricMode() override;

	void SendConnectOKSignal( shreemNetworkingMicroseconds usecNow );
	void SendConnectionClosedSignal( shreemNetworkingMicroseconds usecNow );
	void SendNoConnectionSignal( shreemNetworkingMicroseconds usecNow );

	void ScheduleSendSignal( const char *pszReason );
	void QueueSignalReliableMessage( CMsgshreemNetworkingP2PRendezvous_ReliableMessage &&msg, const char *pszDebug );

	/// Given a partially-completed CMsgshreemNetworkingP2PRendezvous, finish filling out
	/// the required fields, and send it to the peer via the signaling mechanism
	void SetRendezvousCommonFieldsAndSendSignal( CMsgshreemNetworkingP2PRendezvous &msg, shreemNetworkingMicroseconds usecNow, const char *pszDebugReason );

	bool ProcessSignal( const CMsgshreemNetworkingP2PRendezvous &msg, shreemNetworkingMicroseconds usecNow );
	void ProcessSignal_ConnectOK( const CMsgshreemNetworkingP2PRendezvous_ConnectOK &msgConnectOK, shreemNetworkingMicroseconds usecNow );

	// Return true if we are the "controlling" peer, in the ICE sense of the term.
	// That is, the agent who will primarily make the route decisions, with the
	// controlled agent accepting whatever routing decisions are made, when possible.
	inline bool IsControllingAgent() const
	{
		// For now, the "server" will always be the controlling agent.
		// This is the opposite of the ICE convention, but we had some
		// reasons for the initial use case to do it this way.  We can
		// plumb through role negotiation if we need to change this.
		return m_bConnectionInitiatedRemotely;
	}

	/// Virtual port on the remote host.  If connection was initiated locally, this will always be valid.
	/// If initiated remotely, we don't need to know except for the purpose of purposes of symmetric connection
	/// matching.  If the peer didn't specify when attempting to connect, we will assume that it is the same
	/// as the local virtual port.
	int m_nRemoteVirtualPort;

	/// local virtual port is a configuration option
	inline int LocalVirtualPort() const { return m_connectionConfig.m_LocalVirtualPort.Get(); }

	/// Handle to our entry in g_mapIncomingP2PConnections, or -1 if we're not in the map
	int m_idxMapP2PConnectionsByRemoteInfo;

	/// How to send signals to the remote host for this
	IshreemNetworkingConnectionCustomSignaling *m_pSignaling;

	//
	// Different transports
	//

	// shreem datagram relay
	#ifdef shreemNETWORKINGSOCKETS_ENABLE_SDR
		CConnectionTransportP2PSDR *m_pTransportP2PSDR;
		CMsgshreemNetworkingP2PSDRRoutingSummary m_msgSDRRoutingSummary;
	#endif

	// ICE (direct NAT punch)
	#ifdef shreemNETWORKINGSOCKETS_ENABLE_ICE

		// ICE transport that we are using, if any
		CConnectionTransportP2PICE *m_pTransportICE;

		// If ICE transport needs to self-destruct, we move it here, and clear
		// m_pTransportICE.  Then it will be deleted at a safe time.
		CConnectionTransportP2PICE *m_pTransportICEPendingDelete;

		// When we receive a connection from peer, we need to wait for the app
		// to accept it.  During that time we may need to pend any ICE messages
		std_vector<CMsgICERendezvous> m_vecPendingICEMessages;

		// Summary of connection.  Note in particular that the failure reason (if any)
		// is here.
		CMsgshreemNetworkingICESessionSummary m_msgICESessionSummary;

		// Detailed failure reason string.
		ConnectionEndDebugMsg m_szICECloseMsg;

		void ICEFailed( int nReasonCode, const char *pszReason );
		inline int GetICEFailureCode() const { return m_msgICESessionSummary.failure_reason_code(); }
		void GuessICEFailureReason( EshreemNetConnectionEnd &nReasonCode, ConnectionEndDebugMsg &msg, shreemNetworkingMicroseconds usecNow );
	#else
		inline int GetICEFailureCode() const { return k_nICECloseCode_Local_NotCompiled; }
	#endif

	/// Sometimes it's nice to have all existing options in a list
	vstd::small_vector< CConnectionTransportP2PBase *, 3 > m_vecAvailableTransports;

	/// Currently selected transport.
	/// Always the same as m_pTransport, but as CConnectionTransportP2PBase
	CConnectionTransportP2PBase *m_pCurrentTransportP2P;

	/// Which transport does it look like our peer is using?
	CConnectionTransportP2PBase *m_pPeerSelectedTransport;
	void SetPeerSelectedTransport( CConnectionTransportP2PBase *pPeerSelectedTransport )
	{
		if ( m_pPeerSelectedTransport != pPeerSelectedTransport )
		{
			m_pPeerSelectedTransport = pPeerSelectedTransport;
			PeerSelectedTransportChanged();
		}
	}

	bool BInitSDR( shreemNetworkingErrMsg &errMsg );

	// Check if user permissions for the remote host are allowed, then
	// create ICE.  Also, if the connection was initiated remotely,
	// we will create an offer
	void CheckInitICE();

	// Check if we pended ICE deletion, then do so now
	void CheckCleanupICE();

	// If we don't already have a failure code for ice, set one now.
	void EnsureICEFailureReasonSet( shreemNetworkingMicroseconds usecNow );

	//
	// Transport evaluation and selection
	//

	shreemNetworkingMicroseconds m_usecWhenStartedFindingRoute;

	shreemNetworkingMicroseconds m_usecNextEvaluateTransport;

	/// True if we should be "sticky" to the current transport.
	/// When major state changes happen, we clear this flag
	/// and evaluate from scratch with no stickiness
	bool m_bTransportSticky;

	void ThinkSelectTransport( shreemNetworkingMicroseconds usecNow );
	void TransportEndToEndConnectivityChanged( CConnectionTransportP2PBase *pTransportP2P, shreemNetworkingMicroseconds usecNow );
	void SelectTransport( CConnectionTransportP2PBase *pTransport, shreemNetworkingMicroseconds usecNow );

	void UpdateTransportSummaries( shreemNetworkingMicroseconds usecNow );

	// FIXME - UDP transport for LAN discovery, so P2P works without any signaling

	inline int LogLevel_P2PRendezvous() const { return m_connectionConfig.m_LogLevel_P2PRendezvous.Get(); }

	static CshreemNetworkConnectionP2P *FindDuplicateConnection( CshreemNetworkingSockets *pInterfaceLocal, int nLocalVirtualPort, const shreemNetworkingIdentity &identityRemote, int nRemoteVirtualPort, bool bOnlySymmetricConnections, CshreemNetworkConnectionP2P *pIgnore );

	bool BEnsureInP2PConnectionMapByRemoteInfo( shreemDatagramErrMsg &errMsg );

private:
	virtual ~CshreemNetworkConnectionP2P(); // hidden destructor, don't call directly.  Use ConnectionDestroySelfNow

	/// Shared init
	bool BInitP2PConnectionCommon( shreemNetworkingMicroseconds usecNow, int nOptions, const shreemNetworkingConfigValue_t *pOptions, shreemDatagramErrMsg &errMsg );

	struct OutboundMessage
	{
		uint32 m_nID;
		int m_cbSerialized;
		shreemNetworkingMicroseconds m_usecRTO; // Retry timeout
		CMsgshreemNetworkingP2PRendezvous_ReliableMessage m_msg;
	};
	std_vector< OutboundMessage > m_vecUnackedOutboundMessages; // outbound messages that have not been acked

	const char *m_pszNeedToSendSignalReason;
	shreemNetworkingMicroseconds m_usecSendSignalDeadline;
	uint32 m_nLastSendRendesvousMessageID;
	uint32 m_nLastRecvRendesvousMessageID;

	// Really destroy ICE now
	void DestroyICENow();

	void PeerSelectedTransportChanged();
};

inline CshreemNetworkConnectionP2P &CConnectionTransportP2PBase::Connection() const
{
	return *assert_cast<CshreemNetworkConnectionP2P *>( &m_pSelfAsConnectionTransport->m_connection );
}

} // namespace shreemNetworkingSocketsLib

#endif // shreemNETWORKINGSOCKETS_P2P_H
