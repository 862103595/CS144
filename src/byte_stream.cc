#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), error_( false ),
closed_( false ), buffer_(), total_popped_bytes_( 0 ), total_pushed_bytes_( 0 ){}

bool Writer::is_closed() const
{
  // Your code here.
  return closed_;
}

void Writer::push( string data )
{
  // Your code here.
  if ( is_closed() ) {
    return;
  }
  if(data.size() > available_capacity()) {
    data.resize(available_capacity());
  }
  buffer_.append( std::move( data ) );
  total_pushed_bytes_ += data.size();
}

void Writer::close()
{
  // Your code here.
  closed_ = true;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - buffer_.size();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return total_pushed_bytes_;
}

bool Reader::is_finished() const
{
  // Your code here.
  return closed_ && buffer_.empty();
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return total_popped_bytes_;
}

string_view Reader::peek() const
{
  // Your code here.
  if (buffer_.empty()) {
    return std::string_view();  // Return an empty view if nothing to peek.
  }
  return std::string_view(buffer_.data(), buffer_.size());
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  if (len > buffer_.size()) {
    len = buffer_.size();
  }
  buffer_.erase( 0, len );
  total_popped_bytes_ += len;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return buffer_.size();
}
