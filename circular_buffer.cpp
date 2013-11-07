/*
 *  fspc circular token buffer implementation
 *
 *  Copyright (C) 2013  Vincenzo Maffione
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


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

    column += len;

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
	line_aligned = false;
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
    line_aligned = true;
    column = 0;
}

void CircularBuffer::print(const string& context) const
{
    print(context, column);
}

/* Print the circular buffer content. This routine is called
   from an error handler, which also provides a string
   ('context') which contains the input portion responsible
   for the error. The string 'context' is used only if
   the circular buffer is empty. */
void CircularBuffer::print(const string& context, int col) const
{
    int i = head;
    int spaces = used();
    int bufcnt = 0;

    if (!line_aligned) {
	cerr << "... ";
	spaces += 4;
    }
    cerr << "`";
    while (i != tail) {
	cerr << buffer[i];
	i++;
	if (i == Size)
	    i = 0;
        bufcnt++;
    }
    if (!bufcnt) {
        cerr << context;
    }
    cerr << "` ...\n";

    /* Apply a correction, using the hint 'col', which is the same as column,
       by default. */
    spaces -= column - col;

    /* Print the pointer character '^'. */
    for (i=0; i<spaces; i++)
        cerr << " ";
    cerr << "^\n";
}

