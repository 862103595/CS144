#include "wrapping_integers.hh"

#include <valarray>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  return zero_point + n;
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  uint64_t pow2_32 = 1ULL << 32;
  uint64_t one = raw_value_ - zero_point.raw_value_;
  if (checkpoint <= one) return one;
  uint64_t diff = checkpoint - one;
  one = one + diff / pow2_32 * pow2_32;
  uint64_t big = one + pow2_32;
  return (checkpoint - one > big - checkpoint) ? big : one;
}

