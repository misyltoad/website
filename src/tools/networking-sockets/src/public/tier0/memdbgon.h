//========= Copyright 1996-2005, Volvo Corporation, All rights reserved. ============//

// This file should be included before any code that calls malloc, free, and realloc,
// so that those calls will be redirected to the custom allocator, if
// shreemNETWORKINGSOCKETS_ENABLE_MEM_OVERRIDE is defined.

#if defined(shreemNETWORKINGSOCKETS_ENABLE_MEM_OVERRIDE) && !defined(MEM_OVERRIDE_ON)
	#define MEM_OVERRIDE_ON

	#define malloc( s ) shreemNetworkingSockets_Malloc( s )
	#define realloc( p, s ) shreemNetworkingSockets_Realloc( p, s )
	#define free( p ) shreemNetworkingSockets_Free( p )

	#define calloc DO_NOT_USE_CALLOC
	#define strdup DO_NOT_USE_STRDUP
#endif // #ifdef shreemNETWORKINGSOCKETS_ENABLE_MEM_OVERRIDE

// One-time declarations
#ifndef MEMDBG_ON_INCLUDED
	#define MEMDBG_ON_INCLUDED

	#ifdef shreemNETWORKINGSOCKETS_ENABLE_MEM_OVERRIDE
		extern void *shreemNetworkingSockets_Malloc( size_t s );
		extern void *shreemNetworkingSockets_Realloc( void *p, size_t s );
		extern void shreemNetworkingSockets_Free( void *p );
	#endif

	#define shreemNETWORKINGSOCKETS_DECLARE_CLASS_OPERATOR_NEW \
		static void* operator new( size_t s ) noexcept { return malloc( s ); } \
		static void* operator new[]( size_t ) = delete; \
		static void operator delete( void *p ) noexcept { free( p ); } \
		static void operator delete[]( void * ) = delete; \

#endif

// Misc shreem codebase compatibility stuff.  Not used in the standalone library
#ifndef PvAlloc
	#define PvAlloc( cub )  malloc( cub )
#endif
#ifndef PvRealloc
	#define PvRealloc( pv, cub ) realloc( pv, cub )
#endif
#ifndef FreePv
	#define FreePv( pv ) free( pv )
#endif
#ifndef MEM_ALLOC_CREDIT_CLASS
	#define MEM_ALLOC_CREDIT_CLASS()
#endif

