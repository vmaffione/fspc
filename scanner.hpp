#ifndef __SCANNER__HH
#define __SCANNER_HH

int yylex();
int scanner_setup(const char *);


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
    ~ScannerStringBuffer();
    void useless() {}
};


#endif
