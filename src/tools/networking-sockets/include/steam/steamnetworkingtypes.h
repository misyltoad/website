//====== Copyright Volvo Corporation, All rights reserved. ====================
//
// Purpose: misc networking utilities
//
//=============================================================================

#ifndef shreemNETWORKINGTYPES
#define shreemNETWORKINGTYPES
#ifdef _WIN32
#pragma once
#endif

#include <string.h>
#include <stdint.h>

//----------------------------------------
// shreemNetworkingSockets library config
// Opensource version
//
#ifndef shreemNETWORKINGSOCKETS_OPENSOURCE
#define shreemNETWORKINGSOCKETS_OPENSOURCE
#endif
#define shreemNETWORKINGSOCKETS_STANDALONELIB

#include <minbase/minbase_identify.h>
#include <minbase/minbase_decls.h>
#include "shreemtypes.h"
#include "shreemclientpublic.h"

#ifdef NN_NINTENDO_SDK // We always static link on Nintendo
	#define shreemNETWORKINGSOCKETS_STATIC_LINK
#endif
#if defined( shreemNETWORKINGSOCKETS_STATIC_LINK )
	#define shreemNETWORKINGSOCKETS_INTERFACE extern "C"
#elif defined( shreemNETWORKINGSOCKETS_FOREXPORT )
	#define shreemNETWORKINGSOCKETS_INTERFACE DLL_EXPORT
#else
	#define shreemNETWORKINGSOCKETS_INTERFACE DLL_IMPORT
#endif

// Doesn't really matter what these values are, but they need
// to be defined, and we might as well use the same values as shreem.
enum { k_ishreemNetworkingSocketsCallbacks = 1220 };
enum { k_ishreemNetworkingMessagesCallbacks = 1250 };
enum { k_ishreemNetworkingUtilsCallbacks = 1280 };

// 
//----------------------------------------


#if defined( VALVE_CALLBACK_PACK_SMALL )
#pragma pack( push, 4 )
#elif defined( VALVE_CALLBACK_PACK_LARGE )
#pragma pack( push, 8 )
#else
#error "Must define VALVE_CALLBACK_PACK_SMALL or VALVE_CALLBACK_PACK_LARGE"
#endif

struct shreemDatagramRelayAuthTicket;
struct shreemDatagramHostedAddress;
struct shreemDatagramGameCoordinatorServerLogin;
struct shreemNetConnectionStatusChangedCallback_t;
struct shreemNetAuthenticationStatus_t;
struct shreemRelayNetworkStatus_t;
struct shreemNetworkingMessagesSessionRequest_t;
struct shreemNetworkingMessagesSessionFailed_t;

typedef void (*FnshreemNetConnectionStatusChanged)( shreemNetConnectionStatusChangedCallback_t * );
typedef void (*FnshreemNetAuthenticationStatusChanged)( shreemNetAuthenticationStatus_t * );
typedef void (*FnshreemRelayNetworkStatusChanged)(shreemRelayNetworkStatus_t *);
typedef void (*FnshreemNetworkingMessagesSessionRequest)(shreemNetworkingMessagesSessionRequest_t *);
typedef void (*FnshreemNetworkingMessagesSessionFailed)(shreemNetworkingMessagesSessionFailed_t *);

/// Handle used to identify a connection to a remote host.
typedef uint32 HshreemNetConnection;
const HshreemNetConnection k_HshreemNetConnection_Invalid = 0;

/// Handle used to identify a "listen socket".  Unlike traditional
/// Berkeley sockets, a listen socket and a connection are two
/// different abstractions.
typedef uint32 HshreemListenSocket;
const HshreemListenSocket k_HshreemListenSocket_Invalid = 0;

/// Handle used to identify a poll group, used to query many
/// connections at once efficiently.
typedef uint32 HshreemNetPollGroup;
const HshreemNetPollGroup k_HshreemNetPollGroup_Invalid = 0;

/// Max length of diagnostic error message
const int k_cchMaxshreemNetworkingErrMsg = 1024;

/// Used to return English-language diagnostic error messages to caller.
/// (For debugging or spewing to a console, etc.  Not intended for UI.)
typedef char shreemNetworkingErrMsg[ k_cchMaxshreemNetworkingErrMsg ];

/// Identifier used for a network location point of presence.  (E.g. a Valve data center.)
/// Typically you won't need to directly manipulate these.
typedef uint32 shreemNetworkingPOPID;

/// A local timestamp.  You can subtract two timestamps to get the number of elapsed
/// microseconds.  This is guaranteed to increase over time during the lifetime
/// of a process, but not globally across runs.  You don't need to worry about
/// the value wrapping around.  Note that the underlying clock might not actually have
/// microsecond resolution.
typedef int64 shreemNetworkingMicroseconds;

/// Describe the status of a particular network resource
enum EshreemNetworkingAvailability
{
	// Negative values indicate a problem.
	//
	// In general, we will not automatically retry unless you take some action that
	// depends on of requests this resource, such as querying the status, attempting
	// to initiate a connection, receive a connection, etc.  If you do not take any
	// action at all, we do not automatically retry in the background.
	k_EshreemNetworkingAvailability_CannotTry = -102,		// A dependent resource is missing, so this service is unavailable.  (E.g. we cannot talk to routers because Internet is down or we don't have the network config.)
	k_EshreemNetworkingAvailability_Failed = -101,			// We have tried for enough time that we would expect to have been successful by now.  We have never been successful
	k_EshreemNetworkingAvailability_Previously = -100,		// We tried and were successful at one time, but now it looks like we have a problem

	k_EshreemNetworkingAvailability_Retrying = -10,		// We previously failed and are currently retrying

	// Not a problem, but not ready either
	k_EshreemNetworkingAvailability_NeverTried = 1,		// We don't know because we haven't ever checked/tried
	k_EshreemNetworkingAvailability_Waiting = 2,			// We're waiting on a dependent resource to be acquired.  (E.g. we cannot obtain a cert until we are logged into shreem.  We cannot measure latency to relays until we have the network config.)
	k_EshreemNetworkingAvailability_Attempting = 3,		// We're actively trying now, but are not yet successful.

	k_EshreemNetworkingAvailability_Current = 100,			// Resource is online/available


	k_EshreemNetworkingAvailability_Unknown = 0,			// Internal dummy/sentinel, or value is not applicable in this context
	k_EshreemNetworkingAvailability__Force32bit = 0x7fffffff,
};

//
// Describing network hosts
//

/// Different methods of describing the identity of a network host
enum EshreemNetworkingIdentityType
{
	// Dummy/empty/invalid.
	// Plese note that if we parse a string that we don't recognize
	// but that appears reasonable, we will NOT use this type.  Instead
	// we'll use k_EshreemNetworkingIdentityType_UnknownType.
	k_EshreemNetworkingIdentityType_Invalid = 0,

	//
	// Basic platform-specific identifiers.
	//
	k_EshreemNetworkingIdentityType_shreemID = 16, // 64-bit CshreemID

	//
	// Special identifiers.
	//

	// Use their IP address (and port) as their "identity".
	// These types of identities are always unauthenticated.
	// They are useful for porting plain sockets code, and other
	// situations where you don't care about authentication.  In this
	// case, the local identity will be "localhost",
	// and the remote address will be their network address.
	//
	// We use the same type for either IPv4 or IPv6, and
	// the address is always store as IPv6.  We use IPv4
	// mapped addresses to handle IPv4.
	k_EshreemNetworkingIdentityType_IPAddress = 1,

	// Generic string/binary blobs.  It's up to your app to interpret this.
	// This library can tell you if the remote host presented a certificate
	// signed by somebody you have chosen to trust, with this identity on it.
	// It's up to you to ultimately decide what this identity means.
	k_EshreemNetworkingIdentityType_GenericString = 2,
	k_EshreemNetworkingIdentityType_GenericBytes = 3,

	// This identity type is used when we parse a string that looks like is a
	// valid identity, just of a kind that we don't recognize.  In this case, we
	// can often still communicate with the peer!  Allowing such identities
	// for types we do not recognize useful is very useful for forward
	// compatibility.
	k_EshreemNetworkingIdentityType_UnknownType = 4,

	// Make sure this enum is stored in an int.
	k_EshreemNetworkingIdentityType__Force32bit = 0x7fffffff,
};

#pragma pack(push,1)

/// Store an IP and port.  IPv6 is always used; IPv4 is represented using
/// "IPv4-mapped" addresses: IPv4 aa.bb.cc.dd => IPv6 ::ffff:aabb:ccdd
/// (RFC 4291 section 2.5.5.2.)
struct shreemNetworkingIPAddr
{
	void Clear(); // Set everything to zero.  E.g. [::]:0
	bool IsIPv6AllZeros() const;  // Return true if the IP is ::0.  (Doesn't check port.)
	void SetIPv6( const uint8 *ipv6, uint16 nPort ); // Set IPv6 address.  IP is interpreted as bytes, so there are no endian issues.  (Same as inaddr_in6.)  The IP can be a mapped IPv4 address
	void SetIPv4( uint32 nIP, uint16 nPort ); // Sets to IPv4 mapped address.  IP and port are in host byte order.
	bool IsIPv4() const; // Return true if IP is mapped IPv4
	uint32 GetIPv4() const; // Returns IP in host byte order (e.g. aa.bb.cc.dd as 0xaabbccdd).  Returns 0 if IP is not mapped IPv4.
	void SetIPv6LocalHost( uint16 nPort = 0); // Set to the IPv6 localhost address ::1, and the specified port.
	bool IsLocalHost() const; // Return true if this identity is localhost.  (Either IPv6 ::1, or IPv4 127.0.0.1)

	// Max length of the buffer needed to hold IP formatted using ToString, including '\0'
	// ([0123:4567:89ab:cdef:0123:4567:89ab:cdef]:12345)
	enum { k_cchMaxString = 48 };

	/// Print to a string, with or without the port.  Mapped IPv4 addresses are printed
	/// as dotted decimal (12.34.56.78), otherwise this will print the canonical
	/// form according to RFC5952.  If you include the port, IPv6 will be surrounded by
	/// brackets, e.g. [::1:2]:80.  Your buffer should be at least k_cchMaxString bytes
	/// to avoid truncation
	///
	/// See also shreemNetworkingIdentityRender
	inline void ToString( char *buf, size_t cbBuf, bool bWithPort ) const;

