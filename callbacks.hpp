#ifndef __CALLBACKS__HH
#define __CALLBACKS__HH

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
SvpVec * callback__14(FspTranslator& tr, string * one); /*1*/
Pvec * callback__15(FspTranslator& tr, string * one, Pvec * two, SvpVec * three); /*7*/
SvpVec * callback__16(FspTranslator& tr, string * one); /*1*/
Pvec * callback__17(FspTranslator& tr, string * one, SvpVec * two); /*6*/
Pvec * callback__18(FspTranslator& tr, string * one, SvpVec * two, Pvec * three); /*8*/
Pvec * callback__19(FspTranslator& tr, SvpVec * one); /*9*/
Pvec * callback__20(FspTranslator& tr, SvpVec * one, Pvec * two); /*10*/
Pvec * callback__21(FspTranslator& tr, SvpVec * one, Pvec * two, Pvec * three); /*11*/
Pvec * callback__22(FspTranslator& tr); /*12*/
Pvec * callback__23(FspTranslator& tr); /*12*/
Pvec * callback__24(FspTranslator& tr); /*12*/
Pvec * callback__25(FspTranslator& tr); /*12*/
Pvec * callback__26(FspTranslator& tr, string * one, SvpVec * two); /*6*/
Pvec * callback__27(FspTranslator& tr, Pvec * one); /*13*/
Pvec * callback__28(FspTranslator& tr, Pvec * one); /*13*/
Pvec * callback__29(FspTranslator& tr, Pvec * one, Pvec * two); /*14*/
Pvec * callback__30(FspTranslator& tr, SvpVec * one); /*9*/
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


struct Callback {
    virtual void * execute(FspTranslator &tr, vector<void *>& stack) = 0;
};

/*1*/
struct Callback_V_S : public Callback {
    typedef SvpVec * (*FPT)(FspTranslator&, string *);
    FPT cbp;
    string one;

    Callback_V_S(FPT fp, const string& s) : cbp(fp), one(s) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	return (*cbp)(tr, new string(one));
    }
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
};

/*7*/
struct Callback_P_SPV : public Callback {
    typedef Pvec * (*FPT)(FspTranslator&, string *, Pvec *, SvpVec *);
    FPT cbp;
    string one;

    Callback_P_SPV(FPT fp, const string& s) : cbp(fp), one(s) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	SvpVec * three = static_cast<SvpVec *>(stack.back());
	stack.pop_back();
	Pvec * two = static_cast<Pvec *>(stack.back());
	stack.pop_back();
	return (*cbp)(tr, new string(one), two, three);
    }
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
};

/*12*/
struct Callback_P : public Callback {
    typedef Pvec * (*FPT)(FspTranslator&);
    FPT cbp;

    Callback_P(FPT fp) : cbp(fp) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	return (*cbp)(tr);
    }
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
};

/*16*/
struct Callback_V : public Callback {
    typedef SvpVec * (*FPT)(FspTranslator&);
    FPT cbp;

    Callback_V(FPT fp) : cbp(fp) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	return (*cbp)(tr);
    }
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
};

/*18*/
struct Callback_V_I : public Callback {
    typedef SvpVec * (*FPT)(FspTranslator&, int);
    FPT cbp;
    int one;

    Callback_V_I(FPT fp) : cbp(fp) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	return (*cbp)(tr, one);
    }
};
#endif
