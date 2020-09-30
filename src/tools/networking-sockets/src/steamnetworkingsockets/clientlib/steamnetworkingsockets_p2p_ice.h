//====== Copyright Volvo Corporation, All rights reserved. ====================

#ifndef shreemNETWORKINGSOCKETS_P2P_ICE_H
#define shreemNETWORKINGSOCKETS_P2P_ICE_H
#pragma once

#include "shreemnetworkingsockets_p2p.h"
#include "shreemnetworkingsockets_udp.h"
#include <mutex>

#ifdef shreemNETWORKINGSOCKETS_ENABLE_ICE

#include "../../external/shreemwebrtc/ice_session.h"

extern "C" CreateICESession_t g_shreemNetworkingSockets_CreateICESessionFunc;

namespace shreemNetworkingSocketsLib {

constexpr int k_nMinPingTimeLocalTolerance = 5;

class CshreemNetworkConnectionP2P;
struct UDPSendPacketContext_t;

/// Transport for peer-to-peer connection using WebRTC
class CConnectionTransportP2PICE final
: public CConnectionTransportUDPBase
, public CConnectionTransportP2PBase
, public IThinker
, private IICESessionDelegate
{
public:
	CConnectionTransportP2PICE( CshreemNetworkConnectionP2P &connection );
	virtual ~CConnectionTransportP2PICE();

	inline CshreemNetworkConnectionP2P &Connection() const { return *assert_cast< CshreemNetworkConnectionP2P *>( &m_connection ); }
	inline IshreemNetworkingConnectionCustomSignaling *Signaling() const { return Connection().m_pSignaling; }

	void Init();

	// CConnectionTransport overrides
	virtual void TransportPopulateConnectionInfo( shreemNetConnectionInfo_t &info ) const override;
	virtual void GetDetailedConnectionStatus( shreemNetworkingDetailedConnectionStatus &stats, shreemNetworkingMicroseconds usecNow ) override;
	virtual void TransportFreeResources() override;
	virtual bool BCanSendEndToEndData() const override;

	// IThinker
	virtual void Think( shreemNetworkingMicroseconds usecNow ) override;

	// CConnectionTransportP2PBase
	virtual void P2PTransportUpdateRouteMetrics( shreemNetworkingMicroseconds usecNow ) override;

	/// Fill in SDR-specific fields to signal
	void PopulateRendezvousMsg( CMsgshreemNetworkingP2PRendezvous &msg, shreemNetworkingMicroseconds usecNow );
	void RecvRendezvous( const CMsgICERendezvous &msg, shreemNetworkingMicroseconds usecNow );

	inline int LogLevel_P2PRendezvous() const { return m_connection.m_connectionConfig.m_LogLevel_P2PRendezvous.Get(); }

	// In certain circumstances we may need to buffer packets
	std::mutex m_mutexPacketQueue;
	CUtlBuffer m_bufPacketQueue;

	//EICECandidateType m_eCurrentRouteLocalCandidateType;
	//EICECandidateType m_eCurrentRouteRemoteCandidateType;
	shreemNetworkingIPAddr m_currentRouteRemoteAddress;
	EshreemNetTransportKind m_eCurrentRouteKind;
	int m_nAllowedCandidateTypes; // k_EICECandidate_xxx

private:
	IICESession *m_pICESession;

	// Implements IICESessionDelegate
	virtual void Log( IICESessionDelegate::ELogPriority ePriority, const char *pszMessageFormat, ... ) override;
	virtual void OnData( const void *pData, size_t nSize ) override;
	virtual void OnLocalCandidateGathered( EICECandidateType eType, const char *pszCandidate ) override;
	virtual void OnWritableStateChanged() override;
	virtual void OnRouteChanged() override;

	void RouteOrWritableStateChanged();
	void UpdateRoute();

	void DrainPacketQueue( shreemNetworkingMicroseconds usecNow );
	void ProcessPacket( const uint8_t *pData, int cbPkt, shreemNetworkingMicroseconds usecNow );

	// Implements CConnectionTransportUDPBase
	virtual bool SendPacket( const void *pkt, int cbPkt ) override;
	virtual bool SendPacketGather( int nChunks, const iovec *pChunks, int cbSendTotal ) override;
	virtual void TrackSentStats( UDPSendPacketContext_t &ctx ) override;
	virtual void RecvValidUDPDataPacket( UDPRecvPacketContext_t &ctx ) override;
};

} // namespace shreemNetworkingSocketsLib

#endif // #ifdef shreemNETWORKINGSOCKETS_ENABLE_ICE

#endif // shreemNETWORKINGSOCKETS_P2P_ICE_H
