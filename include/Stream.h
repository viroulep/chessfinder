/*
 * Matfinder, a program to help chess engines to find mat
 *
 * Copyright© 2013 Philippe Virouleau
 *
 * You can contact me at firstname.lastname@imag.fr
 * (Replace "firstname" and "lastname" with my actual names)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __STREAM_H__
#define __STREAM_H__

#include <iostream>
#include <unistd.h>
#include <ext/stdio_filebuf.h>

#define BUFFER_SIZE (2048)



class Stream {
protected:
    Stream();
    Stream(int file_descriptor, std::ios_base::openmode openmode);
    ~Stream();
    std::ios_base::openmode openmode() const;
    void open(int file_descriptor, std::ios_base::openmode openmode);

    int file_descriptor_;
    __gnu_cxx::stdio_filebuf<char>* filebuf_;
    std::ios_base::openmode openmode_;
};

class InputStream : public Stream {
public:
    //InputStream();
    InputStream(int file_descriptor);
    ~InputStream();
    void open(int file_descriptor);
    InputStream& operator>> (std::string& str);
    size_t getline (char* s, std::streamsize n);
    size_t getline (char* s, std::streamsize n, char delim);

private:
    std::istream* stdistream_;
};

class OutputStream : public Stream {
public:
    //OutputStream();
    OutputStream(int file_descriptor);
    ~OutputStream();
    void open(int file_descriptor);
    OutputStream& operator<< (const std::string& str);

private:
    std::ostream* stdostream_;
};

size_t getline(InputStream &is, std::string &str);

#endif
