#include <iostream>

#include "arp_message.hh"
#include "exception.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address ) << " and IP address "
       << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  // Your code here.
  if(cache.contains( next_hop.ipv4_numeric() )) {
    const EthernetAddress &dst = cache[next_hop.ipv4_numeric()].first;
    return transmit( {{dst, ethernet_address_, EthernetHeader::TYPE_IPv4}, serialize( dgram )} );
  }
  else {
    waiting[next_hop.ipv4_numeric()] =  dgram;
    if(!last_req_time.contains( next_hop.ipv4_numeric())) {
      ARPMessage arp;
      arp.opcode = ARPMessage::OPCODE_REQUEST;
      arp.sender_ethernet_address = ethernet_address_;
      arp.sender_ip_address = ip_address_.ipv4_numeric();
      arp.target_ip_address = next_hop.ipv4_numeric();
      transmit( {{ETHERNET_BROADCAST, ethernet_address_, EthernetHeader::TYPE_ARP}, serialize( arp )} );
      last_req_time[next_hop.ipv4_numeric()] = 0;
    }

  }

}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  // Your code here.
  if(frame.header.type == EthernetHeader::TYPE_IPv4 && frame.header.dst == ethernet_address_) {
    InternetDatagram data;
    if(parse( data, frame.payload )) {
      datagrams_received().push( data );
    }
  }
  else if(frame.header.type == EthernetHeader::TYPE_ARP) {
    ARPMessage arp;
    parse( arp, frame.payload );
    cache[arp.sender_ip_address] = {arp.sender_ethernet_address, 0};
    if(waiting.contains( arp.sender_ip_address )) {
      send_datagram( waiting[arp.sender_ip_address], Address::from_ipv4_numeric( arp.sender_ip_address ) );
      waiting.erase( arp.sender_ip_address );
    }
    if(arp.opcode == ARPMessage::OPCODE_REPLY) {
      cache[arp.target_ip_address] = {arp.target_ethernet_address, 0};
      if(waiting.contains( arp.target_ip_address )) {
        send_datagram( waiting[arp.target_ip_address], Address::from_ipv4_numeric( arp.target_ip_address ) );
        waiting.erase( arp.target_ip_address );
      }
    }
    if(arp.opcode == ARPMessage::OPCODE_REQUEST && arp.target_ip_address == ip_address_.ipv4_numeric()) {
      ARPMessage reply_arp;
      reply_arp.opcode = ARPMessage::OPCODE_REPLY;
      reply_arp.sender_ethernet_address = ethernet_address_;
      reply_arp.sender_ip_address = ip_address_.ipv4_numeric();
      reply_arp.target_ethernet_address = arp.sender_ethernet_address;
      reply_arp.target_ip_address = arp.sender_ip_address;
      transmit( {{arp.sender_ethernet_address, ethernet_address_, EthernetHeader::TYPE_ARP}, serialize( reply_arp )} );
    }
  }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  for(auto it = cache.begin(); it != cache.end(); ) {
    if(it->second.second + ms_since_last_tick > 30000) {
      it = cache.erase( it );
    }
    else {
      it->second.second += ms_since_last_tick;
      ++it;
    }
  }

  for(auto it = last_req_time.begin(); it != last_req_time.end(); ) {
    if(it->second + ms_since_last_tick > 5000) {
      it = last_req_time.erase( it );
    }
    else {
      it->second += ms_since_last_tick;
      ++it;
    }
  }
}