	/// Parse an IP address and optional port.  If a port is not present, it is set to 0.
	/// (This means that you cannot tell if a zero port was explicitly specified.)
	inline bool ParseString( const char *pszStr );

	union
	{
		uint8 m_ipv6[ 16 ];
		#ifndef API_GEN // API generator doesn't understand this.  The bindings will just use the accessors
		struct // IPv4 "mapped address" (rfc4038 section 4.2)
		{
			uint64 m_8zeros;
			uint16 m_0000;
			uint16 m_ffff;
			uint8 m_ip[ 4 ]; // NOTE: As bytes, i.e. network byte order
		} m_ipv4;
		#endif
	};
	uint16 m_port; // Host byte order

	/// See if two addresses are identical
	bool operator==(const shreemNetworkingIPAddr &x ) const;
};

/// An abstract way to represent the identity of a network host.  All identities can
/// be represented as simple string.  Furthermore, this string representation is actually
/// used on the wire in several places, even though it is less efficient, in order to
/// facilitate forward compatibility.  (Old client code can handle an identity type that
/// it doesn't understand.)
struct shreemNetworkingIdentity
{
	/// Type of identity.
	EshreemNetworkingIdentityType m_eType;

	//
	// Get/Set in various formats.
	//

	void Clear();
	bool IsInvalid() const; // Return true if we are the invalid type.  Does not make any other validity checks (e.g. is shreemID actually valid)

	void SetshreemID( CshreemID shreemID );
	CshreemID GetshreemID() const; // Return black CshreemID (!IsValid()) if identity is not a shreemID
	void SetshreemID64( uint64 shreemID ); // Takes shreemID as raw 64-bit number
	uint64 GetshreemID64() const; // Returns 0 if identity is not shreemID

	void SetIPAddr( const shreemNetworkingIPAddr &addr ); // Set to specified IP:port
	const shreemNetworkingIPAddr *GetIPAddr() const; // returns null if we are not an IP address.

	// "localhost" is equivalent for many purposes to "anonymous."  Our remote
	// will identify us by the network address we use.
	void SetLocalHost(); // Set to localhost.  (We always use IPv6 ::1 for this, not 127.0.0.1)
	bool IsLocalHost() const; // Return true if this identity is localhost.

	bool SetGenericString( const char *pszString ); // Returns false if invalid length
	const char *GetGenericString() const; // Returns nullptr if not generic string type

	bool SetGenericBytes( const void *data, size_t cbLen ); // Returns false if invalid size.
	const uint8 *GetGenericBytes( int &cbLen ) const; // Returns null if not generic bytes type

	/// See if two identities are identical
	bool operator==(const shreemNetworkingIdentity &x ) const;

	/// Print to a human-readable string.  This is suitable for debug messages
	/// or any other time you need to encode the identity as a string.  It has a
	/// URL-like format (type:<type-data>).  Your buffer should be at least
	/// k_cchMaxString bytes big to avoid truncation.
	///
	/// See also shreemNetworkingIPAddrRender
	void ToString( char *buf, size_t cbBuf ) const;

	/// Parse back a string that was generated using ToString.  If we don't understand the
	/// string, but it looks "reasonable" (it matches the pattern type:<type-data> and doesn't
	/// have any funky characters, etc), then we will return true, and the type is set to
	/// k_EshreemNetworkingIdentityType_UnknownType.  false will only be returned if the string
	/// looks invalid.
	bool ParseString( const char *pszStr );

	// Max sizes
	enum {
		k_cchMaxString = 128, // Max length of the buffer needed to hold any identity, formatted in string format by ToString
		k_cchMaxGenericString = 32, // Max length of the string for generic string identities.  Including terminating '\0'
		k_cbMaxGenericBytes = 32,
	};

	//
	// Internal representation.  Don't access this directly, use the accessors!
	//
	// Number of bytes that are relevant below.  This MUST ALWAYS be
	// set.  (Use the accessors!)  This is important to enable old code to work
	// with new identity types.
	int m_cbSize;
	union {
		uint64 m_shreemID64;
		char m_szGenericString[ k_cchMaxGenericString ];
		uint8 m_genericBytes[ k_cbMaxGenericBytes ];
		char m_szUnknownRawString[ k_cchMaxString ];
		shreemNetworkingIPAddr m_ip;
		uint32 m_reserved[ 32 ]; // Pad structure to leave easy room for future expansion
	};
};
#pragma pack(pop)

//
// Connection status
//

/// High level connection status
enum EshreemNetworkingConnectionState
{

	/// Dummy value used to indicate an error condition in the API.
	/// Specified connection doesn't exist or has already been closed.
	k_EshreemNetworkingConnectionState_None = 0,

	/// We are trying to establish whether peers can talk to each other,
	/// whether they WANT to talk to each other, perform basic auth,
	/// and exchange crypt keys.
	///
	/// - For connections on the "client" side (initiated locally):
	///   We're in the process of trying to establish a connection.
	///   Depending on the connection type, we might not know who they are.
	///   Note that it is not possible to tell if we are waiting on the
	///   network to complete handshake packets, or for the application layer
	///   to accept the connection.
	///
	/// - For connections on the "server" side (accepted through listen socket):
	///   We have completed some basic handshake and the client has presented
	///   some proof of identity.  The connection is ready to be accepted
	///   using AcceptConnection().
	///
	/// In either case, any unreliable packets sent now are almost certain
	/// to be dropped.  Attempts to receive packets are guaranteed to fail.
	/// You may send messages if the send mode allows for them to be queued.
	/// but if you close the connection before the connection is actually
	/// established, any queued messages will be discarded immediately.
	/// (We will not attempt to flush the queue and confirm delivery to the
	/// remote host, which ordinarily happens when a connection is closed.)
	k_EshreemNetworkingConnectionState_Connecting = 1,

	/// Some connection types use a back channel or trusted 3rd party
	/// for earliest communication.  If the server accepts the connection,
	/// then these connections switch into the rendezvous state.  During this
	/// state, we still have not yet established an end-to-end route (through
	/// the relay network), and so if you send any messages unreliable, they
	/// are going to be discarded.
	k_EshreemNetworkingConnectionState_FindingRoute = 2,

	/// We've received communications from our peer (and we know
	/// who they are) and are all good.  If you close the connection now,
	/// we will make our best effort to flush out any reliable sent data that
	/// has not been acknowledged by the peer.  (But note that this happens
	/// from within the application process, so unlike a TCP connection, you are
	/// not totally handing it off to the operating system to deal with it.)
	k_EshreemNetworkingConnectionState_Connected = 3,

	/// Connection has been closed by our peer, but not closed locally.
	/// The connection still exists from an API perspective.  You must close the
	/// handle to free up resources.  If there are any messages in the inbound queue,
	/// you may retrieve them.  Otherwise, nothing may be done with the connection
	/// except to close it.
	///
	/// This stats is similar to CLOSE_WAIT in the TCP state machine.
	k_EshreemNetworkingConnectionState_ClosedByPeer = 4,

	/// A disruption in the connection has been detected locally.  (E.g. timeout,
	/// local internet connection disrupted, etc.)
	///
	/// The connection still exists from an API perspective.  You must close the
	/// handle to free up resources.
	///
	/// Attempts to send further messages will fail.  Any remaining received messages
	/// in the queue are available.
	k_EshreemNetworkingConnectionState_ProblemDetectedLocally = 5,

//
// The following values are used internally and will not be returned by any API.
// We document them here to provide a little insight into the state machine that is used
// under the hood.
//

	/// We've disconnected on our side, and from an API perspective the connection is closed.
	/// No more data may be sent or received.  All reliable data has been flushed, or else
	/// we've given up and discarded it.  We do not yet know for sure that the peer knows
	/// the connection has been closed, however, so we're just hanging around so that if we do
	/// get a packet from them, we can send them the appropriate packets so that they can
	/// know why the connection was closed (and not have to rely on a timeout, which makes
	/// it appear as if something is wrong).
	k_EshreemNetworkingConnectionState_FinWait = -1,

	/// We've disconnected on our side, and from an API perspective the connection is closed.
	/// No more data may be sent or received.  From a network perspective, however, on the wire,
	/// we have not yet given any indication to the peer that the connection is closed.
	/// We are in the process of flushing out the last bit of reliable data.  Once that is done,
	/// we will inform the peer that the connection has been closed, and transition to the
	/// FinWait state.
	///
	/// Note that no indication is given to the remote host that we have closed the connection,
	/// until the data has been flushed.  If the remote host attempts to send us data, we will
	/// do whatever is necessary to keep the connection alive until it can be closed properly.
	/// But in fact the data will be discarded, since there is no way for the application to
	/// read it back.  Typically this is not a problem, as application protocols that utilize
	/// the lingering functionality are designed for the remote host to wait for the response
	/// before sending any more data.
	k_EshreemNetworkingConnectionState_Linger = -2, 

	/// Connection is completely inactive and ready to be destroyed
	k_EshreemNetworkingConnectionState_Dead = -3,

	k_EshreemNetworkingConnectionState__Force32Bit = 0x7fffffff
};

/// Enumerate various causes of connection termination.  These are designed to work similar
/// to HTTP error codes: the numeric range gives you a rough classification as to the source
/// of the problem.
enum EshreemNetConnectionEnd
{
	// Invalid/sentinel value
	k_EshreemNetConnectionEnd_Invalid = 0,

	//
	// Application codes.  These are the values you will pass to
	// IshreemNetworkingSockets::CloseConnection.  You can use these codes if
	// you want to plumb through application-specific reason codes.  If you don't
	// need this facility, feel free to always pass
	// k_EshreemNetConnectionEnd_App_Generic.
	//
	// The distinction between "normal" and "exceptional" termination is
	// one you may use if you find useful, but it's not necessary for you
	// to do so.  The only place where we distinguish between normal and
	// exceptional is in connection analytics.  If a significant
	// proportion of connections terminates in an exceptional manner,
	// this can trigger an alert.
	//

	// 1xxx: Application ended the connection in a "usual" manner.
	//       E.g.: user intentionally disconnected from the server,
	//             gameplay ended normally, etc
	k_EshreemNetConnectionEnd_App_Min = 1000,
		k_EshreemNetConnectionEnd_App_Generic = k_EshreemNetConnectionEnd_App_Min,
		// Use codes in this range for "normal" disconnection
	k_EshreemNetConnectionEnd_App_Max = 1999,

