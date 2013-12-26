/*
 *  fspc parse tree
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


#ifndef __TREE_HPP__
#define __TREE_HPP__

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "symbols_table.hpp"
#include "lts.hpp"
#include "context.hpp"
#include "location.hh"


struct FspDriver;

namespace yy {

/* Parse tree node base class. It derives from ParametricTranslator,
   so that a TreeNode* can be used with the ParametricProcess class. */
class TreeNode : public ParametricTranslator {
    protected:
        vector<TreeNode *> children;
        location loc;

        SetS computeActionLabels(FspDriver& c, SetS base,
                                     const vector<TreeNode*>& elements,
                                     unsigned int idx);
        yy::LtsPtr computePrefixActions(FspDriver& c,
                                     const vector<TreeNode *>& als,
                                     unsigned int idx,
                                     vector<Context>& ctxcache);
        void post_process_definition(FspDriver& c, yy::LtsPtr res,
                                           const string& name);

    public:
        TreeNode() { }
        virtual ~TreeNode();
        void addChild(TreeNode *n, const location& loc);
        void print(ofstream& os);
        unsigned int numChildren() const { return children.size(); }
        TreeNode *getChild(unsigned int i) const { return children[i]; }
        location getLocation() const { return loc; }
        virtual void translate(FspDriver& dr);
        virtual void combination(FspDriver& dr, string index, bool first) { }
        virtual string getClassName() const;
        virtual void clear() { }
        void translate_children(FspDriver& dr);
        void clear_children();
        void getNodesByClasses(const vector<string>& classes,
                                vector<TreeNode *>& results);
        /* TODO ? when everything will work, remove this method and
           declare a ProcessRefBase class from which ProcessRefNode an
           ProcessRefSeq derive, and define the translate method of the
           base class with the code contained in process_ref_translate. */
        void process_ref_translate(FspDriver& c, const string& name,
                                            const vector<int> *arguments,
                                            yy::LtsPtr *res);
};


/* ======================== FIRST DERIVATION LEVEL =========================
   The first derivation level adds the content to a parse tree node. This
   content can be an integer, a string, or other object types. */

class IntTreeNode : public TreeNode {
    public:
        int res;

        IntTreeNode() : TreeNode() { }
};

class FloatTreeNode : public TreeNode {
    public:
        float res;

        FloatTreeNode() : TreeNode() { }
};

class StringTreeNode : public TreeNode {
    public:
        std::string res;

        StringTreeNode() : TreeNode() { }
};

class LtsTreeNode : public TreeNode {
    public:
        yy::LtsPtr res;

        LtsTreeNode() : TreeNode() { }
        void clear() { res.clear(); }
};


/* ======================== SECOND DERIVATION LEVEL ========================
   The second level of derivation adds a syntax meaning to a parse tree node:
   this means that the node corresponds to a terminal or non-terminal symbol
   in the FSP grammar. Each node is able to "translate" (towards LTS) the
   subtree rooted in itself. */

class BaseExpressionNode : public IntTreeNode {
    public:
        static string className() { return "BaseExpression"; }
        string getClassName() const { return className(); }
        BaseExpressionNode() : IntTreeNode() { }
        void translate(FspDriver &dr);
};

class IntegerNode : public IntTreeNode {
    public:
        static string className() { return "Integer"; }
        string getClassName() const { return className(); }
        IntegerNode() : IntTreeNode() { }
};

class LowerCaseIdNode : public StringTreeNode {
    public:
        static string className() { return "LowerCaseId"; }
        string getClassName() const { return className(); }
        LowerCaseIdNode() : StringTreeNode() { }
};

class UpperCaseIdNode : public StringTreeNode {
    public:
        static string className() { return "UpperCaseId"; }
        string getClassName() const { return className(); }
        UpperCaseIdNode() : StringTreeNode() { }
};

class VariableIdNode : public LowerCaseIdNode {
    public:
        static string className() { return "VariableId"; }
        string getClassName() const { return className(); }
        VariableIdNode() : LowerCaseIdNode() { }
        void translate(FspDriver &dr);
};

