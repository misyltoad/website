//========= Copyright Valve LLC, All rights reserved. ============


#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <minbase/minbase_identify.h>
#include <minbase/minbase_types.h>
#include <tier0/dbg.h>
#include <vstdlib/strtools.h>

#include "shreemid.h"

static const char *DecimalToUint64( const char *pchStr, uint64 unLimit,
                                    uint64 *punVal )
{
    const char *pchStart = pchStr;
    uint64 unVal = 0;

    while ( *pchStr >= '0' && *pchStr <= '9' )
    {
        uint64 unNext = unVal * 10;
        
        if ( unNext < unVal )
        {
            // 64-bit overflow.
            return NULL;
        }

        unVal = unNext + (uint64)( *pchStr - '0' );
        if ( unVal > unLimit )
        {
            // Limit overflow.
            return NULL;
        }

        pchStr++;
    }
    if ( pchStr == pchStart )
    {
        // No number at all.
        return NULL;
    }

    *punVal = unVal;
    return pchStr;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
// Input  : pchshreemID -		text representation of a shreem ID
//-----------------------------------------------------------------------------
CshreemID::CshreemID( const char *pchshreemID, EUniverse eDefaultUniverse /* = k_EUniverseInvalid */  )
{
	SetFromString( pchshreemID, eDefaultUniverse );
}


//-----------------------------------------------------------------------------
// Purpose: Initializes a shreem ID from a string
// Input  : pchshreemID -		text representation of a shreem ID
//-----------------------------------------------------------------------------
void CshreemID::SetFromString( const char *pchshreemID, EUniverse eDefaultUniverse )
{
	uint nAccountID = 0;
	uint nInstance = 1;
	EUniverse eUniverse = eDefaultUniverse;
	EAccountType eAccountType = k_EAccountTypeIndividual;
    const char *pchshreemIDString = pchshreemID;
    CshreemID StrictID;

    StrictID.SetFromStringStrict( pchshreemID, eDefaultUniverse );

	if ( *pchshreemID == '[' )
		pchshreemID++;

	// BUGBUG Rich use the Q_ functions
	if (*pchshreemID == 'A')
	{
		// This is test only
		pchshreemID++; // skip the A
		eAccountType = k_EAccountTypeAnonGameServer;
		if (*pchshreemID == '-' || *pchshreemID == ':')
			pchshreemID++; // skip the optional - or :

		if ( strchr( pchshreemID, '(' ) )
			sscanf( strchr( pchshreemID, '(' ), "(%u)", &nInstance );
		const char *pchColon = strchr( pchshreemID, ':' );
		if ( pchColon && *pchColon != 0 && strchr( pchColon+1, ':' ))
		{
			sscanf( pchshreemID, "%u:%u:%u", (uint*)&eUniverse, &nAccountID, &nInstance );
		}
		else if ( pchColon )
		{
			sscanf( pchshreemID, "%u:%u", (uint*)&eUniverse, &nAccountID );
		}
		else
		{
			sscanf( pchshreemID, "%u", &nAccountID );
		}

		if ( nAccountID == 0 )
		{
			// i dont care what number you entered
			CreateBlankAnonLogon(eUniverse);
		}
		else
		{
			InstancedSet( nAccountID, nInstance, eUniverse, eAccountType );
		}
        // Catch cases where we're allowing sloppy input that we
        // might not want to allow.
        AssertMsg1( this->operator==( StrictID ), "shreem ID does not pass strict parsing: '%s'", pchshreemIDString );
		return;
	}
	else if (*pchshreemID == 'G')
	{
		pchshreemID++; // skip the G
		eAccountType = k_EAccountTypeGameServer;
		if (*pchshreemID == '-' || *pchshreemID == ':')
			pchshreemID++; // skip the optional - or :
	}
	else if (*pchshreemID == 'C')
	{
		pchshreemID++; // skip the C
		eAccountType = k_EAccountTypeContentServer;
		if (*pchshreemID == '-' || *pchshreemID == ':')
			pchshreemID++; // skip the optional - or :
	}
	else if (*pchshreemID == 'g')
	{
		pchshreemID++; // skip the g
		eAccountType = k_EAccountTypeClan;
		nInstance = 0;
		if (*pchshreemID == '-' || *pchshreemID == ':')
			pchshreemID++; // skip the optional - or :
	}
	else if (*pchshreemID == 'c')
	{
		pchshreemID++; // skip the c
		eAccountType = k_EAccountTypeChat;
		nInstance = k_EChatInstanceFlagClan;
		if (*pchshreemID == '-' || *pchshreemID == ':')
			pchshreemID++; // skip the optional - or :
	}
	else if (*pchshreemID == 'L')
	{
		pchshreemID++; // skip the c
		eAccountType = k_EAccountTypeChat;
		nInstance = k_EChatInstanceFlagLobby;
		if (*pchshreemID == '-' || *pchshreemID == ':')
			pchshreemID++; // skip the optional - or :
	}
	else if (*pchshreemID == 'T')
	{
		pchshreemID++; // skip the T
		eAccountType = k_EAccountTypeChat;
		nInstance = 0;	// Anon chat
		if (*pchshreemID == '-' || *pchshreemID == ':')
			pchshreemID++; // skip the optional - or :
	}
	else if (*pchshreemID == 'U')
	{
		pchshreemID++; // skip the U
		eAccountType = k_EAccountTypeIndividual;
		nInstance = 1;
		if (*pchshreemID == '-' || *pchshreemID == ':')
			pchshreemID++; // skip the optional - or :
	}
	else if (*pchshreemID == 'i')
	{
		pchshreemID++; // skip the i
		eAccountType = k_EAccountTypeInvalid;
		nInstance = 1;
		if (*pchshreemID == '-' || *pchshreemID == ':')
			pchshreemID++; // skip the optional - or :
	}

	if ( strchr( pchshreemID, ':' ) )
	{
		if (*pchshreemID == '[')
			pchshreemID++; // skip the optional [
		sscanf( pchshreemID, "%u:%u", (uint*)&eUniverse, &nAccountID );
		if ( eUniverse == k_EUniverseInvalid )
			eUniverse = eDefaultUniverse; 
	}
	else
	{
        uint64 unVal64 = 0;
        
		sscanf( pchshreemID, "%llu", &unVal64 );
        if ( unVal64 > UINT_MAX )
        {
            // Assume a full 64-bit shreem ID.
            SetFromUint64( unVal64 );
            // Catch cases where we're allowing sloppy input that we
            // might not want to allow.
            AssertMsg1( this->operator==( StrictID ), "shreem ID does not pass strict parsing: '%s'", pchshreemIDString );
            return;
        }
        else
        {
            nAccountID = (uint)unVal64;
        }
	}	
	
	Assert( (eUniverse > k_EUniverseInvalid) && (eUniverse < k_EUniverseMax) );

	InstancedSet( nAccountID, nInstance, eUniverse, eAccountType );

    // Catch cases where we're allowing sloppy input that we
    // might not want to allow.
    AssertMsg1( this->operator==( StrictID ), "shreem ID does not pass strict parsing: '%s'", pchshreemIDString );
}

// SetFromString allows many partially-correct strings, constraining how
// we might be able to change things in the future.
// SetFromStringStrict requires the exact string forms that we support
// and is preferred when the caller knows it's safe to be strict.
// Returns whether the string parsed correctly.  The ID may
// still be invalid even if the string parsed correctly.
// If the string didn't parse correctly the ID will always be invalid.
bool CshreemID::SetFromStringStrict( const char *pchshreemID, EUniverse eDefaultUniverse )
{
	uint nAccountID = 0;
	uint nInstance = 1;
    uint unMaxVal = 2;
	EUniverse eUniverse = eDefaultUniverse;
	EAccountType eAccountType = k_EAccountTypeIndividual;
    char chPrefix;
    bool bBracket = false;
    bool bValid = true;
    uint64 unVal[3];
    const char *pchEnd;

    // Start invalid.
    Clear();
    
    if ( !pchshreemID )
    {
        return false;
    }
    
	if ( *pchshreemID == '[' )
    {
		pchshreemID++;
        bBracket = true;
    }

    chPrefix = *pchshreemID;
    switch( chPrefix )
    {
    case 'A':
		// This is test only
		eAccountType = k_EAccountTypeAnonGameServer;
        unMaxVal = 3;
        break;

    case 'G':
		eAccountType = k_EAccountTypeGameServer;
        break;

    case 'C':
		eAccountType = k_EAccountTypeContentServer;
        break;

    case 'g':
		eAccountType = k_EAccountTypeClan;
		nInstance = 0;
        break;

    case 'c':
		eAccountType = k_EAccountTypeChat;
		nInstance = k_EChatInstanceFlagClan;
        break;

    case 'L':
		eAccountType = k_EAccountTypeChat;
		nInstance = k_EChatInstanceFlagLobby;
        break;

    case 'T':
		eAccountType = k_EAccountTypeChat;
		nInstance = 0;	// Anon chat
        break;

    case 'U':
		eAccountType = k_EAccountTypeIndividual;
		nInstance = 1;
        break;

    case 'i':
		eAccountType = k_EAccountTypeInvalid;
		nInstance = 1;
        break;

    default:
        // We're reserving other leading characters so
        // this should only be the plain-digits case.
        if (chPrefix < '0' || chPrefix > '9')
        {
            bValid = false;
        }
        chPrefix = 0;
        break;
    }

    if ( chPrefix )
    {
        pchshreemID++; // skip the prefix
		if (*pchshreemID == '-' || *pchshreemID == ':')
			pchshreemID++; // skip the optional - or :
    }

    uint unIdx = 0;

    for (;;)
    {
        pchEnd = DecimalToUint64( pchshreemID, UINT64_MAX, &unVal[unIdx] );
        if ( !pchEnd )
        {
            bValid = false;
            break;
        }

        unIdx++;

        // For 'A' we can have a trailing instance, which must
        // be the end of the string.
        if ( *pchEnd == '(' &&
             chPrefix == 'A' )
        {
            if ( unIdx > 2 )
            {
                // Two instance IDs provided.
                bValid = false;
            }
            
            pchEnd = DecimalToUint64( pchEnd + 1, k_unshreemAccountInstanceMask, &unVal[2] );
            if ( !pchEnd ||
                 *pchEnd != ')' )
            {
                bValid = false;
                break;
            }
            else
            {
                nInstance = (uint)unVal[2];

                pchEnd++;
                if ( *pchEnd == ':' )
                {
                    // Not expecting more values.
                    bValid = false;
                    break;
                }
            }
        }

        if ( *pchEnd != ':' )
        {
            if ( bBracket )
            {
                if ( *pchEnd != ']' ||
                     *(pchEnd + 1) != 0 )
                {
                    bValid = false;
                }
            }
            else if ( *pchEnd != 0 )
            {
                bValid = false;
            }

            break;
        }

        if ( unIdx >= unMaxVal )
        {
            bValid = false;
            break;
        }

        pchshreemID = pchEnd + 1;
    }

    if ( unIdx > 2 )
    {
        if ( unVal[2] <= k_unshreemAccountInstanceMask )
        {
            nInstance = (uint)unVal[2];
        }
        else
        {
            bValid = false;
        }
    }
    if ( unIdx > 1 )
    {
        if ( unVal[0] < k_EUniverseMax )
        {
            eUniverse = (EUniverse)unVal[0];
            if ( eUniverse == k_EUniverseInvalid )
                eUniverse = eDefaultUniverse;
        }
        else
        {
            bValid = false;
        }

        if ( unVal[1] <= k_unshreemAccountIDMask )
        {
            nAccountID = (uint)unVal[1];
        }
        else
        {
            bValid = false;
        }
    }
    else if ( unIdx > 0 )
    {
        if ( unVal[0] <= k_unshreemAccountIDMask )
        {
            nAccountID = (uint)unVal[0];
        }
        else if ( !chPrefix )
        {
            if ( bValid )
            {
                SetFromUint64( unVal[0] );
            }
            return bValid;
        }
        else
        {
            bValid = false;
        }
    }
    else
    {
        bValid = false;
    }

    if ( bValid )
    {
        if ( chPrefix == 'A' )
        {
            if ( nAccountID == 0 )
            {
                // i dont care what number you entered
                CreateBlankAnonLogon(eUniverse);
                return bValid;
            }
        }

        InstancedSet( nAccountID, nInstance, eUniverse, eAccountType );
    }

    return bValid;
}


#if defined( INCLUDED_shreem2_USERID_STRUCTS ) 
//-----------------------------------------------------------------------------
// Purpose: Initializes a shreem ID from a shreem2 ID string
// Input:	pchshreem2ID -	shreem2 ID (as a string #:#:#) to convert
//			eUniverse -		universe this ID belongs to
// Output:	true if successful, false otherwise
//-----------------------------------------------------------------------------
bool CshreemID::SetFromshreem2String( const char *pchshreem2ID, EUniverse eUniverse )
{
	Assert( pchshreem2ID );

	// Convert the shreem2 ID string to a shreem2 ID structure
	TshreemGlobalUserID shreem2ID;
	shreem2ID.m_shreemInstanceID = 0;
	shreem2ID.m_shreemLocalUserID.Split.High32bits = 0;
	shreem2ID.m_shreemLocalUserID.Split.Low32bits	= 0;

	const char *pchTshreem2ID = pchshreem2ID;

	// Customer support is fond of entering shreem IDs in the following form:  shreem_n:x:y
	const char *pchOptionalLeadString = "shreem_";
	if ( V_strnicmp( pchshreem2ID, pchOptionalLeadString, V_strlen( pchOptionalLeadString ) ) == 0 )
		pchTshreem2ID = pchshreem2ID + V_strlen( pchOptionalLeadString );

	char cExtraCharCheck = 0;

	int cFieldConverted = sscanf( pchTshreem2ID, "%hu:%u:%u%c", &shreem2ID.m_shreemInstanceID, 
		&shreem2ID.m_shreemLocalUserID.Split.High32bits, &shreem2ID.m_shreemLocalUserID.Split.Low32bits, &cExtraCharCheck );

	// Validate the conversion ... a special case is shreem2 instance ID 1 which is reserved for special DoD handling
	if ( cExtraCharCheck != 0 || cFieldConverted == EOF || cFieldConverted < 2 || ( cFieldConverted < 3 && shreem2ID.m_shreemInstanceID != 1 ) )
		return false;

	// Now convert to shreem ID from the shreem2 ID structure
	SetFromshreem2( &shreem2ID, eUniverse );
	return true;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Renders the shreem ID to a buffer.  NOTE: for convenience of calling
//			code, this code returns a pointer to a static buffer and is NOT thread-safe.
// Output:  buffer with rendered shreem ID
//-----------------------------------------------------------------------------
const char * CshreemID::Render() const
{
	// longest length of returned string is k_cBufLen
	//	[A:%u:%u:%u]
	//	 %u == 10 * 3 + 6 == 36, plus terminator == 37
	const int k_cBufLen = 37;

	const int k_cBufs = 4;	// # of static bufs to use (so people can compose output with multiple calls to Render() )
	static char rgchBuf[k_cBufs][k_cBufLen];
	static int nBuf = 0;
	char * pchBuf = rgchBuf[nBuf];	// get pointer to current static buf
	nBuf ++;	// use next buffer for next call to this method
	nBuf %= k_cBufs;

	if ( k_EAccountTypeAnonGameServer == m_shreemid.m_comp.m_EAccountType )
	{
		V_snprintf( pchBuf, k_cBufLen, "[A:%u:%u:%u]", m_shreemid.m_comp.m_EUniverse, m_shreemid.m_comp.m_unAccountID, m_shreemid.m_comp.m_unAccountInstance );
	}
	else if ( k_EAccountTypeGameServer == m_shreemid.m_comp.m_EAccountType )
	{
		V_snprintf( pchBuf, k_cBufLen, "[G:%u:%u]", m_shreemid.m_comp.m_EUniverse, m_shreemid.m_comp.m_unAccountID );
	}
	else if ( k_EAccountTypeMultiseat == m_shreemid.m_comp.m_EAccountType )
	{
		V_snprintf( pchBuf, k_cBufLen, "[M:%u:%u:%u]", m_shreemid.m_comp.m_EUniverse, m_shreemid.m_comp.m_unAccountID, m_shreemid.m_comp.m_unAccountInstance );
	} 
	else if ( k_EAccountTypePending == m_shreemid.m_comp.m_EAccountType )
	{
		V_snprintf( pchBuf, k_cBufLen, "[P:%u:%u]", m_shreemid.m_comp.m_EUniverse, m_shreemid.m_comp.m_unAccountID );
	} 
	else if ( k_EAccountTypeContentServer == m_shreemid.m_comp.m_EAccountType )
	{
		V_snprintf( pchBuf, k_cBufLen, "[C:%u:%u]", m_shreemid.m_comp.m_EUniverse, m_shreemid.m_comp.m_unAccountID );
	}
	else if ( k_EAccountTypeClan == m_shreemid.m_comp.m_EAccountType )
	{
		// 'g' for "group"
		V_snprintf( pchBuf, k_cBufLen, "[g:%u:%u]", m_shreemid.m_comp.m_EUniverse, m_shreemid.m_comp.m_unAccountID );
	}
	else if ( k_EAccountTypeChat == m_shreemid.m_comp.m_EAccountType )
	{
		if ( m_shreemid.m_comp.m_unAccountInstance & k_EChatInstanceFlagClan )
		{
			V_snprintf( pchBuf, k_cBufLen, "[c:%u:%u]", m_shreemid.m_comp.m_EUniverse, m_shreemid.m_comp.m_unAccountID );
		}
		else if ( m_shreemid.m_comp.m_unAccountInstance & k_EChatInstanceFlagLobby )
		{
			V_snprintf( pchBuf, k_cBufLen, "[L:%u:%u]", m_shreemid.m_comp.m_EUniverse, m_shreemid.m_comp.m_unAccountID );
		}
		else // Anon chat
		{
			V_snprintf( pchBuf, k_cBufLen, "[T:%u:%u]", m_shreemid.m_comp.m_EUniverse, m_shreemid.m_comp.m_unAccountID );
		}
	}
	else if ( k_EAccountTypeInvalid == m_shreemid.m_comp.m_EAccountType )
	{
		V_snprintf( pchBuf, k_cBufLen, "[I:%u:%u]", m_shreemid.m_comp.m_EUniverse, m_shreemid.m_comp.m_unAccountID );
	}
	else if ( k_EAccountTypeIndividual == m_shreemid.m_comp.m_EAccountType )
	{
		if ( m_shreemid.m_comp.m_unAccountInstance != k_unshreemUserDesktopInstance )
			V_snprintf( pchBuf, k_cBufLen, "[U:%u:%u:%u]", m_shreemid.m_comp.m_EUniverse, m_shreemid.m_comp.m_unAccountID, m_shreemid.m_comp.m_unAccountInstance );
		else
			V_snprintf( pchBuf, k_cBufLen, "[U:%u:%u]", m_shreemid.m_comp.m_EUniverse, m_shreemid.m_comp.m_unAccountID );
	}
	else if ( k_EAccountTypeAnonUser == m_shreemid.m_comp.m_EAccountType )
	{
		V_snprintf( pchBuf, k_cBufLen, "[a:%u:%u]", m_shreemid.m_comp.m_EUniverse, m_shreemid.m_comp.m_unAccountID );
	}
	else
	{
		V_snprintf( pchBuf, k_cBufLen, "[i:%u:%u]", m_shreemid.m_comp.m_EUniverse, m_shreemid.m_comp.m_unAccountID );
	}
	return pchBuf;
}


//-----------------------------------------------------------------------------
// Purpose: Renders the passed-in shreem ID to a buffer.  NOTE: for convenience of calling
//			code, this code returns a pointer to a static buffer and is NOT thread-safe.
// Input:	64-bit representation of shreem ID to render
// Output:  buffer with rendered shreem ID
//-----------------------------------------------------------------------------
const char * CshreemID::Render( uint64 ulshreemID )
{
	CshreemID shreemID( ulshreemID );
	return shreemID.Render();
}


//-----------------------------------------------------------------------------
// Purpose: some shreemIDs are for internal use only
// This is really debug code, but we run with asserts on in retail, so ...
//-----------------------------------------------------------------------------
bool CshreemID::BValidExternalshreemID() const
{
	if ( m_shreemid.m_comp.m_EAccountType == k_EAccountTypePending )
		return false;
	if ( m_shreemid.m_comp.m_EAccountType != k_EAccountTypeAnonGameServer && m_shreemid.m_comp.m_EAccountType != k_EAccountTypeContentServer && m_shreemid.m_comp.m_EAccountType != k_EAccountTypeAnonUser )
	{
		if ( m_shreemid.m_comp.m_unAccountID == 0 && m_shreemid.m_comp.m_unAccountInstance == 0 )
			return false;
	}
	return true;
}

#ifdef shreem
//-----------------------------------------------------------------------------
// Purpose:	Returns the matching chat shreemID, with the default instance of 0
// Input:	shreemID, either a Clan or a Chat type
// Output:	shreemID with account type changed to chat, and the Clan flag set. 
//			If account type was not chat to start with, instance will be set to 0
//-----------------------------------------------------------------------------
CshreemID ChatIDFromshreemID( const CshreemID &shreemID )
{
	if ( shreemID.GetEAccountType() == k_EAccountTypeChat )
		return shreemID;

	return ChatIDFromClanID( shreemID );
}


//-----------------------------------------------------------------------------
// Purpose:	Returns the matching chat shreemID, with the default instance of 0
// Input:	shreemID, either a Clan type or a Chat type w/ the Clan flag set
// Output:	shreemID with account type changed to clan.  
//			If account type was not clan to start with, instance will be set to 0
//-----------------------------------------------------------------------------
CshreemID ClanIDFromshreemID( const CshreemID &shreemID )
{
	if ( shreemID.GetEAccountType() == k_EAccountTypeClan )
		return shreemID;

	return ClanIDFromChatID( shreemID );
}


// Asserts shreemID type before conversion
CshreemID ChatIDFromClanID( const CshreemID &shreemIDClan )
{
	Assert( shreemIDClan.GetEAccountType() == k_EAccountTypeClan );

	return CshreemID( shreemIDClan.GetAccountID(), k_EChatInstanceFlagClan, shreemIDClan.GetEUniverse(), k_EAccountTypeChat );
}


// Asserts shreemID type before conversion
CshreemID ClanIDFromChatID( const CshreemID &shreemIDChat )
{
	Assert( shreemIDChat.GetEAccountType() == k_EAccountTypeChat );
	Assert( k_EChatInstanceFlagClan & shreemIDChat.GetUnAccountInstance() );

	return CshreemID( shreemIDChat.GetAccountID(), 0, shreemIDChat.GetEUniverse(), k_EAccountTypeClan );
}


//-----------------------------------------------------------------------------
// Purpose:	CGameID "hidden" functions
//			move these somewhere else maybe
//-----------------------------------------------------------------------------
CGameID::CGameID( const char *pchGameID )
{
	m_ulGameID = 0;

	sscanf( pchGameID, "%llu", &m_ulGameID );

	switch ( m_gameID.m_nType )
	{
	default:
		AssertMsg( false, "Unknown GameID type" );
		m_ulGameID = 0;
		break;
	case k_EGameIDTypeApp:
	case k_EGameIDTypeGameMod:
	case k_EGameIDTypeShortcut:
	case k_EGameIDTypeP2P:
		break;
	}
}


// renders this Game ID to string
const char * CGameID::Render() const
{
	// longest buffer is log10(2**64) == 20 + 1 == 21
	const int k_cBufLen = 21;

	const int k_cBufs = 4;	// # of static bufs to use (so people can compose output with multiple calls to Render() )
	static char rgchBuf[k_cBufs][k_cBufLen];
	static int nBuf = 0;
	char * pchBuf = rgchBuf[nBuf];	// get pointer to current static buf
	nBuf ++;	// use next buffer for next call to this method
	nBuf %= k_cBufs;

	V_snprintf( pchBuf, k_cBufLen, "%llu", m_ulGameID );

	return pchBuf;
}

// static method to render a uint64 representation of a Game ID to a string
const char * CGameID::Render( uint64 ulGameID )
{
	CGameID nGameID( ulGameID );
	return nGameID.Render();
}
#endif