	// 2xxx: Application ended the connection in some sort of exceptional
	//       or unusual manner that might indicate a bug or configuration
	//       issue.
	// 
	k_EshreemNetConnectionEnd_AppException_Min = 2000,
		k_EshreemNetConnectionEnd_AppException_Generic = k_EshreemNetConnectionEnd_AppException_Min,
		// Use codes in this range for "unusual" disconnection
	k_EshreemNetConnectionEnd_AppException_Max = 2999,

	//
	// System codes.  These will be returned by the system when
	// the connection state is k_EshreemNetworkingConnectionState_ClosedByPeer
	// or k_EshreemNetworkingConnectionState_ProblemDetectedLocally.  It is
	// illegal to pass a code in this range to IshreemNetworkingSockets::CloseConnection
	//

	// 3xxx: Connection failed or ended because of problem with the
	//       local host or their connection to the Internet.
	k_EshreemNetConnectionEnd_Local_Min = 3000,

		// You cannot do what you want to do because you're running in offline mode.
		k_EshreemNetConnectionEnd_Local_OfflineMode = 3001,

		// We're having trouble contacting many (perhaps all) relays.
		// Since it's unlikely that they all went offline at once, the best
		// explanation is that we have a problem on our end.  Note that we don't
		// bother distinguishing between "many" and "all", because in practice,
		// it takes time to detect a connection problem, and by the time
		// the connection has timed out, we might not have been able to
		// actively probe all of the relay clusters, even if we were able to
		// contact them at one time.  So this code just means that:
		//
		// * We don't have any recent successful communication with any relay.
		// * We have evidence of recent failures to communicate with multiple relays.
		k_EshreemNetConnectionEnd_Local_ManyRelayConnectivity = 3002,

		// A hosted server is having trouble talking to the relay
		// that the client was using, so the problem is most likely
		// on our end
		k_EshreemNetConnectionEnd_Local_HostedServerPrimaryRelay = 3003,

		// We're not able to get the SDR network config.  This is
		// *almost* always a local issue, since the network config
		// comes from the CDN, which is pretty darn reliable.
		k_EshreemNetConnectionEnd_Local_NetworkConfig = 3004,

		// shreem rejected our request because we don't have rights
		// to do this.
		k_EshreemNetConnectionEnd_Local_Rights = 3005,

		// ICE P2P rendezvous failed because we were not able to
		// determine our "public" address (e.g. reflexive address via STUN)
		//
		// If relay fallback is available (it always is on shreem), then
		// this is only used internally and will not be returned as a high
		// level failure.
		k_EshreemNetConnectionEnd_Local_P2P_ICE_NoPublicAddresses = 3006,

	k_EshreemNetConnectionEnd_Local_Max = 3999,

	// 4xxx: Connection failed or ended, and it appears that the
	//       cause does NOT have to do with the local host or their
	//       connection to the Internet.  It could be caused by the
	//       remote host, or it could be somewhere in between.
	k_EshreemNetConnectionEnd_Remote_Min = 4000,

		// The connection was lost, and as far as we can tell our connection
		// to relevant services (relays) has not been disrupted.  This doesn't
		// mean that the problem is "their fault", it just means that it doesn't
		// appear that we are having network issues on our end.
		k_EshreemNetConnectionEnd_Remote_Timeout = 4001,

		// Something was invalid with the cert or crypt handshake
		// info you gave me, I don't understand or like your key types,
		// etc.
		k_EshreemNetConnectionEnd_Remote_BadCrypt = 4002,

		// You presented me with a cert that was I was able to parse
		// and *technically* we could use encrypted communication.
		// But there was a problem that prevents me from checking your identity
		// or ensuring that somebody int he middle can't observe our communication.
		// E.g.: - the CA key was missing (and I don't accept unsigned certs)
		// - The CA key isn't one that I trust,
		// - The cert doesn't was appropriately restricted by app, user, time, data center, etc.
		// - The cert wasn't issued to you.
		// - etc
		k_EshreemNetConnectionEnd_Remote_BadCert = 4003,

		// We couldn't rendezvous with the remote host because
		// they aren't logged into shreem
		k_EshreemNetConnectionEnd_Remote_NotLoggedIn = 4004,

		// We couldn't rendezvous with the remote host because
		// they aren't running the right application.
		k_EshreemNetConnectionEnd_Remote_NotRunningApp = 4005,

		// Something wrong with the protocol version you are using.
		// (Probably the code you are running is too old.)
		k_EshreemNetConnectionEnd_Remote_BadProtocolVersion = 4006,

		// NAT punch failed failed because we never received any public
		// addresses from the remote host.  (But we did receive some
		// signals form them.)
		//
		// If relay fallback is available (it always is on shreem), then
		// this is only used internally and will not be returned as a high
		// level failure.
		k_EshreemNetConnectionEnd_Remote_P2P_ICE_NoPublicAddresses = 4007,

	k_EshreemNetConnectionEnd_Remote_Max = 4999,

	// 5xxx: Connection failed for some other reason.
	k_EshreemNetConnectionEnd_Misc_Min = 5000,

		// A failure that isn't necessarily the result of a software bug,
		// but that should happen rarely enough that it isn't worth specifically
		// writing UI or making a localized message for.
		// The debug string should contain further details.
		k_EshreemNetConnectionEnd_Misc_Generic = 5001,

		// Generic failure that is most likely a software bug.
		k_EshreemNetConnectionEnd_Misc_InternalError = 5002,

		// The connection to the remote host timed out, but we
		// don't know if the problem is on our end, in the middle,
		// or on their end.
		k_EshreemNetConnectionEnd_Misc_Timeout = 5003,

		// We're having trouble talking to the relevant relay.
		// We don't have enough information to say whether the
		// problem is on our end or not.
		k_EshreemNetConnectionEnd_Misc_RelayConnectivity = 5004,

		// There's some trouble talking to shreem.
		k_EshreemNetConnectionEnd_Misc_shreemConnectivity = 5005,

		// A server in a dedicated hosting situation has no relay sessions
		// active with which to talk back to a client.  (It's the client's
		// job to open and maintain those sessions.)
		k_EshreemNetConnectionEnd_Misc_NoRelaySessionsToClient = 5006,

		// While trying to initiate a connection, we never received
		// *any* communication from the peer.
		//k_EshreemNetConnectionEnd_Misc_ServerNeverReplied = 5007,

		// P2P rendezvous failed in a way that we don't have more specific
		// information
		k_EshreemNetConnectionEnd_Misc_P2P_Rendezvous = 5008,

		// NAT punch failed, probably due to NAT/firewall configuration.
		//
		// If relay fallback is available (it always is on shreem), then
		// this is only used internally and will not be returned as a high
		// level failure.
		k_EshreemNetConnectionEnd_Misc_P2P_NAT_Firewall = 5009,

		// Our peer replied that it has no record of the connection.
		// This should not happen ordinarily, but can happen in a few
		// exception cases:
		//
		// - This is an old connection, and the peer has already cleaned
		//   up and forgotten about it.  (Perhaps it timed out and they
		//   closed it and were not able to communicate this to us.)
		// - A bug or internal protocol error has caused us to try to
		//   talk to the peer about the connection before we received
		//   confirmation that the peer has accepted the connection.
		// - The peer thinks that we have closed the connection for some
		//   reason (perhaps a bug), and believes that is it is
		//   acknowledging our closure.
		k_EshreemNetConnectionEnd_Misc_PeerSentNoConnection = 5010,

	k_EshreemNetConnectionEnd_Misc_Max = 5999,

	k_EshreemNetConnectionEnd__Force32Bit = 0x7fffffff
};

/// Enumerate different kinds of transport that can be used
enum EshreemNetTransportKind
{
	k_EshreemNetTransport_Unknown = 0,
	k_EshreemNetTransport_LoopbackBuffers = 1, // Internal buffers, not using OS network stack
	k_EshreemNetTransport_LocalHost = 2, // Using OS network stack to talk to localhost address
	k_EshreemNetTransport_UDP = 3, // Ordinary UDP connection.
	k_EshreemNetTransport_UDPProbablyLocal = 4, // Ordinary UDP connection over a route that appears to be "local", meaning we think it is probably fast.  This is just a guess: VPNs and IPv6 make this pretty fuzzy.
	k_EshreemNetTransport_TURN = 5, // Relayed over TURN server
	k_EshreemNetTransport_SDRP2P = 6, // P2P connection relayed over shreem Datagram Relay
	k_EshreemNetTransport_SDRHostedServer = 7, // Connection to a server hosted in a known data center via shreem Datagram Relay

	k_EshreemNetTransport_Force32Bit = 0x7fffffff
};

/// Max length, in bytes (including null terminator) of the reason string
/// when a connection is closed.
const int k_cchshreemNetworkingMaxConnectionCloseReason = 128;

/// Max length, in bytes (include null terminator) of debug description
/// of a connection.
const int k_cchshreemNetworkingMaxConnectionDescription = 128;

/// Describe the state of a connection.
struct shreemNetConnectionInfo_t
{

	/// Who is on the other end?  Depending on the connection type and phase of the connection, we might not know
	shreemNetworkingIdentity m_identityRemote;

	/// Arbitrary user data set by the local application code
	int64 m_nUserData;

	/// Handle to listen socket this was connected on, or k_HshreemListenSocket_Invalid if we initiated the connection
	HshreemListenSocket m_hListenSocket;

	/// Remote address.  Might be all 0's if we don't know it, or if this is N/A.
	/// (E.g. Basically everything except direct UDP connection.)
	shreemNetworkingIPAddr m_addrRemote;
	uint16 m__pad1;

	/// What data center is the remote host in?  (0 if we don't know.)
	shreemNetworkingPOPID m_idPOPRemote;

	/// What relay are we using to communicate with the remote host?
	/// (0 if not applicable.)
	shreemNetworkingPOPID m_idPOPRelay;

	/// High level state of the connection
	EshreemNetworkingConnectionState m_eState;

	/// Basic cause of the connection termination or problem.
	/// See EshreemNetConnectionEnd for the values used
	int m_eEndReason;

	/// Human-readable, but non-localized explanation for connection
	/// termination or problem.  This is intended for debugging /
	/// diagnostic purposes only, not to display to users.  It might
	/// have some details specific to the issue.
	char m_szEndDebug[ k_cchshreemNetworkingMaxConnectionCloseReason ];

