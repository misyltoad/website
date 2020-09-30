//====== Copyright Volvo Corporation, All rights reserved. ====================

#include "cshreemnetworkingsockets.h"
#include "shreemnetworkingsockets_lowlevel.h"
#include "shreemnetworkingsockets_connections.h"
#include "shreemnetworkingsockets_udp.h"
#include "../shreemnetworkingsockets_certstore.h"
#include "crypto.h"

#ifdef shreemNETWORKINGSOCKETS_STANDALONELIB
#include <shreem/shreemnetworkingsockets.h>
#endif

#ifdef shreemNETWORKINGSOCKETS_HAS_DEFAULT_P2P_SIGNALING
#include "cshreemnetworkingmessages.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IshreemNetworkingSockets::~IshreemNetworkingSockets() {}
IshreemNetworkingUtils::~IshreemNetworkingUtils() {}

// Put everything in a namespace, so we don't violate the one definition rule
namespace shreemNetworkingSocketsLib {

/////////////////////////////////////////////////////////////////////////////
//
// Configuration Variables
//
/////////////////////////////////////////////////////////////////////////////

DEFINE_GLOBAL_CONFIGVAL( float, FakePacketLoss_Send, 0.0f, 0.0f, 100.0f );
DEFINE_GLOBAL_CONFIGVAL( float, FakePacketLoss_Recv, 0.0f, 0.0f, 100.0f );
DEFINE_GLOBAL_CONFIGVAL( int32, FakePacketLag_Send, 0, 0, 5000 );
DEFINE_GLOBAL_CONFIGVAL( int32, FakePacketLag_Recv, 0, 0, 5000 );
DEFINE_GLOBAL_CONFIGVAL( float, FakePacketReorder_Send, 0.0f, 0.0f, 100.0f );
DEFINE_GLOBAL_CONFIGVAL( float, FakePacketReorder_Recv, 0.0f, 0.0f, 100.0f );
DEFINE_GLOBAL_CONFIGVAL( int32, FakePacketReorder_Time, 15, 0, 5000 );
DEFINE_GLOBAL_CONFIGVAL( float, FakePacketDup_Send, 0.0f, 0.0f, 100.0f );
DEFINE_GLOBAL_CONFIGVAL( float, FakePacketDup_Recv, 0.0f, 0.0f, 100.0f );
DEFINE_GLOBAL_CONFIGVAL( int32, FakePacketDup_TimeMax, 10, 0, 5000 );
DEFINE_GLOBAL_CONFIGVAL( int32, EnumerateDevVars, 0, 0, 1 );

DEFINE_GLOBAL_CONFIGVAL( void *, Callback_AuthStatusChanged, nullptr );
#ifdef shreemNETWORKINGSOCKETS_ENABLE_shreemNETWORKINGMESSAGES
DEFINE_GLOBAL_CONFIGVAL( void*, Callback_MessagesSessionRequest, nullptr );
DEFINE_GLOBAL_CONFIGVAL( void*, Callback_MessagesSessionFailed, nullptr );
#endif

DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, TimeoutInitial, 10000, 0, INT32_MAX );
DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, TimeoutConnected, 10000, 0, INT32_MAX );
DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, SendBufferSize, 512*1024, 0, 0x10000000 );
DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, SendRateMin, 128*1024, 1024, 0x10000000 );
DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, SendRateMax, 1024*1024, 1024, 0x10000000 );
DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, NagleTime, 5000, 0, 20000 );
DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, MTU_PacketSize, 1300, k_cbshreemNetworkingSocketsMinMTUPacketSize, k_cbshreemNetworkingSocketsMaxUDPMsgLen );
#ifdef shreemNETWORKINGSOCKETS_OPENSOURCE
	// We don't have a trusted third party, so allow this by default,
	// and don't warn about it
	DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, IP_AllowWithoutAuth, 2, 0, 2 );
#else
	DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, IP_AllowWithoutAuth, 0, 0, 2 );
#endif
DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, Unencrypted, 0, 0, 3 );
DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, SymmetricConnect, 0, 0, 1 );
DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, LocalVirtualPort, -1, -1, 65535 );
DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, LogLevel_AckRTT, k_EshreemNetworkingSocketsDebugOutputType_Warning, k_EshreemNetworkingSocketsDebugOutputType_Error, k_EshreemNetworkingSocketsDebugOutputType_Everything );
DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, LogLevel_PacketDecode, k_EshreemNetworkingSocketsDebugOutputType_Warning, k_EshreemNetworkingSocketsDebugOutputType_Error, k_EshreemNetworkingSocketsDebugOutputType_Everything );
DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, LogLevel_Message, k_EshreemNetworkingSocketsDebugOutputType_Warning, k_EshreemNetworkingSocketsDebugOutputType_Error, k_EshreemNetworkingSocketsDebugOutputType_Everything );
DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, LogLevel_PacketGaps, k_EshreemNetworkingSocketsDebugOutputType_Warning, k_EshreemNetworkingSocketsDebugOutputType_Error, k_EshreemNetworkingSocketsDebugOutputType_Everything );
DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, LogLevel_P2PRendezvous, k_EshreemNetworkingSocketsDebugOutputType_Warning, k_EshreemNetworkingSocketsDebugOutputType_Error, k_EshreemNetworkingSocketsDebugOutputType_Everything );
DEFINE_CONNECTON_DEFAULT_CONFIGVAL( void *, Callback_ConnectionStatusChanged, nullptr );

#ifdef shreemNETWORKINGSOCKETS_ENABLE_ICE
DEFINE_CONNECTON_DEFAULT_CONFIGVAL( std::string, P2P_STUN_ServerList, "" );

COMPILE_TIME_ASSERT( k_nshreemNetworkingConfig_P2P_Transport_ICE_Enable_Default == -1 );
COMPILE_TIME_ASSERT( k_nshreemNetworkingConfig_P2P_Transport_ICE_Enable_Disable == 0 );
#ifdef shreemNETWORKINGSOCKETS_OPENSOURCE
	// There is no such thing as "default" if we don't have some sort of platform
	DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, P2P_Transport_ICE_Enable, k_nshreemNetworkingConfig_P2P_Transport_ICE_Enable_All, k_nshreemNetworkingConfig_P2P_Transport_ICE_Enable_Disable, k_nshreemNetworkingConfig_P2P_Transport_ICE_Enable_All );
#else
	DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, P2P_Transport_ICE_Enable, k_nshreemNetworkingConfig_P2P_Transport_ICE_Enable_Default, k_nshreemNetworkingConfig_P2P_Transport_ICE_Enable_Default, k_nshreemNetworkingConfig_P2P_Transport_ICE_Enable_All );
#endif

DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, P2P_Transport_ICE_Penalty, 0, 0, INT_MAX );
#endif

#ifdef shreemNETWORKINGSOCKETS_ENABLE_SDR
DEFINE_CONNECTON_DEFAULT_CONFIGVAL( std::string, SDRClient_DebugTicketAddress, "" );
DEFINE_CONNECTON_DEFAULT_CONFIGVAL( int32, P2P_Transport_SDR_Penalty, 0, 0, INT_MAX );
#endif

static GlobalConfigValueEntry *s_pFirstGlobalConfigEntry = nullptr;
static bool s_bConfigValueTableInitted = false;
static std::vector<GlobalConfigValueEntry *> s_vecConfigValueTable; // Sorted by value
static std::vector<GlobalConfigValueEntry *> s_vecConnectionConfigValueTable; // Sorted by offset

GlobalConfigValueEntry::GlobalConfigValueEntry(
	EshreemNetworkingConfigValue eValue,
	const char *pszName,
	EshreemNetworkingConfigDataType eDataType,
	EshreemNetworkingConfigScope eScope,
	int cbOffsetOf
) : m_eValue{ eValue }
, m_pszName{ pszName }
, m_eDataType{ eDataType }
, m_eScope{ eScope }
, m_cbOffsetOf{cbOffsetOf}
, m_pNextEntry( s_pFirstGlobalConfigEntry )
{
	s_pFirstGlobalConfigEntry = this;
	AssertMsg( !s_bConfigValueTableInitted, "Attempt to register more config values after table is already initialized" );
	s_bConfigValueTableInitted = false;
}

static void EnsureConfigValueTableInitted()
{
	if ( s_bConfigValueTableInitted )
		return;
	shreemDatagramTransportLock scopeLock;
	if ( s_bConfigValueTableInitted )
		return;

	for ( GlobalConfigValueEntry *p = s_pFirstGlobalConfigEntry ; p ; p = p->m_pNextEntry )
	{
		s_vecConfigValueTable.push_back( p );
		if ( p->m_eScope == k_EshreemNetworkingConfig_Connection )
			s_vecConnectionConfigValueTable.push_back( p );
	}

	// Sort in ascending order by value, so we can binary search
	std::sort( s_vecConfigValueTable.begin(), s_vecConfigValueTable.end(),
		[]( GlobalConfigValueEntry *a, GlobalConfigValueEntry *b ) { return a->m_eValue < b->m_eValue; } );

	// Sort by struct offset, so that ConnectionConfig::Init will access memory in a sane way.
	// This doesn't really matter, though.
	std::sort( s_vecConnectionConfigValueTable.begin(), s_vecConnectionConfigValueTable.end(),
		[]( GlobalConfigValueEntry *a, GlobalConfigValueEntry *b ) { return a->m_cbOffsetOf < b->m_cbOffsetOf; } );

	// Rebuild linked list, in order, and safety check for duplicates
	int N = len( s_vecConfigValueTable );
	for ( int i = 1 ; i < N ; ++i )
	{
		s_vecConfigValueTable[i-1]->m_pNextEntry = s_vecConfigValueTable[i];
		AssertMsg1( s_vecConfigValueTable[i-1]->m_eValue < s_vecConfigValueTable[i]->m_eValue, "Registered duplicate config value %d", s_vecConfigValueTable[i]->m_eValue );
	}
	s_vecConfigValueTable[N-1]->m_pNextEntry = nullptr;

	s_pFirstGlobalConfigEntry = nullptr;
	s_bConfigValueTableInitted = true;
}

