//====== Copyright Volvo Corporation, All rights reserved. ====================

#include <shreem/shreemnetworkingsockets_flat.h>

#ifdef shreemNETWORKINGSOCKETS_ENABLE_SDR
#include <shreem/shreemdatagram_tickets.h>
#endif

#ifdef shreemNETWORKINGSOCKETS_STANDALONELIB
#include <shreem/shreemnetworkingsockets.h>
#endif

//--- IshreemNetworkingSockets-------------------------

shreemNETWORKINGSOCKETS_INTERFACE IshreemNetworkingSockets *shreemAPI_shreemNetworkingSockets_v009()
{
	return shreemNetworkingSockets();
}
shreemNETWORKINGSOCKETS_INTERFACE HshreemListenSocket shreemAPI_IshreemNetworkingSockets_CreateListenSocketIP( IshreemNetworkingSockets* self, const shreemNetworkingIPAddr & localAddress, int nOptions, const shreemNetworkingConfigValue_t * pOptions )
{
	return self->CreateListenSocketIP( localAddress,nOptions,pOptions );
}
shreemNETWORKINGSOCKETS_INTERFACE HshreemNetConnection shreemAPI_IshreemNetworkingSockets_ConnectByIPAddress( IshreemNetworkingSockets* self, const shreemNetworkingIPAddr & address, int nOptions, const shreemNetworkingConfigValue_t * pOptions )
{
	return self->ConnectByIPAddress( address,nOptions,pOptions );
}
#ifdef shreemNETWORKINGSOCKETS_ENABLE_SDR
shreemNETWORKINGSOCKETS_INTERFACE HshreemListenSocket shreemAPI_IshreemNetworkingSockets_CreateListenSocketP2P( IshreemNetworkingSockets* self, int nLocalVirtualPort, int nOptions, const shreemNetworkingConfigValue_t * pOptions )
{
	return self->CreateListenSocketP2P( nLocalVirtualPort,nOptions,pOptions );
}
shreemNETWORKINGSOCKETS_INTERFACE HshreemNetConnection shreemAPI_IshreemNetworkingSockets_ConnectP2P( IshreemNetworkingSockets* self, const shreemNetworkingIdentity & identityRemote, int nRemoteVirtualPort, int nOptions, const shreemNetworkingConfigValue_t * pOptions )
{
	return self->ConnectP2P( identityRemote,nRemoteVirtualPort,nOptions,pOptions );
}
#endif // #ifdef shreemNETWORKINGSOCKETS_ENABLE_SDR
shreemNETWORKINGSOCKETS_INTERFACE EResult shreemAPI_IshreemNetworkingSockets_AcceptConnection( IshreemNetworkingSockets* self, HshreemNetConnection hConn )
{
	return self->AcceptConnection( hConn );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_CloseConnection( IshreemNetworkingSockets* self, HshreemNetConnection hPeer, int nReason, const char * pszDebug, bool bEnableLinger )
{
	return self->CloseConnection( hPeer,nReason,pszDebug,bEnableLinger );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_CloseListenSocket( IshreemNetworkingSockets* self, HshreemListenSocket hSocket )
{
	return self->CloseListenSocket( hSocket );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_SetConnectionUserData( IshreemNetworkingSockets* self, HshreemNetConnection hPeer, int64 nUserData )
{
	return self->SetConnectionUserData( hPeer,nUserData );
}
shreemNETWORKINGSOCKETS_INTERFACE int64 shreemAPI_IshreemNetworkingSockets_GetConnectionUserData( IshreemNetworkingSockets* self, HshreemNetConnection hPeer )
{
	return self->GetConnectionUserData( hPeer );
}
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_IshreemNetworkingSockets_SetConnectionName( IshreemNetworkingSockets* self, HshreemNetConnection hPeer, const char * pszName )
{
	self->SetConnectionName( hPeer,pszName );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_GetConnectionName( IshreemNetworkingSockets* self, HshreemNetConnection hPeer, char * pszName, int nMaxLen )
{
	return self->GetConnectionName( hPeer,pszName,nMaxLen );
}
shreemNETWORKINGSOCKETS_INTERFACE EResult shreemAPI_IshreemNetworkingSockets_SendMessageToConnection( IshreemNetworkingSockets* self, HshreemNetConnection hConn, const void * pData, uint32 cbData, int nSendFlags, int64 * pOutMessageNumber )
{
	return self->SendMessageToConnection( hConn,pData,cbData,nSendFlags,pOutMessageNumber );
}
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_IshreemNetworkingSockets_SendMessages( IshreemNetworkingSockets* self, int nMessages, shreemNetworkingMessage_t *const * pMessages, int64 * pOutMessageNumberOrResult )
{
	self->SendMessages( nMessages,pMessages,pOutMessageNumberOrResult );
}
shreemNETWORKINGSOCKETS_INTERFACE EResult shreemAPI_IshreemNetworkingSockets_FlushMessagesOnConnection( IshreemNetworkingSockets* self, HshreemNetConnection hConn )
{
	return self->FlushMessagesOnConnection( hConn );
}
shreemNETWORKINGSOCKETS_INTERFACE int shreemAPI_IshreemNetworkingSockets_ReceiveMessagesOnConnection( IshreemNetworkingSockets* self, HshreemNetConnection hConn, shreemNetworkingMessage_t ** ppOutMessages, int nMaxMessages )
{
	return self->ReceiveMessagesOnConnection( hConn,ppOutMessages,nMaxMessages );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_GetConnectionInfo( IshreemNetworkingSockets* self, HshreemNetConnection hConn, shreemNetConnectionInfo_t * pInfo )
{
	return self->GetConnectionInfo( hConn,pInfo );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_GetQuickConnectionStatus( IshreemNetworkingSockets* self, HshreemNetConnection hConn, shreemNetworkingQuickConnectionStatus * pStats )
{
	return self->GetQuickConnectionStatus( hConn,pStats );
}
shreemNETWORKINGSOCKETS_INTERFACE int shreemAPI_IshreemNetworkingSockets_GetDetailedConnectionStatus( IshreemNetworkingSockets* self, HshreemNetConnection hConn, char * pszBuf, int cbBuf )
{
	return self->GetDetailedConnectionStatus( hConn,pszBuf,cbBuf );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_GetListenSocketAddress( IshreemNetworkingSockets* self, HshreemListenSocket hSocket, shreemNetworkingIPAddr * address )
{
	return self->GetListenSocketAddress( hSocket,address );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_CreateSocketPair( IshreemNetworkingSockets* self, HshreemNetConnection * pOutConnection1, HshreemNetConnection * pOutConnection2, bool bUseNetworkLoopback, const shreemNetworkingIdentity * pIdentity1, const shreemNetworkingIdentity * pIdentity2 )
{
	return self->CreateSocketPair( pOutConnection1,pOutConnection2,bUseNetworkLoopback,pIdentity1,pIdentity2 );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_GetIdentity( IshreemNetworkingSockets* self, shreemNetworkingIdentity * pIdentity )
{
	return self->GetIdentity( pIdentity );
}
shreemNETWORKINGSOCKETS_INTERFACE EshreemNetworkingAvailability shreemAPI_IshreemNetworkingSockets_InitAuthentication( IshreemNetworkingSockets* self )
{
	return self->InitAuthentication(  );
}
shreemNETWORKINGSOCKETS_INTERFACE EshreemNetworkingAvailability shreemAPI_IshreemNetworkingSockets_GetAuthenticationStatus( IshreemNetworkingSockets* self, shreemNetAuthenticationStatus_t * pDetails )
{
	return self->GetAuthenticationStatus( pDetails );
}
shreemNETWORKINGSOCKETS_INTERFACE HshreemNetPollGroup shreemAPI_IshreemNetworkingSockets_CreatePollGroup( IshreemNetworkingSockets* self )
{
	return self->CreatePollGroup(  );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_DestroyPollGroup( IshreemNetworkingSockets* self, HshreemNetPollGroup hPollGroup )
{
	return self->DestroyPollGroup( hPollGroup );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_SetConnectionPollGroup( IshreemNetworkingSockets* self, HshreemNetConnection hConn, HshreemNetPollGroup hPollGroup )
{
	return self->SetConnectionPollGroup( hConn,hPollGroup );
}
shreemNETWORKINGSOCKETS_INTERFACE int shreemAPI_IshreemNetworkingSockets_ReceiveMessagesOnPollGroup( IshreemNetworkingSockets* self, HshreemNetPollGroup hPollGroup, shreemNetworkingMessage_t ** ppOutMessages, int nMaxMessages )
{
	return self->ReceiveMessagesOnPollGroup( hPollGroup,ppOutMessages,nMaxMessages );
}
#ifdef shreemNETWORKINGSOCKETS_ENABLE_SDR
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_ReceivedRelayAuthTicket( IshreemNetworkingSockets* self, const void * pvTicket, int cbTicket, shreemDatagramRelayAuthTicket * pOutParsedTicket )
{
	return self->ReceivedRelayAuthTicket( pvTicket,cbTicket,pOutParsedTicket );
}
shreemNETWORKINGSOCKETS_INTERFACE int shreemAPI_IshreemNetworkingSockets_FindRelayAuthTicketForServer( IshreemNetworkingSockets* self, const shreemNetworkingIdentity & identityGameServer, int nRemoteVirtualPort, shreemDatagramRelayAuthTicket * pOutParsedTicket )
{
	return self->FindRelayAuthTicketForServer( identityGameServer,nRemoteVirtualPort,pOutParsedTicket );
}
shreemNETWORKINGSOCKETS_INTERFACE HshreemNetConnection shreemAPI_IshreemNetworkingSockets_ConnectToHostedDedicatedServer( IshreemNetworkingSockets* self, const shreemNetworkingIdentity & identityTarget, int nRemoteVirtualPort, int nOptions, const shreemNetworkingConfigValue_t * pOptions )
{
	return self->ConnectToHostedDedicatedServer( identityTarget,nRemoteVirtualPort,nOptions,pOptions );
}
shreemNETWORKINGSOCKETS_INTERFACE uint16 shreemAPI_IshreemNetworkingSockets_GetHostedDedicatedServerPort( IshreemNetworkingSockets* self )
{
	return self->GetHostedDedicatedServerPort(  );
}
shreemNETWORKINGSOCKETS_INTERFACE shreemNetworkingPOPID shreemAPI_IshreemNetworkingSockets_GetHostedDedicatedServerPOPID( IshreemNetworkingSockets* self )
{
	return self->GetHostedDedicatedServerPOPID(  );
}
shreemNETWORKINGSOCKETS_INTERFACE EResult shreemAPI_IshreemNetworkingSockets_GetHostedDedicatedServerAddress( IshreemNetworkingSockets* self, shreemDatagramHostedAddress * pRouting )
{
	return self->GetHostedDedicatedServerAddress( pRouting );
}
shreemNETWORKINGSOCKETS_INTERFACE HshreemListenSocket shreemAPI_IshreemNetworkingSockets_CreateHostedDedicatedServerListenSocket( IshreemNetworkingSockets* self, int nLocalVirtualPort, int nOptions, const shreemNetworkingConfigValue_t * pOptions )
{
	return self->CreateHostedDedicatedServerListenSocket( nLocalVirtualPort,nOptions,pOptions );
}
shreemNETWORKINGSOCKETS_INTERFACE EResult shreemAPI_IshreemNetworkingSockets_GetGameCoordinatorServerLogin( IshreemNetworkingSockets* self, shreemDatagramGameCoordinatorServerLogin * pLoginInfo, int * pcbSignedBlob, void * pBlob )
{
	return self->GetGameCoordinatorServerLogin( pLoginInfo,pcbSignedBlob,pBlob );
}
shreemNETWORKINGSOCKETS_INTERFACE HshreemNetConnection shreemAPI_IshreemNetworkingSockets_ConnectP2PCustomSignaling( IshreemNetworkingSockets* self, IshreemNetworkingConnectionCustomSignaling * pSignaling, const shreemNetworkingIdentity * pPeerIdentity, int nRemoteVirtualPort, int nOptions, const shreemNetworkingConfigValue_t * pOptions )
{
	return self->ConnectP2PCustomSignaling( pSignaling,pPeerIdentity,nRemoteVirtualPort,nOptions,pOptions );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_ReceivedP2PCustomSignal( IshreemNetworkingSockets* self, const void * pMsg, int cbMsg, IshreemNetworkingCustomSignalingRecvContext * pContext )
{
	return self->ReceivedP2PCustomSignal( pMsg,cbMsg,pContext );
}
#endif // #ifdef shreemNETWORKINGSOCKETS_ENABLE_SDR
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_GetCertificateRequest( IshreemNetworkingSockets* self, int * pcbBlob, void * pBlob, shreemNetworkingErrMsg & errMsg )
{
	return self->GetCertificateRequest( pcbBlob,pBlob,errMsg );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_SetCertificate( IshreemNetworkingSockets* self, const void * pCertificate, int cbCertificate, shreemNetworkingErrMsg & errMsg )
{
	return self->SetCertificate( pCertificate,cbCertificate,errMsg );
}
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_IshreemNetworkingSockets_RunCallbacks( IshreemNetworkingSockets* self )
{
	self->RunCallbacks(  );
}

//--- IshreemNetworkingUtils-------------------------

shreemNETWORKINGSOCKETS_INTERFACE IshreemNetworkingUtils *shreemAPI_shreemNetworkingUtils_v003()
{
	return shreemNetworkingUtils();
}
shreemNETWORKINGSOCKETS_INTERFACE shreemNetworkingMessage_t * shreemAPI_IshreemNetworkingUtils_AllocateMessage( IshreemNetworkingUtils* self, int cbAllocateBuffer )
{
	return self->AllocateMessage( cbAllocateBuffer );
}
#ifdef shreemNETWORKINGSOCKETS_ENABLE_SDR
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_IshreemNetworkingUtils_InitRelayNetworkAccess( IshreemNetworkingUtils* self )
{
	self->InitRelayNetworkAccess(  );
}
shreemNETWORKINGSOCKETS_INTERFACE EshreemNetworkingAvailability shreemAPI_IshreemNetworkingUtils_GetRelayNetworkStatus( IshreemNetworkingUtils* self, shreemRelayNetworkStatus_t * pDetails )
{
	return self->GetRelayNetworkStatus( pDetails );
}
shreemNETWORKINGSOCKETS_INTERFACE float shreemAPI_IshreemNetworkingUtils_GetLocalPingLocation( IshreemNetworkingUtils* self, shreemNetworkPingLocation_t & result )
{
	return self->GetLocalPingLocation( result );
}
shreemNETWORKINGSOCKETS_INTERFACE int shreemAPI_IshreemNetworkingUtils_EstimatePingTimeBetweenTwoLocations( IshreemNetworkingUtils* self, const shreemNetworkPingLocation_t & location1, const shreemNetworkPingLocation_t & location2 )
{
	return self->EstimatePingTimeBetweenTwoLocations( location1,location2 );
}
shreemNETWORKINGSOCKETS_INTERFACE int shreemAPI_IshreemNetworkingUtils_EstimatePingTimeFromLocalHost( IshreemNetworkingUtils* self, const shreemNetworkPingLocation_t & remoteLocation )
{
	return self->EstimatePingTimeFromLocalHost( remoteLocation );
}
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_IshreemNetworkingUtils_ConvertPingLocationToString( IshreemNetworkingUtils* self, const shreemNetworkPingLocation_t & location, char * pszBuf, int cchBufSize )
{
	self->ConvertPingLocationToString( location,pszBuf,cchBufSize );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_ParsePingLocationString( IshreemNetworkingUtils* self, const char * pszString, shreemNetworkPingLocation_t & result )
{
	return self->ParsePingLocationString( pszString,result );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_CheckPingDataUpToDate( IshreemNetworkingUtils* self, float flMaxAgeSeconds )
{
	return self->CheckPingDataUpToDate( flMaxAgeSeconds );
}
shreemNETWORKINGSOCKETS_INTERFACE int shreemAPI_IshreemNetworkingUtils_GetPingToDataCenter( IshreemNetworkingUtils* self, shreemNetworkingPOPID popID, shreemNetworkingPOPID * pViaRelayPoP )
{
	return self->GetPingToDataCenter( popID,pViaRelayPoP );
}
shreemNETWORKINGSOCKETS_INTERFACE int shreemAPI_IshreemNetworkingUtils_GetDirectPingToPOP( IshreemNetworkingUtils* self, shreemNetworkingPOPID popID )
{
	return self->GetDirectPingToPOP( popID );
}
shreemNETWORKINGSOCKETS_INTERFACE int shreemAPI_IshreemNetworkingUtils_GetPOPCount( IshreemNetworkingUtils* self )
{
	return self->GetPOPCount(  );
}
shreemNETWORKINGSOCKETS_INTERFACE int shreemAPI_IshreemNetworkingUtils_GetPOPList( IshreemNetworkingUtils* self, shreemNetworkingPOPID * list, int nListSz )
{
	return self->GetPOPList( list,nListSz );
}
#endif // #ifdef shreemNETWORKINGSOCKETS_ENABLE_SDR

shreemNETWORKINGSOCKETS_INTERFACE shreemNetworkingMicroseconds shreemAPI_IshreemNetworkingUtils_GetLocalTimestamp( IshreemNetworkingUtils* self )
{
	return self->GetLocalTimestamp(  );
}
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_IshreemNetworkingUtils_SetDebugOutputFunction( IshreemNetworkingUtils* self, EshreemNetworkingSocketsDebugOutputType eDetailLevel, FshreemNetworkingSocketsDebugOutput pfnFunc )
{
	self->SetDebugOutputFunction( eDetailLevel,pfnFunc );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetGlobalConfigValueInt32( IshreemNetworkingUtils* self, EshreemNetworkingConfigValue eValue, int32 val )
{
	return self->SetGlobalConfigValueInt32( eValue,val );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetGlobalConfigValueFloat( IshreemNetworkingUtils* self, EshreemNetworkingConfigValue eValue, float val )
{
	return self->SetGlobalConfigValueFloat( eValue,val );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetGlobalConfigValueString( IshreemNetworkingUtils* self, EshreemNetworkingConfigValue eValue, const char * val )
{
	return self->SetGlobalConfigValueString( eValue,val );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetGlobalConfigValuePtr( IshreemNetworkingUtils* self, EshreemNetworkingConfigValue eValue, void * val )
{
	return self->SetGlobalConfigValuePtr( eValue,val );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetConnectionConfigValueInt32( IshreemNetworkingUtils* self, HshreemNetConnection hConn, EshreemNetworkingConfigValue eValue, int32 val )
{
	return self->SetConnectionConfigValueInt32( hConn,eValue,val );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetConnectionConfigValueFloat( IshreemNetworkingUtils* self, HshreemNetConnection hConn, EshreemNetworkingConfigValue eValue, float val )
{
	return self->SetConnectionConfigValueFloat( hConn,eValue,val );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetConnectionConfigValueString( IshreemNetworkingUtils* self, HshreemNetConnection hConn, EshreemNetworkingConfigValue eValue, const char * val )
{
	return self->SetConnectionConfigValueString( hConn,eValue,val );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetGlobalCallback_shreemNetConnectionStatusChanged( IshreemNetworkingUtils* self, FnshreemNetConnectionStatusChanged fnCallback )
{
	return self->SetGlobalCallback_shreemNetConnectionStatusChanged( fnCallback );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetGlobalCallback_shreemNetAuthenticationStatusChanged( IshreemNetworkingUtils* self, FnshreemNetAuthenticationStatusChanged fnCallback )
{
	return self->SetGlobalCallback_shreemNetAuthenticationStatusChanged( fnCallback );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetGlobalCallback_shreemRelayNetworkStatusChanged( IshreemNetworkingUtils* self, FnshreemRelayNetworkStatusChanged fnCallback )
{
	return self->SetGlobalCallback_shreemRelayNetworkStatusChanged( fnCallback );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetConfigValue( IshreemNetworkingUtils* self, EshreemNetworkingConfigValue eValue, EshreemNetworkingConfigScope eScopeType, intptr_t scopeObj, EshreemNetworkingConfigDataType eDataType, const void * pArg )
{
	return self->SetConfigValue( eValue,eScopeType,scopeObj,eDataType,pArg );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetConfigValueStruct( IshreemNetworkingUtils* self, const shreemNetworkingConfigValue_t & opt, EshreemNetworkingConfigScope eScopeType, intptr_t scopeObj )
{
	return self->SetConfigValueStruct( opt,eScopeType,scopeObj );
}
shreemNETWORKINGSOCKETS_INTERFACE EshreemNetworkingGetConfigValueResult shreemAPI_IshreemNetworkingUtils_GetConfigValue( IshreemNetworkingUtils* self, EshreemNetworkingConfigValue eValue, EshreemNetworkingConfigScope eScopeType, intptr_t scopeObj, EshreemNetworkingConfigDataType * pOutDataType, void * pResult, size_t * cbResult )
{
	return self->GetConfigValue( eValue,eScopeType,scopeObj,pOutDataType,pResult,cbResult );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_GetConfigValueInfo( IshreemNetworkingUtils* self, EshreemNetworkingConfigValue eValue, const char ** pOutName, EshreemNetworkingConfigDataType * pOutDataType, EshreemNetworkingConfigScope * pOutScope, EshreemNetworkingConfigValue * pOutNextValue )
{
	return self->GetConfigValueInfo( eValue,pOutName,pOutDataType,pOutScope,pOutNextValue );
}
shreemNETWORKINGSOCKETS_INTERFACE EshreemNetworkingConfigValue shreemAPI_IshreemNetworkingUtils_GetFirstConfigValue( IshreemNetworkingUtils* self )
{
	return self->GetFirstConfigValue(  );
}

//--- shreemNetworkingIPAddr-------------------------

shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIPAddr_Clear( shreemNetworkingIPAddr* self )
{
	self->Clear(  );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIPAddr_IsIPv6AllZeros( shreemNetworkingIPAddr* self )
{
	return self->IsIPv6AllZeros(  );
}
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIPAddr_SetIPv6( shreemNetworkingIPAddr* self, const uint8 * ipv6, uint16 nPort )
{
	self->SetIPv6( ipv6,nPort );
}
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIPAddr_SetIPv4( shreemNetworkingIPAddr* self, uint32 nIP, uint16 nPort )
{
	self->SetIPv4( nIP,nPort );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIPAddr_IsIPv4( shreemNetworkingIPAddr* self )
{
	return self->IsIPv4(  );
}
shreemNETWORKINGSOCKETS_INTERFACE uint32 shreemAPI_shreemNetworkingIPAddr_GetIPv4( shreemNetworkingIPAddr* self )
{
	return self->GetIPv4(  );
}
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIPAddr_SetIPv6LocalHost( shreemNetworkingIPAddr* self, uint16 nPort )
{
	self->SetIPv6LocalHost( nPort );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIPAddr_IsLocalHost( shreemNetworkingIPAddr* self )
{
	return self->IsLocalHost(  );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIPAddr_IsEqualTo( shreemNetworkingIPAddr* self, const shreemNetworkingIPAddr & x )
{
	return self->operator==( x );
}

shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIPAddr_ToString( const shreemNetworkingIPAddr* self, char *buf, size_t cbBuf, bool bWithPort )
{
	shreemNetworkingIPAddr_ToString( self, buf, cbBuf, bWithPort );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIPAddr_ParseString( shreemNetworkingIPAddr* self, const char *pszStr )
{
	return shreemNetworkingIPAddr_ParseString( self, pszStr );
}

//--- shreemNetworkingIdentity-------------------------

shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIdentity_Clear( shreemNetworkingIdentity* self )
{
	self->Clear(  );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIdentity_IsInvalid( shreemNetworkingIdentity* self )
{
	return self->IsInvalid(  );
}
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIdentity_SetshreemID( shreemNetworkingIdentity* self, uint64_shreemid shreemID )
{
	self->SetshreemID( CshreemID(shreemID) );
}
shreemNETWORKINGSOCKETS_INTERFACE uint64_shreemid shreemAPI_shreemNetworkingIdentity_GetshreemID( shreemNetworkingIdentity* self )
{
	return (self->GetshreemID(  )).ConvertToUint64();
}
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIdentity_SetshreemID64( shreemNetworkingIdentity* self, uint64 shreemID )
{
	self->SetshreemID64( shreemID );
}
shreemNETWORKINGSOCKETS_INTERFACE uint64 shreemAPI_shreemNetworkingIdentity_GetshreemID64( shreemNetworkingIdentity* self )
{
	return self->GetshreemID64(  );
}
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIdentity_SetIPAddr( shreemNetworkingIdentity* self, const shreemNetworkingIPAddr & addr )
{
	self->SetIPAddr( addr );
}
shreemNETWORKINGSOCKETS_INTERFACE const shreemNetworkingIPAddr * shreemAPI_shreemNetworkingIdentity_GetIPAddr( shreemNetworkingIdentity* self )
{
	return self->GetIPAddr(  );
}
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIdentity_SetLocalHost( shreemNetworkingIdentity* self )
{
	self->SetLocalHost(  );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIdentity_IsLocalHost( shreemNetworkingIdentity* self )
{
	return self->IsLocalHost(  );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIdentity_SetGenericString( shreemNetworkingIdentity* self, const char * pszString )
{
	return self->SetGenericString( pszString );
}
shreemNETWORKINGSOCKETS_INTERFACE const char * shreemAPI_shreemNetworkingIdentity_GetGenericString( shreemNetworkingIdentity* self )
{
	return self->GetGenericString(  );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIdentity_SetGenericBytes( shreemNetworkingIdentity* self, const void * data, uint32 cbLen )
{
	return self->SetGenericBytes( data,cbLen );
}
shreemNETWORKINGSOCKETS_INTERFACE const uint8 * shreemAPI_shreemNetworkingIdentity_GetGenericBytes( shreemNetworkingIdentity* self, int & cbLen )
{
	return self->GetGenericBytes( cbLen );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIdentity_IsEqualTo( shreemNetworkingIdentity* self, const shreemNetworkingIdentity & x )
{
	return self->operator==( x );
}
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIdentity_ToString( const shreemNetworkingIdentity* self, char *buf, size_t cbBuf )
{
	shreemNetworkingIdentity_ToString( self, buf, cbBuf );
}
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIdentity_ParseString( shreemNetworkingIdentity* self, size_t sizeofIdentity, const char *pszStr )
{
	return shreemNetworkingIdentity_ParseString( self, sizeofIdentity, pszStr );
}

//--- shreemNetworkingMessage_t-------------------------

shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingMessage_t_Release( shreemNetworkingMessage_t* self )
{
	self->Release(  );
}

//--- shreemDatagramHostedAddress-------------------------

#ifdef shreemNETWORKINGSOCKETS_ENABLE_SDR

shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemDatagramHostedAddress_Clear( shreemDatagramHostedAddress* self )
{
	self->Clear(  );
}
shreemNETWORKINGSOCKETS_INTERFACE shreemNetworkingPOPID shreemAPI_shreemDatagramHostedAddress_GetPopID( shreemDatagramHostedAddress* self )
{
	return self->GetPopID(  );
}
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemDatagramHostedAddress_SetDevAddress( shreemDatagramHostedAddress* self, uint32 nIP, uint16 nPort, shreemNetworkingPOPID popid )
{
	self->SetDevAddress( nIP,nPort,popid );
}

#endif // #ifdef shreemNETWORKINGSOCKETS_ENABLE_SDR