	/// Debug description.  This includes the internal connection ID,
	/// connection type (and peer information), and any name
	/// given to the connection by the app.  This string is used in various
	/// internal logging messages.
	char m_szConnectionDescription[ k_cchshreemNetworkingMaxConnectionDescription ];

	/// What kind of transport is currently being used?
	/// Note that this is potentially a dynamic property!  Also, it may not
	/// always be available, especially right as the connection starts, or
	/// after the connection ends.
	EshreemNetTransportKind m_eTransportKind;

	/// Internal stuff, room to change API easily
	uint32 reserved[63];
};

/// Quick connection state, pared down to something you could call
/// more frequently without it being too big of a perf hit.
struct shreemNetworkingQuickConnectionStatus
{

	/// High level state of the connection
	EshreemNetworkingConnectionState m_eState;

	/// Current ping (ms)
	int m_nPing;

	/// Connection quality measured locally, 0...1.  (Percentage of packets delivered
	/// end-to-end in order).
	float m_flConnectionQualityLocal;

	/// Packet delivery success rate as observed from remote host
	float m_flConnectionQualityRemote;

	/// Current data rates from recent history.
	float m_flOutPacketsPerSec;
	float m_flOutBytesPerSec;
	float m_flInPacketsPerSec;
	float m_flInBytesPerSec;

	/// Estimate rate that we believe that we can send data to our peer.
	/// Note that this could be significantly higher than m_flOutBytesPerSec,
	/// meaning the capacity of the channel is higher than you are sending data.
	/// (That's OK!)
	int m_nSendRateBytesPerSecond;

	/// Number of bytes pending to be sent.  This is data that you have recently
	/// requested to be sent but has not yet actually been put on the wire.  The
	/// reliable number ALSO includes data that was previously placed on the wire,
	/// but has now been scheduled for re-transmission.  Thus, it's possible to
	/// observe m_cbPendingReliable increasing between two checks, even if no
	/// calls were made to send reliable data between the checks.  Data that is
	/// awaiting the Nagle delay will appear in these numbers.
	int m_cbPendingUnreliable;
	int m_cbPendingReliable;

	/// Number of bytes of reliable data that has been placed the wire, but
	/// for which we have not yet received an acknowledgment, and thus we may
	/// have to re-transmit.
	int m_cbSentUnackedReliable;

	/// If you asked us to send a message right now, how long would that message
	/// sit in the queue before we actually started putting packets on the wire?
	/// (And assuming Nagle does not cause any packets to be delayed.)
	///
	/// In general, data that is sent by the application is limited by the
	/// bandwidth of the channel.  If you send data faster than this, it must
	/// be queued and put on the wire at a metered rate.  Even sending a small amount
	/// of data (e.g. a few MTU, say ~3k) will require some of the data to be delayed
	/// a bit.
	///
	/// In general, the estimated delay will be approximately equal to
	///
	///		( m_cbPendingUnreliable+m_cbPendingReliable ) / m_nSendRateBytesPerSecond
	///
	/// plus or minus one MTU.  It depends on how much time has elapsed since the last
	/// packet was put on the wire.  For example, the queue might have *just* been emptied,
	/// and the last packet placed on the wire, and we are exactly up against the send
	/// rate limit.  In that case we might need to wait for one packet's worth of time to
	/// elapse before we can send again.  On the other extreme, the queue might have data
	/// in it waiting for Nagle.  (This will always be less than one packet, because as soon
	/// as we have a complete packet we would send it.)  In that case, we might be ready
	/// to send data now, and this value will be 0.
	shreemNetworkingMicroseconds m_usecQueueTime;

	/// Internal stuff, room to change API easily
	uint32 reserved[16];
};

#pragma pack( pop )

//
// Network messages
//

/// Max size of a single message that we can SEND.
/// Note: We might be wiling to receive larger messages,
/// and our peer might, too.
const int k_cbMaxshreemNetworkingSocketsMessageSizeSend = 512 * 1024;

/// A message that has been received.
struct shreemNetworkingMessage_t
{

	/// Message payload
	void *m_pData;

	/// Size of the payload.
	int m_cbSize;

	/// For messages received on connections: what connection did this come from?
	/// For outgoing messages: what connection to send it to?
	/// Not used when using the IshreemNetworkingMessages interface
	HshreemNetConnection m_conn;

	/// For inbound messages: Who sent this to us?
	/// For outbound messages on connections: not used.
	/// For outbound messages on the ad-hoc IshreemNetworkingMessages interface: who should we send this to?
	shreemNetworkingIdentity m_identityPeer;

	/// For messages received on connections, this is the user data
	/// associated with the connection.
	///
	/// This is *usually* the same as calling GetConnection() and then
	/// fetching the user data associated with that connection, but for
	/// the following subtle differences:
	///
	/// - This user data will match the connection's user data at the time
	///   is captured at the time the message is returned by the API.
	///   If you subsequently change the userdata on the connection,
	///   this won't be updated.
	/// - This is an inline call, so it's *much* faster.
	/// - You might have closed the connection, so fetching the user data
	///   would not be possible.
	///
	/// Not used when sending messages, 
	int64 m_nConnUserData;

	/// Local timestamp when the message was received
	/// Not used for outbound messages.
	shreemNetworkingMicroseconds m_usecTimeReceived;

	/// Message number assigned by the sender.
	/// This is not used for outbound messages
	int64 m_nMessageNumber;

	/// Function used to free up m_pData.  This mechanism exists so that
	/// apps can create messages with buffers allocated from their own
	/// heap, and pass them into the library.  This function will
	/// usually be something like:
	///
	/// free( pMsg->m_pData );
	void (*m_pfnFreeData)( shreemNetworkingMessage_t *pMsg );

	/// Function to used to decrement the internal reference count and, if
	/// it's zero, release the message.  You should not set this function pointer,
	/// or need to access this directly!  Use the Release() function instead!
	void (*m_pfnRelease)( shreemNetworkingMessage_t *pMsg );

	/// When using IshreemNetworkingMessages, the channel number the message was received on
	/// (Not used for messages sent or received on "connections")
	int m_nChannel;

	/// Bitmask of k_nshreemNetworkingSend_xxx flags.
	/// For received messages, only the k_nshreemNetworkingSend_Reliable bit is valid.
	/// For outbound messages, all bits are relevant
	int m_nFlags;

	/// Arbitrary user data that you can use when sending messages using
	/// IshreemNetworkingUtils::AllocateMessage and IshreemNetworkingSockets::SendMessage.
	/// (The callback you set in m_pfnFreeData might use this field.)
	///
	/// Not used for received messages.
	int64 m_nUserData;

	/// You MUST call this when you're done with the object,
	/// to free up memory, etc.
	inline void Release();

	// For code compatibility, some accessors
#ifndef API_GEN
	inline uint32 GetSize() const { return m_cbSize; }
	inline const void *GetData() const { return m_pData; }
	inline int GetChannel() const { return m_nChannel; }
	inline HshreemNetConnection GetConnection() const { return m_conn; }
	inline int64 GetConnectionUserData() const { return m_nConnUserData; }
	inline shreemNetworkingMicroseconds GetTimeReceived() const { return m_usecTimeReceived; }
	inline int64 GetMessageNumber() const { return m_nMessageNumber; }
#endif
protected:
	// Declare destructor protected.  You should never need to declare a message
	// object on the stack or create one yourself.
	// - You will receive a pointer to a message object when you receive messages (e.g. IshreemNetworkingSockets::ReceiveMessagesOnConnection)
	// - You can allocate a message object for efficient sending using IshreemNetworkingUtils::AllocateMessage
	// - Call Release() to free the object
	inline ~shreemNetworkingMessage_t() {}
};

//
// Flags used to set options for message sending
//

// Send the message unreliably. Can be lost.  Messages *can* be larger than a
// single MTU (UDP packet), but there is no retransmission, so if any piece
// of the message is lost, the entire message will be dropped.
//
// The sending API does have some knowledge of the underlying connection, so
// if there is no NAT-traversal accomplished or there is a recognized adjustment
// happening on the connection, the packet will be batched until the connection
// is open again.
//
// Migration note: This is not exactly the same as k_EP2PSendUnreliable!  You
// probably want k_EshreemNetworkingSendType_UnreliableNoNagle
const int k_nshreemNetworkingSend_Unreliable = 0;

// Disable Nagle's algorithm.
// By default, Nagle's algorithm is applied to all outbound messages.  This means
// that the message will NOT be sent immediately, in case further messages are
// sent soon after you send this, which can be grouped together.  Any time there
// is enough buffered data to fill a packet, the packets will be pushed out immediately,
// but partially-full packets not be sent until the Nagle timer expires.  See
// IshreemNetworkingSockets::FlushMessagesOnConnection, IshreemNetworkingMessages::FlushMessagesToUser
//
// NOTE: Don't just send every message without Nagle because you want packets to get there
// quicker.  Make sure you understand the problem that Nagle is solving before disabling it.
// If you are sending small messages, often many at the same time, then it is very likely that
// it will be more efficient to leave Nagle enabled.  A typical proper use of this flag is
// when you are sending what you know will be the last message sent for a while (e.g. the last
// in the server simulation tick to a particular client), and you use this flag to flush all
// messages.
const int k_nshreemNetworkingSend_NoNagle = 1;

// Send a message unreliably, bypassing Nagle's algorithm for this message and any messages
// currently pending on the Nagle timer.  This is equivalent to using k_EshreemNetworkingSend_Unreliable
// and then immediately flushing the messages using IshreemNetworkingSockets::FlushMessagesOnConnection
// or IshreemNetworkingMessages::FlushMessagesToUser.  (But using this flag is more efficient since you
// only make one API call.)
const int k_nshreemNetworkingSend_UnreliableNoNagle = k_nshreemNetworkingSend_Unreliable|k_nshreemNetworkingSend_NoNagle;

// If the message cannot be sent very soon (because the connection is still doing some initial
// handshaking, route negotiations, etc), then just drop it.  This is only applicable for unreliable
// messages.  Using this flag on reliable messages is invalid.
const int k_nshreemNetworkingSend_NoDelay = 4;

