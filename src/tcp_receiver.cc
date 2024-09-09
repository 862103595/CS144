#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Your code here.
  if(message.RST) {
    error_ = true;
    reassembler_.reader().set_error();
  }
  if (message.SYN) {
    SYN = true;
    zero_point = message.seqno;
      // Initialize checkpoint when SYN is received
  }
  if(!SYN ) return;

  // Adjust checkpoint for non-SYN packets

  uint64_t index = message.seqno.unwrap( zero_point, reassembler_.writer().bytes_pushed() ) -1 ;
  if(message.SYN) index = 0;
  reassembler_.insert(index, message.payload, message.FIN);
}

TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  uint64_t n = reassembler_.writer().bytes_pushed() + 1;
  if(reassembler_.writer().is_closed()) n+=1;
  uint64_t available = reassembler_.writer().available_capacity();
  if (available > UINT16_MAX) {
    available = UINT16_MAX;
  }
  uint16_t window = available;
  std::optional<Wrap32> ackno;
  if(SYN) ackno = Wrap32::wrap( n, zero_point );
  return TCPReceiverMessage{ackno, window, error_ || reassembler_.writer().has_error()};
}
