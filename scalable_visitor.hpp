/*
 *  Scalable Visitor - header-only library
 *
 *  Copyright (C) 2014  Cosimo Sacco
 *  Email contact: <cosimosacco@gmail.com>
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

#ifndef __SCALABLE_VISITOR__HH
#define __SCALABLE_VISITOR__HH

#include <stdexcept>

/* This simple library offers a framework of template classes inspired to the
 * Acyclic Visitor design pattern (Acyclic Visitor - Robert C. Martin, Object
 * Mentor).
 * __________________________________Rationale_________________________________
 * GoF's Visitor pattern has some drawbacks:
 * - it should be used with a relatively static (ie. which doesn't change often)
 * hierarchy of visitable classes, since each addition to the visitable
 * hierarchy reflects directly on the visitor interface, which must be extended
 * with a new visit method (visit(Circle*), visit(Square*)...) even if that kind
 * of visit has no meaning (visit(HTMLMeta&) in a HTMLRenderingVisitor);
 * - the programmer must copy an accept method and paste it into each visitable
 * class (each visitable class must redefine accept, but the redefinition is
 * always the same (visitor.visit(*this););
 *
 * The Acyclic Visitor pattern solves the first problem using RTTI and multiple
 * inheritance. Here's a brief description:
 * - the Visitor class becomes a degenerate class;
 * - for each XVisitable class a XVisitor interface must be defined, which
 * offers a virtual void visit(X&) method;
 * - A ParticularVisitor extends Visitor (so that it IS a Visitor) and an
 * XVisitor interface for each XVisitable class it should be able to visit;
 * - Each XVisitable class must implement an accept(Visitor&) method which tries
 * to side-cast[1] the visitor to an XVisitor and, if possible, calls visit on
 * it, else it throws and exception.
 *
 * Also the Acyclic Visitor pattern, as is, suffers of some limitations:
 * - As in GoF Visitor, each XVisitable must re-implement the same accept method
 * (code replication) but the only difference between those accept methods is
 * the type of XVisitor to use when side-casting the accepted visitor;
 * - Each XVisitable must know which XVisitor interface exposes
 * visit(XVisitable&);
 *
 * Those problems are addressed by the Scalable Visitor framework:
 * - A XVisitor must extend Vistor and the AddVisit<XVisitable, T> interface,
 * which adds a virtual T visit(XVisitable&) method to be defined in each
 * particular implementation;
 * - A XVisitable must extend AddAccept<XVisitable, T> (Curiously Recurring
 * Template Pattern), which offers the right accept method (the visitor is
 * side-cast to AddVisit<Xvisitable, T>).
 *
 * Some use cases:
 *
 * ______________________________Visitable Objects_____________________________
 * class GeometricFigure{...};
 * class Dot : public GeometricFigure, public AddAccept<Dot>{...};
 * class Line : public GeometricFigure, public AddAccept<Line>{...};
 * class Square : public GeometricFigure, public AddAccept<Square>{...};
 * class Circle : public GeometricFigure, public AddAccept<Circle>{...};
 *
 * __________________________________Visitors__________________________________
 * class PositionVisitor :
 *  public Visitor,
 *  public AddVisit<GeometricFigure, Coordinates> {...};
 *
 * class PerimeterVisitor :
 *  public Visitor,
 *  public AddVisit<Line, double>,
 *  public AddVisit<Square, double>,
 *  public AddVisit<Circle, double> {...};
 *
 * class AreaVisitor :
 *  public Visitor,
 *  public AddVisit<Square, double>,
 *  public AddVisit<Circle, double> {...};
 *
 * class SlopeVisitor :
 *  public Visitor,
 *  public AddVisit<Line> {...};
 * __________________________________Use case__________________________________
 * Cicle c(...);
 * AreaVisitor v;
 * double circleArea = v.visit(c);
 *
 * Line l(...);
 * double lineArea = v.visit(l); <- compile time error: v has no visit(Line&)
 *
 * Dot d(...);
 * d.accept(v); <- runtime error: v can't be side-cast to AddVisit<Dot, void>
 *
 * __________________________Advantages and Drawbacks__________________________
 * Advantages:
 * (+) no accept boilerplate code
 *      => better than GoF Visitor and Acyclic Visitor;
 * (+) no need to define a XVisitor interface for each XVisitable class
 *      => better than Acyclic Visitor;
 * (+) no dependencies between visitable classes and visit methods (you have to
 * define only meaningful visit methods)
 *      ~~ like Acyclic Visitor;
 *
 * Drawbacks:
 * (-) This pattern uses RTTI, which can have high performance costs;
 *      ~~ like Acyclic Visitor;
 * (-) This pattern uses multiple inheritance;
 *      ~~ like Acyclic Visitor;
 *
 * ____________________________________Notes____________________________________
 * [1] By side-cast i mean this:
 * class C : public A, public B {...};
 * class D : public A, public Z {...};
 * ...
 * A* pC = new C(...);
 * A* pD = new D(...);
 * ...
 * B* p1 = dynamic_cast<B>(pC); <---successful side-cast
 * B* p2 = dynamic_cast<B>(pD); <---fails, D doesn't extend B
 * */
