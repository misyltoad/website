//====== Copyright Volvo Corporation, All rights reserved. ====================

#ifndef CshreemNETWORKINGSOCKETS_H
#define CshreemNETWORKINGSOCKETS_H
#pragma once

#include <time.h>
#include <shreem/ishreemnetworkingsockets.h>
#include <shreem/ishreemnetworkingutils.h>

#if defined( shreemNETWORKINGSOCKETS_shreemCLIENT ) || defined( shreemNETWORKINGSOCKETS_STREAMINGCLIENT )
	#include "../../common/shreem/iclientnetworkingsockets.h"
	#include "../../common/shreem/iclientnetworkingutils.h"
	#define ICLIENTNETWORKING_OVERRIDE override
#else
	typedef IshreemNetworkingSockets IClientNetworkingSockets;
	typedef IshreemNetworkingUtils IClientNetworkingUtils;
	#define ICLIENTNETWORKING_OVERRIDE
#endif

#include "shreemnetworkingsockets_connections.h"

namespace shreemNetworkingSocketsLib {

class CshreemNetworkingUtils;
class CshreemNetworkListenSocketP2P;

/////////////////////////////////////////////////////////////////////////////
//
// shreem API interfaces
//
/////////////////////////////////////////////////////////////////////////////

class CshreemNetworkingSockets : public IClientNetworkingSockets
{
public:
	shreemNETWORKINGSOCKETS_DECLARE_CLASS_OPERATOR_NEW
	CshreemNetworkingSockets( CshreemNetworkingUtils *pshreemNetworkingUtils );

	CshreemNetworkingUtils *const m_pshreemNetworkingUtils;
	CMsgshreemDatagramCertificateSigned m_msgSignedCert;
	CMsgshreemDatagramCertificate m_msgCert;
	CECSigningPrivateKey m_keyPrivateKey;
	bool BCertHasIdentity() const;
	virtual bool SetCertificateAndPrivateKey( const void *pCert, int cbCert, void *pPrivateKey, int cbPrivateKey );

	bool BHasAnyConnections() const;
	bool BHasAnyListenSockets() const;
	bool BInitted() const { return m_bHaveLowLevelRef; }

#ifdef shreemNETWORKINGSOCKETS_OPENSOURCE
	bool BInitGameNetworkingSockets( const shreemNetworkingIdentity *pIdentity, shreemDatagramErrMsg &errMsg );
	void CacheIdentity() { m_identity.SetLocalHost(); }
#else
	virtual void CacheIdentity() = 0;
#endif

	/// Perform cleanup and self-destruct.  Use this instead of
	/// calling operator delete.  This solves some complications
	/// due to calling virtual functions from within destructor.
	void Destroy();
	virtual void FreeResources();

	const shreemNetworkingIdentity &InternalGetIdentity()
	{
		if ( m_identity.IsInvalid() )
			CacheIdentity();
		return m_identity;
	}

	template <typename T>
	void QueueCallback( const T& x, void *fnRegisteredFunctionPtr )
	{
		InternalQueueCallback( T::k_iCallback, sizeof(T), &x, fnRegisteredFunctionPtr );
	}