class ConstantIdNode : public UpperCaseIdNode {
    public:
        static string className() { return "ConstantId"; }
        string getClassName() const { return className(); }
        ConstantIdNode() : UpperCaseIdNode() { }
        void translate(FspDriver &dr);
};

class RangeIdNode : public UpperCaseIdNode {
    public:
        static string className() { return "RangeId"; }
        string getClassName() const { return className(); }
        RangeIdNode() : UpperCaseIdNode() { }
        void translate(FspDriver &dr);
};

class SetIdNode : public UpperCaseIdNode {
    public:
        static string className() { return "SetId"; }
        string getClassName() const { return className(); }
        SetIdNode() : UpperCaseIdNode() { }
        void translate(FspDriver &dr);
};

class ConstParameterIdNode : public UpperCaseIdNode {
    public:
        static string className() { return "ConstParameterId"; }
        string getClassName() const { return className(); }
        ConstParameterIdNode() : UpperCaseIdNode() { }
        void translate(FspDriver &dr);
};

class ParameterIdNode : public UpperCaseIdNode {
    public:
        static string className() { return "ParameterId"; }
        string getClassName() const { return className(); }
        ParameterIdNode() : UpperCaseIdNode() { }
        void translate(FspDriver &dr);
};

class ProcessIdNode : public UpperCaseIdNode {
    public:
        static string className() { return "ProcessId"; }
        string getClassName() const { return className(); }
        ProcessIdNode() : UpperCaseIdNode() { }
        void translate(FspDriver &dr);
};

class ProgressIdNode : public UpperCaseIdNode {
    public:
        static string className() { return "ProgressId"; }
        string getClassName() const { return className(); }
        ProgressIdNode() : UpperCaseIdNode() { }
        void translate(FspDriver &dr);
};

class MenuIdNode : public UpperCaseIdNode {
    public:
        static string className() { return "MenuId"; }
        string getClassName() const { return className(); }
        MenuIdNode() : UpperCaseIdNode() { }
        void translate(FspDriver &dr);
};

class ExpressionNode : public IntTreeNode {
    public:
        static string className() { return "Expression"; }
        string getClassName() const { return className(); }
        ExpressionNode() : IntTreeNode() { }
        void translate(FspDriver&);
};

class OperatorNode : public TreeNode {
    public:
        string sign;

        OperatorNode(const string& s) : TreeNode(), sign(s) { }
        static string className() { return "OperatorNode"; }
        string getClassName() const { return sign; }
};

class OpenParenNode : public TreeNode {
    public:
        static string className() { return "("; }
        string getClassName() const { return className(); }
        OpenParenNode() : TreeNode() { }
};

class CloseParenNode : public TreeNode {
    public:
        static string className() { return ")"; }
        string getClassName() const { return className(); }
        CloseParenNode() : TreeNode() { }
};

class ProgressDefNode : public TreeNode {
    public:
        static string className() { return "ProgressDef"; }
        string getClassName() const { return className(); }
        void translate(FspDriver& c);
        void combination(FspDriver& c, string index, bool first);
        ProgressDefNode() : TreeNode() { }
};

class PropertyNode : public TreeNode {
    public:
        static string className() { return "property"; }
        string getClassName() const { return className(); }
        PropertyNode() : TreeNode() { }
};

class MenuDefNode : public TreeNode {
    public:
        static string className() { return "MenuDef"; }
        string getClassName() const { return className(); }
        void translate(FspDriver& c);
        MenuDefNode() : TreeNode() { }
};

class MenuKwdNode : public TreeNode {
    public:
        static string className() { return "menu"; }
        string getClassName() const { return className(); }
        MenuKwdNode() : TreeNode() { }
};

class HidingInterfNode : public TreeNode {
    public:
        HidingS res;

        static string className() { return "HidingInterf"; }
        string getClassName() const { return className(); }
        HidingInterfNode() : TreeNode() { }
        void translate(FspDriver& c);
};

class HidingNode : public TreeNode {
    public:
        HidingNode() : TreeNode() { }
        static string className() { return "Hiding"; }
        string getClassName() const { return className(); }
};

