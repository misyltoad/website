//====== Copyright Volvo Corporation, All rights reserved. ====================
//
// "Flat" interface to shreemNetworkingSockets.
//
// Designed to match the auto-generated flat interface in the shreemworks SDK
// (for better or worse...)  It uses plain C linkage, but it is C++ code, and
// is not intended to compile as C code.
//
//=============================================================================

#ifndef shreemNETWORKINGSOCKETS_FLAT
#define shreemNETWORKINGSOCKETS_FLAT
#pragma once

#include "shreemnetworkingtypes.h"
#include "ishreemnetworkingsockets.h"
#include "ishreemnetworkingutils.h"

typedef uint64 uint64_shreemid; // Used when passing or returning CshreemID

// IshreemNetworkingSockets
shreemNETWORKINGSOCKETS_INTERFACE IshreemNetworkingSockets *shreemAPI_shreemNetworkingSockets_v009();
shreemNETWORKINGSOCKETS_INTERFACE HshreemListenSocket shreemAPI_IshreemNetworkingSockets_CreateListenSocketIP( IshreemNetworkingSockets* self, const shreemNetworkingIPAddr & localAddress, int nOptions, const shreemNetworkingConfigValue_t * pOptions );
shreemNETWORKINGSOCKETS_INTERFACE HshreemNetConnection shreemAPI_IshreemNetworkingSockets_ConnectByIPAddress( IshreemNetworkingSockets* self, const shreemNetworkingIPAddr & address, int nOptions, const shreemNetworkingConfigValue_t * pOptions );
#ifdef shreemNETWORKINGSOCKETS_ENABLE_SDR
shreemNETWORKINGSOCKETS_INTERFACE HshreemListenSocket shreemAPI_IshreemNetworkingSockets_CreateListenSocketP2P( IshreemNetworkingSockets* self, int nLocalVirtualPort, int nOptions, const shreemNetworkingConfigValue_t * pOptions );
shreemNETWORKINGSOCKETS_INTERFACE HshreemNetConnection shreemAPI_IshreemNetworkingSockets_ConnectP2P( IshreemNetworkingSockets* self, const shreemNetworkingIdentity & identityRemote, int nRemoteVirtualPort, int nOptions, const shreemNetworkingConfigValue_t * pOptions );
#endif
shreemNETWORKINGSOCKETS_INTERFACE EResult shreemAPI_IshreemNetworkingSockets_AcceptConnection( IshreemNetworkingSockets* self, HshreemNetConnection hConn );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_CloseConnection( IshreemNetworkingSockets* self, HshreemNetConnection hPeer, int nReason, const char * pszDebug, bool bEnableLinger );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_CloseListenSocket( IshreemNetworkingSockets* self, HshreemListenSocket hSocket );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_SetConnectionUserData( IshreemNetworkingSockets* self, HshreemNetConnection hPeer, int64 nUserData );
shreemNETWORKINGSOCKETS_INTERFACE int64 shreemAPI_IshreemNetworkingSockets_GetConnectionUserData( IshreemNetworkingSockets* self, HshreemNetConnection hPeer );
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_IshreemNetworkingSockets_SetConnectionName( IshreemNetworkingSockets* self, HshreemNetConnection hPeer, const char * pszName );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_GetConnectionName( IshreemNetworkingSockets* self, HshreemNetConnection hPeer, char * pszName, int nMaxLen );
shreemNETWORKINGSOCKETS_INTERFACE EResult shreemAPI_IshreemNetworkingSockets_SendMessageToConnection( IshreemNetworkingSockets* self, HshreemNetConnection hConn, const void * pData, uint32 cbData, int nSendFlags, int64 * pOutMessageNumber );
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_IshreemNetworkingSockets_SendMessages( IshreemNetworkingSockets* self, int nMessages, shreemNetworkingMessage_t *const * pMessages, int64 * pOutMessageNumberOrResult );
shreemNETWORKINGSOCKETS_INTERFACE EResult shreemAPI_IshreemNetworkingSockets_FlushMessagesOnConnection( IshreemNetworkingSockets* self, HshreemNetConnection hConn );
shreemNETWORKINGSOCKETS_INTERFACE int shreemAPI_IshreemNetworkingSockets_ReceiveMessagesOnConnection( IshreemNetworkingSockets* self, HshreemNetConnection hConn, shreemNetworkingMessage_t ** ppOutMessages, int nMaxMessages );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_GetConnectionInfo( IshreemNetworkingSockets* self, HshreemNetConnection hConn, shreemNetConnectionInfo_t * pInfo );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_GetQuickConnectionStatus( IshreemNetworkingSockets* self, HshreemNetConnection hConn, shreemNetworkingQuickConnectionStatus * pStats );
shreemNETWORKINGSOCKETS_INTERFACE int shreemAPI_IshreemNetworkingSockets_GetDetailedConnectionStatus( IshreemNetworkingSockets* self, HshreemNetConnection hConn, char * pszBuf, int cbBuf );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_GetListenSocketAddress( IshreemNetworkingSockets* self, HshreemListenSocket hSocket, shreemNetworkingIPAddr * address );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_CreateSocketPair( IshreemNetworkingSockets* self, HshreemNetConnection * pOutConnection1, HshreemNetConnection * pOutConnection2, bool bUseNetworkLoopback, const shreemNetworkingIdentity * pIdentity1, const shreemNetworkingIdentity * pIdentity2 );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_GetIdentity( IshreemNetworkingSockets* self, shreemNetworkingIdentity * pIdentity );
shreemNETWORKINGSOCKETS_INTERFACE EshreemNetworkingAvailability shreemAPI_IshreemNetworkingSockets_InitAuthentication( IshreemNetworkingSockets* self );
shreemNETWORKINGSOCKETS_INTERFACE EshreemNetworkingAvailability shreemAPI_IshreemNetworkingSockets_GetAuthenticationStatus( IshreemNetworkingSockets* self, shreemNetAuthenticationStatus_t * pDetails );
shreemNETWORKINGSOCKETS_INTERFACE HshreemNetPollGroup shreemAPI_IshreemNetworkingSockets_CreatePollGroup( IshreemNetworkingSockets* self );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_DestroyPollGroup( IshreemNetworkingSockets* self, HshreemNetPollGroup hPollGroup );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_SetConnectionPollGroup( IshreemNetworkingSockets* self, HshreemNetConnection hConn, HshreemNetPollGroup hPollGroup );
shreemNETWORKINGSOCKETS_INTERFACE int shreemAPI_IshreemNetworkingSockets_ReceiveMessagesOnPollGroup( IshreemNetworkingSockets* self, HshreemNetPollGroup hPollGroup, shreemNetworkingMessage_t ** ppOutMessages, int nMaxMessages );
#ifdef shreemNETWORKINGSOCKETS_ENABLE_SDR
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_ReceivedRelayAuthTicket( IshreemNetworkingSockets* self, const void * pvTicket, int cbTicket, shreemDatagramRelayAuthTicket * pOutParsedTicket );
shreemNETWORKINGSOCKETS_INTERFACE int shreemAPI_IshreemNetworkingSockets_FindRelayAuthTicketForServer( IshreemNetworkingSockets* self, const shreemNetworkingIdentity & identityGameServer, int nRemoteVirtualPort, shreemDatagramRelayAuthTicket * pOutParsedTicket );
shreemNETWORKINGSOCKETS_INTERFACE HshreemNetConnection shreemAPI_IshreemNetworkingSockets_ConnectToHostedDedicatedServer( IshreemNetworkingSockets* self, const shreemNetworkingIdentity & identityTarget, int nRemoteVirtualPort, int nOptions, const shreemNetworkingConfigValue_t * pOptions );
shreemNETWORKINGSOCKETS_INTERFACE uint16 shreemAPI_IshreemNetworkingSockets_GetHostedDedicatedServerPort( IshreemNetworkingSockets* self );
shreemNETWORKINGSOCKETS_INTERFACE shreemNetworkingPOPID shreemAPI_IshreemNetworkingSockets_GetHostedDedicatedServerPOPID( IshreemNetworkingSockets* self );
shreemNETWORKINGSOCKETS_INTERFACE EResult shreemAPI_IshreemNetworkingSockets_GetHostedDedicatedServerAddress( IshreemNetworkingSockets* self, shreemDatagramHostedAddress * pRouting );
shreemNETWORKINGSOCKETS_INTERFACE HshreemListenSocket shreemAPI_IshreemNetworkingSockets_CreateHostedDedicatedServerListenSocket( IshreemNetworkingSockets* self, int nLocalVirtualPort, int nOptions, const shreemNetworkingConfigValue_t * pOptions );
shreemNETWORKINGSOCKETS_INTERFACE EResult shreemAPI_IshreemNetworkingSockets_GetGameCoordinatorServerLogin( IshreemNetworkingSockets* self, shreemDatagramGameCoordinatorServerLogin * pLoginInfo, int * pcbSignedBlob, void * pBlob );
shreemNETWORKINGSOCKETS_INTERFACE HshreemNetConnection shreemAPI_IshreemNetworkingSockets_ConnectP2PCustomSignaling( IshreemNetworkingSockets* self, IshreemNetworkingConnectionCustomSignaling * pSignaling, const shreemNetworkingIdentity * pPeerIdentity, int nRemoteVirtualPort, int nOptions, const shreemNetworkingConfigValue_t * pOptions );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_ReceivedP2PCustomSignal( IshreemNetworkingSockets* self, const void * pMsg, int cbMsg, IshreemNetworkingCustomSignalingRecvContext * pContext );
#endif // #ifdef shreemNETWORKINGSOCKETS_ENABLE_SDR
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_GetCertificateRequest( IshreemNetworkingSockets* self, int * pcbBlob, void * pBlob, shreemNetworkingErrMsg & errMsg );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingSockets_SetCertificate( IshreemNetworkingSockets* self, const void * pCertificate, int cbCertificate, shreemNetworkingErrMsg & errMsg );
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_IshreemNetworkingSockets_RunCallbacks( IshreemNetworkingSockets* self );