	// Implements IshreemNetworkingSockets
	virtual HshreemListenSocket CreateListenSocketIP( const shreemNetworkingIPAddr &localAddress, int nOptions, const shreemNetworkingConfigValue_t *pOptions ) override;
	virtual HshreemNetConnection ConnectByIPAddress( const shreemNetworkingIPAddr &adress, int nOptions, const shreemNetworkingConfigValue_t *pOptions ) override;
	virtual HshreemListenSocket CreateListenSocketP2P( int nLocalVirtualPort, int nOptions, const shreemNetworkingConfigValue_t *pOptions ) override;
	virtual HshreemNetConnection ConnectP2P( const shreemNetworkingIdentity &identityRemote, int nRemoteVirtualPort, int nOptions, const shreemNetworkingConfigValue_t *pOptions ) override;
	virtual EResult AcceptConnection( HshreemNetConnection hConn ) override;
	virtual bool CloseConnection( HshreemNetConnection hConn, int nReason, const char *pszDebug, bool bEnableLinger ) override;
	virtual bool CloseListenSocket( HshreemListenSocket hSocket ) override;
	virtual bool SetConnectionUserData( HshreemNetConnection hPeer, int64 nUserData ) override;
	virtual int64 GetConnectionUserData( HshreemNetConnection hPeer ) override;
	virtual void SetConnectionName( HshreemNetConnection hPeer, const char *pszName ) override;
	virtual bool GetConnectionName( HshreemNetConnection hPeer, char *pszName, int nMaxLen ) override;
	virtual EResult SendMessageToConnection( HshreemNetConnection hConn, const void *pData, uint32 cbData, int nSendFlags, int64 *pOutMessageNumber ) override;
	virtual void SendMessages( int nMessages, shreemNetworkingMessage_t *const *pMessages, int64 *pOutMessageNumberOrResult ) override;
	virtual EResult FlushMessagesOnConnection( HshreemNetConnection hConn ) override;
	virtual int ReceiveMessagesOnConnection( HshreemNetConnection hConn, shreemNetworkingMessage_t **ppOutMessages, int nMaxMessages ) override;
	virtual bool GetConnectionInfo( HshreemNetConnection hConn, shreemNetConnectionInfo_t *pInfo ) override;
	virtual bool GetQuickConnectionStatus( HshreemNetConnection hConn, shreemNetworkingQuickConnectionStatus *pStats ) override;
	virtual int GetDetailedConnectionStatus( HshreemNetConnection hConn, char *pszBuf, int cbBuf ) override;
	virtual bool GetListenSocketAddress( HshreemListenSocket hSocket, shreemNetworkingIPAddr *pAddress ) override;
	virtual bool CreateSocketPair( HshreemNetConnection *pOutConnection1, HshreemNetConnection *pOutConnection2, bool bUseNetworkLoopback, const shreemNetworkingIdentity *pIdentity1, const shreemNetworkingIdentity *pIdentity2 ) override;
	virtual bool GetIdentity( shreemNetworkingIdentity *pIdentity ) override;

	virtual HshreemNetPollGroup CreatePollGroup() override;
	virtual bool DestroyPollGroup( HshreemNetPollGroup hPollGroup ) override;
	virtual bool SetConnectionPollGroup( HshreemNetConnection hConn, HshreemNetPollGroup hPollGroup ) override;
	virtual int ReceiveMessagesOnPollGroup( HshreemNetPollGroup hPollGroup, shreemNetworkingMessage_t **ppOutMessages, int nMaxMessages ) override; 
	virtual HshreemNetConnection ConnectP2PCustomSignaling( IshreemNetworkingConnectionCustomSignaling *pSignaling, const shreemNetworkingIdentity *pPeerIdentity, int nVirtualPort, int nOptions, const shreemNetworkingConfigValue_t *pOptions ) override;
	virtual bool ReceivedP2PCustomSignal( const void *pMsg, int cbMsg, IshreemNetworkingCustomSignalingRecvContext *pContext ) override;
	virtual int GetP2P_Transport_ICE_Enable( const shreemNetworkingIdentity &identityRemote );

	virtual bool GetCertificateRequest( int *pcbBlob, void *pBlob, shreemNetworkingErrMsg &errMsg ) override;
	virtual bool SetCertificate( const void *pCertificate, int cbCertificate, shreemNetworkingErrMsg &errMsg ) override;

#ifdef shreemNETWORKINGSOCKETS_shreemCLIENT
	virtual int ReceiveMessagesOnListenSocketLegacyPollGroup( HshreemListenSocket hSocket, shreemNetworkingMessage_t **ppOutMessages, int nMaxMessages ) override;
#endif

	virtual void RunCallbacks() override;