class InterfNode : public TreeNode {
    public:
        InterfNode() : TreeNode() { }
        static string className() { return "Interf"; }
        string getClassName() const { return className(); }
};

class RelabelDefNode : public TreeNode {
    public:
        RelabelingS res;

        static string className() { return "RelabelDef"; }
        string getClassName() const { return className(); }
        RelabelDefNode() : TreeNode() { }
        void translate(FspDriver& c);
        void combination(FspDriver& dr, string index, bool first);
};

class SlashNode : public TreeNode {
    public:
        SlashNode() : TreeNode() { }
        static string className() { return "Slash"; }
        string getClassName() const { return className(); }
};

class ForallNode : public TreeNode {
    public:
        ForallNode() : TreeNode() { }
        static string className() { return "forall"; }
        string getClassName() const { return className(); }
};

class RelabelDefsNode : public TreeNode {
    public:
        RelabelingS res;

        RelabelDefsNode() : TreeNode() { }
        static string className() { return "RelabelDefs"; }
        string getClassName() const { return className(); }
        void translate(FspDriver& c);
};

class CommaNode : public TreeNode {
    public:
        CommaNode() : TreeNode() { }
        static string className() { return ","; }
        string getClassName() const { return className(); }
};

class OpenCurlyNode : public TreeNode {
    public:
        OpenCurlyNode() : TreeNode() { }
        static string className() { return "{"; }
        string getClassName() const { return className(); }
};

class CloseCurlyNode : public TreeNode {
    public:
        CloseCurlyNode() : TreeNode() { }
        static string className() { return "}"; }
        string getClassName() const { return className(); }
};

class BracesRelabelDefsNode : public TreeNode {
    public:
        RelabelingS res;

        static string className() { return "BracesRelabelDefs"; }
        string getClassName() const { return className(); }
        BracesRelabelDefsNode() : TreeNode() { }
        void translate(FspDriver& c);
};

class ParameterNode : public TreeNode {
    public:
        ParameterNode() : TreeNode() { } 
        static string className() { return "Parameter"; }
        string getClassName() const { return className(); }
};

class AssignNode : public TreeNode {
    public:
        AssignNode() : TreeNode() { }
        static string className() { return "="; }
        string getClassName() const { return className(); }
};

class ParameterListNode : public TreeNode {
    public:
        ParameterListNode() : TreeNode() { }
        static string className() { return "ParameterList"; }
        string getClassName() const { return className(); }
};

class ParamNode : public TreeNode {
    public:
        ParamNode() : TreeNode() { }
        static string className() { return "Param"; }
        string getClassName() const { return className(); }
};

class ColonNode : public TreeNode {
    public:
        ColonNode() : TreeNode() { }
        static string className() { return ":"; }
        string getClassName() const { return className(); }
};

class LabelingNode : public TreeNode {
    public:
        SetS res;

        static string className() { return "Labeling"; }
        string getClassName() const { return className(); }
        LabelingNode() : TreeNode() { }
        void translate(FspDriver& c);
};

class DoubleColonNode : public TreeNode {
    public:
        DoubleColonNode() : TreeNode() { }
        static string className() { return "::"; }
        string getClassName() const { return className(); }
};

class SharingNode : public TreeNode {
    public:
        SetS res;

        static string className() { return "Sharing"; }
        string getClassName() const { return className(); }
        SharingNode() : TreeNode() { }
        void translate(FspDriver& c);
};

class ProcessRefNode : public LtsTreeNode {
    public:
        static string className() { return "ProcessRef"; }
        string getClassName() const { return className(); }
        ProcessRefNode() : LtsTreeNode() { }
        void translate(FspDriver& c);
};

class ParallelCompNode : public TreeNode {
    public:
        vector<LtsPtr> res;

        static string className() { return "ParallelComp"; }
        string getClassName() const { return className(); }
        ParallelCompNode() : TreeNode() { }
        void translate(FspDriver& c);
};

class CompositeElseNode : public LtsTreeNode {
    public:
        static string className() { return "CompositeElse"; }
        string getClassName() const { return className(); }
        CompositeElseNode() : LtsTreeNode() { }
        void translate(FspDriver& c);
};

