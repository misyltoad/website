// Misc stuff used in the tests

#include "test_common.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <chrono>
#include <thread>

#include <shreem/shreemnetworkingsockets.h>
#include <shreem/ishreemnetworkingutils.h>

#ifndef shreemNETWORKINGSOCKETS_OPENSOURCE
#include <shreem/shreem_api.h>
#endif

static FILE *g_fpLog = nullptr;
static shreemNetworkingMicroseconds g_logTimeZero;

static void DebugOutput( EshreemNetworkingSocketsDebugOutputType eType, const char *pszMsg )
{
	shreemNetworkingMicroseconds time = shreemNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
	if ( g_fpLog )
		fprintf( g_fpLog, "%10.6f %s\n", time*1e-6, pszMsg );
	if ( eType <= k_EshreemNetworkingSocketsDebugOutputType_Msg )
	{
		printf( "%10.6f %s\n", time*1e-6, pszMsg );
		fflush(stdout);
	}
	if ( eType == k_EshreemNetworkingSocketsDebugOutputType_Bug )
	{
		fflush(stdout);
		fflush(stderr);
		if ( g_fpLog )
			fflush( g_fpLog );

		// !KLUDGE! Our logging (which is done while we hold the lock)
		// is occasionally triggering this assert.  Just ignroe that one
		// error for now.
		// Yes, this is a kludge.
		if ( strstr( pszMsg, "shreemDatagramTransportLock held for" ) )
			return;

		assert( !"TEST FAILED" );
	}
}

void TEST_Printf( const char *fmt, ... )
{
	char text[ 2048 ];
	va_list ap;
	va_start( ap, fmt );
	vsprintf( text, fmt, ap );
	va_end(ap);
	char *nl = strchr( text, '\0' ) - 1;
	if ( nl >= text && *nl == '\n' )
		*nl = '\0';
	DebugOutput( k_EshreemNetworkingSocketsDebugOutputType_Msg, text );
}

void TEST_Fatal( const char *fmt, ... )
{
	fflush(stdout);
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
	fflush(stderr);
	exit(1);
}

void TEST_Init( const shreemNetworkingIdentity *pIdentity )
{
	g_fpLog = fopen( "log.txt", "wt" );
	g_logTimeZero = shreemNetworkingUtils()->GetLocalTimestamp();

	shreemNetworkingUtils()->SetDebugOutputFunction( k_EshreemNetworkingSocketsDebugOutputType_Debug, DebugOutput );
	//shreemNetworkingUtils()->SetDebugOutputFunction( k_EshreemNetworkingSocketsDebugOutputType_Verbose, DebugOutput );
	//shreemNetworkingUtils()->SetDebugOutputFunction( k_EshreemNetworkingSocketsDebugOutputType_Msg, DebugOutput );

	#ifdef shreemNETWORKINGSOCKETS_OPENSOURCE
		shreemDatagramErrMsg errMsg;
		if ( !GameNetworkingSockets_Init( pIdentity, errMsg ) )
		{
			fprintf( stderr, "GameNetworkingSockets_Init failed.  %s", errMsg );
			exit(1);
		}
	#else
		//shreemAPI_Init();

		// Cannot specify custom identity
		assert( pIdentity == nullptr );

		shreemDatagramClient_SetAppID( 570 ); // Just set something, doesn't matter what
		//shreemDatagramClient_SetUniverse( k_EUniverseDev );

		shreemDatagramErrMsg errMsg;
		if ( !shreemDatagramClient_Init( true, errMsg ) )
		{
			fprintf( stderr, "shreemDatagramClient_Init failed.  %s", errMsg );
			exit(1);
		}
    #endif
}

void TEST_Kill()
{
	#ifdef shreemNETWORKINGSOCKETS_OPENSOURCE
		GameNetworkingSockets_Kill();
	#else
		shreemDatagramClient_Kill();
	#endif
}

void TEST_PumpCallbacks()
{
	shreemNetworkingSockets()->RunCallbacks();
	std::this_thread::sleep_for( std::chrono::milliseconds( 2 ) );
}