namespace visitor {


/**
 * @class Concept
 * @tparam Dummy A dummy parameter used to let the class be a template.
 * @brief Abstract degenerate template class representing an idea.
 *
 * Visitor must be only a "handle type", its purpose is only to let us know
 * that an object is a Visitor and may implement other visitor interfaces. So,
 * it should be a degenerate class and should't be instantiated (abstract).
 * A class must declare at least one pure virtual method in order to be
 * abstract. Since Visitor should be degenerate, the only method that can be
 * made pure virtual is the destructor. Anyway, a destructor body must always be
 * defined for a base class, even if empty. Defining a method in an header file
 * results in linking problems, so it should be defined in an implementation
 * file. However this library is a header only library, so here's the trick:
 * make a Concept class, which is a template class and so must be defined
 * in the header file (that causes no linking problems since the template
 * disappears and only its instantiations remain), and call it Visitor with a
 * simple typedef.
 **/
template <typename Dummy> class Concept {
    public:
        virtual ~Concept() = 0;
};
template <typename Dummy> Concept <Dummy>::~Concept() {}
typedef Concept<void> Visitor;

/**
 * @class AddVisit
 * @tparam ElementType The type of the visited object
 * @tparam ReturnType The type returned by visit
 * @brief Adds a visit interface to the visitor
 *
 * A visitor class (ie. a class extending Visitor) may implement several
 * AddVisit interfaces. If a visitor class inherits from AddVisit<E, R>,
 * a virtual R visit(E&) method is added to its definition and must be defined.
 **/
template <typename ElementType, typename ReturnType> class AddAccept;
template <typename ElementType, typename ReturnType =
          void>class AddVisit {
    public:
        virtual ReturnType visit(ElementType&) = 0;
};

/**
 * @class Element
 * @tparam ReturnType The type returned by accept
 * @brief A generic visitable element
 *
 * Classes extending Element must implement an accept method
 **/
template <typename ReturnType = void> class Element {
    public:
        virtual ~ Element() = 0;
        virtual ReturnType accept(Visitor&) = 0;
};
template <typename ReturnType> Element <ReturnType>::~Element() {}

/**
 * @class AddAccept
 * @tparam ElementType The actual type of the visitable object
 * @tparam ReturnType The type returned by accept
 * @brief Interface to make an object visitable
 *
 * In order to make class X visitable, it must inherit from AddAccept<X, R>
 * (Curiously Recurring Template Pattern), which defines exactly the right
 * accept method for X.
 **/
template <typename ElementType, typename ReturnType = void> class AddAccept:
    public virtual Element <ReturnType> {
    public:
        typedef AddVisit<ElementType, ReturnType> ActualVisitor;
        /**
         * The accept method receives a Visitor. If the actual type of the
         * visitable object (known through type injection using CRTP) is X,
         * the visitor must implement interface AddVisit<X, R>. If the passed
         * visitor implements it, the visit can be performed (the visitor offers
         * the right method); else, an exception is thrown.
         * @param visitor The visiting visitor
         * @return Result of the visit
         **/
        virtual ReturnType accept(Visitor& visitor)
        {
            ActualVisitor* v =
                dynamic_cast<ActualVisitor*>(&visitor);
            ElementType* self =
                dynamic_cast<ElementType*>(this);

            if (v != nullptr && self != nullptr)
                return v->visit(*self);
            else
                throw
                std::runtime_error("Impossible element visit");
        }
        virtual ~ AddAccept() = 0;
};
template <typename ElementType, typename ReturnType> AddAccept
<ElementType, ReturnType>::~AddAccept() {}

}

#endif