class ElseNode : public TreeNode {
    public:
        ElseNode() : TreeNode() { } 
        static string className() { return "else"; }
        string getClassName() const { return className(); }
};

class CompositeBodyNode : public LtsTreeNode {
    public:
        static string className() { return "CompositeBody"; }
        string getClassName() const { return className(); }
        CompositeBodyNode() : LtsTreeNode() { }
        void translate(FspDriver& c);
        void combination(FspDriver& dr, string index, bool first);
};

class IfNode : public TreeNode {
    public:
        IfNode() : TreeNode() { } 
        static string className() { return "if"; }
        string getClassName() const { return className(); }
};

class ThenNode : public TreeNode {
    public:
        ThenNode() : TreeNode() { } 
        static string className() { return "Then"; }
        string getClassName() const { return className(); }
};

class CompositeDefNode : public LtsTreeNode {
    public:
        CompositeDefNode() : LtsTreeNode() { }
        static string className() { return "CompositeDef"; }
        string getClassName() const { return className(); }
        void translate(FspDriver& c);
};

class ArgumentListNode : public TreeNode {
    public:
        vector<int> res;

        static string className() { return "ArgumentList"; }
        string getClassName() const { return className(); }
        ArgumentListNode() : TreeNode() { }
        void translate(FspDriver& c);
};

class ArgumentsNode : public TreeNode {
    public:
        vector<int> res;

        static string className() { return "Arguments"; }
        string getClassName() const { return className(); }
        ArgumentsNode() : TreeNode() { }
        void translate(FspDriver& c);
};

class ProcessRefSeqNode : public LtsTreeNode {
    public:
        static string className() { return "ProcessRefSeq"; }
        string getClassName() const { return className(); }
        ProcessRefSeqNode() : LtsTreeNode() { }
        void translate(FspDriver& c);
};

class SeqProcessListNode : public LtsTreeNode {
    public:
        static string className() { return "SeqProcessList"; }
        string getClassName() const { return className(); }
        SeqProcessListNode() : LtsTreeNode() { }
        void translate(FspDriver& c);
};

class SemicolonNode : public TreeNode {
    public:
        SemicolonNode() : TreeNode() { }
        static string className() { return ";"; }
        string getClassName() const { return className(); }
};

class SeqCompNode : public LtsTreeNode {
    public:
        static string className() { return "SeqComp"; }
        string getClassName() const { return className(); }
        SeqCompNode() : LtsTreeNode() { }
        void translate(FspDriver& c);
};

class IndexRangesNode : public TreeNode {
    public:
        vector<TreeNode *> res;

        static string className() { return "IndexRanges"; }
        string getClassName() const { return className(); }
        IndexRangesNode() : TreeNode() { }
        void translate(FspDriver& c);
};

class OpenSquareNode : public TreeNode {
    public:
        OpenSquareNode() : TreeNode() { } 
        static string className() { return "["; }
        string getClassName() const { return className(); }
};

class CloseSquareNode : public TreeNode {
    public:
        CloseSquareNode() : TreeNode() { } 
        static string className() { return "]"; }
        string getClassName() const { return className(); }
};

class IndicesNode : public StringTreeNode {
    public:
        static string className() { return "Indices"; }
        string getClassName() const { return className(); }
        IndicesNode() : StringTreeNode() { }
        void translate(FspDriver& c);
};

class GuardNode : public IntTreeNode {
    public:
        static string className() { return "Guard"; }
        string getClassName() const { return className(); }
        GuardNode() : IntTreeNode() { }
};

class WhenNode : public TreeNode {
    public:
        WhenNode() : TreeNode() { } 
        static string className() { return "when"; }
        string getClassName() const { return className(); }
};

class PrefixActionsNode : public TreeNode {
    public:
        vector<TreeNode *> res;

        static string className() { return "PrefixActions"; }
        string getClassName() const { return className(); }
        PrefixActionsNode() : TreeNode() { }
        void translate(FspDriver& dr);
};

class ArrowNode : public TreeNode {
    public:
        ArrowNode() : TreeNode() { } 
        static string className() { return "->"; }
        string getClassName() const { return className(); }
};

