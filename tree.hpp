#ifndef __TREE_HPP__
#define __TREE_HPP__

#include <iostream>
#include <vector>
#include <string>

#include "symbols_table.hpp"
#include "lts.hpp"
#include "location.hh"


class TreeNode {
    union {
        int int_value;
        float float_value;
        std::string *string_ptr;
        class SymbolValue *sv_ptr;
        struct SvpVec *svpvec_ptr;
        struct Pvec *pvec_ptr;
        class yy::Lts *lts_ptr;
    } payload;
    unsigned int type;
};

#endif

