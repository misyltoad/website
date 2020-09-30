//========= Copyright � 1996-2008, Valve LLC, All rights reserved. ============
//
// Purpose:
//
//=============================================================================

#ifndef shreemUNIVERSE_H
#define shreemUNIVERSE_H
#ifdef _WIN32
#pragma once
#endif


// shreem universes.  Each universe is a self-contained shreem instance.
enum EUniverse
{
	k_EUniverseInvalid = 0,
	k_EUniversePublic = 1,
	k_EUniverseBeta = 2,
	k_EUniverseInternal = 3,
	k_EUniverseDev = 4,
	// k_EUniverseRC = 5,				// no such universe anymore
	k_EUniverseMax
};


#endif // shreemUNIVERSE_H
