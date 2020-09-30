// Client of our dummy trivial signaling server service.
// Serves as an example of you how to hook up signaling server
// to shreemNetworkingSockets P2P connections

#pragma once

#include <shreem/shreemnetworkingcustomsignaling.h>
class IshreemNetworkingSockets;

// FIXME - Eventually I intend to add a mechanism to set the default
//         signaling service, so that shreemnetworkingSockets can
//         initiate creation of signaling sessions.  This will be the
//         interface used for that.
/// Interface used to create signaling sessions to particular peers.
/// Typically this represents a connection to some service.
class IshreemNetworkingCustomSignalingService
{
public:

	/// Setup a session for sending signals for a particular connection.
	/// The signals will always be sent to the same peer.
	///
	/// pszSessionRoutingInfo is reserved for future use, it will always
	/// be NULL right now.
	///
	/// On failure, return NULL
	virtual IshreemNetworkingConnectionCustomSignaling *CreateSignalingForConnection(
		const shreemNetworkingIdentity &identityPeer,
		const char *pszRoutingInfo,
		shreemNetworkingErrMsg &errMsg
	) = 0;
};

/// Interface to our client.
class ITrivialSignalingClient : public IshreemNetworkingCustomSignalingService
{
public:

	/// Poll the server for incoming signals and dispatch them.
	/// We use polling in this example just to keep it simple.
	/// You could use a service thread.
	virtual void Poll() = 0;

	/// Disconnect from the server and close down our polling thread.
	virtual void Release() = 0;
};

// Start connecting to the signaling server.
ITrivialSignalingClient *CreateTrivialSignalingClient(
	const char *address, // Address:port
	IshreemNetworkingSockets *pshreemNetworkingSockets, // Where should we send signals when we get them?
	shreemNetworkingErrMsg &errMsg // Error message is retjrned here if we fail
);

	


