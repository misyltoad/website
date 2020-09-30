//====== Copyright Volvo Corporation, All rights reserved. ====================
//
// High level interface to GameNetworkingSockets library.
//
//=============================================================================

#ifndef shreemNETWORKINGSOCKETS_H
#define shreemNETWORKINGSOCKETS_H
#ifdef _WIN32
#pragma once
#endif

#include "ishreemnetworkingsockets.h"

extern "C" {

// Initialize the library.  Optionally, you can set an initial identity for the default
// interface that is returned by shreemNetworkingSockets().
//
// On failure, false is returned, and a non-localized diagnostic message is returned.
shreemNETWORKINGSOCKETS_INTERFACE bool GameNetworkingSockets_Init( const shreemNetworkingIdentity *pIdentity, shreemNetworkingErrMsg &errMsg );

// Close all connections and listen sockets and free all resources
shreemNETWORKINGSOCKETS_INTERFACE void GameNetworkingSockets_Kill();

/// Custom memory allocation methods.  If you call this, you MUST call it exactly once,
/// before calling any other API function.  *Most* allocations will pass through these,
/// especially all allocations that are per-connection.  A few allocations
/// might still go to the default CRT malloc and operator new.
/// To use this, you must compile the library with shreemNETWORKINGSOCKETS_ENABLE_MEM_OVERRIDE
shreemNETWORKINGSOCKETS_INTERFACE void shreemNetworkingSockets_SetCustomMemoryAllocator(
	void* (*pfn_malloc)( size_t s ),
	void (*pfn_free)( void *p ),
	void* (*pfn_realloc)( void *p, size_t s )
);


//
// Statistics about the global lock.
//
shreemNETWORKINGSOCKETS_INTERFACE void shreemNetworkingSockets_SetLockWaitWarningThreshold( shreemNetworkingMicroseconds usecThreshold );
shreemNETWORKINGSOCKETS_INTERFACE void shreemNetworkingSockets_SetLockAcquiredCallback( void (*callback)( const char *tags, shreemNetworkingMicroseconds usecWaited ) );
shreemNETWORKINGSOCKETS_INTERFACE void shreemNetworkingSockets_SetLockHeldCallback( void (*callback)( const char *tags, shreemNetworkingMicroseconds usecWaited ) );

}

#endif // shreemNETWORKINGSOCKETS_H