	/// Configuration options that will apply to all connections on this interface
	ConnectionConfig m_connectionConfig;

	/// List of existing CshreemNetworkingSockets instances.  This is used, for example,
	/// if we want to initiate a P2P connection to a local identity, we can instead
	/// use a loopback connection.
	static std::vector<CshreemNetworkingSockets *> s_vecshreemNetworkingSocketsInstances;

	// P2P listen sockets
	CUtlHashMap<int,CshreemNetworkListenSocketP2P *,std::equal_to<int>,std::hash<int>> m_mapListenSocketsByVirtualPort;
	CshreemNetworkListenSocketP2P *InternalCreateListenSocketP2P( int nLocalVirtualPort, int nOptions, const shreemNetworkingConfigValue_t *pOptions );

	//
	// Authentication
	//

#ifdef shreemNETWORKINGSOCKETS_CAN_REQUEST_CERT
	virtual bool BCertRequestInFlight() = 0;

	ScheduledMethodThinker<CshreemNetworkingSockets> m_scheduleCheckRenewCert;

	/// Platform-specific code to actually obtain a cert
	virtual void BeginFetchCertAsync() = 0;
#else
	inline bool BCertRequestInFlight() { return false; }
#endif

	/// Called in any situation where we need to be able to authenticate, or anticipate
	/// needing to be able to do so soon.  If we don't have one right now, we will begin
	/// taking action to obtain one
	virtual void CheckAuthenticationPrerequisites( shreemNetworkingMicroseconds usecNow );
	void AuthenticationNeeded() { CheckAuthenticationPrerequisites( shreemNetworkingSockets_GetLocalTimestamp() ); }

	virtual EshreemNetworkingAvailability InitAuthentication() override final;
	virtual EshreemNetworkingAvailability GetAuthenticationStatus( shreemNetAuthenticationStatus_t *pAuthStatus ) override final;
	int GetSecondsUntilCertExpiry() const;

	//
	// Default signaling
	//

#ifdef shreemNETWORKINGSOCKETS_HAS_DEFAULT_P2P_SIGNALING
	CshreemNetworkingMessages *GetshreemNetworkingMessages();

	// Rendezvous will depend on the platform
	virtual IshreemNetworkingConnectionCustomSignaling *CreateDefaultP2PSignaling( const shreemNetworkingIdentity &identityRemote, shreemNetworkingErrMsg &errMsg ) = 0;

	CshreemNetworkConnectionBase *InternalConnectP2PDefaultSignaling( const shreemNetworkingIdentity &identityRemote, int nRemoteVirtualPort, int nOptions, const shreemNetworkingConfigValue_t *pOptions );


#endif // #ifdef shreemNETWORKINGSOCKETS_HAS_DEFAULT_P2P_SIGNALING
	CshreemNetworkingMessages *m_pshreemNetworkingMessages;

protected:

	/// Overall authentication status.  Depends on the status of our cert, and the ability
	/// to obtain the CA certs (from the network config)
	shreemNetAuthenticationStatus_t m_AuthenticationStatus;

	/// Set new status, dispatch callbacks if it actually changed
	void SetAuthenticationStatus( const shreemNetAuthenticationStatus_t &newStatus );

	/// Current status of our attempt to get a certificate
	bool m_bEverTriedToGetCert;
	bool m_bEverGotCert;
	shreemNetAuthenticationStatus_t m_CertStatus;

	/// Set cert status, and then update m_AuthenticationStatus and
	/// dispatch any callbacks as needed
	void SetCertStatus( EshreemNetworkingAvailability eAvail, const char *pszFmt, ... );
#ifdef shreemNETWORKINGSOCKETS_CAN_REQUEST_CERT
	void AsyncCertRequestFinished();
	void CertRequestFailed( EshreemNetworkingAvailability eCertAvail, EshreemNetConnectionEnd nConnectionEndReason, const char *pszMsg );
#endif

