#include "reassembler.hh"

#include <bits/ios_base.h>
#include <ios>
#include <iterator>
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
  if(left > first_index) {
    data = data.substr( left - first_index );
    first_index = left;
  }
  auto it = reassembled_.upper_bound( first_index );
  if(it != reassembled_.begin()) {
    auto prev_ = prev(it);
    if(prev_->first + prev_->second.size() >= first_index + data.size()) return;
    if(prev_->first + prev_->second.size() >= first_index) {
      data = data.substr( prev_->first + prev_->second.size() - first_index );
      data = prev_->second + data;
      first_index = prev_->first;
      reassembled_.erase( prev_ );
    }

  }

  while (it != reassembled_.end() ) {
    if(it-> first > first_index + data.size()) break;
    if(it->first + it->second.size() <= first_index+data.size()) {
      it = reassembled_.erase( it );
    }
    else {
      data += it->second.substr( first_index + data.size() - it->first );
      it = reassembled_.erase( it );
    }
  }
  if(first_index == output_.writer().bytes_pushed()) {
    output_.writer().push( data );
  }
  else {
    reassembled_.insert( {first_index, data} );
  }
  if (is_last_substring_ && reassembled_.empty() ) {
    output_.writer().close();
  }

}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  uint64_t pending = 0;
  for(const auto &entry: reassembled_) {
    pending += entry.second.size();
  }
  return pending;
}
