// Client of our dummy trivial signaling server service.
// Serves as an example of you how to hook up signaling server
// to shreemNetworkingSockets P2P connections

#include "../tests/test_common.h"

#include <string>
#include <mutex>
#include <assert.h>

#include "trivial_signaling_client.h"
#include <shreem/ishreemnetworkingsockets.h>
#include <shreem/ishreemnetworkingutils.h>

#ifdef POSIX
	#include <unistd.h>
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <netinet/in.h>
	#include <netdb.h>
	typedef int SOCKET;
	constexpr SOCKET INVALID_SOCKET = -1;
	inline void closesocket( SOCKET s ) { close(s); }
	inline int GetSocketError() { return errno; }
#endif
#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	typedef int socklen_t;
	inline int GetSocketError() { return WSAGetLastError(); }
#endif

inline int HexDigitVal( char c )
{
	if ( '0' <= c && c <= '9' )
		return c - '0';
	if ( 'a' <= c && c <= 'f' )
		return c - 'a' + 0xa;
	if ( 'A' <= c && c <= 'F' )
		return c - 'A' + 0xa;
	return -1;
}

/// Implementation of ITrivialSignalingClient
class CTrivialSignalingClient : public ITrivialSignalingClient
{

	// This is the thing we'll actually create to send signals for a particular
	// connection.
	struct ConnectionSignaling : IshreemNetworkingConnectionCustomSignaling
	{
		CTrivialSignalingClient *const m_pOwner;
		std::string const m_sPeerIdentity; // Save off the string encoding of the identity we're talking to

		ConnectionSignaling( CTrivialSignalingClient *owner, const char *pszPeerIdentity )
		: m_pOwner( owner )
		, m_sPeerIdentity( pszPeerIdentity )
		{
		}

		//
		// Implements IshreemNetworkingConnectionCustomSignaling
		//

		// This is called from shreemNetworkingSockets to send a signal.  This could be called from any thread,
		// so we need to be threadsafe, and avoid duoing slow stuff or calling back into shreemNetworkingSockets
		virtual bool SendSignal( HshreemNetConnection hConn, const shreemNetConnectionInfo_t &info, const void *pMsg, int cbMsg ) override
		{

			// We'll use a dumb hex encoding.
			std::string signal;
			signal.reserve( m_sPeerIdentity.length() + cbMsg*2 + 4 );
			signal.append( m_sPeerIdentity );
			signal.push_back( ' ' );
			for ( const uint8_t *p = (const uint8_t *)pMsg ; cbMsg > 0 ; --cbMsg, ++p )
			{
				static const char hexdigit[] = "0123456789abcdef";
				signal.push_back( hexdigit[ *p >> 4U ] );
				signal.push_back( hexdigit[ *p & 0xf ] );
			}
			signal.push_back('\n');

			m_pOwner->Send( signal );
			return true;
		}

		// Self destruct.  This will be called by shreemNetworkingSockets when it's done with us.
		virtual void Release() override
		{
			delete this;
		}
	};

	sockaddr_storage m_adrServer;
	size_t const m_adrServerSize;
	IshreemNetworkingSockets *const m_pshreemNetworkingSockets;
	std::string m_sGreeting;

	std::mutex sockMutex;
	SOCKET m_sock;
	std::string m_sBufferedData;

	void CloseSocket()
	{
		if ( m_sock != INVALID_SOCKET )
		{
			closesocket( m_sock );
			m_sock = INVALID_SOCKET;
		}
		m_sBufferedData.clear();
	}