class ActionPrefixNode : public LtsTreeNode {
    public:
        static string className() { return "ActionPrefix"; }
        string getClassName() const { return className(); }
        ActionPrefixNode() : LtsTreeNode() { }
        void translate(FspDriver& dr);
};

class ChoiceNode : public LtsTreeNode {
    public:
        static string className() { return "Choice"; }
        string getClassName() const { return className(); }
        ChoiceNode() : LtsTreeNode() { }
        void translate(FspDriver& dr);
};

class BaseLocalProcessNode : public LtsTreeNode {
    public:
        static string className() { return "BaseLocalProcess"; }
        string getClassName() const { return className(); }
        BaseLocalProcessNode() : LtsTreeNode() { }
        void translate(FspDriver& dr);
};

class EndNode : public TreeNode {
    public:
        EndNode() : TreeNode() { }
        static string className() { return "END"; }
        string getClassName() const { return className(); }
};

class StopNode : public TreeNode {
    public:
        StopNode() : TreeNode() { } 
        static string className() { return "STOP"; }
        string getClassName() const { return className(); }
};

class ErrorNode : public TreeNode {
    public:
        ErrorNode() : TreeNode() { } 
        static string className() { return "ERROR"; }
        string getClassName() const { return className(); }
};

class ProcessElseNode : public LtsTreeNode {
    public:
        static string className() { return "ProcessElse"; }
        string getClassName() const { return className(); }
        ProcessElseNode() : LtsTreeNode() { }
        void translate(FspDriver& dr);
};

class LocalProcessNode : public LtsTreeNode {
    public:
        static string className() { return "LocalProcess"; }
        string getClassName() const { return className(); }
        LocalProcessNode() : LtsTreeNode() { }
        void translate(FspDriver& dr);
};

class AlphaExtNode : public TreeNode {
    public:
        SetS res;

        static string className() { return "AlphaExt"; }
        string getClassName() const { return className(); }
        AlphaExtNode() : TreeNode() { }
        void translate(FspDriver& dr);
};

class LocalProcessDefNode : public LtsTreeNode {
    public:
        static string className() { return "LocalProcessDef"; }
        string getClassName() const { return className(); }
        LocalProcessDefNode() : LtsTreeNode() { }
        void translate(FspDriver& c);
        void combination(FspDriver& dr, string index, bool first);
};

class LocalProcessDefsNode : public LtsTreeNode {
    public:
        static string className() { return "LocalProcessDefs"; }
        string getClassName() const { return className(); }
        LocalProcessDefsNode() : LtsTreeNode() { }
        void translate(FspDriver& c);
};

class ProcessBodyNode : public LtsTreeNode {
    public:
        static string className() { return "ProcessBody"; }
        string getClassName() const { return className(); }
        ProcessBodyNode() : LtsTreeNode() { }
        void translate(FspDriver& dr);
};

class ProcessDefNode : public LtsTreeNode {
    public:
        static string className() { return "ProcessDef"; }
        string getClassName() const { return className(); }
        ProcessDefNode() : LtsTreeNode() { }
        void translate(FspDriver& dr);
};

class PeriodNode : public TreeNode {
    public:
        PeriodNode() : TreeNode() { } 
        static string className() { return "."; }
        string getClassName() const { return className(); }
};

class SetElementsNode : public TreeNode {
    public:
        SetS res;

        static string className() { return "SetElements"; }
        string getClassName() const { return className(); }
        SetElementsNode() : TreeNode() { }
        void translate(FspDriver&);
};

class SetDefNode : public TreeNode {
    public:
        SetDefNode() : TreeNode() { } 
        static string className() { return "SetDef"; }
        string getClassName() const { return className(); }
        void translate(FspDriver& c);
};

class SetKwdNode : public TreeNode {
    public:
        SetKwdNode() : TreeNode() { } 
        static string className() { return "SetKwd"; }
        string getClassName() const { return className(); }
};

class RangeDefNode : public TreeNode {
    public:
        RangeDefNode() : TreeNode() { } 
        static string className() { return "RangeDef"; }
        string getClassName() const { return className(); }
        void translate(FspDriver& c);
};

