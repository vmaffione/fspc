#ifndef __CIRCULAR__BUFFER__HH
#define __CIRCULAR__BUFFER__HH

class CircularBuffer {
    static const int Size = 30;

    char buffer[CircularBuffer::Size];
    int head, tail;
    bool line_aligned;

  public:
    CircularBuffer() : head(0), tail(0), line_aligned(true) { }
    int used() const;
    int available() const;
    void insert(const char * token, int len);
    void flush();
    void print() const;
};

#endif
