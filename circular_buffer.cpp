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

void CircularBuffer::print() const
{
    print(column);
}

void CircularBuffer::print(int col) const
{
    int i = head;
    int spaces = used();

    if (!line_aligned) {
	cout << "... ";
	spaces += 4;
    }
    cout << "`";
    while (i != tail) {
	cout << buffer[i];
	i++;
	if (i == Size)
	    i = 0;
    }
    cout << "` ...\n";

    /* Apply a correction, using the hint 'col', which is the same as column,
       by default. */
    spaces -= column - col;

    for (i=0; i<spaces; i++) cout << " "; cout << "^\n";
    cout << "COLUMN=" << column << endl;
}

void CircularBuffer::location_extend(struct YYLTYPE& loc, int len)
{
    loc.last_column += len;
}

/* Restart: move the first cursor to the last position. */
void CircularBuffer::location_step(struct YYLTYPE& loc)
{
    loc.first_column = loc.last_column;
    loc.first_line = loc.last_line;
}

/* Advance of NUM lines. */
void CircularBuffer::location_lines(struct YYLTYPE& loc, int num)
{
    loc.last_column = 1;
    loc.last_line += num;
}

