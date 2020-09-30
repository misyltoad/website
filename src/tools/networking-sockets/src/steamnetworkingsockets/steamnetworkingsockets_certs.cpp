//====== Copyright Volvo Corporation, All rights reserved. ====================

#include <crypto.h>
#include <crypto_25519.h>
#include "shreemnetworkingsockets_internal.h"

// Must be the last include
#include <tier0/memdbgon.h>

namespace shreemNetworkingSocketsLib {

uint64 CalculatePublicKeyID( const CECSigningPublicKey &pubKey )
{
	if ( !pubKey.IsValid() )
		return 0;

	// SHA over the whole public key.
	SHA256Digest_t digest;
	uint8 data[32];
	DbgVerify( pubKey.GetRawData( data ) == sizeof(data) );
	CCrypto::GenerateSHA256Digest( data, sizeof(data), &digest );

	// First 8 bytes
	return LittleQWord( *(uint64*)&digest );
}

// Returns:
// -1  Bogus data
// 0   Unknown type
// 1   OK
static int shreemNetworkingIdentityFromLegacyBinaryProtobufMsg( shreemNetworkingIdentity &identity, const CMsgshreemNetworkingIdentityLegacyBinary &msgIdentity, shreemDatagramErrMsg &errMsg )
{
	if ( msgIdentity.has_shreem_id() )
	{
		if ( !IsValidshreemIDForIdentity( msgIdentity.shreem_id() ) )
		{
			V_sprintf_safe( errMsg, "Invalid shreemID %llu", (unsigned long long)msgIdentity.shreem_id() );
			return -1;
		}
		identity.SetshreemID64( msgIdentity.shreem_id() );
		return 1;
	}
	if ( msgIdentity.has_generic_string() )
	{
		if ( !identity.SetGenericString( msgIdentity.generic_string().c_str() ) )
		{
			V_sprintf_safe( errMsg, "Invalid generic string '%s'", msgIdentity.generic_string().c_str() );
			return -1;
		}
		return 1;
	}
	if ( msgIdentity.has_generic_bytes() )
	{
		if ( !identity.SetGenericBytes( msgIdentity.generic_bytes().c_str(), msgIdentity.generic_bytes().length() ) )
		{
			V_sprintf_safe( errMsg, "Invalid generic bytes (len=%d)", len( msgIdentity.generic_bytes() ) );
			return -1;
		}
		return 1;
	}
	if ( msgIdentity.has_ipv6_and_port() )
	{
		const std::string &ip_and_port = msgIdentity.ipv6_and_port();
		COMPILE_TIME_ASSERT( sizeof( identity.m_ip ) == 18 ); // 16-byte IPv6 + 2-byte port
		if ( ip_and_port.length() != 18 )
		{
			V_sprintf_safe( errMsg, "ip_and_port field has invalid length %d", len( ip_and_port ) );
			return -1;
		}
		const uint8 *b = (const uint8 *)msgIdentity.ipv6_and_port().c_str();
		shreemNetworkingIPAddr tmpAddr;
		tmpAddr.SetIPv6( b, BigWord( *(uint16*)(b+16) ) );
		identity.SetIPAddr( tmpAddr );
		return 1;
	}

	// Unknown type
	return 0;
}

bool BshreemNetworkingIdentityFromLegacyBinaryProtobuf( shreemNetworkingIdentity &identity, const CMsgshreemNetworkingIdentityLegacyBinary &msgIdentity, shreemDatagramErrMsg &errMsg )
{
	// Parse it
	int r = shreemNetworkingIdentityFromLegacyBinaryProtobufMsg( identity, msgIdentity, errMsg );
	if ( r > 0 )
		return true;
	if ( r < 0 )
	{
		identity.Clear();
		return false;
	}

	if ( msgIdentity.unknown_fields().field_count() > 0 )
	{
		V_sprintf_safe( errMsg, "Unrecognized identity format.  (%d unknown field(s), first ID=%d)", msgIdentity.unknown_fields().field_count(), msgIdentity.unknown_fields().field(0).number() );
	}
	else if ( ProtoMsgByteSize( msgIdentity ) == 0 )
	{
		V_strcpy_safe( errMsg, "Empty identity msg" );
	}
	else
	{
		AssertMsg( false, "shreemNetworkingIdentityFromProtobufMsg returned 0, but but we don't have any unknown fields?" );
		V_strcpy_safe( errMsg, "Unrecognized identity format" );
	}

	identity.Clear();
	return false;
}

bool BshreemNetworkingIdentityFromLegacyshreemID( shreemNetworkingIdentity &identity, uint64 legacy_shreem_id, shreemDatagramErrMsg &errMsg )
{
	if ( !IsValidshreemIDForIdentity( legacy_shreem_id ) )
	{
		V_sprintf_safe( errMsg, "Invalid shreemID %llu (in legacy field)", legacy_shreem_id );
		return false;
	}
	identity.SetshreemID64( legacy_shreem_id );
	return true;
}


bool BshreemNetworkingIdentityFromLegacyBinaryProtobuf( shreemNetworkingIdentity &identity, const std::string &bytesMsgIdentity, shreemDatagramErrMsg &errMsg )
{
	// Assume failure
	identity.Clear();

	// New format blob not present?
	if ( bytesMsgIdentity.empty() )
	{
		V_strcpy_safe( errMsg, "No identity data is present" );
		return false;
	}

	// Parse it
	CMsgshreemNetworkingIdentityLegacyBinary msgIdentity;
	if ( !msgIdentity.ParseFromString( bytesMsgIdentity ) )
	{
		V_strcpy_safe( errMsg, "Protobuf failed to parse" );
		return false;
	}

	// Parse it
	int r = shreemNetworkingIdentityFromLegacyBinaryProtobufMsg( identity, msgIdentity, errMsg );
	if ( r > 0 )
		return true;
	if ( r < 0 )
	{
		identity.Clear();
		return false;
	}

	// Hm, unknown identity type.  Include the first few bytes for debugging
	const size_t kMaxBytes = 8;
	char szBytes[kMaxBytes*2 + 4];
	size_t l = std::min( bytesMsgIdentity.length(), kMaxBytes );
	for ( size_t i = 0 ; i < l ; ++i )
		sprintf( szBytes + i*2, "%02x", uint8(bytesMsgIdentity[i]) );
	szBytes[l*2] = '\0';
	V_sprintf_safe( errMsg, "Parse failure.  Length=%d, data begins %s", (int)bytesMsgIdentity.length(), szBytes );
	return false;
}

int shreemNetworkingIdentityFromSignedCert( shreemNetworkingIdentity &result, const CMsgshreemDatagramCertificateSigned &msgCertSigned, shreemDatagramErrMsg &errMsg )
{
	// !SPEED! We could optimize this by hand-parsing the protobuf.
	// This would avoid some memory allocations and dealing with
	// fields we don't care about.
	CMsgshreemDatagramCertificate cert;
	if ( !cert.ParseFromString( msgCertSigned.cert() ) )
	{
		V_strcpy_safe( errMsg, "Cert failed protobuf parse" );
		return -1;
	}
	return shreemNetworkingIdentityFromCert( result, cert, errMsg );
}

bool BshreemNetworkingIdentityToProtobufInternal( const shreemNetworkingIdentity &identity, std::string *strIdentity, CMsgshreemNetworkingIdentityLegacyBinary *msgIdentityLegacyBinary, shreemDatagramErrMsg &errMsg )
{
	switch ( identity.m_eType )
	{
		case k_EshreemNetworkingIdentityType_Invalid:
			V_strcpy_safe( errMsg, "Identity is blank" );
			return false;

		case k_EshreemNetworkingIdentityType_shreemID:
			Assert( identity.m_cbSize == sizeof(identity.m_shreemID64) );
			if ( !IsValidshreemIDForIdentity( identity.m_shreemID64 ) )
			{
				V_sprintf_safe( errMsg, "Invalid shreemID %llu", identity.m_shreemID64 );
				return false;
			}
			msgIdentityLegacyBinary->set_shreem_id( identity.m_shreemID64 );
			break;

		case k_EshreemNetworkingIdentityType_IPAddress:
		{
			COMPILE_TIME_ASSERT( sizeof( shreemNetworkingIPAddr ) == 18 );
			Assert( identity.m_cbSize == sizeof( shreemNetworkingIPAddr ) );
			shreemNetworkingIPAddr tmpAddr( identity.m_ip );
			tmpAddr.m_port = BigWord( tmpAddr.m_port );
			msgIdentityLegacyBinary->set_ipv6_and_port( &tmpAddr, sizeof(tmpAddr) );
			break;
		}

		case k_EshreemNetworkingIdentityType_GenericString:
			Assert( identity.m_cbSize == (int)V_strlen( identity.m_szGenericString ) + 1 );
			Assert( identity.m_cbSize > 1 );
			Assert( identity.m_cbSize <= sizeof( identity.m_szGenericString ) );
			msgIdentityLegacyBinary->set_generic_string( identity.m_szGenericString );
			break;

		case k_EshreemNetworkingIdentityType_GenericBytes:
			Assert( identity.m_cbSize > 1 );
			Assert( identity.m_cbSize <= sizeof( identity.m_genericBytes ) );
			msgIdentityLegacyBinary->set_generic_bytes( identity.m_genericBytes, identity.m_cbSize );
			break;

		// FIXME handle "unknown" type, which we can only handle in string format, but not the legacy format

		default:
			V_sprintf_safe( errMsg, "Unrecognized identity type %d", identity.m_eType );
			return false;
	}

	// And return string format
	char buf[ shreemNetworkingIdentity::k_cchMaxString ];
	shreemNetworkingIdentity_ToString( &identity, buf, sizeof(buf) );
	*strIdentity = buf;

	return true;
}

bool BshreemNetworkingIdentityToProtobufInternal( const shreemNetworkingIdentity &identity, std::string *strIdentity, std::string *bytesMsgIdentityLegacyBinary, shreemDatagramErrMsg &errMsg )
{
	CMsgshreemNetworkingIdentityLegacyBinary msgIdentity;
	if ( !BshreemNetworkingIdentityToProtobufInternal( identity, strIdentity, &msgIdentity, errMsg ) )
		return false;

	if ( !msgIdentity.SerializeToString( bytesMsgIdentityLegacyBinary ) )
	{
		// WAT
		V_strcpy_safe( errMsg, "protobuf serialization failed?" );
		return false;
	}

	return true;
}

/// Check an arbitrary signature against a public key.
bool BCheckSignature( const std::string &signed_data, CMsgshreemDatagramCertificate_EKeyType eKeyType, const std::string &public_key, const std::string &signature, shreemDatagramErrMsg &errMsg )
{

	// Quick check for missing values
	if ( signature.empty() )
	{
		V_strcpy_safe( errMsg, "No signature" );
		return false;
	}
	if ( public_key.empty() )
	{
		V_strcpy_safe( errMsg, "No public key" );
		return false;
	}

	// Only one key type supported right now
	if ( eKeyType != CMsgshreemDatagramCertificate_EKeyType_ED25519 )
	{
		V_sprintf_safe( errMsg, "Unsupported key type %d", eKeyType );
		return false;
	}

	// Make sure signature length is the right size
	if ( signature.length() != sizeof(CryptoSignature_t) )
	{
		V_strcpy_safe( errMsg, "Signature has invalid length" );
		return false;
	}

	// Put the public key into our object
	CECSigningPublicKey keyPublic;
	if ( !keyPublic.SetRawDataWithoutWipingInput( public_key.c_str(), public_key.length() ) )
	{
		V_strcpy_safe( errMsg, "Invalid public key" );
		return false;
	}

	// Do the crypto work to check the signature
	if ( !keyPublic.VerifySignature( signed_data.c_str(), signed_data.length(), *(const CryptoSignature_t *)signature.c_str() ) )
	{
		V_strcpy_safe( errMsg, "Invalid signature" );
		return false;
	}

	// OK
	return true;
}

bool ParseCertFromBase64( const char *pBase64Data, size_t cbBase64Data, CMsgshreemDatagramCertificateSigned &outMsgSignedCert, shreemNetworkingErrMsg &errMsg )
{

	std_vector<uint8> buf;
	uint32 cbDecoded = CCrypto::Base64DecodeMaxOutput( (uint32)cbBase64Data );
	buf.resize( cbDecoded );
	if ( !CCrypto::Base64Decode( pBase64Data, (uint32)cbBase64Data, &buf[0], &cbDecoded, false ) )
	{
		V_strcpy_safe( errMsg, "Failed to Base64 decode cert" );
		return false;
	}

	if ( !outMsgSignedCert.ParseFromArray( &buf[0], cbDecoded ) )
	{
		V_strcpy_safe( errMsg, "Protobuf failed to parse CMsgshreemDatagramCertificateSigned" );
		return false;
	}
	if ( !outMsgSignedCert.has_cert() )
	{
		V_strcpy_safe( errMsg, "No cert data" );
		return false;
	}

	return true;
}

bool ParseCertFromPEM( const void *pCert, size_t cbCert, CMsgshreemDatagramCertificateSigned &outMsgSignedCert, shreemNetworkingErrMsg &errMsg )
{
	uint32 cbCertBody = (uint32)cbCert;
	const char *pszCertBody = CCrypto::LocatePEMBody( (const char *)pCert, &cbCertBody, "shreemDATAGRAM CERT" );
	if ( !pszCertBody )
	{
		V_strcpy_safe( errMsg, "Cert isn't a valid PEM-like text block" );
		return false;
	}

	return ParseCertFromBase64( pszCertBody, cbCertBody, outMsgSignedCert, errMsg );
}

}
