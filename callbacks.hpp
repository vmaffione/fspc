#ifndef __CALLBACKS__HH
#define __CALLBACKS__HH

/* Left contains a SetValue*, while right is the result of 'action_range'. */
SvpVec * indexize_svpvec(struct FspTranslator * gp, SvpVec * left, SvpVec * right);

SvpVec * callback__1(FspTranslator& tr, string * one); /*1*/
SvpVec * callback__2(FspTranslator& tr, SvpVec * one, string * two); /*2*/
SvpVec * callback__3(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__4(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__5(FspTranslator& tr, string * one); /*1*/
SvpVec * callback__6(FspTranslator& tr, string * one, string * two);
SvpVec * callback__7(FspTranslator& tr, string * one, SvpVec * two);
SvpVec * callback__8(FspTranslator& tr, string * one, SvpVec * two);
SvpVec * callback__9(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__13(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__14(FspTranslator& tr, string * one); /*1*/
void   * callback__15(FspTranslator& tr, string * one, Pvec * two, SvpVec * three);
SvpVec * callback__16(FspTranslator& tr, string * one); /*1*/
void * callback__17(FspTranslator& tr, string * one, SvpVec * two);
Pvec * callback__18(FspTranslator& tr, string * one, SvpVec * two, Pvec * three);
void * callback__19(FspTranslator& tr, SvpVec * one);
void * callback__20(FspTranslator& tr, SvpVec * one, Pvec * two);
Pvec * callback__21(FspTranslator& tr, SvpVec * one, Pvec * two, Pvec * three);
Pvec * callback__22(FspTranslator& tr);
Pvec * callback__23(FspTranslator& tr);
Pvec * callback__24(FspTranslator& tr);
Pvec * callback__25(FspTranslator& tr);
Pvec * callback__26(FspTranslator& tr, string * one, SvpVec * two);
Pvec * callback__27(FspTranslator& tr, Pvec * one);
Pvec * callback__28(FspTranslator& tr, Pvec * one);
Pvec * callback__29(FspTranslator& tr, Pvec * one, Pvec * two);
Pvec * callback__30(FspTranslator& tr, SvpVec * one);
Pvec * callback__31(FspTranslator& tr, SvpVec * one, Pvec * two, Pvec * three);
Pvec * callback__32(FspTranslator& tr, SvpVec * one);
Pvec * callback__33(FspTranslator& tr, Pvec * one, SvpVec * two);
SvpVec * callback__34(FspTranslator& tr);
SvpVec * callback__35(FspTranslator& tr, SvpVec * one);
SvpVec * callback__36(FspTranslator& tr, SvpVec * one, SvpVec * two); /*3*/
SvpVec * callback__37(FspTranslator& tr);
SvpVec * callback__38(FspTranslator& tr, SvpVec * one);
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
SvpVec * callback__59(FspTranslator& tr, SvpVec * one);
SvpVec * callback__60(FspTranslator& tr, SvpVec * one);
SvpVec * callback__61(FspTranslator& tr, int one);
SvpVec * callback__62(FspTranslator& tr, string * one); /*1*/
SvpVec * callback__63(FspTranslator& tr, string * one); /*1*/


struct Callback {
    virtual void * execute(FspTranslator &tr, vector<void *>& stack) = 0;
};

struct Callback_V_S : public Callback {
    typedef SvpVec * (*FPT)(FspTranslator&, string *);
    FPT cbp;
    string one;

    Callback_V_S(FPT fp, const string& s) : cbp(fp), one(s) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	return (*cbp)(tr, &one);
    }
};

struct Callback_V_VS : public Callback {
    typedef SvpVec * (*FPT)(FspTranslator&, SvpVec *, string *);
    FPT cbp;
    string two;

    Callback_V_VS(FPT fp, const string &s) : cbp(fp), two(s) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	SvpVec * one = static_cast<SvpVec *>(stack.back());
	stack.pop_back();
	return (*cbp)(tr, one, &two);
    }
};

struct Callback_V_VV : public Callback {
    typedef SvpVec * (*FPT)(FspTranslator&, SvpVec *, SvpVec *);
    FPT cbp;

    Callback_V_VV(FPT fp) : cbp(fp) { }
    void * execute(FspTranslator &tr, vector<void *>& stack) {
	SvpVec * two = static_cast<SvpVec *>(stack.back());
	SvpVec * one = static_cast<SvpVec *>(stack.back());
	stack.pop_back();
	stack.pop_back();
	return (*cbp)(tr, one, two);
    }
};

#endif