	void Connect()
	{
		CloseSocket();

		int sockType = SOCK_STREAM;
		#ifdef LINUX
			sockType |= SOCK_CLOEXEC;
		#endif
		#if !defined( _WIN32 )
			sockType |= SOCK_NONBLOCK;
		#endif
		m_sock = socket( m_adrServer.ss_family, sockType, IPPROTO_TCP );
		if ( m_sock == INVALID_SOCKET )
		{
			TEST_Printf( "socket() failed, error=%d\n", GetSocketError() );
			return;
		}
		#ifdef _WIN32
			unsigned long opt = 1;
			if ( ioctlsocket( m_sock, FIONBIO, &opt ) == -1 )
			{
				CloseSocket();
				TEST_Printf( "ioctlsocket() failed, error=%d\n", GetSocketError() );
				return;
			}
		#endif

		connect( m_sock, (const sockaddr *)&m_adrServer, (socklen_t )m_adrServerSize );

		// And immediate send our greeting.  This just puts in in the buffer and
		// it will go out once the socket connects.
		Send( m_sGreeting );
	}

public:
	CTrivialSignalingClient( const sockaddr *adrServer, size_t adrServerSize, IshreemNetworkingSockets *pshreemNetworkingSockets )
	: m_adrServerSize( adrServerSize ), m_pshreemNetworkingSockets( pshreemNetworkingSockets )
	{
		memcpy( &m_adrServer, adrServer, adrServerSize );
		m_sock = INVALID_SOCKET;

		// Save off our identity
		shreemNetworkingIdentity identitySelf; identitySelf.Clear();
		pshreemNetworkingSockets->GetIdentity( &identitySelf );
		assert( !identitySelf.IsInvalid() );
		assert( !identitySelf.IsLocalHost() ); // We need something more specific than that
		m_sGreeting = shreemNetworkingIdentityRender( identitySelf ).c_str();
		assert( strchr( m_sGreeting.c_str(), ' ' ) == nullptr ); // Our protocol is dumb and doesn't support this
		m_sGreeting.push_back( '\n' );

		// Begin connecting immediately
		Connect();
	}

	// Send the signal.
	void Send( const std::string &s )
	{
		assert( s.length() > 0 && s[ s.length()-1 ] == '\n' ); // All of our signals are '\n'-terminated

		sockMutex.lock();
		if ( m_sock != INVALID_SOCKET )
		{
			int l = s.length();
			int r = ::send( m_sock, s.c_str(), l, 0 );
			if ( r != l && r != 0 )
			{
				// Socket hosed, or we sent a partial signal.
				// We need to restart connection
				TEST_Printf( "Failed to send %d bytes to trivial signaling server.  send() returned %d, errno=%d.  Closing and restarting connection.\n",
					l, r, GetSocketError() );
				CloseSocket();
			}
		}
		sockMutex.unlock();
	}

	//
	// Implements IshreemNetworkingCustomSignalingService
	//
	virtual IshreemNetworkingConnectionCustomSignaling *CreateSignalingForConnection(
		const shreemNetworkingIdentity &identityPeer,
		const char *pszRoutingInfo,
		shreemNetworkingErrMsg &errMsg
	) override {
		shreemNetworkingIdentityRender sIdentityPeer( identityPeer );

		// FIXME - here we really ouight to confirm that the string version of the
		// identity does not have spaces, since our protocol doesn't permit it.
		TEST_Printf( "Creating signaling session for peer '%s'\n", sIdentityPeer.c_str() );

		return new ConnectionSignaling( this, sIdentityPeer.c_str() );
	}

