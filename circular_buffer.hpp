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


#ifndef __CIRCULAR__BUFFER__HH
#define __CIRCULAR__BUFFER__HH

#include <string>

using namespace std;


class CircularBuffer {
    static const int Size = 30;

    char buffer[CircularBuffer::Size];
    int head, tail;
    bool line_aligned;
    int column;

  public:
    CircularBuffer() : head(0), tail(0), line_aligned(true), column(0) { }
    int used() const;
    int available() const;
    void insert(const char *token, int len);
    void flush();
    void print(const string& context, int col) const;
    void print(const string& context) const;
};

#endif