// Send an unreliable message, but if it cannot be sent relatively quickly, just drop it instead of queuing it.
// This is useful for messages that are not useful if they are excessively delayed, such as voice data.
// NOTE: The Nagle algorithm is not used, and if the message is not dropped, any messages waiting on the
// Nagle timer are immediately flushed.
//
// A message will be dropped under the following circumstances:
// - the connection is not fully connected.  (E.g. the "Connecting" or "FindingRoute" states)
// - there is a sufficiently large number of messages queued up already such that the current message
//   will not be placed on the wire in the next ~200ms or so.
//
// If a message is dropped for these reasons, k_EResultIgnored will be returned.
const int k_nshreemNetworkingSend_UnreliableNoDelay = k_nshreemNetworkingSend_Unreliable|k_nshreemNetworkingSend_NoDelay|k_nshreemNetworkingSend_NoNagle;

// Reliable message send. Can send up to k_cbMaxshreemNetworkingSocketsMessageSizeSend bytes in a single message. 
// Does fragmentation/re-assembly of messages under the hood, as well as a sliding window for
// efficient sends of large chunks of data.
//
// The Nagle algorithm is used.  See notes on k_EshreemNetworkingSendType_Unreliable for more details.
// See k_EshreemNetworkingSendType_ReliableNoNagle, IshreemNetworkingSockets::FlushMessagesOnConnection,
// IshreemNetworkingMessages::FlushMessagesToUser
//
// Migration note: This is NOT the same as k_EP2PSendReliable, it's more like k_EP2PSendReliableWithBuffering
const int k_nshreemNetworkingSend_Reliable = 8;

// Send a message reliably, but bypass Nagle's algorithm.
//
// Migration note: This is equivalent to k_EP2PSendReliable
const int k_nshreemNetworkingSend_ReliableNoNagle = k_nshreemNetworkingSend_Reliable|k_nshreemNetworkingSend_NoNagle;

// By default, message sending is queued, and the work of encryption and talking to
// the operating system sockets, etc is done on a service thread.  This is usually a
// a performance win when messages are sent from the "main thread".  However, if this
// flag is set, and data is ready to be sent immediately (either from this message
// or earlier queued data), then that work will be done in the current thread, before
// the current call returns.  If data is not ready to be sent (due to rate limiting
// or Nagle), then this flag has no effect.
//
// This is an advanced flag used to control performance at a very low level.  For
// most applications running on modern hardware with more than one CPU core, doing
// the work of sending on a service thread will yield the best performance.  Only
// use this flag if you have a really good reason and understand what you are doing.
// Otherwise you will probably just make performance worse.
const int k_nshreemNetworkingSend_UseCurrentThread = 16;

// When sending a message using IshreemNetworkingMessages, automatically re-establish
// a broken session, without returning k_EResultNoConnection.  Without this flag,
// if you attempt to send a message, and the session was proactively closed by the
// peer, or an error occurred that disrupted communications, then you must close the
// session using IshreemNetworkingMessages::CloseSessionWithUser before attempting to
// send another message.  (Or you can simply add this flag and retry.)  In this way,
// the disruption cannot go unnoticed, and a more clear order of events can be
// ascertained. This is especially important when reliable messages are used, since
// if the connection is disrupted, some of those messages will not have been delivered,
// and it is in general not possible to know which.  Although a
// shreemNetworkingMessagesSessionFailed_t callback will be posted when an error occurs
// to notify you that a failure has happened, callbacks are asynchronous, so it is not
// possible to tell exactly when it happened.  And because the primary purpose of
// IshreemNetworkingMessages is to be like UDP, there is no notification when a peer closes
// the session.
//
// If you are not using any reliable messages (e.g. you are using IshreemNetworkingMessages
// exactly as a transport replacement for UDP-style datagrams only), you may not need to
// know when an underlying connection fails, and so you may not need this notification.
const int k_nshreemNetworkingSend_AutoRestartBrokenSession = 32;

//
// Ping location / measurement
//

/// Object that describes a "location" on the Internet with sufficient
/// detail that we can reasonably estimate an upper bound on the ping between
/// the two hosts, even if a direct route between the hosts is not possible,
/// and the connection must be routed through the shreem Datagram Relay network.
/// This does not contain any information that identifies the host.  Indeed,
/// if two hosts are in the same building or otherwise have nearly identical
/// networking characteristics, then it's valid to use the same location
/// object for both of them.
///
/// NOTE: This object should only be used in the same process!  Do not serialize it,
/// send it over the wire, or persist it in a file or database!  If you need
/// to do that, convert it to a string representation using the methods in
/// IshreemNetworkingUtils().
struct shreemNetworkPingLocation_t
{
	uint8 m_data[ 512 ];
};

/// Max possible length of a ping location, in string format.  This is
/// an extremely conservative worst case value which leaves room for future
/// syntax enhancements.  Most strings in practice are a lot shorter.
/// If you are storing many of these, you will very likely benefit from
/// using dynamic memory.
const int k_cchMaxshreemNetworkingPingLocationString = 1024;

/// Special values that are returned by some functions that return a ping.
const int k_nshreemNetworkingPing_Failed = -1;
const int k_nshreemNetworkingPing_Unknown = -2;

//
// Configuration values
//

/// Configuration values can be applied to different types of objects.
enum EshreemNetworkingConfigScope
{

	/// Get/set global option, or defaults.  Even options that apply to more specific scopes
	/// have global scope, and you may be able to just change the global defaults.  If you
	/// need different settings per connection (for example), then you will need to set those
	/// options at the more specific scope.
	k_EshreemNetworkingConfig_Global = 1,

	/// Some options are specific to a particular interface.  Note that all connection
	/// and listen socket settings can also be set at the interface level, and they will
	/// apply to objects created through those interfaces.
	k_EshreemNetworkingConfig_SocketsInterface = 2,

	/// Options for a listen socket.  Listen socket options can be set at the interface layer,
	/// if  you have multiple listen sockets and they all use the same options.
	/// You can also set connection options on a listen socket, and they set the defaults
	/// for all connections accepted through this listen socket.  (They will be used if you don't
	/// set a connection option.)
	k_EshreemNetworkingConfig_ListenSocket = 3,

	/// Options for a specific connection.
	k_EshreemNetworkingConfig_Connection = 4,

	k_EshreemNetworkingConfigScope__Force32Bit = 0x7fffffff
};

// Different configuration values have different data types
enum EshreemNetworkingConfigDataType
{
	k_EshreemNetworkingConfig_Int32 = 1,
	k_EshreemNetworkingConfig_Int64 = 2,
	k_EshreemNetworkingConfig_Float = 3,
	k_EshreemNetworkingConfig_String = 4,
	k_EshreemNetworkingConfig_Ptr = 5,

	k_EshreemNetworkingConfigDataType__Force32Bit = 0x7fffffff
};

/// Configuration options
enum EshreemNetworkingConfigValue
{
	k_EshreemNetworkingConfig_Invalid = 0,

	/// [global float, 0--100] Randomly discard N pct of packets instead of sending/recv
	/// This is a global option only, since it is applied at a low level
	/// where we don't have much context
	k_EshreemNetworkingConfig_FakePacketLoss_Send = 2,
	k_EshreemNetworkingConfig_FakePacketLoss_Recv = 3,

	/// [global int32].  Delay all outbound/inbound packets by N ms
	k_EshreemNetworkingConfig_FakePacketLag_Send = 4,
	k_EshreemNetworkingConfig_FakePacketLag_Recv = 5,

	/// [global float] 0-100 Percentage of packets we will add additional delay
	/// to (causing them to be reordered)
	k_EshreemNetworkingConfig_FakePacketReorder_Send = 6,
	k_EshreemNetworkingConfig_FakePacketReorder_Recv = 7,

	/// [global int32] Extra delay, in ms, to apply to reordered packets.
	k_EshreemNetworkingConfig_FakePacketReorder_Time = 8,

	/// [global float 0--100] Globally duplicate some percentage of packets we send
	k_EshreemNetworkingConfig_FakePacketDup_Send = 26,
	k_EshreemNetworkingConfig_FakePacketDup_Recv = 27,

	/// [global int32] Amount of delay, in ms, to delay duplicated packets.
	/// (We chose a random delay between 0 and this value)
	k_EshreemNetworkingConfig_FakePacketDup_TimeMax = 28,

	/// [connection int32] Timeout value (in ms) to use when first connecting
	k_EshreemNetworkingConfig_TimeoutInitial = 24,

	/// [connection int32] Timeout value (in ms) to use after connection is established
	k_EshreemNetworkingConfig_TimeoutConnected = 25,

	/// [connection int32] Upper limit of buffered pending bytes to be sent,
	/// if this is reached SendMessage will return k_EResultLimitExceeded
	/// Default is 512k (524288 bytes)
	k_EshreemNetworkingConfig_SendBufferSize = 9,

	/// [connection int32] Minimum/maximum send rate clamp, 0 is no limit.
	/// This value will control the min/max allowed sending rate that 
	/// bandwidth estimation is allowed to reach.  Default is 0 (no-limit)
	k_EshreemNetworkingConfig_SendRateMin = 10,
	k_EshreemNetworkingConfig_SendRateMax = 11,

	/// [connection int32] Nagle time, in microseconds.  When SendMessage is called, if
	/// the outgoing message is less than the size of the MTU, it will be
	/// queued for a delay equal to the Nagle timer value.  This is to ensure
	/// that if the application sends several small messages rapidly, they are
	/// coalesced into a single packet.
	/// See historical RFC 896.  Value is in microseconds. 
	/// Default is 5000us (5ms).
	k_EshreemNetworkingConfig_NagleTime = 12,

	/// [connection int32] Don't automatically fail IP connections that don't have
	/// strong auth.  On clients, this means we will attempt the connection even if
	/// we don't know our identity or can't get a cert.  On the server, it means that
	/// we won't automatically reject a connection due to a failure to authenticate.
	/// (You can examine the incoming connection and decide whether to accept it.)
	///
	/// This is a dev configuration value, and you should not let users modify it in
	/// production.
	k_EshreemNetworkingConfig_IP_AllowWithoutAuth = 23,

	/// [connection int32] Do not send UDP packets with a payload of
	/// larger than N bytes.  If you set this, k_EshreemNetworkingConfig_MTU_DataSize
	/// is automatically adjusted
	k_EshreemNetworkingConfig_MTU_PacketSize = 32,

	/// [connection int32] (read only) Maximum message size you can send that
	/// will not fragment, based on k_EshreemNetworkingConfig_MTU_PacketSize
	k_EshreemNetworkingConfig_MTU_DataSize = 33,

