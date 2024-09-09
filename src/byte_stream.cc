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
  if ( is_closed() || available_capacity() == 0 || data.empty()) {
    return;
  }
  // 遍历数据中的每个字符
  if(data.size() > available_capacity()) {
     data.resize( available_capacity() );
  }
  total_pushed_bytes_ += data.size();
  buffer_.push_back( std::move( data ) );

}

void Writer::close()
{
  // Your code here.
  closed_ = true;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - (total_pushed_bytes_ - total_popped_bytes_);
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
    return std::string_view();  // 返回空的 string_view
  }
  return string_view(buffer_.front());
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  len = min(len, total_pushed_bytes_-total_popped_bytes_);
  while(len > 0) {
    string& front = buffer_.front();
    if (len >= front.size()) {
      len -= front.size();
      total_popped_bytes_ += front.size();
      buffer_.pop_front();

    }
    else {
      front = front.substr( len );
      total_popped_bytes_ += len;
      len = 0;
    }
  }

}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return total_pushed_bytes_-total_popped_bytes_;
}
