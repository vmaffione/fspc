#include <iostream>
#include <map>
#include <cassert>

/* Debug the refcounts.
    0 --> no check is performed
    1 --> we check against memory leaks
    2 --> we check against memory leaks, being verbose
 */
#define DBG_REFCNT 1

#if (DBG_REFCNT >= 2)
#define DBR(x) x
#else
#define DBR(x)
#endif

#if (DBG_REFCNT >= 1)
#define DBRT(x) x
#else
#define DBRT(x)
#endif


namespace fsp {

/* A singleton class containing a table which maps pointers to reference
   counters.
   The methods ref() and unref() are called by the smart pointers
   to debug/record their operations. When check() is called (this is done
   once when the compilation is done) it checks that all the reference
   counters stored in the table are 0, meaning that all the objects
   have correctly destroyed. */
class PtrCheckTable {
        std::map<void *, unsigned int> t;
        static PtrCheckTable *instance;
        PtrCheckTable() { }

    public:
        static PtrCheckTable *get();
        void ref(void *ptr);
        void unref(void *ptr);
        void check();
};

/* Smart pointer to an object of type T. */
template <class T>
class SmartPtr {
        T *ptr;

        void get(const char *nm);
        void put(const char *nm);

    public:
        SmartPtr();
        SmartPtr(T *);
        SmartPtr(const SmartPtr&);
        SmartPtr& operator=(T* p);
        SmartPtr& operator=(SmartPtr&);
        T* operator->() const { return ptr; }
        T* delegate();
        operator T*() const;
        ~SmartPtr();
        void clear();
};


/* =========================== SmartPtr ============================== */

template <class T>
fsp::SmartPtr<T>::SmartPtr() : ptr(NULL)
{
}

/* Build a SmartPtr instance from a pointer. */
template <class T>
fsp::SmartPtr<T>::SmartPtr(T *p)
{
    ptr = p;

    get(__func__);
}

template <class T>
void fsp::SmartPtr<T>::get(const char *nm)
{
    if (ptr) {
        /* Now *this holds *lts, so increment the refcount. */
        ptr->refcount++;
        DBR(std::cout << nm << ":"<< ptr << ":" <<
            ptr->refcount - ptr->delegated  << "+" <<
            ptr->delegated << "\n");
        DBRT(PtrCheckTable::get()->ref(ptr));
    }
}

template <class T>
void fsp::SmartPtr<T>::put(const char *nm)
{
    if (ptr) {
        /* We are going to loose what we held previously.
           Decrement the refcount of what we held and maybe free the
           referenced object. */
        ptr->refcount--;
        DBR(std::cout << nm << ":"<< ptr << ":" <<
            ptr->refcount - ptr->delegated  << "+" <<
            ptr->delegated << "\n");
        DBRT(PtrCheckTable::get()->unref(ptr));
        assert(ptr->refcount >= 0);
        if (ptr->refcount == 0) {
            delete ptr;
        }
    }
}

/* Build a SmartPtr instance from another SmartPtr instance. */
template <class T>
fsp::SmartPtr<T>::SmartPtr(const SmartPtr& p)
{
    ptr = p.ptr;

    get(__func__);
}

/* Assign the object pointed by another SmartPtr instance. */
template <class T>
fsp::SmartPtr<T>& fsp::SmartPtr<T>::operator=(fsp::SmartPtr<T>& p)
{
    put(__func__);

    ptr = p.ptr;

    get(__func__);

    return *this;
}

/* Assign the object pointed by the input pointer. */
template <class T>
fsp::SmartPtr<T>& fsp::SmartPtr<T>::operator=(T *p)
{
    put(__func__);

    ptr = p;

    get(__func__);

    return *this;
}

/* The user is going to either:
   - pass *this to a function argument of type T*, or
   - assign *this to an T*

   In both cases (well, it's just the same case) just make the
   request go through, without modify the referenced object refcount.
   We want the user to ask explicitely to take care of the referenced
   object (see fsp::SmartPtr<fsp::Lts>::delegate).
   */
template <class T>
fsp::SmartPtr<T>::operator T*() const
{
    return ptr;
}

/* The user is explicitly asking to take care of the referenced object,
   if any. This means that even when the last fsp::SmartPtr<fsp::Lts> instance that was
   assigned the referenced object dies, we don't want to destroy the
   object, because now also the user takes care of it. We implement
   this behaviour by incrementing the refcount, that is we add a reference
   that we delegate to the caller. From now on __also__ the caller is
   responsible of the deallocation. If the user does not decrement
   the refcount, the object will be never freed by fsp::SmartPtr<fsp::Lts> (but it can be
   freed by the user manually, of course).
 */
template <class T>
T* fsp::SmartPtr<T>::delegate()
{
    get(__func__);
    DBR(if (ptr) ptr->delegated++);
    DBRT(if (ptr) PtrCheckTable::get()->unref(ptr));

    return ptr;
}

template <class T>
fsp::SmartPtr<T>::~SmartPtr()
{
    put(__func__);

    ptr = NULL;
}

/* This emulates a call to the destructor, and resets the fsp::SmartPtr<fsp::Lts>
   instance. */
template <class T>
void fsp::SmartPtr<T>::clear()
{
    put(__func__);

    ptr = NULL;
}

}  /* namespace fsp */

