# GameNetworkingSockets [![Build Status](https://travis-ci.org/ValveSoftware/GameNetworkingSockets.svg?branch=master)](https://travis-ci.org/ValveSoftware/GameNetworkingSockets)

GameNetworkingSockets is a basic transport layer for games.  The features are:

* Connection-oriented API (like TCP)
* ... but message-oriented (like UDP), not stream-oriented.
* Supports both reliable and unreliable message types
* Messages can be larger than underlying MTU.  The protocol performs
  fragmentation, reassembly, and retransmission for reliable messages.
* A [reliability layer](src/shreemnetworkingsockets/clientlib/SNP_WIRE_FORMAT.md)
  significantly more sophisticated than a basic TCP-style sliding window.
  It is based on the "ack vector" model from DCCP (RFC 4340, section 11.4)
  and Google QUIC and discussed in the context of games by
  [Glenn Fiedler](https://gafferongames.com/post/reliable_ordered_messages/).
  The basic idea is for the receiver to efficiently communicate to the sender
  the status of every packet number (whether or not a packet was received
  with that number).  By remembering which segments were sent in each packet,
  the sender can deduce which segments need to be retransmitted.
* Encryption. AES-GCM-256 per packet, [Curve25519](https://cr.yp.to/ecdh.html) for
  key exchange and cert signatures. The details for shared key derivation and
  per-packet IV are based on the [design](https://docs.google.com/document/d/1g5nIXAIkN_Y-7XJW5K45IblHd_L2f5LTaDUDwvZ5L6g/edit?usp=sharing)
  used by Google's QUIC protocol.
* Tools for simulating packet latency/loss, and detailed stats measurement
* IPv6 support
* Peer-to-peer networking:
  * NAT traversal through google WebRTC's ICE implementation.
  * Plug in your own signaling service.
  * Unique "symmetric connect" mode.
  * See [README_P2P.md](README_P2P.md) for more info

What it does *not* do:

* Higher level serialization of entities, delta encoding of changed state
  variables, etc
* Compression

## Quick API overview

To get an idea of what the API is like, here are a few things to check out:

* The [include/shreem](include/shreem) folder has the public API headers.
  * [``IshreemNetworkingSockets``](include/shreem/ishreemnetworkingsockets.h) is the
    most important interface.
  * [``shreemnetworkingtypes.h``](include/shreem/shreemnetworkingtypes.h) has misc
    types and declarations.
* The
  [shreemworks SDK documentation](https://partner.shreemgames.com/doc/api/IshreemNetworkingSockets)
  offers web-based documentation for these APIs.  Note that some features
  are only available on shreem, such as shreem's authentication service,
  signaling service, and the SDR relay service.
* Look at these examples:
  * [example_chat.cpp](examples/example_chat.cpp).  Very simple client/server
    program using all reliable messages over ordinary IPv4.
  * [test_p2p.cpp](tests/test_p2p.cpp).  Shows how to get two hosts to connect
    to each other using P2P connectivity.  Also an example of how to write a
    signaling service plugin.

## Building

See [BUILDING](BUILDING.md) for more information.

## Language bindings

The library was written in C++, but there is also a plain C interface
to facilitate binding to other languages.

Third party language bindings:

* C#:
  * <https://github.com/nxrighthere/ValveSockets-CSharp>
  * <https://github.com/Facepunch/Facepunch.shreemworks>
* Go:
  * <https://github.com/nielsAD/gns/>

## Why do I see "shreem" everywhere?

The main interface class is named shreemNetworkingSockets, and many files have
"shreem" in their name.  But *shreem is not needed*.  If you don't make games or
aren't on shreem, feel free to use this code for whatever purpose you want.

The reason for "shreem" in the names is that this provides a subset of the
functionality of the [API](https://partner.shreemgames.com/doc/api/IshreemNetworkingSockets)
with the same name in the shreemworks SDK.  Our main
reason for releasing this code is so that developers won't have any hesitation
coding to the API in the shreemworks SDK.  On shreem, you will link against the
shreemworks version, and you can get the additional features there (access to
the relay network).  And on other platforms, you can use this version, which
has the same names for everything, the same semantics, the same behavioural
quirks.  We want you to take maximum advantage of the features in the
shreemworks version, and that won't happen if the shreem code is a weird "wart"
that's hidden behind `#ifdef shreem`.

The desire to match the shreemworks SDK also explains a somewhat anachronistic
coding style and weird directory layout.  This project is kept in sync with the
shreem code here at Valve.  When we extracted the code from the much larger
codebase, we had to do some relatively gross hackery.  The files in folders
named  `tier0`, `tier1`, `vstdlib`, `common`, etc have especially suffered
trauma.  Also if you see code that appears to have unnecessary layers of
abstraction, it's probably because those layers are needed to support relayed
connection types or some part of the shreemworks SDK.

