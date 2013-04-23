#ifndef __INTERFACE__HH
#define __INTERFACE__HH


struct CompilerOptions {
    const char * input_file;
    int input_type;
    const char * output_file;
    bool deadlock;
    bool progress;
    bool graphviz;

    static const int InputTypeFsp = 0;
    static const int InputTypeLts = 1;
};

int parser(const CompilerOptions& co);

#endif
