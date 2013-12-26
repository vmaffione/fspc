/*
 *  fspc command line options
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


#ifndef __INTERFACE__HH
#define __INTERFACE__HH

/* Options that a client can pass to the FSP compiler. */
struct CompilerOptions {
    const char * input_file;
    int input_type;
    const char * output_file;
    bool deadlock;
    bool progress;
    bool graphviz;
    bool shell;
    bool script;
    unsigned int max_reference_depth;
    const char *script_file;

    static const int InputTypeFsp = 0;
    static const int InputTypeLts = 1;
};

#endif