class RangeKwdNode : public TreeNode {
    public:
        RangeKwdNode() : TreeNode() { } 
        static string className() { return "RangeKwd"; }
        string getClassName() const { return className(); }
};

class DotDotNode : public TreeNode {
    public:
        DotDotNode() : TreeNode() { } 
        static string className() { return ".."; }
        string getClassName() const { return className(); }
};

class ConstantDefNode : public TreeNode {
    public:
        ConstantDefNode() : TreeNode() { } 
        static string className() { return "ConstDef"; }
        string getClassName() const { return className(); }
        void translate(FspDriver& c);
};

class ConstKwdNode : public TreeNode {
    public:
        ConstKwdNode() : TreeNode() { } 
        static string className() { return "ConstKwd"; }
        string getClassName() const { return className(); }
};

class RangeExprNode : public TreeNode {
    public:
        RangeS res;

        static string className() { return "RangeExpr"; }
        string getClassName() const { return className(); }
        RangeExprNode() : TreeNode() { }
        void translate(FspDriver&);
};

class ActionRangeNode : public TreeNode {
    public:
        SetS res;

        static string className() { return "ActionRange"; }
        string getClassName() const { return className(); }
        ActionRangeNode() : TreeNode() { }
        void translate(FspDriver&);
};

class RangeNode : public TreeNode {
    public:
        RangeS res;

        static string className() { return "Range"; }
        string getClassName() const { return className(); }
        RangeNode() : TreeNode() { }
        void translate(FspDriver&);
};

class SetExprNode : public TreeNode {
    public:
        SetS res;

        static string className() { return "SetExpr"; }
        string getClassName() const { return className(); }
        SetExprNode() : TreeNode() { }
        void translate(FspDriver&);
};

class SetNode : public TreeNode {
    public:
        SetS res;

        static string className() { return "Set"; }
        string getClassName() const { return className(); }
        SetNode() : TreeNode() { }
        void translate(FspDriver&);
};

class ActionLabelsNode : public TreeNode {
    public:
        vector<TreeNode *> res;

        static string className() { return "ActionLabels"; }
        string getClassName() const { return className(); }
        ActionLabelsNode() : TreeNode() { }
        void translate(FspDriver& dr);
};

class RootNode : public TreeNode {
    public:
        RootNode() : TreeNode() { }
        static string className() { return "Root"; }
        string getClassName() const { return className(); }
};

class PrioritySNode : public TreeNode {
    public:
        PriorityS res;

        static string className() { return "PriorityS"; }
        string getClassName() const { return className(); }
        PrioritySNode() : TreeNode() { }
        void translate(FspDriver& c);
};

class RelabelingNode : public TreeNode {
    public:
        RelabelingS res;

        static string className() { return "Relabeling"; }
        string getClassName() const { return className(); }
        RelabelingNode() : TreeNode() { }
        void translate(FspDriver& c);
};

class ProgressKwdNode : public TreeNode {
    public:
        ProgressKwdNode() : TreeNode() { } 
        static string className() { return "ProgressKwd"; }
        string getClassName() const { return className(); }
};

/* Useful wrappers for downcasting a TreeNode pointer to pointers to derived types. */
template <class T>
T* tree_downcast_safe(yy::TreeNode *n)
{
    T *ret = dynamic_cast<T*>(n);

    return ret;
}

template <class T>
T* tree_downcast(yy::TreeNode *n)
{
    T *ret = tree_downcast_safe<T>(n);

    assert(ret);

    return ret;
}

template <class T>
T* tree_downcast_null(yy::TreeNode *n)
{
    if (!n)
        return NULL;
    return tree_downcast<T>(n);
}


} /* namespace yy */

/* Declare "_n" as a "_t"*, and assign to it the downcasted "_x". */
#define DTC(_t, _n, _x) \
    _t *_n = yy::tree_downcast<_t>(_x);

/* Same as the previous one, but using the safe downcast function. */
#define DTCS(_t, _n, _x) \
    _t *_n = yy::tree_downcast_safe<_t>(_x);

#endif