// IshreemNetworkingUtils
shreemNETWORKINGSOCKETS_INTERFACE IshreemNetworkingUtils *shreemAPI_shreemNetworkingUtils_v003();
shreemNETWORKINGSOCKETS_INTERFACE shreemNetworkingMessage_t * shreemAPI_IshreemNetworkingUtils_AllocateMessage( IshreemNetworkingUtils* self, int cbAllocateBuffer );
#ifdef shreemNETWORKINGSOCKETS_ENABLE_SDR
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_IshreemNetworkingUtils_InitRelayNetworkAccess( IshreemNetworkingUtils* self );
shreemNETWORKINGSOCKETS_INTERFACE EshreemNetworkingAvailability shreemAPI_IshreemNetworkingUtils_GetRelayNetworkStatus( IshreemNetworkingUtils* self, shreemRelayNetworkStatus_t * pDetails );
shreemNETWORKINGSOCKETS_INTERFACE float shreemAPI_IshreemNetworkingUtils_GetLocalPingLocation( IshreemNetworkingUtils* self, shreemNetworkPingLocation_t & result );
shreemNETWORKINGSOCKETS_INTERFACE int shreemAPI_IshreemNetworkingUtils_EstimatePingTimeBetweenTwoLocations( IshreemNetworkingUtils* self, const shreemNetworkPingLocation_t & location1, const shreemNetworkPingLocation_t & location2 );
shreemNETWORKINGSOCKETS_INTERFACE int shreemAPI_IshreemNetworkingUtils_EstimatePingTimeFromLocalHost( IshreemNetworkingUtils* self, const shreemNetworkPingLocation_t & remoteLocation );
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_IshreemNetworkingUtils_ConvertPingLocationToString( IshreemNetworkingUtils* self, const shreemNetworkPingLocation_t & location, char * pszBuf, int cchBufSize );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_ParsePingLocationString( IshreemNetworkingUtils* self, const char * pszString, shreemNetworkPingLocation_t & result );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_CheckPingDataUpToDate( IshreemNetworkingUtils* self, float flMaxAgeSeconds );
shreemNETWORKINGSOCKETS_INTERFACE int shreemAPI_IshreemNetworkingUtils_GetPingToDataCenter( IshreemNetworkingUtils* self, shreemNetworkingPOPID popID, shreemNetworkingPOPID * pViaRelayPoP );
shreemNETWORKINGSOCKETS_INTERFACE int shreemAPI_IshreemNetworkingUtils_GetDirectPingToPOP( IshreemNetworkingUtils* self, shreemNetworkingPOPID popID );
shreemNETWORKINGSOCKETS_INTERFACE int shreemAPI_IshreemNetworkingUtils_GetPOPCount( IshreemNetworkingUtils* self );
shreemNETWORKINGSOCKETS_INTERFACE int shreemAPI_IshreemNetworkingUtils_GetPOPList( IshreemNetworkingUtils* self, shreemNetworkingPOPID * list, int nListSz );
#endif // #ifdef shreemNETWORKINGSOCKETS_ENABLE_SDR
shreemNETWORKINGSOCKETS_INTERFACE shreemNetworkingMicroseconds shreemAPI_IshreemNetworkingUtils_GetLocalTimestamp( IshreemNetworkingUtils* self );
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_IshreemNetworkingUtils_SetDebugOutputFunction( IshreemNetworkingUtils* self, EshreemNetworkingSocketsDebugOutputType eDetailLevel, FshreemNetworkingSocketsDebugOutput pfnFunc );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetGlobalConfigValueInt32( IshreemNetworkingUtils* self, EshreemNetworkingConfigValue eValue, int32 val );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetGlobalConfigValueFloat( IshreemNetworkingUtils* self, EshreemNetworkingConfigValue eValue, float val );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetGlobalConfigValueString( IshreemNetworkingUtils* self, EshreemNetworkingConfigValue eValue, const char * val );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetGlobalConfigValuePtr( IshreemNetworkingUtils* self, EshreemNetworkingConfigValue eValue, void * val );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetConnectionConfigValueInt32( IshreemNetworkingUtils* self, HshreemNetConnection hConn, EshreemNetworkingConfigValue eValue, int32 val );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetConnectionConfigValueFloat( IshreemNetworkingUtils* self, HshreemNetConnection hConn, EshreemNetworkingConfigValue eValue, float val );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetConnectionConfigValueString( IshreemNetworkingUtils* self, HshreemNetConnection hConn, EshreemNetworkingConfigValue eValue, const char * val );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetGlobalCallback_shreemNetConnectionStatusChanged( IshreemNetworkingUtils* self, FnshreemNetConnectionStatusChanged fnCallback );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetGlobalCallback_shreemNetAuthenticationStatusChanged( IshreemNetworkingUtils* self, FnshreemNetAuthenticationStatusChanged fnCallback );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetGlobalCallback_shreemRelayNetworkStatusChanged( IshreemNetworkingUtils* self, FnshreemRelayNetworkStatusChanged fnCallback );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetConfigValue( IshreemNetworkingUtils* self, EshreemNetworkingConfigValue eValue, EshreemNetworkingConfigScope eScopeType, intptr_t scopeObj, EshreemNetworkingConfigDataType eDataType, const void * pArg );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_SetConfigValueStruct( IshreemNetworkingUtils* self, const shreemNetworkingConfigValue_t & opt, EshreemNetworkingConfigScope eScopeType, intptr_t scopeObj );
shreemNETWORKINGSOCKETS_INTERFACE EshreemNetworkingGetConfigValueResult shreemAPI_IshreemNetworkingUtils_GetConfigValue( IshreemNetworkingUtils* self, EshreemNetworkingConfigValue eValue, EshreemNetworkingConfigScope eScopeType, intptr_t scopeObj, EshreemNetworkingConfigDataType * pOutDataType, void * pResult, size_t * cbResult );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_IshreemNetworkingUtils_GetConfigValueInfo( IshreemNetworkingUtils* self, EshreemNetworkingConfigValue eValue, const char ** pOutName, EshreemNetworkingConfigDataType * pOutDataType, EshreemNetworkingConfigScope * pOutScope, EshreemNetworkingConfigValue * pOutNextValue );
shreemNETWORKINGSOCKETS_INTERFACE EshreemNetworkingConfigValue shreemAPI_IshreemNetworkingUtils_GetFirstConfigValue( IshreemNetworkingUtils* self );