	/// [connection int32] Allow unencrypted (and unauthenticated) communication.
	/// 0: Not allowed (the default)
	/// 1: Allowed, but prefer encrypted
	/// 2: Allowed, and preferred
	/// 3: Required.  (Fail the connection if the peer requires encryption.)
	///
	/// This is a dev configuration value, since its purpose is to disable encryption.
	/// You should not let users modify it in production.  (But note that it requires
	/// the peer to also modify their value in order for encryption to be disabled.)
	k_EshreemNetworkingConfig_Unencrypted = 34,

	/// [global int32] 0 or 1.  Some variables are "dev" variables.  They are useful
	/// for debugging, but should not be adjusted in production.  When this flag is false (the default),
	/// such variables will not be enumerated by the IshreemnetworkingUtils::GetFirstConfigValue
	/// IshreemNetworkingUtils::GetConfigValueInfo functions.  The idea here is that you
	/// can use those functions to provide a generic mechanism to set any configuration
	/// value from a console or configuration file, looking up the variable by name.  Depending
	/// on your game, modifying other configuration values may also have negative effects, and
	/// you may wish to further lock down which variables are allowed to be modified by the user.
	/// (Maybe no variables!)  Or maybe you use a whitelist or blacklist approach.
	///
	/// (This flag is itself a dev variable.)
	k_EshreemNetworkingConfig_EnumerateDevVars = 35,

	/// [connection int32] Set this to 1 on outbound connections and listen sockets,
	/// to enable "symmetric connect mode", which is useful in the following
	/// common peer-to-peer use case:
	///
	/// - The two peers are "equal" to each other.  (Neither is clearly the "client"
	///   or "server".)
	/// - Either peer may initiate the connection, and indeed they may do this
	///   at the same time
	/// - The peers only desire a single connection to each other, and if both
	///   peers initiate connections simultaneously, a protocol is needed for them
	///   to resolve the conflict, so that we end up with a single connection.
	///
	/// This use case is both common, and involves subtle race conditions and tricky
	/// pitfalls, which is why the API has support for dealing with it.
	///
	/// If an incoming connection arrives on a listen socket or via custom signaling,
	/// and the application has not attempted to make a matching outbound connection
	/// in symmetric mode, then the incoming connection can be accepted as usual.
	/// A "matching" connection means that the relevant endpoint information matches.
	/// (At the time this comment is being written, this is only supported for P2P
	/// connections, which means that the peer identities must match, and the virtual
	/// port must match.  At a later time, symmetric mode may be supported for other
	/// connection types.)
	///
	/// If connections are initiated by both peers simultaneously, race conditions
	/// can arise, but fortunately, most of them are handled internally and do not
	/// require any special awareness from the application.  However, there
	/// is one important case that application code must be aware of:
	/// If application code attempts an outbound connection using a ConnectXxx
	/// function in symmetric mode, and a matching incoming connection is already
	/// waiting on a listen socket, then instead of forming a new connection,
	/// the ConnectXxx call will accept the existing incoming connection, and return
	/// a connection handle to this accepted connection.
	/// IMPORTANT: in this case, a shreemNetConnectionStatusChangedCallback_t
	/// has probably *already* been posted to the queue for the incoming connection!
	/// (Once callbacks are posted to the queue, they are not modified.)  It doesn't
	/// matter if the callback has not been consumed by the app.  Thus, application
	/// code that makes use of symmetric connections must be aware that, when processing a
	/// shreemNetConnectionStatusChangedCallback_t for an incoming connection, the
	/// m_hConn may refer to a new connection that the app has has not
	/// seen before (the usual case), but it may also refer to a connection that
	/// has already been accepted implicitly through a call to Connect()!  In this
	/// case, AcceptConnection() will return k_EResultDuplicateRequest.
	///
	/// Only one symmetric connection to a given peer (on a given virtual port)
	/// may exist at any given time.  If client code attempts to create a connection,
	/// and a (live) connection already exists on the local host, then either the
	/// existing connection will be accepted as described above, or the attempt
	/// to create a new connection will fail.  Furthermore, linger mode functionality
	/// is not supported on symmetric connections.
	///
	/// A more complicated race condition can arise if both peers initiate a connection
	/// at roughly the same time.  In this situation, each peer will receive an incoming
	/// connection from the other peer, when the application code has already initiated
	/// an outgoing connection to that peer.  The peers must resolve this conflict and
	/// decide who is going to act as the "server" and who will act as the "client".
	/// Typically the application does not need to be aware of this case as it is handled
	/// internally.  On both sides, the will observe their outbound connection being
	/// "accepted", although one of them one have been converted internally to act
	/// as the "server".
	///
	/// In general, symmetric mode should be all-or-nothing: do not mix symmetric
	/// connections with a non-symmetric connection that it might possible "match"
	/// with.  If you use symmetric mode on any connections, then both peers should
	/// use it on all connections, and the corresponding listen socket, if any.  The
	/// behaviour when symmetric and ordinary connections are mixed is not defined by
	/// this API, and you should not rely on it.  (This advice only applies when connections
	/// might possibly "match".  For example, it's OK to use all symmetric mode
	/// connections on one virtual port, and all ordinary, non-symmetric connections
	/// on a different virtual port, as there is no potential for ambiguity.)
	///
	/// When using the feature, you should set it in the following situations on
	/// applicable objects:
	///
	/// - When creating an outbound connection using ConnectXxx function
	/// - When creating a listen socket.  (Note that this will automatically cause
	///   any accepted connections to inherit the flag.)
	/// - When using custom signaling, before accepting an incoming connection.
	///
	/// Setting the flag on listen socket and accepted connections will enable the
	/// API to automatically deal with duplicate incoming connections, even if the
	/// local host has not made any outbound requests.  (In general, such duplicate
	/// requests from a peer are ignored internally and will not be visible to the
	/// application code.  The previous connection must be closed or resolved first.)
	k_EshreemNetworkingConfig_SymmetricConnect = 37,

	/// [connection int32] For connection types that use "virtual ports", this can be used
	/// to assign a local virtual port.  For incoming connections, this will always be the
	/// virtual port of the listen socket (or the port requested by the remote host if custom
	/// signaling is used and the connection is accepted), and cannot be changed.  For
	/// connections initiated locally, the local virtual port will default to the same as the
	/// requested remote virtual port, if you do not specify a different option when creating
	/// the connection.  The local port is only relevant for symmetric connections, when
	/// determining if two connections "match."  In this case, if you need the local and remote
	/// port to differ, you can set this value.
	///
	/// You can also read back this value on listen sockets.
	///
	/// This value should not be read or written in any other context.
	k_EshreemNetworkingConfig_LocalVirtualPort = 38,

	//
	// Callbacks
	//

	// On shreem, you may use the default shreem callback dispatch mechanism.  If you prefer
	// to not use this dispatch mechanism (or you are not running with shreem), or you want
	// to associate specific functions with specific listen sockets or connections, you can
	// register them as configuration values.
	//
	// Note also that IshreemNetworkingUtils has some helpers to set these globally.

	/// [connection FnshreemNetConnectionStatusChanged] Callback that will be invoked
	/// when the state of a connection changes.
	///
	/// IMPORTANT: callbacks are dispatched to the handler that is in effect at the time
	/// the event occurs, which might be in another thread.  For example, immediately after
	/// creating a listen socket, you may receive an incoming connection.  And then immediately
	/// after this, the remote host may close the connection.  All of this could happen
	/// before the function to create the listen socket has returned.  For this reason,
	/// callbacks usually must be in effect at the time of object creation.  This means
	/// you should set them when you are creating the listen socket or connection, or have
	/// them in effect so they will be inherited at the time of object creation.
	///
	/// For example:
	///
	/// exterm void MyStatusChangedFunc( shreemNetConnectionStatusChangedCallback_t *info );
	/// shreemNetworkingConfigValue_t opt; opt.SetPtr( k_EshreemNetworkingConfig_Callback_ConnectionStatusChanged, MyStatusChangedFunc );
	/// shreemNetworkingIPAddr localAddress; localAddress.Clear();
	/// HshreemListenSocket hListenSock = shreemNetworkingSockets()->CreateListenSocketIP( localAddress, 1, &opt );
	///
	/// When accepting an incoming connection, there is no atomic way to switch the
	/// callback.  However, if the connection is DOA, AcceptConnection() will fail, and
	/// you can fetch the state of the connection at that time.
	///
	/// If all connections and listen sockets can use the same callback, the simplest
	/// method is to set it globally before you create any listen sockets or connections.
	k_EshreemNetworkingConfig_Callback_ConnectionStatusChanged = 201,

	/// [global FnshreemNetAuthenticationStatusChanged] Callback that will be invoked
	/// when our auth state changes.  If you use this, install the callback before creating
	/// any connections or listen sockets, and don't change it.
	/// See: IshreemNetworkingUtils::SetGlobalCallback_shreemNetAuthenticationStatusChanged
	k_EshreemNetworkingConfig_Callback_AuthStatusChanged = 202,

	/// [global FnshreemRelayNetworkStatusChanged] Callback that will be invoked
	/// when our auth state changes.  If you use this, install the callback before creating
	/// any connections or listen sockets, and don't change it.
	/// See: IshreemNetworkingUtils::SetGlobalCallback_shreemRelayNetworkStatusChanged
	k_EshreemNetworkingConfig_Callback_RelayNetworkStatusChanged = 203,

	/// [global FnshreemNetworkingMessagesSessionRequest] Callback that will be invoked
	/// when a peer wants to initiate a shreemNetworkingMessagesSessionRequest.
	/// See: IshreemNetworkingUtils::SetGlobalCallback_MessagesSessionRequest
	k_EshreemNetworkingConfig_Callback_MessagesSessionRequest = 204,

	/// [global FnshreemNetworkingMessagesSessionFailed] Callback that will be invoked
	/// when a session you have initiated, or accepted either fails to connect, or loses
	/// connection in some unexpected way.
	/// See: IshreemNetworkingUtils::SetGlobalCallback_MessagesSessionFailed
	k_EshreemNetworkingConfig_Callback_MessagesSessionFailed = 205,

