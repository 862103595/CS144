#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <queue>
#include <map>

class RetransmissionTimer
{
public:
  explicit RetransmissionTimer(uint64_t initial_RTO_ms):initial_RTO(initial_RTO_ms) ,RTO(initial_RTO_ms) {};

  void set_closed() {closed_ = true;}

  void set_opened() {closed_ = false;}

  bool is_closed() const {return closed_;}

  bool tick(uint64_t ms_since_last_tick)
  {
    retransmission_time += ms_since_last_tick;
    if (retransmission_time >= RTO) return true;
    return  false;
  }

  void double_RTO()
  {
    RTO *= 2;
  }

  void set_zero()
  {
    retransmission_time = 0;
  }

  void reset()
  {
    RTO = initial_RTO;
  }

private:
  const uint64_t initial_RTO;
  uint64_t RTO;
  uint64_t retransmission_time{ };
  bool closed_ {  true };
};

class TCPSender
{
public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( ByteStream&& input, Wrap32 isn, uint64_t initial_RTO_ms )
    : input_( std::move( input ) ), isn_( isn ), initial_RTO_ms_( initial_RTO_ms ), timer(initial_RTO_ms)
  {}

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage make_empty_message() const;

  /* Receive and process a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Type of the `transmit` function that the push and tick methods can use to send messages */
  using TransmitFunction = std::function<void( const TCPSenderMessage& )>;

  /* Push bytes from the outbound stream */
  void push( const TransmitFunction& transmit );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called */
  void tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit );

  // Accessors
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding/?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
  Writer& writer() { return input_.writer(); }
  const Writer& writer() const { return input_.writer(); }

  // Access input stream reader, but const-only (can't read from outside)
  const Reader& reader() const { return input_.reader(); }

private:
  // Variables initialized in constructor
  ByteStream input_;
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;
  std::queue<TCPSenderMessage> outstanding {};
  uint64_t retran_uint64_ {};
  uint64_t total_outstanding {};
  uint64_t window {1};
  u_int64_t ackno {};
  u_int64_t next_abs { };
  RetransmissionTimer timer;
  bool has_sent { };
  bool has_fin { };
};
