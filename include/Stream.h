#ifndef __STREAM_H__
#define __STREAM_H__

#include <iostream>
#include <unistd.h>
#include <ext/stdio_filebuf.h>

#define BUFFER_SIZE (512)


using namespace std;

class Stream {
protected:
    Stream();
    Stream(int file_descriptor, ios_base::openmode openmode);
    ~Stream();
    ios_base::openmode openmode() const;
    void open(int file_descriptor, ios_base::openmode openmode);

    int file_descriptor_;
    __gnu_cxx::stdio_filebuf<char>* filebuf_;
    ios_base::openmode openmode_;
};

class InputStream : public Stream {
public:
    //InputStream();
    InputStream(int file_descriptor);
    ~InputStream();
    void open(int file_descriptor);
    InputStream& operator>> (string& str);
    size_t getline (char* s, streamsize n);
    size_t getline (char* s, streamsize n, char delim);

private:
    istream* stdistream_;
};

class OutputStream : public Stream {
public:
    //OutputStream();
    OutputStream(int file_descriptor);
    ~OutputStream();
    void open(int file_descriptor);
    OutputStream& operator<< (const string& str);

private:
    ostream* stdostream_;
};

size_t getline(InputStream& is, string& str);

#endif