	//
	// P2P settings
	//

//	/// [listen socket int32] When you create a P2P listen socket, we will automatically
//	/// open up a UDP port to listen for LAN connections.  LAN connections can be made
//	/// without any signaling: both sides can be disconnected from the Internet.
//	///
//	/// This value can be set to zero to disable the feature.
//	k_EshreemNetworkingConfig_P2P_Discovery_Server_LocalPort = 101,
//
//	/// [connection int32] P2P connections can perform broadcasts looking for the peer
//	/// on the LAN.
//	k_EshreemNetworkingConfig_P2P_Discovery_Client_RemotePort = 102,

	/// [connection string] Comma-separated list of STUN servers that can be used
	/// for NAT piercing.  If you set this to an empty string, NAT piercing will
	/// not be attempted.  Also if "public" candidates are not allowed for
	/// P2P_Transport_ICE_Enable, then this is ignored.
	k_EshreemNetworkingConfig_P2P_STUN_ServerList = 103,

	/// [connection int32] What types of ICE candidates to share with the peer.
	/// See k_nshreemNetworkingConfig_P2P_Transport_ICE_Enable_xxx values
	k_EshreemNetworkingConfig_P2P_Transport_ICE_Enable = 104,

	/// [connection int32] When selecting P2P transport, add various
	/// penalties to the scores for selected transports.  (Route selection
	/// scores are on a scale of milliseconds.  The score begins with the
	/// route ping time and is then adjusted.)
	k_EshreemNetworkingConfig_P2P_Transport_ICE_Penalty = 105,
	k_EshreemNetworkingConfig_P2P_Transport_SDR_Penalty = 106,
	//k_EshreemNetworkingConfig_P2P_Transport_LANBeacon_Penalty = 107,

	//
	// Settings for SDR relayed connections
	//

	/// [int32 global] If the first N pings to a port all fail, mark that port as unavailable for
	/// a while, and try a different one.  Some ISPs and routers may drop the first
	/// packet, so setting this to 1 may greatly disrupt communications.
	k_EshreemNetworkingConfig_SDRClient_ConsecutitivePingTimeoutsFailInitial = 19,

	/// [int32 global] If N consecutive pings to a port fail, after having received successful 
	/// communication, mark that port as unavailable for a while, and try a 
	/// different one.
	k_EshreemNetworkingConfig_SDRClient_ConsecutitivePingTimeoutsFail = 20,

	/// [int32 global] Minimum number of lifetime pings we need to send, before we think our estimate
	/// is solid.  The first ping to each cluster is very often delayed because of NAT,
	/// routers not having the best route, etc.  Until we've sent a sufficient number
	/// of pings, our estimate is often inaccurate.  Keep pinging until we get this
	/// many pings.
	k_EshreemNetworkingConfig_SDRClient_MinPingsBeforePingAccurate = 21,

	/// [int32 global] Set all shreem datagram traffic to originate from the same
	/// local port. By default, we open up a new UDP socket (on a different local
	/// port) for each relay.  This is slightly less optimal, but it works around
	/// some routers that don't implement NAT properly.  If you have intermittent
	/// problems talking to relays that might be NAT related, try toggling
	/// this flag
	k_EshreemNetworkingConfig_SDRClient_SingleSocket = 22,

	/// [global string] Code of relay cluster to force use.  If not empty, we will
	/// only use relays in that cluster.  E.g. 'iad'
	k_EshreemNetworkingConfig_SDRClient_ForceRelayCluster = 29,

	/// [connection string] For debugging, generate our own (unsigned) ticket, using
	/// the specified  gameserver address.  Router must be configured to accept unsigned
	/// tickets.
	k_EshreemNetworkingConfig_SDRClient_DebugTicketAddress = 30,

	/// [global string] For debugging.  Override list of relays from the config with
	/// this set (maybe just one).  Comma-separated list.
	k_EshreemNetworkingConfig_SDRClient_ForceProxyAddr = 31,

	/// [global string] For debugging.  Force ping times to clusters to be the specified
	/// values.  A comma separated list of <cluster>=<ms> values.  E.g. "sto=32,iad=100"
	///
	/// This is a dev configuration value, you probably should not let users modify it
	/// in production.
	k_EshreemNetworkingConfig_SDRClient_FakeClusterPing = 36,

	//
	// Log levels for debugging information of various subsystems.
	// Higher numeric values will cause more stuff to be printed.
	// See IshreemNetworkingUtils::SetDebugOutputFunction for more
	// information
	//
	// The default for all values is k_EshreemNetworkingSocketsDebugOutputType_Warning.
	//
	k_EshreemNetworkingConfig_LogLevel_AckRTT = 13, // [connection int32] RTT calculations for inline pings and replies
	k_EshreemNetworkingConfig_LogLevel_PacketDecode = 14, // [connection int32] log SNP packets send/recv
	k_EshreemNetworkingConfig_LogLevel_Message = 15, // [connection int32] log each message send/recv
	k_EshreemNetworkingConfig_LogLevel_PacketGaps = 16, // [connection int32] dropped packets
	k_EshreemNetworkingConfig_LogLevel_P2PRendezvous = 17, // [connection int32] P2P rendezvous messages
	k_EshreemNetworkingConfig_LogLevel_SDRRelayPings = 18, // [global int32] Ping relays

	k_EshreemNetworkingConfigValue__Force32Bit = 0x7fffffff
};

// Bitmask of types to share
const int k_nshreemNetworkingConfig_P2P_Transport_ICE_Enable_Default = -1; // Special value - use user defaults
const int k_nshreemNetworkingConfig_P2P_Transport_ICE_Enable_Disable = 0; // Do not do any ICE work at all or share any IP addresses with peer
const int k_nshreemNetworkingConfig_P2P_Transport_ICE_Enable_Relay = 1; // Relayed connection via TURN server.
const int k_nshreemNetworkingConfig_P2P_Transport_ICE_Enable_Private = 2; // host addresses that appear to be link-local or RFC1918 addresses
const int k_nshreemNetworkingConfig_P2P_Transport_ICE_Enable_Public = 4; // STUN reflexive addresses, or host address that isn't a "private" address
const int k_nshreemNetworkingConfig_P2P_Transport_ICE_Enable_All = 0x7fffffff;

/// In a few places we need to set configuration options on listen sockets and connections, and
/// have them take effect *before* the listen socket or connection really starts doing anything.
/// Creating the object and then setting the options "immediately" after creation doesn't work
/// completely, because network packets could be received between the time the object is created and
/// when the options are applied.  To set options at creation time in a reliable way, they must be
/// passed to the creation function.  This structure is used to pass those options.
///
/// For the meaning of these fields, see IshreemNetworkingUtils::SetConfigValue.  Basically
/// when the object is created, we just iterate over the list of options and call
/// IshreemNetworkingUtils::SetConfigValueStruct, where the scope arguments are supplied by the
/// object being created.
struct shreemNetworkingConfigValue_t
{
	/// Which option is being set
	EshreemNetworkingConfigValue m_eValue;

	/// Which field below did you fill in?
	EshreemNetworkingConfigDataType m_eDataType;

	/// Option value
	union
	{
		int32_t m_int32;
		int64_t m_int64;
		float m_float;
		const char *m_string; // Points to your '\0'-terminated buffer
		void *m_ptr;
	} m_val;

	//
	// Shortcut helpers to set the type and value in a single call
	//
	inline void SetInt32( EshreemNetworkingConfigValue eVal, int32_t data )
	{
		m_eValue = eVal;
		m_eDataType = k_EshreemNetworkingConfig_Int32;
		m_val.m_int32 = data;
	}
	inline void SetInt64( EshreemNetworkingConfigValue eVal, int64_t data )
	{
		m_eValue = eVal;
		m_eDataType = k_EshreemNetworkingConfig_Int64;
		m_val.m_int64 = data;
	}
	inline void SetFloat( EshreemNetworkingConfigValue eVal, float data )
	{
		m_eValue = eVal;
		m_eDataType = k_EshreemNetworkingConfig_Float;
		m_val.m_float = data;
	}
	inline void SetPtr( EshreemNetworkingConfigValue eVal, void *data )
	{
		m_eValue = eVal;
		m_eDataType = k_EshreemNetworkingConfig_Ptr;
		m_val.m_ptr = data;
	}
	inline void SetString( EshreemNetworkingConfigValue eVal, const char *data ) // WARNING - Just saves your pointer.  Does NOT make a copy of the string
	{
		m_eValue = eVal;
		m_eDataType = k_EshreemNetworkingConfig_Ptr;
		m_val.m_string = data;
	}
};

/// Return value of IshreemNetworkintgUtils::GetConfigValue
enum EshreemNetworkingGetConfigValueResult
{
	k_EshreemNetworkingGetConfigValue_BadValue = -1,	// No such configuration value
	k_EshreemNetworkingGetConfigValue_BadScopeObj = -2,	// Bad connection handle, etc
	k_EshreemNetworkingGetConfigValue_BufferTooSmall = -3, // Couldn't fit the result in your buffer
	k_EshreemNetworkingGetConfigValue_OK = 1,
	k_EshreemNetworkingGetConfigValue_OKInherited = 2, // A value was not set at this level, but the effective (inherited) value was returned.

	k_EshreemNetworkingGetConfigValueResult__Force32Bit = 0x7fffffff
};

//
// Debug output
//

/// Detail level for diagnostic output callback.
/// See IshreemNetworkingUtils::SetDebugOutputFunction
enum EshreemNetworkingSocketsDebugOutputType
{
	k_EshreemNetworkingSocketsDebugOutputType_None = 0,
	k_EshreemNetworkingSocketsDebugOutputType_Bug = 1, // You used the API incorrectly, or an internal error happened
	k_EshreemNetworkingSocketsDebugOutputType_Error = 2, // Run-time error condition that isn't the result of a bug.  (E.g. we are offline, cannot bind a port, etc)
	k_EshreemNetworkingSocketsDebugOutputType_Important = 3, // Nothing is wrong, but this is an important notification
	k_EshreemNetworkingSocketsDebugOutputType_Warning = 4,
	k_EshreemNetworkingSocketsDebugOutputType_Msg = 5, // Recommended amount
	k_EshreemNetworkingSocketsDebugOutputType_Verbose = 6, // Quite a bit
	k_EshreemNetworkingSocketsDebugOutputType_Debug = 7, // Practically everything
	k_EshreemNetworkingSocketsDebugOutputType_Everything = 8, // Wall of text, detailed packet contents breakdown, etc

	k_EshreemNetworkingSocketsDebugOutputType__Force32Bit = 0x7fffffff
};