	virtual void Poll() override
	{
		// Drain the socket into the buffer, and check for reconnecting
		sockMutex.lock();
		if ( m_sock == INVALID_SOCKET )
		{
			Connect();
		}
		else
		{
			for (;;)
			{
				char buf[256];
				int r = recv( m_sock, buf, sizeof(buf), 0 );
				if ( r == 0 )
					break;
				if ( r < 0 )
				{
					int e = GetSocketError();
					if ( e != EAGAIN && e != EWOULDBLOCK )
					{
						TEST_Printf( "Failed to recv from trivial signaling server.  recv() returned %d, errno=%d.  Closing and restarting connection\n", r, e );
						CloseSocket();
					}
					break;
				}

				m_sBufferedData.append( buf, r );
			}
		}

		// Release the lock now.  See the notes below about why it's very important
		// to release the lock early and not hold it while we try to dispatch the
		// received callbacks.
		sockMutex.unlock();

		// Now dispatch any buffered signals
		for (;;)
		{

			// Find end of line.  Do we have a complete signal?
			size_t l = m_sBufferedData.find( '\n' );
			if ( l == std::string::npos )
				break;

			// Locate the space that seperates [from] [payload]
			size_t spc = m_sBufferedData.find( ' ' );
			if ( spc != std::string::npos && spc < l )
			{

				// Hex decode the payload.  As it turns out, we actually don't
				// need the sender's identity.  The payload has everything needed
				// to process the message.  Maybe we should remove it from our
				// dummy signaling protocol?  It might be useful for debugging, tho.
				std::string data; data.reserve( ( l - spc ) / 2 );
				for ( size_t i = spc+1 ; i+2 <= l ; i += 2 )
				{
					int h = HexDigitVal( m_sBufferedData[i] );
					int l = HexDigitVal( m_sBufferedData[i+1] );
					if ( ( h | l ) & ~0xf )
					{
						// Failed hex decode.  Not a bug in our code here, but this is just example code, so we'll handle it this way
						assert( !"Failed hex decode from signaling server?!" );
						goto next_message;
					}
					data.push_back( (char)(h<<4 | l ) );
				}

				// Setup a context object that can respond if this signal is a connection request.
				struct Context : IshreemNetworkingCustomSignalingRecvContext
				{
					CTrivialSignalingClient *m_pOwner;

					virtual IshreemNetworkingConnectionCustomSignaling *OnConnectRequest(
						HshreemNetConnection hConn,
						const shreemNetworkingIdentity &identityPeer,
						int nLocalVirtualPort
					) override {

						// We will just always handle requests thorugh the usual listen socket state
						// machine.  See the docuemntation for this function for other behaviour we
						// might take.

						// Also, note that if there was routing/session info, it should have been in
						// our envelope that we know how to parse, and we should save it off in this
						// context object.
						shreemNetworkingErrMsg ignoreErrMsg;
						return m_pOwner->CreateSignalingForConnection( identityPeer, nullptr, ignoreErrMsg );
					}

					virtual void SendRejectionSignal(
						const shreemNetworkingIdentity &identityPeer,
						const void *pMsg, int cbMsg
					) override {

						// We'll just silently ignore all failures.  This is actually the more secure
						// Way to handle it in many cases.  Actively returning failure might allow
						// an attacker to just scrape random peers to see who is online.  If you know
						// the peer has a good reason for trying to connect, sending an active failure
						// can improve error handling and the UX, instead of relying on timeout.  But
						// just consider the security implications.
					}
				};
				Context context;
				context.m_pOwner = this;

				// Dispatch.
				// Remember: From inside this function, our context object might get callbacks.
				// And we might get asked to send signals, either now, or really at any time
				// from any thread!  If possible, avoid calling this function while holding locks.
				// To process this call, shreemnetworkingSockets will need take its own internal lock.
				// That lock may be held by another thread that is asking you to send a signal!  So
				// be warned that deadlocks are a possibility here.
				m_pshreemNetworkingSockets->ReceivedP2PCustomSignal( data.c_str(), (int)data.length(), &context );
			}

next_message:
			m_sBufferedData.erase( 0, l+1 );
		}
	}

	virtual void Release()
	{
		// NOTE: Here we are assuming that the calling code has already cleaned
		// up all the connections, to keep the example simple.
		CloseSocket();
	}
};

// Start connecting to the signaling server.
ITrivialSignalingClient *CreateTrivialSignalingClient(
	const char *pszServerAddress, // Address of the server.
	IshreemNetworkingSockets *pshreemNetworkingSockets, // Where should we send signals when we get them?
	shreemNetworkingErrMsg &errMsg // Error message is retjrned here if we fail
) {

	std::string sAddress( pszServerAddress );
	std::string sService;
	size_t colon = sAddress.find( ':' );
	if ( colon == std::string::npos )
	{
		sService = "10000"; // Default port
	}
	else
	{
		sService = sAddress.substr( colon+1 );
		sAddress.erase( colon );
	}

	// Resolve name synchronously
	addrinfo *pAddrInfo = nullptr;
	int r = getaddrinfo( sAddress.c_str(), sService.c_str(), nullptr, &pAddrInfo );
	if ( r != 0 || pAddrInfo == nullptr )
	{
		sprintf( errMsg, "Invalid/unknown server address.  getaddrinfo returned %d", r );
		return nullptr;
	}

	auto *pClient = new CTrivialSignalingClient( pAddrInfo->ai_addr, pAddrInfo->ai_addrlen, pshreemNetworkingSockets );

	freeaddrinfo( pAddrInfo );

	return pClient;
}

	


