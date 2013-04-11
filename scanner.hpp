#ifndef __SCANNER__HH
#define __SCANNER_HH

int yylex();

/* Interface 1 */
int scanner_setup(const char *);

/* Interface 2 */
class ScannerBuffer {
    virtual void useless() = 0;

  public:
    struct yy_buffer_state * yybs;

    virtual void select();
    ~ScannerBuffer();
};

class ScannerFileBuffer: public ScannerBuffer {
    FILE * fin;

  public:
    ScannerFileBuffer(const char * input_name);
    ~ScannerFileBuffer();
    void useless() {}
};

class ScannerStringBuffer: public ScannerBuffer {
    const char * buffer;
    int size;

  public:
    ScannerStringBuffer(const char *, int);
    void useless() {}
};

/* Interface 3 */
struct BufferInfo {
    struct yy_buffer_state * yybs;
    int type;
    FILE * fin;
    const char * buffer;
    int size;

    static const int File = 0;
    static const int String = 1;
};

class InputBuffersStack {
    vector<BufferInfo> buffers;

  public:
    void push(const char * input_name);
    void push(const char * buffer, int size);
    void pop();
};

#endif
