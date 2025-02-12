#pragma once

#include <stdexcept>
#include <memory>

#include <quicr/encode.h>
#include <quicr/message_buffer.h>
#include <quicr/quicr_common.h>
#include <quicr/quicr_server_delegate.h>
#include <quicr/quicr_server_session.h>
#include <transport/transport.h>

/*
 * API for implementing server side of the QUICR protocol
 */
namespace quicr {

// Exception that may be thrown if there is a critical error
class QuicRServerException : public std::runtime_error
{
  using std::runtime_error::runtime_error;
};

class QuicRServer
{
public:
  /**
   * Start the  QUICR server at the port specified.
   *
   * @param relayInfo        : Relay Information to be used by the transport
   * @param tconfig          : Transport configuration
   * @param delegate         : Server delegate
   * @param logger           : Log handler instance. Will be used by transport
   *                           quicr api
   */
  QuicRServer(RelayInfo& relayInfo,
              qtransport::TransportConfig tconfig,
              ServerDelegate& delegate,
              qtransport::LogHandler& logger);

  /**
   * API for unit test cases .
   */
  QuicRServer(
    std::shared_ptr<qtransport::ITransport> transport,
    ServerDelegate& delegate /* TODO: Considering shared or weak pointer */,
    qtransport::LogHandler& logger);

  // Transport APIs
  bool is_transport_ready();

  /**
   * @brief Run Server API event loop
   *
   * @details This method will open listening sockets and run an event loop
   *    for callbacks.
   *
   * @returns true if error, false if no error
   */
  bool run();

  /**
   * @brief Send publish intent response
   *
   * @param quicr_namespace       : Identifies QUICR namespace
   * @param result                : Status of Publish Intetn
   *
   * @details Entities processing the Subscribe Request MUST validate the
   * request
   * @todo: Add payload with origin signed blob
   */
  void publishIntentResponse(const quicr::Namespace& quicr_namespace,
                             const PublishIntentResult& result);

  /**
   * @brief Send subscribe response
   *
   * @details Entities processing the Subscribe Request MUST validate the
   * request
   *
   * @param subscriber_id         : Subscriber ID to send the message to
   * @param quicr_namespace       : Identifies QUICR namespace
   * @param result                : Status of Subscribe operation
   *
   */
  void subscribeResponse(const uint64_t& subscriber_id,
                         const quicr::Namespace& quicr_namespace,
                         const SubscribeResult& result);

  /**
   * @brief Send subscription end message
   *
   * @details  Subscription can terminate when a publisher terminated
   *           the stream or subscription timeout, upon unsubscribe,
   *           or other application reasons
   *
   * @param subscriber_id         : Subscriber ID to send the message to
   * @param quicr_namespace       : Identifies QUICR namespace
   * @param reason                : Reason of Subscribe End operation
   *
   */
  void subscriptionEnded(const uint64_t& subscriber_id,
                         const quicr::Namespace& quicr_namespace,
                         const SubscribeResult::SubscribeStatus& reason);

  /**
   * @brief Send a named QUICR media object
   *
   * @param subscriber_id            : Subscriber ID to send the message to
   * @param use_reliable_transport   : Indicates the preference for the object's
   *                                   transport, if forwarded.
   * @param priority                 : Identifies the relative priority of the
   *                                   current object
   * @param expiry_age_ms            : Time hint for the object to be in cache
   *                                   before being purged after reception
   * @param datagram                 : QuicR Publish Datagram to send
   *
   */
  void sendNamedObject(const uint64_t& subscriber_id,
                       bool use_reliable_transport,
                       uint8_t priority,
                       uint16_t expiry_age_ms,
                       const messages::PublishDatagram& datagram);

protected:

  std::unique_ptr<QuicRServerSession> server_session;

};

} // namespace quicr
