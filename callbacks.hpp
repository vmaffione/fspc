#ifndef __CALLBACKS__HH
#define __CALLBACKS__HH

#include "translator.hpp"
#include "symbols_table.hpp"

/* Left contains a SetValue*, while right is the result of 'action_range'. */
SvpVec * indexize_svpvec(struct FspTranslator * gp, SvpVec * left, SvpVec * right);

SvpVec * callback__1(FspTranslator& tr, string * one); /*1*/
SvpVec * callback__2(FspTranslator& tr, SvpVec * one, string * two); /*2*/
SvpVec * callback__3(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__4(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__5(FspTranslator& tr, string * one); /*1*/
SvpVec * callback__6(FspTranslator& tr, string * one, string * two); /*4*/
SvpVec * callback__7(FspTranslator& tr, string * one, SvpVec * two); /*5*/
SvpVec * callback__8(FspTranslator& tr, string * one, SvpVec * two); /*5*/
SvpVec * callback__9(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__13(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
void * callback__14(FspTranslator& tr, string * one); /*19*/
class Lts * callback__15(FspTranslator& tr, string * one, Pvec * two, SvpVec * three, SvpVec * four, SvpVec * five); /*7*/
void * callback__16(FspTranslator& tr, string * one); /*19*/
void * callback__17(FspTranslator& tr, string * one, SvpVec * two); /*20*/
Pvec * callback__18(FspTranslator& tr, string * one, SvpVec * two, Pvec * three); /*8*/
void * callback__19(FspTranslator& tr, SvpVec * one); /*21*/
void * callback__20(FspTranslator& tr, SvpVec * one, Pvec * two); /*22*/
Pvec * callback__21(FspTranslator& tr, SvpVec * one, Pvec * two, Pvec * three); /*11*/
void * callback__22(FspTranslator& tr); /*24*/
Pvec * callback__23(FspTranslator& tr); /*12*/
Pvec * callback__24(FspTranslator& tr); /*12*/
Pvec * callback__25(FspTranslator& tr); /*12*/
Pvec * callback__26(FspTranslator& tr, string * one, SvpVec * two); /*6*/
Pvec * callback__27(FspTranslator& tr, Pvec * one); /*13*/
void * callback__28(FspTranslator& tr, Pvec * one); /*23*/
Pvec * callback__29(FspTranslator& tr, Pvec * one, Pvec * two); /*14*/
void * callback__30(FspTranslator& tr, SvpVec * one); /*21*/
Pvec * callback__31(FspTranslator& tr, SvpVec * one, Pvec * two, Pvec * three); /*11*/
Pvec * callback__32(FspTranslator& tr, SvpVec * one); /*9*/
Pvec * callback__33(FspTranslator& tr, Pvec * one, SvpVec * two); /*15*/
SvpVec * callback__34(FspTranslator& tr); /*16*/
SvpVec * callback__35(FspTranslator& tr, SvpVec * one); /*17*/
SvpVec * callback__36(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__37(FspTranslator& tr); /*16*/
SvpVec * callback__38(FspTranslator& tr, SvpVec * one); /*17*/
SvpVec * callback__39(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__41(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__42(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__43(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__44(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__45(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__46(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__47(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__48(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__49(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__50(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__51(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__52(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__53(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__54(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__55(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__56(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__57(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__58(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__59(FspTranslator& tr, SvpVec * one); /*17*/
SvpVec * callback__60(FspTranslator& tr, SvpVec * one); /*17*/
SvpVec * callback__61(FspTranslator& tr, int one); /*18*/
SvpVec * callback__62(FspTranslator& tr, string * one); /*1*/
SvpVec * callback__63(FspTranslator& tr, string * one); /*1*/
Pvec * callback__64(FspTranslator& tr, Pvec * one, Pvec * two); /*14*/
Pvec * callback__65(FspTranslator& tr, Pvec * one, Pvec * two); /*14*/
Pvec * callback__66(FspTranslator& tr, string * one, SvpVec * two); /*6*/
SvpVec * callback__67(FspTranslator& tr, SvpVec * one); /*17*/
SvpVec * callback__68(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__69(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__70(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__71(FspTranslator& tr, SvpVec * one); /*17*/
SvpVec * callback__72(FspTranslator& tr, SvpVec * one); /*17*/
SvpVec * callback__73(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
void * callback__74(FspTranslator&tr, SvpVec * one); /*21*/

struct Callback {
    virtual void * execute(FspTranslator &tr, vector<void *>& stack) = 0;
    virtual bool is_void() const { return false; }
    virtual void print() const = 0;
};

struct ParametricProcess: public SymbolValue {
    vector<string> parameter_names;
    vector<int> parameter_defaults;
    vector<Callback *> record;

    class Lts * replay(struct FspCompiler& c, const vector<int>& values);
    void print() const;
    int type() const { return SymbolValue::ParametricProcess; }
    SymbolValue * clone() const { return NULL; }
};

ParametricProcess* err_if_not_parametric(SymbolValue * svp);


/*1*/
struct Callback_V_S : public Callback {
    typedef SvpVec * (*FPT)(FspTranslator&, string *);
    FPT cbp;
    string one;

    Callback_V_S(FPT fp, const string& s) : cbp(fp), one(s) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	return (*cbp)(tr, new string(one));
    }
    void print() const { cout << "V_S" << "\n";}
};

/*2*/
struct Callback_V_VS : public Callback {
    typedef SvpVec * (*FPT)(FspTranslator&, SvpVec *, string *);
    FPT cbp;
    string two;

    Callback_V_VS(FPT fp, const string &s) : cbp(fp), two(s) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	SvpVec * one = static_cast<SvpVec *>(stack.back());
	stack.pop_back();
	return (*cbp)(tr, one, new string(two));
    }
    void print() const { cout << "V_VS" << "\n";}
};

/*3*/
struct Callback_V_VV : public Callback {
    typedef SvpVec * (*FPT)(FspTranslator&, SvpVec *, SvpVec *);
    FPT cbp;

    Callback_V_VV(FPT fp) : cbp(fp) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	SvpVec * two = static_cast<SvpVec *>(stack.back());
	stack.pop_back();
	SvpVec * one = static_cast<SvpVec *>(stack.back());
	stack.pop_back();
	return (*cbp)(tr, one, two);
    }
    void print() const { cout << "V_VV" << "\n";}
};

/*4*/
struct Callback_V_SS : public Callback {
    typedef SvpVec * (*FPT)(FspTranslator&, string *, string *);
    FPT cbp;
    string one;
    string two;

    Callback_V_SS(FPT fp, const string& s1, const string& s2) : cbp(fp),
							one(s1), two(s2){ }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	return (*cbp)(tr, new string(one), new string(two));
    }
    void print() const { cout << "V_SS" << "\n";}
};

/*5*/
struct Callback_V_SV : public Callback {
    typedef SvpVec * (*FPT)(FspTranslator&, string *, SvpVec *);
    FPT cbp;
    string one;

    Callback_V_SV(FPT fp, const string& s) : cbp(fp), one(s) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	SvpVec * two = static_cast<SvpVec *>(stack.back());
	stack.pop_back();
	return (*cbp)(tr, new string(one), two);
    }
    void print() const { cout << "V_SV" << "\n";}
};

/*6*/
struct Callback_P_SV : public Callback {
    typedef Pvec * (*FPT)(FspTranslator&, string *, SvpVec *);
    FPT cbp;
    string one;

    Callback_P_SV(FPT fp, const string& s) : cbp(fp), one(s) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	SvpVec * two = static_cast<SvpVec *>(stack.back());
	stack.pop_back();
	return (*cbp)(tr, new string(one), two);
    }
    void print() const { cout << "P_SV" << "\n";}
};

/*7*/
struct Callback_P_SPVVV : public Callback {
    typedef Lts * (*FPT)(FspTranslator&, string *, Pvec *, SvpVec *,
							SvpVec *, SvpVec *);
    FPT cbp;
    string one;

    Callback_P_SPVVV(FPT fp, const string& s) : cbp(fp), one(s) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	SvpVec * five = static_cast<SvpVec *>(stack.back());
	stack.pop_back();
	SvpVec * four = static_cast<SvpVec *>(stack.back());
	stack.pop_back();
	SvpVec * three = static_cast<SvpVec *>(stack.back());
	stack.pop_back();
	Pvec * two = static_cast<Pvec *>(stack.back());
	stack.pop_back();
	return (*cbp)(tr, new string(one), two, three, four, five);
    }
    void print() const { cout << "P_SPVVV" << "\n";}
};

/*8*/
struct Callback_P_SVP : public Callback {
    typedef Pvec * (*FPT)(FspTranslator&, string *, SvpVec *, Pvec *);
    FPT cbp;
    string one;

    Callback_P_SVP(FPT fp, const string& s) : cbp(fp), one(s) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	Pvec * three = static_cast<Pvec *>(stack.back());
	stack.pop_back();
	SvpVec * two = static_cast<SvpVec *>(stack.back());
	stack.pop_back();
	return (*cbp)(tr, new string(one), two, three);
    }
    void print() const { cout << "P_SVP" << "\n";}
};

/*9*/
struct Callback_P_V : public Callback {
    typedef Pvec * (*FPT)(FspTranslator&, SvpVec *);
    FPT cbp;

    Callback_P_V(FPT fp) : cbp(fp) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	SvpVec * one = static_cast<SvpVec *>(stack.back());
	stack.pop_back();
	return (*cbp)(tr, one);
    }
    void print() const { cout << "P_V" << "\n";}
};

/*10*/
struct Callback_P_VP : public Callback {
    typedef Pvec * (*FPT)(FspTranslator&, SvpVec *, Pvec *);
    FPT cbp;

    Callback_P_VP(FPT fp) : cbp(fp) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	Pvec * two = static_cast<Pvec *>(stack.back());
	stack.pop_back();
	SvpVec * one = static_cast<SvpVec *>(stack.back());
	stack.pop_back();
	return (*cbp)(tr, one, two);
    }
    void print() const { cout << "P_VP" << "\n";}
};

/*11*/
struct Callback_P_VPP : public Callback {
    typedef Pvec * (*FPT)(FspTranslator&, SvpVec *, Pvec *, Pvec *);
    FPT cbp;

    Callback_P_VPP(FPT fp) : cbp(fp) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	Pvec * three = static_cast<Pvec *>(stack.back());
	stack.pop_back();
	Pvec * two = static_cast<Pvec *>(stack.back());
	stack.pop_back();
	SvpVec * one = static_cast<SvpVec *>(stack.back());
	stack.pop_back();
	return (*cbp)(tr, one, two, three);
    }
    void print() const { cout << "P_VPP" << "\n";}
};

/*12*/
struct Callback_P : public Callback {
    typedef Pvec * (*FPT)(FspTranslator&);
    FPT cbp;

    Callback_P(FPT fp) : cbp(fp) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	return (*cbp)(tr);
    }
    void print() const { cout << "P" << "\n";}
};

/*13*/
struct Callback_P_P : public Callback {
    typedef Pvec * (*FPT)(FspTranslator&, Pvec *);
    FPT cbp;

    Callback_P_P(FPT fp) : cbp(fp) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	Pvec * one = static_cast<Pvec *>(stack.back());
	stack.pop_back();
	return (*cbp)(tr, one);
    }
    void print() const { cout << "P_P" << "\n";}
};

/*14*/
struct Callback_P_PP : public Callback {
    typedef Pvec * (*FPT)(FspTranslator&, Pvec *, Pvec *);
    FPT cbp;

    Callback_P_PP(FPT fp) : cbp(fp) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	Pvec * two = static_cast<Pvec *>(stack.back());
	stack.pop_back();
	Pvec * one = static_cast<Pvec *>(stack.back());
	stack.pop_back();
	return (*cbp)(tr, one, two);
    }
    void print() const { cout << "P_PP" << "\n";}
};

/*15*/
struct Callback_P_PV : public Callback {
    typedef Pvec * (*FPT)(FspTranslator&, Pvec *, SvpVec *);
    FPT cbp;

    Callback_P_PV(FPT fp) : cbp(fp) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	SvpVec * two = static_cast<SvpVec *>(stack.back());
	stack.pop_back();
	Pvec * one = static_cast<Pvec *>(stack.back());
	stack.pop_back();
	return (*cbp)(tr, one, two);
    }
    void print() const { cout << "P_PV" << "\n";}
};

/*16*/
struct Callback_V : public Callback {
    typedef SvpVec * (*FPT)(FspTranslator&);
    FPT cbp;

    Callback_V(FPT fp) : cbp(fp) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	return (*cbp)(tr);
    }
    void print() const { cout << "V" << "\n";}
};

/*17*/
struct Callback_V_V : public Callback {
    typedef SvpVec * (*FPT)(FspTranslator&, SvpVec *);
    FPT cbp;

    Callback_V_V(FPT fp) : cbp(fp) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	SvpVec * one = static_cast<SvpVec *>(stack.back());
	stack.pop_back();
	return (*cbp)(tr, one);
    }
    void print() const { cout << "V_V" << "\n";}
};

/*18*/
struct Callback_V_I : public Callback {
    typedef SvpVec * (*FPT)(FspTranslator&, int);
    FPT cbp;
    int one;

    Callback_V_I(FPT fp, int i) : cbp(fp), one(i) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	return (*cbp)(tr, one);
    }
    void print() const { cout << "V_I" << "\n";}
};


/*19*/
struct Callback_N_S : public Callback {
    typedef void * (*FPT)(FspTranslator&, string *);
    FPT cbp;
    string one;

    Callback_N_S(FPT fp, const string& s) : cbp(fp), one(s) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	return (*cbp)(tr, new string(one));
    }
    void print() const { cout << "N_S" << "\n";}
    bool is_void() const { return true; }
};

/*20*/
struct Callback_N_SV : public Callback {
    typedef void * (*FPT)(FspTranslator&, string *, SvpVec *);
    FPT cbp;
    string one;

    Callback_N_SV(FPT fp, const string& s) : cbp(fp), one(s) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	SvpVec * two = static_cast<SvpVec *>(stack.back());
	return (*cbp)(tr, new string(one), two);
    }
    void print() const { cout << "N_SV" << "\n";}
    bool is_void() const { return true; }
};

/*21*/
struct Callback_N_V : public Callback {
    typedef void * (*FPT)(FspTranslator&, SvpVec *);
    FPT cbp;

    Callback_N_V(FPT fp) : cbp(fp) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	SvpVec * one = static_cast<SvpVec *>(stack.back());
	return (*cbp)(tr, one);
    }
    void print() const { cout << "N_V" << "\n";}
    bool is_void() const { return true; }
};

/*22*/
struct Callback_N_VP : public Callback {
    typedef void * (*FPT)(FspTranslator&, SvpVec *, Pvec *);
    FPT cbp;

    Callback_N_VP(FPT fp) : cbp(fp) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	Pvec * two = static_cast<Pvec *>(stack.back());
	stack.pop_back();
	SvpVec * one = static_cast<SvpVec *>(stack.back());
	stack.push_back(two);
	return (*cbp)(tr, one, two);
    }
    void print() const { cout << "N_VP" << "\n";}
    bool is_void() const { return true; }
};

/*23*/
struct Callback_N_P : public Callback {
    typedef void * (*FPT)(FspTranslator&, Pvec *);
    FPT cbp;

    Callback_N_P(FPT fp) : cbp(fp) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	Pvec * one = static_cast<Pvec *>(stack.back());
	return (*cbp)(tr, one);
    }
    void print() const { cout << "N_P" << "\n";}
    bool is_void() const { return true; }
};

/*24*/
struct Callback_N : public Callback {
    typedef void * (*FPT)(FspTranslator&);
    FPT cbp;

    Callback_N(FPT fp) : cbp(fp) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	return (*cbp)(tr);
    }
    void print() const { cout << "N" << "\n";}
    bool is_void() const { return true; }
};

/*25*/
struct Callback_null : public Callback {

    void * execute(FspTranslator &tr, vector<void *>& stack) {
	return NULL;
    }
    void print() const { cout << "null" << "\n";}
};

/*26*/
struct Callback_pop : public Callback {

    void * execute(FspTranslator &tr, vector<void *>& stack) {
	void * ret;

	stack.pop_back();
	ret = stack.back();
	stack.pop_back();
	return ret;
    }
    void print() const { cout << "pop2" << "\n";}
};

#endif
