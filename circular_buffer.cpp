#include <iostream>
#include <cstring>

using namespace std;

#include "circular_buffer.hpp"


/* ========================== CircularBuffer ============================ */
int CircularBuffer::used() const
{
    int used = tail - head;

    if (used < 0)
	used += Size;

    return used;
}

int CircularBuffer::available() const
{
    int avail = head - tail - 1;

    if (avail < 0)
	avail += Size;

    return avail;
}

void CircularBuffer::insert(const char * token, int len)
{
    int avail = available();
    int copy;

    /* If the input token is longer that CircularBuffer::Size,
       truncate its beginning so that the resulting length is
       Size - 1. */
    if (len > Size - 1) {
	token += len - (Size - 1);
	len = Size - 1;
    }

    /* Make room for 'token'. */
    if (len > avail) {
	head += len - avail;
	if (head >= Size)
	    head -= Size;
    }

    copy = min(len, Size - tail);
    len -= copy;
    memcpy(buffer + tail, token, copy);
    tail += copy;
    if (tail == Size)
	tail = 0;

    if (len) {
	memcpy(buffer, token + copy, len);
	tail = len;
    }
}

void CircularBuffer::flush()
{
    head = tail = 0;
}

void CircularBuffer::print() const
{
    int i = head;

    cout << "...'";
    while (i != tail) {
	cout << buffer[i];
	i++;
	if (i == Size)
	    i = 0;
    }
    cout << "'...\n";
}
