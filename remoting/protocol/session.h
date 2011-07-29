// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_PROTOCOL_SESSION_H_
#define REMOTING_PROTOCOL_SESSION_H_

#include <string>

#include "base/callback.h"
#include "base/threading/non_thread_safe.h"
#include "remoting/protocol/buffered_socket_writer.h"
#include "remoting/protocol/session_config.h"

class Task;

namespace net {
class Socket;
class StreamSocket;
}  // namespace net

namespace remoting {
namespace protocol {

// Generic interface for Chromotocol connection used by both client and host.
// Provides access to the connection channels, but doesn't depend on the
// protocol used for each channel.
class Session : public base::NonThreadSafe {
 public:
  enum State {
    INITIALIZING,
    CONNECTING,
    CONNECTED,
    CLOSED,
    FAILED,
  };

  typedef Callback1<State>::Type StateChangeCallback;
  typedef base::Callback<void(const std::string&, net::StreamSocket*)>
      StreamChannelCallback;
  typedef base::Callback<void(const std::string&, net::Socket*)>
      DatagramChannelCallback;

  Session() { }
  virtual ~Session() { }

  // Set callback that is called when state of the connection is changed.
  // Must be called on the jingle thread only.
  virtual void SetStateChangeCallback(StateChangeCallback* callback) = 0;

  // Creates new channels for this connection. The specified callback
  // is called when then new channel is created and connected. The
  // callback is called with NULL if connection failed for any reason.
  // Ownership of the channel socket is given to the caller when the
  // callback is called. All channels must be destroyed before the
  // session is destroyed. Can be called only when in CONNECTING or
  // CONNECTED state.
  virtual void CreateStreamChannel(
      const std::string& name, const StreamChannelCallback& callback) = 0;
  virtual void CreateDatagramChannel(
      const std::string& name, const DatagramChannelCallback& callback) = 0;

  // TODO(sergeyu): Remove these methods, and use CreateChannel()
  // instead.
  virtual net::Socket* control_channel() = 0;
  virtual net::Socket* event_channel() = 0;
  virtual net::Socket* video_channel() = 0;
  virtual net::Socket* video_rtp_channel() = 0;
  virtual net::Socket* video_rtcp_channel() = 0;

  // JID of the other side.
  virtual const std::string& jid() = 0;

  // Configuration of the protocol that was sent or received in the
  // session-initiate jingle message. Returned pointer is valid until
  // connection is closed.
  virtual const CandidateSessionConfig* candidate_config() = 0;

  // Protocol configuration. Can be called only after session has been accepted.
  // Returned pointer is valid until connection is closed.
  virtual const SessionConfig* config() = 0;

  // Set protocol configuration for an incoming session. Must be called
  // on the host before the connection is accepted, from
  // ChromotocolServer::IncomingConnectionCallback. Ownership of |config| is
  // given to the connection.
  virtual void set_config(const SessionConfig* config) = 0;

  // The raw auth tokens from the session-initiate, or session-accept stanzas.
  virtual const std::string& initiator_token() = 0;
  virtual void set_initiator_token(const std::string& initiator_token) = 0;
  virtual const std::string& receiver_token() = 0;
  virtual void set_receiver_token(const std::string& receiver_token) = 0;

  // A shared secret to use to mutually-authenticate the SSL channels.
  virtual void set_shared_secret(const std::string& secret) = 0;
  virtual const std::string& shared_secret() = 0;

  // Closes connection. Callbacks are guaranteed not to be called
  // after this method returns. Must be called before the object is
  // destroyed, unless the state is set to FAILED or CLOSED.
  virtual void Close() = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(Session);
};

}  // namespace protocol
}  // namespace remoting

#endif  // REMOTING_PROTOCOL_SESSION_H_
