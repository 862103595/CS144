#include "reassembler.hh"

#include <bits/ios_base.h>
#include <ios>

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // Your code here.
  uint64_t left = output_.writer().bytes_pushed();
  uint64_t right = left + output_.writer().available_capacity();
  uint64_t last_index = first_index + data.size();
  if (last_index <= left) {
    if (is_last_substring && reassembled_.empty() ) {
      output_.writer().close();
    }
    return;
  }
  if (first_index  >= right) {
    return;
  }
  if(last_index <= right && is_last_substring) {
    is_last_substring_ = true;
  }
  if(last_index > right) {
    data.resize(right - first_index);
  }
  if(first_index <= left) {
    data = data.substr( left - first_index );
    output_.writer().push( data );
    for(auto i = left; i < left + data.size(); ++i) {
      if(reassembled_.contains( i )) {
        reassembled_.erase( i );
      }
    }
    uint64_t next_index = left + data.size();
    while(reassembled_.contains( next_index )) {
      output_.writer().push( reassembled_.at( next_index ) );
      reassembled_.erase( next_index );
      next_index++;
    }
  }
  else{
    for(auto it = data.begin(); it != data.end(); it++) {
      reassembled_[first_index + (it - data.begin())] = *it;
    }
  }

  if (is_last_substring_ && reassembled_.empty() ) {
    output_.writer().close();
  }

}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  uint64_t pending_bytes = 0;
  for (const auto& entry : reassembled_) {
    pending_bytes += entry.second.size();
  }
  return pending_bytes;
}