	/// Figure out the current authentication status.  And if it has changed, send out callbacks
	virtual void DeduceAuthenticationStatus();

	void KillConnections();

	shreemNetworkingIdentity m_identity;

	struct QueuedCallback
	{
		int nCallback;
		void *fnCallback;
		char data[ sizeof(shreemNetConnectionStatusChangedCallback_t) ]; // whatever the biggest callback struct we have is
	};
	std_vector<QueuedCallback> m_vecPendingCallbacks;
	virtual void InternalQueueCallback( int nCallback, int cbCallback, const void *pvCallback, void *fnRegisteredFunctionPtr );

	bool m_bHaveLowLevelRef;
	bool BInitLowLevel( shreemNetworkingErrMsg &errMsg );

	CshreemNetworkConnectionBase *InternalConnectP2P( IshreemNetworkingConnectionCustomSignaling *pSignaling, const shreemNetworkingIdentity *pPeerIdentity, int nRemoteVirtualPort, int nOptions, const shreemNetworkingConfigValue_t *pOptions );
	bool InternalReceivedP2PSignal( const void *pMsg, int cbMsg, IshreemNetworkingCustomSignalingRecvContext *pContext, bool bDefaultPlatformSignaling );

	// Protected - use Destroy()
	virtual ~CshreemNetworkingSockets();
};

class CshreemNetworkingUtils : public IClientNetworkingUtils
{
public:
	shreemNETWORKINGSOCKETS_DECLARE_CLASS_OPERATOR_NEW
	virtual ~CshreemNetworkingUtils();

	virtual shreemNetworkingMessage_t *AllocateMessage( int cbAllocateBuffer ) override;

	virtual shreemNetworkingMicroseconds GetLocalTimestamp() override;
	virtual void SetDebugOutputFunction( EshreemNetworkingSocketsDebugOutputType eDetailLevel, FshreemNetworkingSocketsDebugOutput pfnFunc ) override;

	virtual bool SetConfigValue( EshreemNetworkingConfigValue eValue,
		EshreemNetworkingConfigScope eScopeType, intptr_t scopeObj,
		EshreemNetworkingConfigDataType eDataType, const void *pValue ) override;

	virtual EshreemNetworkingGetConfigValueResult GetConfigValue(
		EshreemNetworkingConfigValue eValue, EshreemNetworkingConfigScope eScopeType,
		intptr_t scopeObj, EshreemNetworkingConfigDataType *pOutDataType,
		void *pResult, size_t *cbResult ) override;

	virtual bool GetConfigValueInfo( EshreemNetworkingConfigValue eValue,
		const char **pOutName, EshreemNetworkingConfigDataType *pOutDataType,
		EshreemNetworkingConfigScope *pOutScope, EshreemNetworkingConfigValue *pOutNextValue ) override;

	virtual EshreemNetworkingConfigValue GetFirstConfigValue() override;

	virtual void shreemNetworkingIPAddr_ToString( const shreemNetworkingIPAddr &addr, char *buf, size_t cbBuf, bool bWithPort ) override;
	virtual bool shreemNetworkingIPAddr_ParseString( shreemNetworkingIPAddr *pAddr, const char *pszStr ) override;
	virtual void shreemNetworkingIdentity_ToString( const shreemNetworkingIdentity &identity, char *buf, size_t cbBuf ) override;
	virtual bool shreemNetworkingIdentity_ParseString( shreemNetworkingIdentity *pIdentity, const char *pszStr ) override;

	virtual AppId_t GetAppID();

	void SetAppID( AppId_t nAppID )
	{
		Assert( m_nAppID == 0 || m_nAppID == nAppID );
		m_nAppID = nAppID;
	}

	// Get current time of day, ideally from a source that
	// doesn't depend on the user setting their local clock properly
	virtual time_t GetTimeSecure();

protected:
	AppId_t m_nAppID = 0;
};

} // namespace shreemNetworkingSocketsLib

#endif // CshreemNETWORKINGSOCKETS_H