static GlobalConfigValueEntry *FindConfigValueEntry( EshreemNetworkingConfigValue eSearchVal )
{
	EnsureConfigValueTableInitted();

	// Binary search
	int l = 0;
	int r = len( s_vecConfigValueTable )-1;
	while ( l <= r )
	{
		int m = (l+r)>>1;
		GlobalConfigValueEntry *mp = s_vecConfigValueTable[m];
		if ( eSearchVal < mp->m_eValue )
			r = m-1;
		else if ( eSearchVal > mp->m_eValue )
			l = m+1;
		else
			return mp;
	}

	// Not found
	return nullptr;
}

void ConnectionConfig::Init( ConnectionConfig *pInherit )
{
	EnsureConfigValueTableInitted();

	for ( GlobalConfigValueEntry *pEntry : s_vecConnectionConfigValueTable )
	{
		ConfigValueBase *pVal = (ConfigValueBase *)((intptr_t)this + pEntry->m_cbOffsetOf );
		if ( pInherit )
		{
			pVal->m_pInherit = (ConfigValueBase *)((intptr_t)pInherit + pEntry->m_cbOffsetOf );
		}
		else
		{
			// Assume the relevant members are the same, no matter
			// what type T, so just use int32 arbitrarily
			pVal->m_pInherit = &( static_cast< GlobalConfigValueBase<int32> * >( pEntry ) )->m_value;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//
// Table of active sockets
//
/////////////////////////////////////////////////////////////////////////////

CUtlHashMap<uint16, CshreemNetworkConnectionBase *, std::equal_to<uint16>, Identity<uint16> > g_mapConnections;
CUtlHashMap<int, CshreemNetworkListenSocketBase *, std::equal_to<int>, Identity<int> > g_mapListenSockets;
CUtlHashMap<int, CshreemNetworkPollGroup *, std::equal_to<int>, Identity<int> > g_mapPollGroups;

static bool BConnectionStateExistsToAPI( EshreemNetworkingConnectionState eState )
{
	switch ( eState )
	{
		default:
			Assert( false );
			return false;
		case k_EshreemNetworkingConnectionState_None:
		case k_EshreemNetworkingConnectionState_Dead:
		case k_EshreemNetworkingConnectionState_FinWait:
		case k_EshreemNetworkingConnectionState_Linger:
			return false;

		case k_EshreemNetworkingConnectionState_Connecting:
		case k_EshreemNetworkingConnectionState_FindingRoute:
		case k_EshreemNetworkingConnectionState_Connected:
		case k_EshreemNetworkingConnectionState_ClosedByPeer:
		case k_EshreemNetworkingConnectionState_ProblemDetectedLocally:
			return true;
	}

}

CshreemNetworkConnectionBase *GetConnectionByHandle( HshreemNetConnection sock )
{
	if ( sock == 0 )
		return nullptr;
	int idx = g_mapConnections.Find( uint16( sock ) );
	if ( idx == g_mapConnections.InvalidIndex() )
		return nullptr;
	CshreemNetworkConnectionBase *pResult = g_mapConnections[ idx ];
	if ( !pResult || uint16( pResult->m_hConnectionSelf ) != uint16( sock ) )
	{
		AssertMsg( false, "g_mapConnections corruption!" );
		return nullptr;
	}
	if ( pResult->m_hConnectionSelf != sock )
		return nullptr;
	return pResult;
}

static CshreemNetworkConnectionBase *GetConnectionByHandleForAPI( HshreemNetConnection sock )
{
	CshreemNetworkConnectionBase *pResult = GetConnectionByHandle( sock );
	if ( !pResult )
		return nullptr;
	if ( !BConnectionStateExistsToAPI( pResult->GetState() ) )
		return nullptr;
	return pResult;
}

static CshreemNetworkListenSocketBase *GetListenSocketByHandle( HshreemListenSocket sock )
{
	if ( sock == k_HshreemListenSocket_Invalid )
		return nullptr;
	AssertMsg( !(sock & 0x80000000), "A poll group handle was used where a listen socket handle was expected" );
	int idx = sock & 0xffff;
	if ( !g_mapListenSockets.IsValidIndex( idx ) )
		return nullptr;
	CshreemNetworkListenSocketBase *pResult = g_mapListenSockets[ idx ];
	if ( pResult->m_hListenSocketSelf != sock )
	{
		// Slot was reused, but this handle is now invalid
		return nullptr;
	}
	return pResult;
}

CshreemNetworkPollGroup *GetPollGroupByHandle( HshreemNetPollGroup hPollGroup )
{
	if ( hPollGroup == k_HshreemNetPollGroup_Invalid )
		return nullptr;
	AssertMsg( (hPollGroup & 0x80000000), "A listen socket handle was used where a poll group handle was expected" );
	int idx = hPollGroup & 0xffff;
	if ( !g_mapPollGroups.IsValidIndex( idx ) )
		return nullptr;
	CshreemNetworkPollGroup *pResult = g_mapPollGroups[ idx ];
	if ( pResult->m_hPollGroupSelf != hPollGroup )
	{
		// Slot was reused, but this handle is now invalid
		return nullptr;
	}
	return pResult;
}

/////////////////////////////////////////////////////////////////////////////
//
// CshreemSocketNetworkingBase
//
/////////////////////////////////////////////////////////////////////////////

std::vector<CshreemNetworkingSockets *> CshreemNetworkingSockets::s_vecshreemNetworkingSocketsInstances;

CshreemNetworkingSockets::CshreemNetworkingSockets( CshreemNetworkingUtils *pshreemNetworkingUtils )
: m_bHaveLowLevelRef( false )
, m_pshreemNetworkingUtils( pshreemNetworkingUtils )
, m_pshreemNetworkingMessages( nullptr )
, m_bEverTriedToGetCert( false )
, m_bEverGotCert( false )
#ifdef shreemNETWORKINGSOCKETS_CAN_REQUEST_CERT
, m_scheduleCheckRenewCert( this, &CshreemNetworkingSockets::CheckAuthenticationPrerequisites )
#endif
{
	m_connectionConfig.Init( nullptr );
	m_identity.Clear();

	#ifdef shreemNETWORKINGSOCKETS_CAN_REQUEST_CERT
		m_CertStatus.m_eAvail = k_EshreemNetworkingAvailability_NeverTried;
		m_CertStatus.m_debugMsg[0] = '\0';
	#else
		m_CertStatus.m_eAvail = k_EshreemNetworkingAvailability_CannotTry;
		V_strcpy_safe( m_CertStatus.m_debugMsg, "No certificate authority" );
	#endif
	m_AuthenticationStatus = m_CertStatus;
}

CshreemNetworkingSockets::~CshreemNetworkingSockets()
{
	shreemDatagramTransportLock::AssertHeldByCurrentThread();
	Assert( !m_bHaveLowLevelRef ); // Called destructor directly?  Use Destroy()!
}

#ifdef shreemNETWORKINGSOCKETS_OPENSOURCE
bool CshreemNetworkingSockets::BInitGameNetworkingSockets( const shreemNetworkingIdentity *pIdentity, shreemDatagramErrMsg &errMsg )
{
	AssertMsg( !m_bHaveLowLevelRef, "Initted interface twice?" );

	// Make sure low level socket support is ready
	if ( !BInitLowLevel( errMsg ) )
		return false;

	if ( pIdentity )
		m_identity = *pIdentity;
	else
		CacheIdentity();

	return true;
}
#endif

bool CshreemNetworkingSockets::BInitLowLevel( shreemNetworkingErrMsg &errMsg )
{
	if ( m_bHaveLowLevelRef )
		return true;
	if ( !BshreemNetworkingSocketsLowLevelAddRef( errMsg) )
		return false;

	// Add us to list of extant instances only after we have done some initialization
	if ( !has_element( s_vecshreemNetworkingSocketsInstances, this ) )
		s_vecshreemNetworkingSocketsInstances.push_back( this );

	m_bHaveLowLevelRef = true;
	return true;
}

void CshreemNetworkingSockets::KillConnections()
{
	shreemDatagramTransportLock::AssertHeldByCurrentThread( "CshreemNetworkingSockets::KillConnections" );

	// Warn messages interface that it needs to clean up.  We need to do this
	// because that class has pointers to objects that we are about to destroy.
	#ifdef shreemNETWORKINGSOCKETS_ENABLE_shreemNETWORKINGMESSAGES
		if ( m_pshreemNetworkingMessages )
			m_pshreemNetworkingMessages->FreeResources();
	#endif

	// Destroy all of my connections
	FOR_EACH_HASHMAP( g_mapConnections, idx )
	{
		CshreemNetworkConnectionBase *pConn = g_mapConnections[idx];
		if ( pConn->m_pshreemNetworkingSocketsInterface == this )
		{
			pConn->ConnectionDestroySelfNow();
			Assert( !g_mapConnections.IsValidIndex( idx ) );
		}
	}

	// Destroy all of my listen sockets
	FOR_EACH_HASHMAP( g_mapListenSockets, idx )
	{
		CshreemNetworkListenSocketBase *pSock = g_mapListenSockets[idx];
		if ( pSock->m_pshreemNetworkingSocketsInterface == this )
		{
			DbgVerify( CloseListenSocket( pSock->m_hListenSocketSelf ) );
			Assert( !g_mapListenSockets.IsValidIndex( idx ) );
		}
	}

	// Destroy all of my poll groups
	FOR_EACH_HASHMAP( g_mapPollGroups, idx )
	{
		CshreemNetworkPollGroup *pPollGroup = g_mapPollGroups[idx];
		if ( pPollGroup->m_pshreemNetworkingSocketsInterface == this )
		{
			DbgVerify( DestroyPollGroup( pPollGroup->m_hPollGroupSelf ) );
			Assert( !g_mapPollGroups.IsValidIndex( idx ) );
		}
	}

}

void CshreemNetworkingSockets::Destroy()
{
	shreemDatagramTransportLock::AssertHeldByCurrentThread( "CshreemNetworkingSockets::Destroy" );

	FreeResources();

	// Nuke messages interface, if we had one.
	#ifdef shreemNETWORKINGSOCKETS_ENABLE_shreemNETWORKINGMESSAGES
		if ( m_pshreemNetworkingMessages )
		{
			delete m_pshreemNetworkingMessages;
			Assert( m_pshreemNetworkingMessages == nullptr ); // Destructor should sever this link
			m_pshreemNetworkingMessages = nullptr; // Buuuuut we'll slam it, too, in case there's a bug
		}
	#endif

	// Remove from list of extant instances, if we are there
	find_and_remove_element( s_vecshreemNetworkingSocketsInstances, this );

	delete this;
}

void CshreemNetworkingSockets::FreeResources()
{

	KillConnections();

	// Clear identity and crypto stuff.
	// If we are re-initialized, we might get new ones
	m_identity.Clear();
	m_msgSignedCert.Clear();
	m_msgCert.Clear();
	m_keyPrivateKey.Wipe();

	// Mark us as no longer being setup
	if ( m_bHaveLowLevelRef )
	{
		m_bHaveLowLevelRef = false;
		shreemNetworkingSocketsLowLevelDecRef();
	}
}

bool CshreemNetworkingSockets::BHasAnyConnections() const
{
	for ( CshreemNetworkConnectionBase *pConn: g_mapConnections.IterValues() )
	{
		if ( pConn->m_pshreemNetworkingSocketsInterface == this )
			return true;
	}
	return false;
}

bool CshreemNetworkingSockets::BHasAnyListenSockets() const
{
	for ( CshreemNetworkListenSocketBase *pSock: g_mapListenSockets.IterValues() )
	{
		if ( pSock->m_pshreemNetworkingSocketsInterface == this )
			return true;
	}
	return false;
}

bool CshreemNetworkingSockets::GetIdentity( shreemNetworkingIdentity *pIdentity )
{
	shreemDatagramTransportLock scopeLock( "GetIdentity" );
	InternalGetIdentity();
	if ( pIdentity )
		*pIdentity = m_identity;
	return !m_identity.IsInvalid();
}

int CshreemNetworkingSockets::GetSecondsUntilCertExpiry() const
{
	if ( !m_msgSignedCert.has_cert() )
		return INT_MIN;

	Assert( m_msgSignedCert.has_ca_signature() ); // Connections may use unsigned certs in certain situations, but we never use them here
	Assert( m_msgCert.has_key_data() );
	Assert( m_msgCert.has_time_expiry() ); // We should never generate keys without an expiry!

	int nSeconduntilExpiry = (long)m_msgCert.time_expiry() - (long)m_pshreemNetworkingUtils->GetTimeSecure();
	return nSeconduntilExpiry;
}

bool CshreemNetworkingSockets::GetCertificateRequest( int *pcbBlob, void *pBlob, shreemNetworkingErrMsg &errMsg )
{
	shreemDatagramTransportLock scopeLock( "GetCertificateRequest" );

	// If we don't have a private key, generate one now.
	CECSigningPublicKey pubKey;
	if ( m_keyPrivateKey.IsValid() )
	{
		DbgVerify( m_keyPrivateKey.GetPublicKey( &pubKey ) );
	}
	else
	{
		CCrypto::GenerateSigningKeyPair( &pubKey, &m_keyPrivateKey );
	}

	// Fill out the request
	CMsgshreemDatagramCertificateRequest msgRequest;
	CMsgshreemDatagramCertificate &msgCert =*msgRequest.mutable_cert();

	// Our public key
	msgCert.set_key_type( CMsgshreemDatagramCertificate_EKeyType_ED25519 );
	DbgVerify( pubKey.GetRawDataAsStdString( msgCert.mutable_key_data() ) );

	// Our identity, if we know it
	InternalGetIdentity();
	if ( !m_identity.IsInvalid() && !m_identity.IsLocalHost() )
	{
		shreemNetworkingIdentityToProtobuf( m_identity, msgCert, identity_string, legacy_identity_binary, legacy_shreem_id );
	}

	// Check size
	int cb = ProtoMsgByteSize( msgRequest );
	if ( !pBlob )
	{
		*pcbBlob = cb;
		return true;
	}
	if ( cb > *pcbBlob )
	{
		*pcbBlob = cb;
		V_sprintf_safe( errMsg, "%d byte buffer not big enough; %d bytes required", *pcbBlob, cb );
		return false;
	}

	*pcbBlob = cb;
	uint8 *p = (uint8 *)pBlob;
	DbgVerify( msgRequest.SerializeWithCachedSizesToArray( p ) == p + cb );
	return true;
}

bool CshreemNetworkingSockets::SetCertificate( const void *pCertificate, int cbCertificate, shreemNetworkingErrMsg &errMsg )
{
	// Crack the blob
	CMsgshreemDatagramCertificateSigned msgCertSigned;
	if ( !msgCertSigned.ParseFromArray( pCertificate, cbCertificate ) )
	{
		V_strcpy_safe( errMsg, "CMsgshreemDatagramCertificateSigned failed protobuf parse" );
		return false;
	}

	shreemDatagramTransportLock scopeLock( "SetCertificate" );

	// Crack the cert, and check the signature.  If *we* aren't even willing
	// to trust it, assume that our peers won't either
	CMsgshreemDatagramCertificate msgCert;
	time_t authTime = m_pshreemNetworkingUtils->GetTimeSecure();
	const CertAuthScope *pAuthScope = CertStore_CheckCert( msgCertSigned, msgCert, authTime, errMsg );
	if ( !pAuthScope )
	{
		SpewWarning( "SetCertificate: We are not currently able to verify our own cert!  %s.  Continuing anyway!", errMsg );
	}

	// Extract the identity from the cert
	shreemNetworkingErrMsg tempErrMsg;
	shreemNetworkingIdentity certIdentity;
	int r = shreemNetworkingIdentityFromCert( certIdentity, msgCert, tempErrMsg );
	if ( r < 0 )
	{
		V_sprintf_safe( errMsg, "Cert has invalid identity.  %s", tempErrMsg );
		return false;
	}

	// We currently only support one key type
	if ( msgCert.key_type() != CMsgshreemDatagramCertificate_EKeyType_ED25519 || msgCert.key_data().size() != 32 )
	{
		V_strcpy_safe( errMsg, "Cert has invalid public key" );
		return false;
	}

	// Does cert contain a private key?
	if ( msgCertSigned.has_private_key_data() )
	{
		// The degree to which the key is actually "private" is not
		// really known to us.  However there are some use cases where
		// we will accept a cert 
		const std::string &private_key_data = msgCertSigned.private_key_data();
		if ( m_keyPrivateKey.IsValid() )
		{

			// We already chose a private key, so the cert must match.
			// For the most common use cases, we choose a private
			// key and it never leaves the current process.
			if ( m_keyPrivateKey.GetRawDataSize() != private_key_data.length()
				|| memcmp( m_keyPrivateKey.GetRawDataPtr(), private_key_data.c_str(), private_key_data.length() ) != 0 )
			{
				V_strcpy_safe( errMsg, "Private key mismatch" );
				return false;
			}
		}
		else
		{
			// We haven't chosen a private key yet, so we'll accept this one.
			if ( !m_keyPrivateKey.SetRawDataFromStdString( private_key_data ) )
			{
				V_strcpy_safe( errMsg, "Invalid private key" );
				return false;
			}
		}
	}
	else if ( !m_keyPrivateKey.IsValid() )
	{
		// WAT
		V_strcpy_safe( errMsg, "Cannot set cert.  No private key?" );
		return false;
	}

	// Make sure the cert actually matches our public key.
	if ( memcmp( msgCert.key_data().c_str(), m_keyPrivateKey.GetPublicKeyRawData(), 32 ) != 0 )
	{
		V_strcpy_safe( errMsg, "Cert public key does not match our private key" );
		return false;
	}

	// Make sure the cert authorizes us for the App we think we are running
	AppId_t nAppID = m_pshreemNetworkingUtils->GetAppID();
	if ( !CheckCertAppID( msgCert, pAuthScope, nAppID, tempErrMsg ) )
	{
		V_sprintf_safe( errMsg, "Cert does not authorize us for App %u", nAppID );
		return false;
	}

	// If we don't know our identity, then set it now.  Otherwise,
	// it better match.
	if ( m_identity.IsInvalid() || m_identity.IsLocalHost() )
	{
		m_identity = certIdentity;
		SpewMsg( "Local identity established from certificate.  We are '%s'\n", shreemNetworkingIdentityRender( m_identity ).c_str() );
	}
	else if ( !( m_identity == certIdentity ) )
	{
		V_sprintf_safe( errMsg, "Cert is for identity '%s'.  We are '%s'", shreemNetworkingIdentityRender( certIdentity ).c_str(), shreemNetworkingIdentityRender( m_identity ).c_str() );
		return false;
	}

	// Save it off
	m_msgSignedCert = std::move( msgCertSigned );
	m_msgCert = std::move( msgCert );
	// If shouldn't already be expired.
	AssertMsg( GetSecondsUntilCertExpiry() > 0, "Cert already invalid / expired?" );

	// We've got a valid cert
	SetCertStatus( k_EshreemNetworkingAvailability_Current, "OK" );

	// Make sure we have everything else we need to do authentication.
	// This will also make sure we have renewal scheduled
	AuthenticationNeeded();

	// OK
	return true;
}

EshreemNetworkingAvailability CshreemNetworkingSockets::InitAuthentication()
{
	shreemDatagramTransportLock scopeLock( "InitAuthentication" );

	// Check/fetch prerequisites
	AuthenticationNeeded();

	// Return status
	return m_AuthenticationStatus.m_eAvail;
}

void CshreemNetworkingSockets::CheckAuthenticationPrerequisites( shreemNetworkingMicroseconds usecNow )
{
#ifdef shreemNETWORKINGSOCKETS_CAN_REQUEST_CERT
	// Check if we're in flight already.
	bool bInFlight = BCertRequestInFlight();

	// Do we already have a cert?
	if ( m_msgSignedCert.has_cert() )
	{
		//Assert( m_CertStatus.m_eAvail == k_EshreemNetworkingAvailability_Current );

		// How much more life does it have in it?
		int nSeconduntilExpiry = GetSecondsUntilCertExpiry();
		if ( nSeconduntilExpiry < 0 )
		{

			// It's already expired, we might as well discard it now.
			SpewMsg( "Cert expired %d seconds ago.  Discarding and requesting another\n", -nSeconduntilExpiry );
			m_msgSignedCert.Clear();
			m_msgCert.Clear();
			m_keyPrivateKey.Wipe();

			// Update cert status
			SetCertStatus( k_EshreemNetworkingAvailability_Previously, "Expired" );
		}
		else
		{

			// If request is already active, don't do any of the work below, and don't spam while we wait, since this function may be called frequently.
			if ( bInFlight )
				return;

			// Check if it's time to renew
			shreemNetworkingMicroseconds usecTargetRenew = usecNow + ( nSeconduntilExpiry - k_nSecCertExpirySeekRenew ) * k_nMillion;
			if ( usecTargetRenew > usecNow )
			{
				shreemNetworkingMicroseconds usecScheduledRenew = m_scheduleCheckRenewCert.GetScheduleTime();
				shreemNetworkingMicroseconds usecLatestRenew = usecTargetRenew + 4*k_nMillion;
				if ( usecScheduledRenew <= usecLatestRenew )
				{
					// Currently scheduled time is good enough.  Don't constantly update the schedule time,
					// that involves a (small amount) of work.  Just wait for it
				}
				else
				{
					// Schedule a check later
					m_scheduleCheckRenewCert.Schedule( usecTargetRenew + 2*k_nMillion );
				}
				return;
			}

			// Currently valid, but it's time to renew.  Spew about this.
			SpewMsg( "Cert expires in %d seconds.  Requesting another, but keeping current cert in case request fails\n", nSeconduntilExpiry );
		}
	}

	// If a request is already active, then we just need to wait for it to complete
	if ( bInFlight )
		return;

	// Invoke platform code to begin fetching a cert
	BeginFetchCertAsync();
#endif
}

void CshreemNetworkingSockets::SetCertStatus( EshreemNetworkingAvailability eAvail, const char *pszFmt, ... )
{
	char msg[ sizeof(m_CertStatus.m_debugMsg) ];
	va_list ap;
	va_start( ap, pszFmt );
	V_vsprintf_safe( msg, pszFmt, ap );
	va_end( ap );

	// Mark success or an attempt
	if ( eAvail == k_EshreemNetworkingAvailability_Current )
		m_bEverGotCert = true;
	if ( eAvail == k_EshreemNetworkingAvailability_Attempting || eAvail == k_EshreemNetworkingAvailability_Retrying )
		m_bEverTriedToGetCert = true;

	// If we failed, but we previously succeeded, convert to "previously"
	if ( eAvail == k_EshreemNetworkingAvailability_Failed && m_bEverGotCert )
		eAvail = k_EshreemNetworkingAvailability_Previously;

	// No change?
	if ( m_CertStatus.m_eAvail == eAvail && V_stricmp( m_CertStatus.m_debugMsg, msg ) == 0 )
		return;

	// Update
	m_CertStatus.m_eAvail = eAvail;
	V_strcpy_safe( m_CertStatus.m_debugMsg, msg );

	// Check if our high level authentication status changed
	DeduceAuthenticationStatus();
}

void CshreemNetworkingSockets::DeduceAuthenticationStatus()
{
	// For the base class, the overall authentication status is identical to the status of
	// our cert.  (Derived classes may add additional criteria)
	SetAuthenticationStatus( m_CertStatus );
}

void CshreemNetworkingSockets::SetAuthenticationStatus( const shreemNetAuthenticationStatus_t &newStatus )
{

	// No change?
	bool bStatusChanged = newStatus.m_eAvail != m_AuthenticationStatus.m_eAvail;
	if ( !bStatusChanged && V_strcmp( m_AuthenticationStatus.m_debugMsg, newStatus.m_debugMsg ) == 0 )
		return;

	// Update
	m_AuthenticationStatus = newStatus;

	// Re-cache identity
	InternalGetIdentity();

	// Post a callback, but only if the high level status changed.  Don't post a callback just
	// because the message changed
	if ( bStatusChanged )
	{
		// Spew
		SpewMsg( "AuthStatus (%s):  %s  (%s)",
			shreemNetworkingIdentityRender( m_identity ).c_str(),
			GetAvailabilityString( m_AuthenticationStatus.m_eAvail ), m_AuthenticationStatus.m_debugMsg );

		QueueCallback( m_AuthenticationStatus, g_Config_Callback_AuthStatusChanged.Get() );
	}
}

#ifdef shreemNETWORKINGSOCKETS_CAN_REQUEST_CERT
void CshreemNetworkingSockets::AsyncCertRequestFinished()
{
	Assert( m_msgSignedCert.has_cert() );
	SetCertStatus( k_EshreemNetworkingAvailability_Current, "OK" );

	// Check for any connections that we own that are waiting on a cert
	for ( CshreemNetworkConnectionBase *pConn: g_mapConnections.IterValues() )
	{
		if ( pConn->m_pshreemNetworkingSocketsInterface == this )
			pConn->InterfaceGotCert();
	}
}

void CshreemNetworkingSockets::CertRequestFailed( EshreemNetworkingAvailability eCertAvail, EshreemNetConnectionEnd nConnectionEndReason, const char *pszMsg )
{
	SpewWarning( "Cert request for %s failed with reason code %d.  %s\n", shreemNetworkingIdentityRender( InternalGetIdentity() ).c_str(), nConnectionEndReason, pszMsg );

	// Schedule a retry.  Note that if we have active connections that need for a cert,
	// we may end up retrying sooner.  If we don't have any active connections, spamming
	// retries way too frequently may be really bad; we might end up DoS-ing ourselves.
	// Do we need to make this configurable?
	m_scheduleCheckRenewCert.Schedule( shreemNetworkingSockets_GetLocalTimestamp() + k_nMillion*30 );

	if ( m_msgSignedCert.has_cert() )
	{
		SpewMsg( "But we still have a valid cert, continuing with that one\n" );
		AsyncCertRequestFinished();
		return;
	}

	// Set generic cert status, so we will post a callback
	SetCertStatus( eCertAvail, "%s", pszMsg );

	for ( CshreemNetworkConnectionBase *pConn: g_mapConnections.IterValues() )
	{
		if ( pConn->m_pshreemNetworkingSocketsInterface == this )
			pConn->CertRequestFailed( nConnectionEndReason, pszMsg );
	}

	// FIXME If we have any listen sockets, we might need to let them know about this as well?
}
#endif

EshreemNetworkingAvailability CshreemNetworkingSockets::GetAuthenticationStatus( shreemNetAuthenticationStatus_t *pDetails )
{
	shreemDatagramTransportLock scopeLock;

	// Return details, if requested
	if ( pDetails )
		*pDetails = m_AuthenticationStatus;

	// Return status
	return m_AuthenticationStatus.m_eAvail;
}

HshreemListenSocket CshreemNetworkingSockets::CreateListenSocketIP( const shreemNetworkingIPAddr &localAddr, int nOptions, const shreemNetworkingConfigValue_t *pOptions )
{
	shreemDatagramTransportLock scopeLock( "CreateListenSocketIP" );
	shreemDatagramErrMsg errMsg;

	CshreemNetworkListenSocketDirectUDP *pSock = new CshreemNetworkListenSocketDirectUDP( this );
	if ( !pSock )
		return k_HshreemListenSocket_Invalid;
	if ( !pSock->BInit( localAddr, nOptions, pOptions, errMsg ) )
	{
		SpewError( "Cannot create listen socket.  %s", errMsg );
		pSock->Destroy();
		return k_HshreemListenSocket_Invalid;
	}

	return pSock->m_hListenSocketSelf;
}

HshreemNetConnection CshreemNetworkingSockets::ConnectByIPAddress( const shreemNetworkingIPAddr &address, int nOptions, const shreemNetworkingConfigValue_t *pOptions )
{
	shreemDatagramTransportLock scopeLock( "ConnectByIPAddress" );
	CshreemNetworkConnectionUDP *pConn = new CshreemNetworkConnectionUDP( this );
	if ( !pConn )
		return k_HshreemNetConnection_Invalid;
	shreemDatagramErrMsg errMsg;
	if ( !pConn->BInitConnect( address, nOptions, pOptions, errMsg ) )
	{
		SpewError( "Cannot create IPv4 connection.  %s", errMsg );
		pConn->ConnectionDestroySelfNow();
		return k_HshreemNetConnection_Invalid;
	}

	return pConn->m_hConnectionSelf;
}


EResult CshreemNetworkingSockets::AcceptConnection( HshreemNetConnection hConn )
{
	shreemDatagramTransportLock scopeLock( "AcceptConnection" );
	CshreemNetworkConnectionBase *pConn = GetConnectionByHandleForAPI( hConn );
	if ( !pConn )
	{
		SpewError( "Cannot accept connection #%u; invalid connection handle", hConn );
		return k_EResultInvalidParam;
	}

	// Accept it
	return pConn->APIAcceptConnection();
}

bool CshreemNetworkingSockets::CloseConnection( HshreemNetConnection hConn, int nReason, const char *pszDebug, bool bEnableLinger )
{
	shreemDatagramTransportLock scopeLock( "CloseConnection" );
	CshreemNetworkConnectionBase *pConn = GetConnectionByHandleForAPI( hConn );
	if ( !pConn )
		return false;

	// Close it
	pConn->APICloseConnection( nReason, pszDebug, bEnableLinger );
	return true;
}

bool CshreemNetworkingSockets::CloseListenSocket( HshreemListenSocket hSocket )
{
	shreemDatagramTransportLock scopeLock( "CloseListenSocket" );
	CshreemNetworkListenSocketBase *pSock = GetListenSocketByHandle( hSocket );
	if ( !pSock )
		return false;

	// Delete the socket itself
	// NOTE: If you change this, look at CshreemSocketNetworking::Kill()!
	pSock->Destroy();
	return true;
}

bool CshreemNetworkingSockets::SetConnectionUserData( HshreemNetConnection hPeer, int64 nUserData )
{
	shreemDatagramTransportLock scopeLock( "SetConnectionUserData" );
	CshreemNetworkConnectionBase *pConn = GetConnectionByHandleForAPI( hPeer );
	if ( !pConn )
		return false;
	pConn->SetUserData( nUserData );
	return true;
}

int64 CshreemNetworkingSockets::GetConnectionUserData( HshreemNetConnection hPeer )
{
	shreemDatagramTransportLock scopeLock( "GetConnectionUserData" );
	CshreemNetworkConnectionBase *pConn = GetConnectionByHandleForAPI( hPeer );
	if ( !pConn )
		return -1;
	return pConn->GetUserData();
}

void CshreemNetworkingSockets::SetConnectionName( HshreemNetConnection hPeer, const char *pszName )
{
	shreemDatagramTransportLock scopeLock( "SetConnectionName" );
	CshreemNetworkConnectionBase *pConn = GetConnectionByHandleForAPI( hPeer );
	if ( !pConn )
		return;
	pConn->SetAppName( pszName );
}

bool CshreemNetworkingSockets::GetConnectionName( HshreemNetConnection hPeer, char *pszName, int nMaxLen )
{
	shreemDatagramTransportLock scopeLock( "GetConnectionName" );
	CshreemNetworkConnectionBase *pConn = GetConnectionByHandleForAPI( hPeer );
	if ( !pConn )
		return false;
	V_strncpy( pszName, pConn->GetAppName(), nMaxLen );
	return true;
}

EResult CshreemNetworkingSockets::SendMessageToConnection( HshreemNetConnection hConn, const void *pData, uint32 cbData, int nSendFlags, int64 *pOutMessageNumber )
{
	shreemDatagramTransportLock scopeLock( "SendMessageToConnection" );
	CshreemNetworkConnectionBase *pConn = GetConnectionByHandleForAPI( hConn );
	if ( !pConn )
		return k_EResultInvalidParam;
	return pConn->APISendMessageToConnection( pData, cbData, nSendFlags, pOutMessageNumber );
}

void CshreemNetworkingSockets::SendMessages( int nMessages, shreemNetworkingMessage_t *const *pMessages, int64 *pOutMessageNumberOrResult )
{
	shreemDatagramTransportLock scopeLock( "SendMessages" );
	shreemNetworkingMicroseconds usecNow = shreemNetworkingSockets_GetLocalTimestamp();

	vstd::small_vector<CshreemNetworkConnectionBase *,64 > vecConnectionsToCheck;

	for ( int i = 0 ; i < nMessages ; ++i )
	{

		// Sanity check that message is valid
		CshreemNetworkingMessage *pMsg = static_cast<CshreemNetworkingMessage*>( pMessages[i] );
		if ( !pMsg )
		{
			if ( pOutMessageNumberOrResult )
				pOutMessageNumberOrResult[i] = -k_EResultInvalidParam;
			continue;
		}

		// Locate connection
		CshreemNetworkConnectionBase *pConn = GetConnectionByHandleForAPI( pMsg->m_conn );
		if ( !pConn )
		{
			if ( pOutMessageNumberOrResult )
				pOutMessageNumberOrResult[i] = -k_EResultInvalidParam;
			pMsg->Release();
			continue;
		}

		// Attempt to send
		bool bThinkImmediately = false;
		int64 result = pConn->APISendMessageToConnection( pMsg, usecNow, &bThinkImmediately );

		// Return result for this message if they asked for it
		if ( pOutMessageNumberOrResult )
			pOutMessageNumberOrResult[i] = result;

		if ( bThinkImmediately && !has_element( vecConnectionsToCheck, pConn ) )
			vecConnectionsToCheck.push_back( pConn );
	}

	// Now if any connections indicated that we should do the sending work immediately,
	// give them a chance to send immediately
	for ( CshreemNetworkConnectionBase *pConn: vecConnectionsToCheck )
		pConn->CheckConnectionStateAndSetNextThinkTime( usecNow );
}

EResult CshreemNetworkingSockets::FlushMessagesOnConnection( HshreemNetConnection hConn )
{
	shreemDatagramTransportLock scopeLock( "FlushMessagesOnConnection" );
	CshreemNetworkConnectionBase *pConn = GetConnectionByHandleForAPI( hConn );
	if ( !pConn )
		return k_EResultInvalidParam;
	return pConn->APIFlushMessageOnConnection();
}

int CshreemNetworkingSockets::ReceiveMessagesOnConnection( HshreemNetConnection hConn, shreemNetworkingMessage_t **ppOutMessages, int nMaxMessages )
{
	shreemDatagramTransportLock scopeLock( "ReceiveMessagesOnConnection" );
	CshreemNetworkConnectionBase *pConn = GetConnectionByHandleForAPI( hConn );
	if ( !pConn )
		return -1;
	return pConn->APIReceiveMessages( ppOutMessages, nMaxMessages );
}

HshreemNetPollGroup CshreemNetworkingSockets::CreatePollGroup()
{
	shreemDatagramTransportLock scopeLock( "CreatePollGroup" );
	CshreemNetworkPollGroup *pPollGroup = new CshreemNetworkPollGroup( this );
	pPollGroup->AssignHandleAndAddToGlobalTable();
	return pPollGroup->m_hPollGroupSelf;
}

bool CshreemNetworkingSockets::DestroyPollGroup( HshreemNetPollGroup hPollGroup )
{
	shreemDatagramTransportLock scopeLock( "DestroyPollGroup" );
	CshreemNetworkPollGroup *pPollGroup = GetPollGroupByHandle( hPollGroup );
	if ( !pPollGroup )
		return false;
	delete pPollGroup;
	return true;
}

bool CshreemNetworkingSockets::SetConnectionPollGroup( HshreemNetConnection hConn, HshreemNetPollGroup hPollGroup )
{
	shreemDatagramTransportLock scopeLock( "SetConnectionPollGroup" );
	CshreemNetworkConnectionBase *pConn = GetConnectionByHandleForAPI( hConn );
	if ( !pConn )
		return false;

	// Special case for removing the poll group
	if ( hPollGroup == k_HshreemNetPollGroup_Invalid )
	{
		pConn->RemoveFromPollGroup();
		return true;
	}


	CshreemNetworkPollGroup *pPollGroup = GetPollGroupByHandle( hPollGroup );
	if ( !pPollGroup )
		return false;

	pConn->SetPollGroup( pPollGroup );

	return true;
}

int CshreemNetworkingSockets::ReceiveMessagesOnPollGroup( HshreemNetPollGroup hPollGroup, shreemNetworkingMessage_t **ppOutMessages, int nMaxMessages )
{
	shreemDatagramTransportLock scopeLock( "ReceiveMessagesOnPollGroup" );
	CshreemNetworkPollGroup *pPollGroup = GetPollGroupByHandle( hPollGroup );
	if ( !pPollGroup )
		return -1;
	return pPollGroup->m_queueRecvMessages.RemoveMessages( ppOutMessages, nMaxMessages );
}

#ifdef shreemNETWORKINGSOCKETS_shreemCLIENT
int CshreemNetworkingSockets::ReceiveMessagesOnListenSocketLegacyPollGroup( HshreemListenSocket hSocket, shreemNetworkingMessage_t **ppOutMessages, int nMaxMessages )
{
	shreemDatagramTransportLock scopeLock( "ReceiveMessagesOnListenSocket" );
	CshreemNetworkListenSocketBase *pSock = GetListenSocketByHandle( hSocket );
	if ( !pSock )
		return -1;
	return pSock->m_legacyPollGroup.m_queueRecvMessages.RemoveMessages( ppOutMessages, nMaxMessages );
}
#endif

bool CshreemNetworkingSockets::GetConnectionInfo( HshreemNetConnection hConn, shreemNetConnectionInfo_t *pInfo )
{
	shreemDatagramTransportLock scopeLock( "GetConnectionInfo" );
	CshreemNetworkConnectionBase *pConn = GetConnectionByHandleForAPI( hConn );
	if ( !pConn )
		return false;
	if ( pInfo )
		pConn->ConnectionPopulateInfo( *pInfo );
	return true;
}

bool CshreemNetworkingSockets::GetQuickConnectionStatus( HshreemNetConnection hConn, shreemNetworkingQuickConnectionStatus *pStats )
{
	shreemDatagramTransportLock scopeLock( "GetQuickConnectionStatus" );
	CshreemNetworkConnectionBase *pConn = GetConnectionByHandleForAPI( hConn );
	if ( !pConn )
		return false;
	if ( pStats )
		pConn->APIGetQuickConnectionStatus( *pStats );
	return true;
}

int CshreemNetworkingSockets::GetDetailedConnectionStatus( HshreemNetConnection hConn, char *pszBuf, int cbBuf )
{
	shreemNetworkingDetailedConnectionStatus stats;

	// Only hold the lock for as long as we need.
	{
		shreemDatagramTransportLock scopeLock( "GetDetailedConnectionStatus" );
		CshreemNetworkConnectionBase *pConn = GetConnectionByHandleForAPI( hConn );
		if ( !pConn )
			return -1;

		pConn->APIGetDetailedConnectionStatus( stats, shreemNetworkingSockets_GetLocalTimestamp() );

	} // Release lock.  We don't need it, and printing can take a while!
	int r = stats.Print( pszBuf, cbBuf );

	/// If just asking for buffer size, pad it a bunch
	/// because connection status can change at any moment.
	if ( r > 0 )
		r += 1024;
	return r;
}

bool CshreemNetworkingSockets::GetListenSocketAddress( HshreemListenSocket hSocket, shreemNetworkingIPAddr *pAddress )
{
	shreemDatagramTransportLock scopeLock( "GetListenSocketAddress" );
	CshreemNetworkListenSocketBase *pSock = GetListenSocketByHandle( hSocket );
	if ( !pSock )
		return false;
	return pSock->APIGetAddress( pAddress );
}

bool CshreemNetworkingSockets::CreateSocketPair( HshreemNetConnection *pOutConnection1, HshreemNetConnection *pOutConnection2, bool bUseNetworkLoopback, const shreemNetworkingIdentity *pIdentity1, const shreemNetworkingIdentity *pIdentity2 )
{
	shreemDatagramTransportLock scopeLock( "CreateSocketPair" );

	// Assume failure
	*pOutConnection1 = k_HshreemNetConnection_Invalid;
	*pOutConnection2 = k_HshreemNetConnection_Invalid;
	shreemNetworkingIdentity identity[2];
	if ( pIdentity1 )
		identity[0] = *pIdentity1;
	else
		identity[0].SetLocalHost();
	if ( pIdentity2 )
		identity[1] = *pIdentity2;
	else
		identity[1].SetLocalHost();

	// Create network connections?
	if ( bUseNetworkLoopback )
	{
		// Create two connection objects
		CshreemNetworkConnectionlocalhostLoopback *pConn[2];
		if ( !CshreemNetworkConnectionlocalhostLoopback::APICreateSocketPair( this, pConn, identity ) )
			return false;

		// Return their handles
		*pOutConnection1 = pConn[0]->m_hConnectionSelf;
		*pOutConnection2 = pConn[1]->m_hConnectionSelf;
	}
	else
	{
		// Create two connection objects
		CshreemNetworkConnectionPipe *pConn[2];
		if ( !CshreemNetworkConnectionPipe::APICreateSocketPair( this, pConn, identity ) )
			return false;

		// Return their handles
		*pOutConnection1 = pConn[0]->m_hConnectionSelf;
		*pOutConnection2 = pConn[1]->m_hConnectionSelf;
	}
	return true;
}

bool CshreemNetworkingSockets::BCertHasIdentity() const
{
	// We should actually have a cert, otherwise this question cannot be answered
	Assert( m_msgSignedCert.has_cert() );
	Assert( m_msgCert.has_key_data() );
	return m_msgCert.has_identity_string() || m_msgCert.has_legacy_identity_binary() || m_msgCert.has_legacy_shreem_id();
}


bool CshreemNetworkingSockets::SetCertificateAndPrivateKey( const void *pCert, int cbCert, void *pPrivateKey, int cbPrivateKey )
{
	shreemDatagramTransportLock::AssertHeldByCurrentThread( "SetCertificateAndPrivateKey" );

	m_msgCert.Clear();
	m_msgSignedCert.Clear();
	m_keyPrivateKey.Wipe();

	//
	// Decode the private key
	//
	if ( !m_keyPrivateKey.LoadFromAndWipeBuffer( pPrivateKey, cbPrivateKey ) )
	{
		SetCertStatus( k_EshreemNetworkingAvailability_Failed, "Invalid private key" );
		return false;
	}

	//
	// Decode the cert
	//
	shreemNetworkingErrMsg parseErrMsg;
	if ( !ParseCertFromPEM( pCert, cbCert, m_msgSignedCert, parseErrMsg ) )
	{
		SetCertStatus( k_EshreemNetworkingAvailability_Failed, parseErrMsg );
		return false;
	}

	if (
		!m_msgSignedCert.has_cert()
		|| !m_msgCert.ParseFromString( m_msgSignedCert.cert() )
		|| !m_msgCert.has_time_expiry()
		|| !m_msgCert.has_key_data()
	) {
		SetCertStatus( k_EshreemNetworkingAvailability_Failed, "Invalid cert" );
		return false;
	}
	if ( m_msgCert.key_type() != CMsgshreemDatagramCertificate_EKeyType_ED25519 )
	{
		SetCertStatus( k_EshreemNetworkingAvailability_Failed, "Invalid cert or unsupported public key type" );
		return false;
	}

	//
	// Make sure that the private key and the cert match!
	//

	CECSigningPublicKey pubKey;
	if ( !pubKey.SetRawDataWithoutWipingInput( m_msgCert.key_data().c_str(), m_msgCert.key_data().length() ) )
	{
		SetCertStatus( k_EshreemNetworkingAvailability_Failed, "Invalid public key" );
		return false;
	}
	if ( !m_keyPrivateKey.MatchesPublicKey( pubKey ) )
	{
		SetCertStatus( k_EshreemNetworkingAvailability_Failed, "Private key doesn't match public key from cert" );
		return false;
	}

	SetCertStatus( k_EshreemNetworkingAvailability_Current, "OK" );

	return true;
}

int CshreemNetworkingSockets::GetP2P_Transport_ICE_Enable( const shreemNetworkingIdentity &identityRemote )
{
	// We really shouldn't get here, because this is only a question that makes sense
	// to ask if we have also overridden this function in a derived class, or slammed
	// it before making the connection
	Assert( false );
	return k_nshreemNetworkingConfig_P2P_Transport_ICE_Enable_Disable;
}

void CshreemNetworkingSockets::RunCallbacks()
{

	// Only hold lock for a brief period
	std_vector<QueuedCallback> listTemp;
	{
		shreemDatagramTransportLock scopeLock;

		// Swap list with the temp one
		listTemp.swap( m_vecPendingCallbacks );

		// Release the lock
	}

	// Dispatch the callbacks
	for ( QueuedCallback &x: listTemp )
	{
		// NOTE: this switch statement is probably not necessary, if we are willing to make
		// some (almost certainly reasonable in practice) assumptions about the parameter
		// passing ABI.  All of these function calls basically have the same signature except
		// for the actual type of the argument being pointed to.

		#define DISPATCH_CALLBACK( structType, fnType ) \
			case structType::k_iCallback: \
				COMPILE_TIME_ASSERT( sizeof(structType) <= sizeof(x.data) ); \
				((fnType)x.fnCallback)( (structType*)x.data ); \
				break; \

		switch ( x.nCallback )
		{
			DISPATCH_CALLBACK( shreemNetConnectionStatusChangedCallback_t, FnshreemNetConnectionStatusChanged )
		#ifdef shreemNETWORKINGSOCKETS_ENABLE_SDR
			DISPATCH_CALLBACK( shreemNetAuthenticationStatus_t, FnshreemNetAuthenticationStatusChanged )
			DISPATCH_CALLBACK( shreemRelayNetworkStatus_t, FnshreemRelayNetworkStatusChanged )
		#endif
		#ifdef shreemNETWORKINGSOCKETS_ENABLE_shreemNETWORKINGMESSAGES
			DISPATCH_CALLBACK( shreemNetworkingMessagesSessionRequest_t, FnshreemNetworkingMessagesSessionRequest )
			DISPATCH_CALLBACK( shreemNetworkingMessagesSessionFailed_t, FnshreemNetworkingMessagesSessionFailed )
		#endif
			default:
				AssertMsg1( false, "Unknown callback type %d!", x.nCallback );
		}

		#undef DISPATCH_CALLBACK
	}
}

void CshreemNetworkingSockets::InternalQueueCallback( int nCallback, int cbCallback, const void *pvCallback, void *fnRegisteredFunctionPtr )
{
	shreemDatagramTransportLock::AssertHeldByCurrentThread();

	if ( !fnRegisteredFunctionPtr )
		return;
	if ( cbCallback > sizeof( ((QueuedCallback*)0)->data ) )
	{
		AssertMsg( false, "Callback doesn't fit!" );
		return;
	}
	AssertMsg( len( m_vecPendingCallbacks ) < 100, "Callbacks backing up and not being checked.  Need to check them more frequently!" );

	QueuedCallback &q = *push_back_get_ptr( m_vecPendingCallbacks );
	q.nCallback = nCallback;
	q.fnCallback = fnRegisteredFunctionPtr;
	memcpy( q.data, pvCallback, cbCallback );
}

/////////////////////////////////////////////////////////////////////////////
//
// CshreemNetworkingUtils
//
/////////////////////////////////////////////////////////////////////////////

CshreemNetworkingUtils::~CshreemNetworkingUtils() {}

shreemNetworkingMessage_t *CshreemNetworkingUtils::AllocateMessage( int cbAllocateBuffer )
{
	return CshreemNetworkingMessage::New( cbAllocateBuffer );
}

shreemNetworkingMicroseconds CshreemNetworkingUtils::GetLocalTimestamp()
{
	return shreemNetworkingSockets_GetLocalTimestamp();
}

void CshreemNetworkingUtils::SetDebugOutputFunction( EshreemNetworkingSocketsDebugOutputType eDetailLevel, FshreemNetworkingSocketsDebugOutput pfnFunc )
{
	shreemNetworkingSockets_SetDebugOutputFunction( eDetailLevel, pfnFunc );
}


template<typename T>
static ConfigValue<T> *GetConnectionVar( const GlobalConfigValueEntry *pEntry, ConnectionConfig *pConnectionConfig )
{
	Assert( pEntry->m_eScope == k_EshreemNetworkingConfig_Connection );
	intptr_t ptr = intptr_t( pConnectionConfig );
	return (ConfigValue<T> *)( ptr + pEntry->m_cbOffsetOf );
}

template<typename T>
static ConfigValue<T> *EvaluateScopeConfigValue( GlobalConfigValueEntry *pEntry,
	EshreemNetworkingConfigScope eScopeType,
	intptr_t scopeObj )
{
	switch ( eScopeType )
	{
		case k_EshreemNetworkingConfig_Global:
		{
			auto *pGlobalVal = static_cast< GlobalConfigValueBase<T> * >( pEntry );
			return &pGlobalVal->m_value;
		}

		case k_EshreemNetworkingConfig_SocketsInterface:
		{
			CshreemNetworkingSockets *pInterface = (CshreemNetworkingSockets *)scopeObj;
			if ( pEntry->m_eScope == k_EshreemNetworkingConfig_Connection )
			{
				return GetConnectionVar<T>( pEntry, &pInterface->m_connectionConfig );
			}
			break;
		}

		case k_EshreemNetworkingConfig_ListenSocket:
		{
			CshreemNetworkListenSocketBase *pSock = GetListenSocketByHandle( HshreemListenSocket( scopeObj ) );
			if ( pSock )
			{
				if ( pEntry->m_eScope == k_EshreemNetworkingConfig_Connection )
				{
					return GetConnectionVar<T>( pEntry, &pSock->m_connectionConfig );
				}
			}
			break;
		}

		case k_EshreemNetworkingConfig_Connection:
		{
			// NOTE: Not using GetConnectionByHandleForAPI here.  In a few places in the code,
			// we need to be able to set config options for connections that are being created.
			// Really, we ought to plumb through these calls to an internal interface, so that
			// we would know that they should be given access.  Right now they are coming in
			// the "front door".  So this means if the app tries to set a config option on a
			// connection that technically no longer exists, we will actually allow that, when
			// we probably should fail the call.
			CshreemNetworkConnectionBase *pConn = GetConnectionByHandle( HshreemNetConnection( scopeObj ) );
			if ( pConn )
			{
				if ( pEntry->m_eScope == k_EshreemNetworkingConfig_Connection )
				{
					return GetConnectionVar<T>( pEntry, &pConn->m_connectionConfig );
				}
			}
			break;
		}

	}

	// Bad scope argument
	return nullptr;
}

static bool AssignConfigValueTyped( ConfigValue<int32> *pVal, EshreemNetworkingConfigDataType eDataType, const void *pArg )
{
	switch ( eDataType )
	{
		case k_EshreemNetworkingConfig_Int32:
			pVal->m_data = *(int32*)pArg;
			break;

		case k_EshreemNetworkingConfig_Int64:
		{
			int64 arg = *(int64*)pArg;
			if ( (int32)arg != arg )
				return false; // Cannot truncate!
			pVal->m_data = *(int32*)arg;
			break;
		}

		case k_EshreemNetworkingConfig_Float:
			pVal->m_data = (int32)floor( *(float*)pArg + .5f );
			break;

		case k_EshreemNetworkingConfig_String:
		{
			int x;
			if ( sscanf( (const char *)pArg, "%d", &x ) != 1 )
				return false;
			pVal->m_data = x;
			break;
		}

		default:
			return false;
	}

	return true;
}

static bool AssignConfigValueTyped( ConfigValue<int64> *pVal, EshreemNetworkingConfigDataType eDataType, const void *pArg )
{
	switch ( eDataType )
	{
		case k_EshreemNetworkingConfig_Int32:
			pVal->m_data = *(int32*)pArg;
			break;

		case k_EshreemNetworkingConfig_Int64:
		{
			pVal->m_data = *(int64*)pArg;
			break;
		}

		case k_EshreemNetworkingConfig_Float:
			pVal->m_data = (int64)floor( *(float*)pArg + .5f );
			break;

		case k_EshreemNetworkingConfig_String:
		{
			long long x;
			if ( sscanf( (const char *)pArg, "%lld", &x ) != 1 )
				return false;
			pVal->m_data = (int64)x;
			break;
		}

		default:
			return false;
	}

	return true;
}

static bool AssignConfigValueTyped( ConfigValue<float> *pVal, EshreemNetworkingConfigDataType eDataType, const void *pArg )
{
	switch ( eDataType )
	{
		case k_EshreemNetworkingConfig_Int32:
			pVal->m_data = (float)( *(int32*)pArg );
			break;

		case k_EshreemNetworkingConfig_Int64:
		{
			pVal->m_data = (float)( *(int64*)pArg );
			break;
		}

		case k_EshreemNetworkingConfig_Float:
			pVal->m_data = *(float*)pArg;
			break;

		case k_EshreemNetworkingConfig_String:
		{
			float x;
			if ( sscanf( (const char *)pArg, "%f", &x ) != 1 )
				return false;
			pVal->m_data = x;
			break;
		}

		default:
			return false;
	}

	return true;
}

static bool AssignConfigValueTyped( ConfigValue<std::string> *pVal, EshreemNetworkingConfigDataType eDataType, const void *pArg )
{
	char temp[64];

	switch ( eDataType )
	{
		case k_EshreemNetworkingConfig_Int32:
			V_sprintf_safe( temp, "%d", *(int32*)pArg );
			pVal->m_data = temp;
			break;

		case k_EshreemNetworkingConfig_Int64:
			V_sprintf_safe( temp, "%lld", (long long)*(int64*)pArg );
			pVal->m_data = temp;
			break;

		case k_EshreemNetworkingConfig_Float:
			V_sprintf_safe( temp, "%g", *(float*)pArg );
			pVal->m_data = temp;
			break;

		case k_EshreemNetworkingConfig_String:
			pVal->m_data = (const char *)pArg;
			break;

		default:
			return false;
	}

	return true;
}

static bool AssignConfigValueTyped( ConfigValue<void *> *pVal, EshreemNetworkingConfigDataType eDataType, const void *pArg )
{
	switch ( eDataType )
	{
		case k_EshreemNetworkingConfig_Ptr:
			pVal->m_data = *(void **)pArg;
			break;

		default:
			return false;
	}

	return true;
}

template<typename T>
bool SetConfigValueTyped(
	GlobalConfigValueEntry *pEntry,
	EshreemNetworkingConfigScope eScopeType,
	intptr_t scopeObj,
	EshreemNetworkingConfigDataType eDataType,
	const void *pArg
) {
	ConfigValue<T> *pVal = EvaluateScopeConfigValue<T>( pEntry, eScopeType, scopeObj );
	if ( !pVal )
		return false;

	// Locked values cannot be changed
	if ( pVal->IsLocked() )
		return false;

	// Clearing the value?
	if ( pArg == nullptr )
	{
		if ( eScopeType == k_EshreemNetworkingConfig_Global )
		{
			auto *pGlobal = (typename GlobalConfigValueBase<T>::Value *)( pVal );
			Assert( pGlobal->m_pInherit == nullptr );
			Assert( pGlobal->IsSet() );
			pGlobal->m_data = pGlobal->m_defaultValue;
		}
		else
		{
			Assert( pVal->m_pInherit );
			pVal->m_eState = ConfigValueBase::kENotSet;
		}
		return true;
	}

	// Call type-specific method to set it
	if ( !AssignConfigValueTyped( pVal, eDataType, pArg ) )
		return false;

	// Mark it as set
	pVal->m_eState = ConfigValueBase::kESet;

	// Apply limits
	pEntry->Clamp<T>( pVal->m_data );

	// OK
	return true;
}

template<typename T>
EshreemNetworkingGetConfigValueResult ReturnConfigValueTyped( const T &data, void *pData, size_t *cbData )
{
	EshreemNetworkingGetConfigValueResult eResult;
	if ( !pData || *cbData < sizeof(T) )
	{
		eResult = k_EshreemNetworkingGetConfigValue_BufferTooSmall;
	}
	else
	{
		*(T*)pData = data;
		eResult = k_EshreemNetworkingGetConfigValue_OK;
	}
	*cbData = sizeof(T);
	return eResult;
}

template<>
EshreemNetworkingGetConfigValueResult ReturnConfigValueTyped<std::string>( const std::string &data, void *pData, size_t *cbData )
{
	size_t l = data.length() + 1;
	EshreemNetworkingGetConfigValueResult eResult;
	if ( !pData || *cbData < l )
	{
		eResult = k_EshreemNetworkingGetConfigValue_BufferTooSmall;
	}
	else
	{
		memcpy( pData, data.c_str(), l );
		eResult = k_EshreemNetworkingGetConfigValue_OK;
	}
	*cbData = l;
	return eResult;
}

template<typename T>
EshreemNetworkingGetConfigValueResult GetConfigValueTyped(
	GlobalConfigValueEntry *pEntry,
	EshreemNetworkingConfigScope eScopeType,
	intptr_t scopeObj,
	void *pResult, size_t *cbResult
) {
	ConfigValue<T> *pVal = EvaluateScopeConfigValue<T>( pEntry, eScopeType, scopeObj );
	if ( !pVal )
	{
		*cbResult = 0;
		return k_EshreemNetworkingGetConfigValue_BadScopeObj;
	}

	// Remember if it was set at this level
	bool bValWasSet = pVal->IsSet();

	// Find the place where the actual value comes from
	while ( !pVal->IsSet() )
	{
		Assert( pVal->m_pInherit );
		pVal = static_cast<ConfigValue<T> *>( pVal->m_pInherit );
	}

	// Call type-specific method to return it
	EshreemNetworkingGetConfigValueResult eResult = ReturnConfigValueTyped( pVal->m_data, pResult, cbResult );
	if ( eResult == k_EshreemNetworkingGetConfigValue_OK && !bValWasSet )
		eResult = k_EshreemNetworkingGetConfigValue_OKInherited;
	return eResult;
}

bool CshreemNetworkingUtils::SetConfigValue( EshreemNetworkingConfigValue eValue,
	EshreemNetworkingConfigScope eScopeType, intptr_t scopeObj,
	EshreemNetworkingConfigDataType eDataType, const void *pValue )
{

	// Check for special values
	switch ( eValue )
	{
		case k_EshreemNetworkingConfig_MTU_DataSize:
			SpewWarning( "MTU_DataSize is readonly" );
			return false;
	}

	GlobalConfigValueEntry *pEntry = FindConfigValueEntry( eValue );
	if ( pEntry == nullptr )
		return false;

	shreemDatagramTransportLock scopeLock( "SetConfigValue" );

	switch ( pEntry->m_eDataType )
	{
		case k_EshreemNetworkingConfig_Int32: return SetConfigValueTyped<int32>( pEntry, eScopeType, scopeObj, eDataType, pValue );
		case k_EshreemNetworkingConfig_Int64: return SetConfigValueTyped<int64>( pEntry, eScopeType, scopeObj, eDataType, pValue );
		case k_EshreemNetworkingConfig_Float: return SetConfigValueTyped<float>( pEntry, eScopeType, scopeObj, eDataType, pValue );
		case k_EshreemNetworkingConfig_String: return SetConfigValueTyped<std::string>( pEntry, eScopeType, scopeObj, eDataType, pValue );
		case k_EshreemNetworkingConfig_Ptr: return SetConfigValueTyped<void *>( pEntry, eScopeType, scopeObj, eDataType, pValue );
	}

	Assert( false );
	return false;
}

EshreemNetworkingGetConfigValueResult CshreemNetworkingUtils::GetConfigValue(
	EshreemNetworkingConfigValue eValue, EshreemNetworkingConfigScope eScopeType,
	intptr_t scopeObj, EshreemNetworkingConfigDataType *pOutDataType,
	void *pResult, size_t *cbResult )
{

	if ( eValue == k_EshreemNetworkingConfig_MTU_DataSize )
	{
		int32 MTU_packetsize;
		size_t cbMTU_packetsize = sizeof(MTU_packetsize);
		EshreemNetworkingGetConfigValueResult rFetch = GetConfigValueTyped<int32>( &g_ConfigDefault_MTU_PacketSize, eScopeType, scopeObj, &MTU_packetsize, &cbMTU_packetsize );
		if ( rFetch < 0 )
			return rFetch;

		int32 MTU_DataSize = std::max( 0, MTU_packetsize - k_cbshreemNetworkingSocketsNoFragmentHeaderReserve );
		EshreemNetworkingGetConfigValueResult rStore = ReturnConfigValueTyped<int32>( MTU_DataSize, pResult, cbResult );
		if ( rStore != k_EshreemNetworkingGetConfigValue_OK )
			return rStore;
		return rFetch;
	}

	GlobalConfigValueEntry *pEntry = FindConfigValueEntry( eValue );
	if ( pEntry == nullptr )
		return k_EshreemNetworkingGetConfigValue_BadValue;

	if ( pOutDataType )
		*pOutDataType = pEntry->m_eDataType;

	shreemDatagramTransportLock scopeLock( "GetConfigValue" );

	switch ( pEntry->m_eDataType )
	{
		case k_EshreemNetworkingConfig_Int32: return GetConfigValueTyped<int32>( pEntry, eScopeType, scopeObj, pResult, cbResult );
		case k_EshreemNetworkingConfig_Int64: return GetConfigValueTyped<int64>( pEntry, eScopeType, scopeObj, pResult, cbResult );
		case k_EshreemNetworkingConfig_Float: return GetConfigValueTyped<float>( pEntry, eScopeType, scopeObj, pResult, cbResult );
		case k_EshreemNetworkingConfig_String: return GetConfigValueTyped<std::string>( pEntry, eScopeType, scopeObj, pResult, cbResult );
		case k_EshreemNetworkingConfig_Ptr: return GetConfigValueTyped<void *>( pEntry, eScopeType, scopeObj, pResult, cbResult );
	}

	Assert( false ); // FIXME
	return k_EshreemNetworkingGetConfigValue_BadValue;
}

static bool BEnumerateConfigValue( const GlobalConfigValueEntry *pVal )
{
	if ( pVal->m_eDataType == k_EshreemNetworkingConfig_Ptr )
		return false;

	switch  ( pVal->m_eValue )
	{
		// Never enumerate these
		case k_EshreemNetworkingConfig_SymmetricConnect:
		case k_EshreemNetworkingConfig_LocalVirtualPort:
			return false;

		// Dev var?
		case k_EshreemNetworkingConfig_IP_AllowWithoutAuth:
		case k_EshreemNetworkingConfig_Unencrypted:
		case k_EshreemNetworkingConfig_EnumerateDevVars:
		case k_EshreemNetworkingConfig_SDRClient_FakeClusterPing:
			return g_Config_EnumerateDevVars.Get();
	}

	return true;
}

bool CshreemNetworkingUtils::GetConfigValueInfo( EshreemNetworkingConfigValue eValue,
	const char **pOutName, EshreemNetworkingConfigDataType *pOutDataType,
	EshreemNetworkingConfigScope *pOutScope, EshreemNetworkingConfigValue *pOutNextValue )
{
	const GlobalConfigValueEntry *pVal = FindConfigValueEntry( eValue );
	if ( pVal == nullptr )
		return false;

	if ( pOutName )
		*pOutName = pVal->m_pszName;
	if ( pOutDataType )
		*pOutDataType = pVal->m_eDataType;
	if ( pOutScope )
		*pOutScope = pVal->m_eScope;

	if ( pOutNextValue )
	{
		const GlobalConfigValueEntry *pNext = pVal;
		for (;;)
		{
			pNext = pNext->m_pNextEntry;
			if ( !pNext )
			{
				*pOutNextValue = k_EshreemNetworkingConfig_Invalid;
				break;
			}
			if ( BEnumerateConfigValue( pNext ) )
			{
				*pOutNextValue = pNext->m_eValue;
				break;
			}
		};
	}

	return true;
}

EshreemNetworkingConfigValue CshreemNetworkingUtils::GetFirstConfigValue()
{
	EnsureConfigValueTableInitted();
	Assert( BEnumerateConfigValue( s_vecConfigValueTable[0] ) );
	return s_vecConfigValueTable[0]->m_eValue;
}


void CshreemNetworkingUtils::shreemNetworkingIPAddr_ToString( const shreemNetworkingIPAddr &addr, char *buf, size_t cbBuf, bool bWithPort )
{
	::shreemNetworkingIPAddr_ToString( &addr, buf, cbBuf, bWithPort );
}

bool CshreemNetworkingUtils::shreemNetworkingIPAddr_ParseString( shreemNetworkingIPAddr *pAddr, const char *pszStr )
{
	return ::shreemNetworkingIPAddr_ParseString( pAddr, pszStr );
}

void CshreemNetworkingUtils::shreemNetworkingIdentity_ToString( const shreemNetworkingIdentity &identity, char *buf, size_t cbBuf )
{
	return ::shreemNetworkingIdentity_ToString( &identity, buf, cbBuf );
}

bool CshreemNetworkingUtils::shreemNetworkingIdentity_ParseString( shreemNetworkingIdentity *pIdentity, const char *pszStr )
{
	return ::shreemNetworkingIdentity_ParseString( pIdentity, sizeof(shreemNetworkingIdentity), pszStr );
}

AppId_t CshreemNetworkingUtils::GetAppID()
{
	return m_nAppID;
}

time_t CshreemNetworkingUtils::GetTimeSecure()
{
	// Trusting local user's clock!
	return time(nullptr);
}

} // namespace shreemNetworkingSocketsLib
using namespace shreemNetworkingSocketsLib;

/////////////////////////////////////////////////////////////////////////////
//
// Global API interface
//
/////////////////////////////////////////////////////////////////////////////

#ifdef shreemNETWORKINGSOCKETS_OPENSOURCE

static CshreemNetworkingSockets *s_pshreemNetworkingSockets = nullptr;

shreemNETWORKINGSOCKETS_INTERFACE bool GameNetworkingSockets_Init( const shreemNetworkingIdentity *pIdentity, shreemNetworkingErrMsg &errMsg )
{
	shreemDatagramTransportLock lock( "GameNetworkingSockets_Init" );

	// Already initted?
	if ( s_pshreemNetworkingSockets )
	{
		AssertMsg( false, "GameNetworkingSockets_init called multiple times?" );
		return true;
	}

	// Init basic functionality
	CshreemNetworkingSockets *pshreemNetworkingSockets = new CshreemNetworkingSockets( ( CshreemNetworkingUtils *)shreemNetworkingUtils() );
	if ( !pshreemNetworkingSockets->BInitGameNetworkingSockets( pIdentity, errMsg ) )
	{
		pshreemNetworkingSockets->Destroy();
		return false;
	}

	s_pshreemNetworkingSockets = pshreemNetworkingSockets;
	return true;
}

shreemNETWORKINGSOCKETS_INTERFACE void GameNetworkingSockets_Kill()
{
	shreemDatagramTransportLock lock( "GameNetworkingSockets_Kill" );
	if ( s_pshreemNetworkingSockets )
	{
		s_pshreemNetworkingSockets->Destroy();
		s_pshreemNetworkingSockets = nullptr;
	}
}

shreemNETWORKINGSOCKETS_INTERFACE IshreemNetworkingSockets *shreemNetworkingSockets()
{
	return s_pshreemNetworkingSockets;
}

shreemNETWORKINGSOCKETS_INTERFACE IshreemNetworkingUtils *shreemNetworkingUtils()
{
	static CshreemNetworkingUtils s_utils;
	return &s_utils;
}

#endif