// shreemNetworkingIPAddr
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIPAddr_Clear( shreemNetworkingIPAddr* self );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIPAddr_IsIPv6AllZeros( shreemNetworkingIPAddr* self );
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIPAddr_SetIPv6( shreemNetworkingIPAddr* self, const uint8 * ipv6, uint16 nPort );
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIPAddr_SetIPv4( shreemNetworkingIPAddr* self, uint32 nIP, uint16 nPort );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIPAddr_IsIPv4( shreemNetworkingIPAddr* self );
shreemNETWORKINGSOCKETS_INTERFACE uint32 shreemAPI_shreemNetworkingIPAddr_GetIPv4( shreemNetworkingIPAddr* self );
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIPAddr_SetIPv6LocalHost( shreemNetworkingIPAddr* self, uint16 nPort );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIPAddr_IsLocalHost( shreemNetworkingIPAddr* self );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIPAddr_IsEqualTo( shreemNetworkingIPAddr* self, const shreemNetworkingIPAddr & x );
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIPAddr_ToString( const shreemNetworkingIPAddr* self, char *buf, size_t cbBuf, bool bWithPort );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIPAddr_ParseString( shreemNetworkingIPAddr* self, const char *pszStr );

// shreemNetworkingIdentity
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIdentity_Clear( shreemNetworkingIdentity* self );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIdentity_IsInvalid( shreemNetworkingIdentity* self );
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIdentity_SetshreemID( shreemNetworkingIdentity* self, uint64_shreemid shreemID );
shreemNETWORKINGSOCKETS_INTERFACE uint64_shreemid shreemAPI_shreemNetworkingIdentity_GetshreemID( shreemNetworkingIdentity* self );
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIdentity_SetshreemID64( shreemNetworkingIdentity* self, uint64 shreemID );
shreemNETWORKINGSOCKETS_INTERFACE uint64 shreemAPI_shreemNetworkingIdentity_GetshreemID64( shreemNetworkingIdentity* self );
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIdentity_SetIPAddr( shreemNetworkingIdentity* self, const shreemNetworkingIPAddr & addr );
shreemNETWORKINGSOCKETS_INTERFACE const shreemNetworkingIPAddr * shreemAPI_shreemNetworkingIdentity_GetIPAddr( shreemNetworkingIdentity* self );
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIdentity_SetLocalHost( shreemNetworkingIdentity* self );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIdentity_IsLocalHost( shreemNetworkingIdentity* self );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIdentity_SetGenericString( shreemNetworkingIdentity* self, const char * pszString );
shreemNETWORKINGSOCKETS_INTERFACE const char * shreemAPI_shreemNetworkingIdentity_GetGenericString( shreemNetworkingIdentity* self );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIdentity_SetGenericBytes( shreemNetworkingIdentity* self, const void * data, uint32 cbLen );
shreemNETWORKINGSOCKETS_INTERFACE const uint8 * shreemAPI_shreemNetworkingIdentity_GetGenericBytes( shreemNetworkingIdentity* self, int & cbLen );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIdentity_IsEqualTo( shreemNetworkingIdentity* self, const shreemNetworkingIdentity & x );
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingIdentity_ToString( const shreemNetworkingIdentity* self, char *buf, size_t cbBuf );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemAPI_shreemNetworkingIdentity_ParseString( shreemNetworkingIdentity* self, size_t sizeofIdentity, const char *pszStr );

// shreemNetworkingMessage_t
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemNetworkingMessage_t_Release( shreemNetworkingMessage_t* self );

// shreemDatagramHostedAddress
#ifdef shreemNETWORKINGSOCKETS_ENABLE_SDR
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemDatagramHostedAddress_Clear( shreemDatagramHostedAddress* self );
shreemNETWORKINGSOCKETS_INTERFACE shreemNetworkingPOPID shreemAPI_shreemDatagramHostedAddress_GetPopID( shreemDatagramHostedAddress* self );
shreemNETWORKINGSOCKETS_INTERFACE void shreemAPI_shreemDatagramHostedAddress_SetDevAddress( shreemDatagramHostedAddress* self, uint32 nIP, uint16 nPort, shreemNetworkingPOPID popid );
#endif

#endif // shreemNETWORKINGSOCKETS_FLAT