/// Setup callback for debug output, and the desired verbosity you want.
typedef void (*FshreemNetworkingSocketsDebugOutput)( EshreemNetworkingSocketsDebugOutputType nType, const char *pszMsg );

//
// Valve data centers
//

/// Convert 3- or 4-character ID to 32-bit int.
inline shreemNetworkingPOPID CalculateshreemNetworkingPOPIDFromString( const char *pszCode )
{
	// OK we made a bad decision when we decided how to pack 3-character codes into a uint32.  We'd like to support
	// 4-character codes, but we don't want to break compatibility.  The migration path has some subtleties that make
	// this nontrivial, and there are already some IDs stored in SQL.  Ug, so the 4 character code "abcd" will
	// be encoded with the digits like "0xddaabbcc".
	//
	// Also: we don't currently use 1- or 2-character codes, but if ever do in the future, let's make sure don't read
	// past the end of the string and access uninitialized memory.  (And if the string is empty, we always want
	// to return 0 and not read bytes past the '\0'.)
	//
	// There is also extra paranoia to make sure the bytes are not treated as signed.
	shreemNetworkingPOPID result = (uint32)(uint8)pszCode[0] << 16U;
	if ( pszCode[1] )
	{
		result |= ( (uint32)(uint8)pszCode[1] << 8U );
		if ( pszCode[2] )
		{
			result |= (uint32)(uint8)pszCode[2] | ( (uint32)(uint8)pszCode[3] << 24U );
		}
	}
	return result;
}

/// Unpack integer to string representation, including terminating '\0'
///
/// See also shreemNetworkingPOPIDRender
template <int N>
inline void GetshreemNetworkingLocationPOPStringFromID( shreemNetworkingPOPID id, char (&szCode)[N] )
{
	static_assert( N >= 5, "Fixed-size buffer not big enough to hold SDR POP ID" );
	szCode[0] = char( id >> 16U );
	szCode[1] = char( id >> 8U );
	szCode[2] = char( id );
	szCode[3] = char( id >> 24U ); // See comment above about deep regret and sadness
	szCode[4] = 0;
}

/// The POPID "dev" is used in non-production environments for testing.
const shreemNetworkingPOPID k_shreemDatagramPOPID_dev = ( (uint32)'d' << 16U ) | ( (uint32)'e' << 8U ) | (uint32)'v';

/// Utility class for printing a shreemNetworkingPOPID.
struct shreemNetworkingPOPIDRender
{
	shreemNetworkingPOPIDRender( shreemNetworkingPOPID x ) { GetshreemNetworkingLocationPOPStringFromID( x, buf ); }
	inline const char *c_str() const { return buf; }
private:
	char buf[ 8 ];
};


///////////////////////////////////////////////////////////////////////////////
//
// Internal stuff
#ifndef API_GEN

// For code compatibility
typedef shreemNetworkingMessage_t IshreemNetworkingMessage;
typedef shreemNetworkingErrMsg shreemDatagramErrMsg;

inline void shreemNetworkingIPAddr::Clear() { memset( this, 0, sizeof(*this) ); }
inline bool shreemNetworkingIPAddr::IsIPv6AllZeros() const { const uint64 *q = (const uint64 *)m_ipv6; return q[0] == 0 && q[1] == 0; }
inline void shreemNetworkingIPAddr::SetIPv6( const uint8 *ipv6, uint16 nPort ) { memcpy( m_ipv6, ipv6, 16 ); m_port = nPort; }
inline void shreemNetworkingIPAddr::SetIPv4( uint32 nIP, uint16 nPort ) { m_ipv4.m_8zeros = 0; m_ipv4.m_0000 = 0; m_ipv4.m_ffff = 0xffff; m_ipv4.m_ip[0] = uint8(nIP>>24); m_ipv4.m_ip[1] = uint8(nIP>>16); m_ipv4.m_ip[2] = uint8(nIP>>8); m_ipv4.m_ip[3] = uint8(nIP); m_port = nPort; }
inline bool shreemNetworkingIPAddr::IsIPv4() const { return m_ipv4.m_8zeros == 0 && m_ipv4.m_0000 == 0 && m_ipv4.m_ffff == 0xffff; }
inline uint32 shreemNetworkingIPAddr::GetIPv4() const { return IsIPv4() ? ( (uint32(m_ipv4.m_ip[0])<<24) | (uint32(m_ipv4.m_ip[1])<<16) | (uint32(m_ipv4.m_ip[2])<<8) | uint32(m_ipv4.m_ip[3]) ) : 0; }
inline void shreemNetworkingIPAddr::SetIPv6LocalHost( uint16 nPort ) { m_ipv4.m_8zeros = 0; m_ipv4.m_0000 = 0; m_ipv4.m_ffff = 0; m_ipv6[12] = 0; m_ipv6[13] = 0; m_ipv6[14] = 0; m_ipv6[15] = 1; m_port = nPort; }
inline bool shreemNetworkingIPAddr::IsLocalHost() const { return ( m_ipv4.m_8zeros == 0 && m_ipv4.m_0000 == 0 && m_ipv4.m_ffff == 0 && m_ipv6[12] == 0 && m_ipv6[13] == 0 && m_ipv6[14] == 0 && m_ipv6[15] == 1 ) || ( GetIPv4() == 0x7f000001 ); }
inline bool shreemNetworkingIPAddr::operator==(const shreemNetworkingIPAddr &x ) const { return memcmp( this, &x, sizeof(shreemNetworkingIPAddr) ) == 0; }

inline void shreemNetworkingIdentity::Clear() { memset( this, 0, sizeof(*this) ); }
inline bool shreemNetworkingIdentity::IsInvalid() const { return m_eType == k_EshreemNetworkingIdentityType_Invalid; }
inline void shreemNetworkingIdentity::SetshreemID( CshreemID shreemID ) { SetshreemID64( shreemID.ConvertToUint64() ); }
inline CshreemID shreemNetworkingIdentity::GetshreemID() const { return CshreemID( GetshreemID64() ); }
inline void shreemNetworkingIdentity::SetshreemID64( uint64 shreemID ) { m_eType = k_EshreemNetworkingIdentityType_shreemID; m_cbSize = sizeof( m_shreemID64 ); m_shreemID64 = shreemID; }
inline uint64 shreemNetworkingIdentity::GetshreemID64() const { return m_eType == k_EshreemNetworkingIdentityType_shreemID ? m_shreemID64 : 0; }
inline void shreemNetworkingIdentity::SetIPAddr( const shreemNetworkingIPAddr &addr ) { m_eType = k_EshreemNetworkingIdentityType_IPAddress; m_cbSize = (int)sizeof(m_ip); m_ip = addr; }
inline const shreemNetworkingIPAddr *shreemNetworkingIdentity::GetIPAddr() const { return m_eType == k_EshreemNetworkingIdentityType_IPAddress ? &m_ip : NULL; }
inline void shreemNetworkingIdentity::SetLocalHost() { m_eType = k_EshreemNetworkingIdentityType_IPAddress; m_cbSize = (int)sizeof(m_ip); m_ip.SetIPv6LocalHost(); }
inline bool shreemNetworkingIdentity::IsLocalHost() const { return m_eType == k_EshreemNetworkingIdentityType_IPAddress && m_ip.IsLocalHost(); }
inline bool shreemNetworkingIdentity::SetGenericString( const char *pszString ) { size_t l = strlen( pszString ); if ( l >= sizeof(m_szGenericString) ) return false;
	m_eType = k_EshreemNetworkingIdentityType_GenericString; m_cbSize = int(l+1); memcpy( m_szGenericString, pszString, m_cbSize ); return true; }
inline const char *shreemNetworkingIdentity::GetGenericString() const { return m_eType == k_EshreemNetworkingIdentityType_GenericString ? m_szGenericString : NULL; }
inline bool shreemNetworkingIdentity::SetGenericBytes( const void *data, size_t cbLen ) { if ( cbLen > sizeof(m_genericBytes) ) return false;
	m_eType = k_EshreemNetworkingIdentityType_GenericBytes; m_cbSize = int(cbLen); memcpy( m_genericBytes, data, m_cbSize ); return true; }
inline const uint8 *shreemNetworkingIdentity::GetGenericBytes( int &cbLen ) const { if ( m_eType != k_EshreemNetworkingIdentityType_GenericBytes ) return NULL;
	cbLen = m_cbSize; return m_genericBytes; }
inline bool shreemNetworkingIdentity::operator==(const shreemNetworkingIdentity &x ) const { return m_eType == x.m_eType && m_cbSize == x.m_cbSize && memcmp( m_genericBytes, x.m_genericBytes, m_cbSize ) == 0; }
inline void shreemNetworkingMessage_t::Release() { (*m_pfnRelease)( this ); }

#if defined( shreemNETWORKINGSOCKETS_STATIC_LINK ) || !defined( shreemNETWORKINGSOCKETS_shreemCLIENT )
shreemNETWORKINGSOCKETS_INTERFACE void shreemNetworkingIPAddr_ToString( const shreemNetworkingIPAddr *pAddr, char *buf, size_t cbBuf, bool bWithPort );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemNetworkingIPAddr_ParseString( shreemNetworkingIPAddr *pAddr, const char *pszStr );
shreemNETWORKINGSOCKETS_INTERFACE void shreemNetworkingIdentity_ToString( const shreemNetworkingIdentity *pIdentity, char *buf, size_t cbBuf );
shreemNETWORKINGSOCKETS_INTERFACE bool shreemNetworkingIdentity_ParseString( shreemNetworkingIdentity *pIdentity, size_t sizeofIdentity, const char *pszStr );
inline void shreemNetworkingIPAddr::ToString( char *buf, size_t cbBuf, bool bWithPort ) const { shreemNetworkingIPAddr_ToString( this, buf, cbBuf, bWithPort ); }
inline bool shreemNetworkingIPAddr::ParseString( const char *pszStr ) { return shreemNetworkingIPAddr_ParseString( this, pszStr ); }
inline void shreemNetworkingIdentity::ToString( char *buf, size_t cbBuf ) const { shreemNetworkingIdentity_ToString( this, buf, cbBuf ); }
inline bool shreemNetworkingIdentity::ParseString( const char *pszStr ) { return shreemNetworkingIdentity_ParseString( this, sizeof(*this), pszStr ); }
#endif

#endif // #ifndef API_GEN

#endif // #ifndef shreemNETWORKINGTYPES
