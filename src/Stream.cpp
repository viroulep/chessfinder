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
#include <ext/stdio_filebuf.h>
#include "Stream.h"

using namespace std;

Stream::Stream() :
    file_descriptor_(-1), filebuf_(NULL)
{
}

Stream::Stream(int file_descriptor, ios_base::openmode openmode) :
    file_descriptor_(file_descriptor), openmode_(openmode)
{
    filebuf_ = NULL;
    open(file_descriptor, openmode);
}

ios_base::openmode Stream::openmode() const
{ 
    return openmode_;
}

void Stream::open(int file_descriptor, ios_base::openmode openmode)
{
    if (!filebuf_)
        // We create a C++ stream from a file descriptor
        // stdio_filebuf is not synced with stdio.
        // From GCC 3.4.0 on exists in addition stdio_sync_filebuf
        // You can also create the filebuf from a FILE* with
        // FILE* f = fdopen(file_descriptor, mode);
        filebuf_ = new __gnu_cxx::stdio_filebuf<char> (file_descriptor,
                openmode);
}

Stream::~Stream()
{
    //filebuf_->close();
    //delete filebuf_;
    /*
     *if (file_descriptor_)
     *    close(file_descriptor_);
     *if (filebuf_)
     *    delete filebuf_;
     */
}


//InputStream::InputStream() :
    //Stream(), stdistream_(NULL)
//{ }

InputStream::InputStream(int file_descriptor) :
    Stream(file_descriptor, ios_base::in)
{
    stdistream_ = new istream(filebuf_);
}

void InputStream::open(int file_descriptor)
{
    if (!stdistream_)
    {
        Stream::open(file_descriptor, ios_base::in);
        stdistream_ = new istream(filebuf_);
    }
}

InputStream& InputStream::operator>> (string& str)
{
    (*stdistream_) >> str;

    return *this;
}

size_t InputStream::getline(char* s, streamsize n)
{
    return (getline(s, n, '\n'));
}

size_t InputStream::getline(char* s, streamsize n, char delim)
{
    int i = 0;
    do{
        s[i] = stdistream_->get();
        i++;
    } while(i < n-1 && s[i-1] != delim && s[i-1] != '\0');

    s[i-1] = '\0'; // overwrite the delimiter given with string end

    return i-1;
}

InputStream::~InputStream()
{
    this->~Stream();
    delete stdistream_;
}

//OutputStream::OutputStream() :
    //Stream(), stdostream_(NULL) {}

OutputStream::OutputStream(int file_descriptor) :
    Stream(file_descriptor, ios_base::out)
{
    stdostream_ = new ostream(filebuf_);
}

void OutputStream::open(int file_descriptor)
{
    if (!stdostream_)
    {
        Stream::open(file_descriptor, ios_base::out);
        stdostream_ = new ostream(filebuf_);
    }
}


OutputStream& OutputStream::operator<< (const string& str)
{
    if (stdostream_->good())
        (*stdostream_) << str;

    stdostream_->flush();
    return *this;
}

OutputStream::~OutputStream()
{
    this->~Stream();
    delete stdostream_;
}

size_t getline(InputStream& is, string& str)
{
    char tmp[BUFFER_SIZE];
    size_t ret = is.getline(tmp, BUFFER_SIZE);
    str = tmp;
    return ret;
}
