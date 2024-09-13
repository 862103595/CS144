#include "tcp_sender.hh"
#include "tcp_config.hh"

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return total_outstanding;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return retran_uint64_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  // Your code here.
  if(has_fin) return;
  uint64_t pretend = (window == 0) ? 1 : window;
  uint64_t left_window = pretend - total_outstanding;
  if(window < total_outstanding) left_window = 0;
  while(left_window > 0 && !has_fin) {
    TCPSenderMessage new_message = make_empty_message();
    if(!has_sent) { new_message.SYN = true;  has_sent = true;}
    string payload;
    read( input_.reader(), min(left_window-new_message.sequence_length(), TCPConfig::MAX_PAYLOAD_SIZE), payload );
    new_message.payload = std::move( payload );
    if(input_.reader().is_finished() && left_window > new_message.sequence_length()) { new_message.FIN = true; has_fin = true;}

    if(new_message.sequence_length() == 0) return;
    total_outstanding += new_message.sequence_length();
    next_abs += new_message.sequence_length();
    outstanding.push( new_message );
    transmit(new_message);
    left_window -= new_message.sequence_length();
    timer.set_opened();
  }

}

TCPSenderMessage TCPSender::make_empty_message() const
{
  // Your code here.
  return {isn_ + next_abs, false, std::string(), false, input_.has_error()};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  window = msg.window_size;
  if(msg.RST) {input_.set_error();}
  if(!msg.ackno.has_value()) { return; }
  uint64_t new_ackno = msg.ackno->unwrap( isn_, input_.reader().bytes_popped() );
  if(new_ackno > next_abs) return;


  if(new_ackno <= ackno) return;
  ackno = new_ackno;

  if(outstanding.empty()) return;
  TCPSenderMessage front = outstanding.front();
  while(front.seqno.unwrap( isn_, ackno ) + front.sequence_length() <= ackno) {
    total_outstanding -= front.sequence_length();
    outstanding.pop();
    if(outstanding.empty()) break;
    front = outstanding.front();
  }
  timer.reset();
  if(outstanding.empty()) {timer.set_closed();}
  else { timer.set_zero();}
  retran_uint64_ = 0;
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // Your code here.
  if(timer.is_closed()) {
    return;
  }
  if(timer.tick( ms_since_last_tick )) {
    transmit(outstanding.front());

    if(window != 0) {
      retran_uint64_ +=1;
      timer.double_RTO();
    }

    timer.set_zero();
  }
}
