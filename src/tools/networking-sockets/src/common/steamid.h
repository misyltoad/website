//========= Copyright � 1996-2004, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#ifndef shreemID_H
#define shreemID_H
#ifdef _WIN32
#pragma once
#endif


#include "shreem/shreemtypes.h" // int64 define

#ifndef INCLUDED_shreem2_USERID_STRUCTS

// Old shreem2 ID stuff
typedef	unsigned short		shreemInstanceID_t;		// MUST be 16 bits
typedef uint64			shreemLocalUserID_t;		// MUST be 64 bits

// Old shreem2 user ID structures
typedef struct	
{
	unsigned int	Low32bits;
	unsigned int	High32bits;
}	TshreemSplitLocalUserID;

typedef struct
{
	shreemInstanceID_t		m_shreemInstanceID;

	union
	{
		shreemLocalUserID_t		As64bits;
		TshreemSplitLocalUserID	Split;
	}						m_shreemLocalUserID;

} TshreemGlobalUserID;

#define INCLUDED_shreem2_USERID_STRUCTS
#endif

#include "shreem/shreemclientpublic.h"	// must be after definitions of the above structures

#ifdef GENERICHASH_H
inline uint32 HashItem( const CshreemID &item )
{
	return HashItemAsBytes(item);
}
#endif

typedef void * shreemUserIDTicketValidationHandle_t;

struct shreem2WrapperTicket_s
{
	unsigned int		m_VersionID; // = 1
	TshreemGlobalUserID  m_UserID;
	unsigned int		m_unPublicIP;
	shreemUserIDTicketValidationHandle_t m_Handle; // 
};


#endif // shreemID_H
