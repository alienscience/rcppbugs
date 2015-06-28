#ifndef MULITMETHODS_H
#define MULITMETHODS_H

/**
    The MIT License (MIT)

    Copyright (c) 2015 Saul Hazledine

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
**/

#include <memory>
#include <cassert>

namespace multi {

// PossibleTypes is a container template that holds all the possible types
// of multimethod arguments
// TArgs - Possible types
template <typename... TArgs>
struct PossibleTypes
{
    //--------------------------------------------------------------------------

    // Visitor interface - used externally
    // Create an interface based on the given parameter pack
    template <typename... Ts> struct Visitor
    {
    };

    template <typename T>
    struct Visitor<T>
    {
        virtual ~Visitor() {}
        virtual void visit(const T& v) = 0;
    };

    template <typename T, typename... Ts>
    struct Visitor<T, Ts...> : public Visitor<Ts...>
    {
        // Premote functions from the base classes
        using Visitor<Ts...>::visit;
        virtual void visit(const T& v) = 0;
    };

    //--------------------------------------------------------------------------

    // ArgList interface - used internally
    // Create an interface based on the given parameter pack
    template <typename... Ts> struct ArgList
    {
    };

    template <typename T>
    struct ArgList<T>
    {
        virtual ~ArgList() {}
        virtual ArgList<TArgs...>* addArg(const T& v) = 0;
    };

    template <typename T, typename... Ts>
    struct ArgList<T, Ts...> : public ArgList<Ts...>
    {
        // Premote functions from the base classes
        using ArgList<Ts...>::addArg;
        virtual ArgList<TArgs...>* addArg(const T& v) = 0;
        virtual void apply() = 0;
    };

    //--------------------------------------------------------------------------

    // Implementation is a container template that holds the implementation
    // class
    template <typename TImpl>
    struct Implementation
    {
        //------------------------------------------------------------------

        template <typename... Ts>
        struct ArgStart :
                public ArgList<TArgs...>,
                public TImpl
        {
            ArgStart(const TImpl& impl) : TImpl(impl) {}
        };

        template <typename T, typename... Ts>
        struct ArgStart<T, Ts...> : public ArgStart<Ts...>
        {
            ArgStart(const TImpl& impl) : ArgStart<Ts...>(impl) {}

            using ArgStart<Ts...>::addArg;
            virtual ArgList<TArgs...>* addArg(const T& v);
            virtual void apply(){}
        };

        //-------------------------------------------------------------------

        template <typename T0, typename... Ts>
        struct Arg0 : public ArgStart<>
        {
            T0 arg0_;
            Arg0(const ArgStart<>& argStart, const T0& v) :
                ArgStart<>(argStart),
                arg0_(v) {}
        };

        template <typename T0, typename T, typename... Ts>
        struct Arg0<T0, T, Ts...> : public Arg0<T0,Ts...>
        {
            Arg0<T0,T,Ts...>(const ArgStart<>& argStart, const T0& v) :
                Arg0<T0,Ts...>(argStart,v) {}

            using Arg0<T0,Ts...>::addArg;
            virtual ArgList<TArgs...>* addArg(const T& v);
            void apply()
            {
                TImpl::apply(Arg0<T0>::arg0_);
            }
        };

        //--------------------------------------------------------------------

        template <typename T0, typename T1, typename... Ts>
        struct Arg1 : public Arg0<T0>
        {
            T1 arg1_;
            Arg1(const Arg0<T0>& arg0, const T1& v) :
                Arg0<T0>(arg0),
                arg1_(v) {}
        };

        template <typename T0, typename T1, typename T, typename... Ts>
        struct Arg1<T0, T1, T, Ts...> : public Arg1<T0,T1,Ts...>
        {
            Arg1<T0,T1,T,Ts...>(const Arg0<T0>& arg0, const T1& v) :
                Arg1<T0,T1,Ts...>(arg0,v) {}

            using Arg1<T0,T1,Ts...>::addArg;
            virtual ArgList<TArgs...>* addArg(const T& v);
            virtual void apply()
            {
                TImpl::apply(Arg0<T0>::arg0_, Arg1<T0,T1>::arg1_);
            }
        };

        //---------------------------------------------------------------------

        template <typename T0, typename T1, typename T2, typename... Ts>
        struct Arg2 : public Arg1<T0,T1>
        {
            T2 arg2_;
            Arg2(const Arg1<T0,T1>& arg1, const T2& v) :
                Arg1<T0,T1>(arg1),
                arg2_(v) {}
        };

        template <typename T0, typename T1, typename T2, typename T, typename... Ts>
        struct Arg2<T0, T1, T2, T, Ts...> : public Arg2<T0,T1,T2,Ts...>
        {
            Arg2<T0,T1,T2,T,Ts...>(const Arg1<T0,T1>& arg1, const T2& v) :
                Arg2<T0,T1,T2,Ts...>(arg1,v) {}
            using Arg2<T0,T1,T2,Ts...>::addArg;
            virtual ArgList<TArgs...>* addArg(const T& v) { return this; }
            virtual void apply()
            {
                TImpl::apply(Arg0<T0>::arg0_,
                             Arg1<T0,T1>::arg1_,
                             Arg2<T0,T1,T2>::arg2_);
            }
        };

    }; // Implementation

    //--------------------------------------------------------------------------

    template <typename TState, typename... Ts>
    struct Args : public Visitor<TArgs...>
    {
        Args(TState& state, ArgList<TArgs...>* args) :
            state_(state),
            args_(args) {}

        void apply()
        {
            args_->apply();
            // TODO: this would be cleaner as a static_cast
            state_ = TState(dynamic_cast<TState&>(*args_));
        }

    protected:
        TState& state_;
        std::unique_ptr<ArgList<TArgs...>> args_;
    };

    template <typename TState, typename T, typename... Ts>
    struct Args<TState, T, Ts...> : public Args<TState, Ts...>
    {
        Args<TState,T,Ts...>(TState& state, ArgList<TArgs...>* args) :
            Args<TState,Ts...>(state, args) {}

        using Args<TState,Ts...>::visit;
        using Args<TState,Ts...>::apply;
        void visit(const T& v)
        {
            Args<TState>::args_ =
                    std::unique_ptr<ArgList<TArgs...>>(Args<TState>::args_->addArg(v));
        }
    };

}; // PossibleTypes

//----- Template method implementations ----------------------------------------

template <typename... TArgs>
template <typename TImpl>
template <typename T, typename... Ts>
PossibleTypes<TArgs...>::ArgList<TArgs...>*
PossibleTypes<TArgs...>::Implementation<TImpl>::ArgStart<T,Ts...>::addArg(const T& v)
{
    return new Arg0<T,TArgs...>(*this,v);
}

template <typename... TArgs>
template <typename TImpl>
template <typename T0, typename T, typename... Ts>
PossibleTypes<TArgs...>::ArgList<TArgs...>*
PossibleTypes<TArgs...>::Implementation<TImpl>::Arg0<T0,T,Ts...>::addArg(const T& v)
{
    return new Arg1<T0,T,TArgs...>(*this,v);
}

template <typename... TArgs>
template <typename TImpl>
template <typename T0, typename T1, typename T, typename... Ts>
PossibleTypes<TArgs...>::ArgList<TArgs...>*
PossibleTypes<TArgs...>::Implementation<TImpl>::Arg1<T0,T1,T,Ts...>::addArg(const T& v)
{
    return new Arg2<T0,T1,T,TArgs...>(*this,v);
}

//----- Decorate the implementation with unimplemented arities -----------------

struct Base
{
    virtual ~Base() {}

    template <typename T0>
    void apply(const T0& arg0) { assert(false); }

    template <typename T0, typename T1>
    void apply(const T0& arg0, const T1& arg1) { assert(false); }

    template <typename T0, typename T1, typename T2>
    void apply(const T0& arg0, const T1& arg1, const T2& arg2) { assert(false); }

};

template <typename TImpl>
struct Mixin : public TImpl, public Base
{
    Mixin(const TImpl& impl) : TImpl(impl) {}
    virtual ~Mixin() {}
    TImpl impl()  { return static_cast<TImpl>(this); }

    using TImpl::apply; // Highest precedence
    using Base::apply;
};

//----- Convenience typedefs ---------------------------------------------------

template <typename... TArgs>
using Visitor = typename PossibleTypes<TArgs...>::template Visitor<TArgs...>;

//----- Create a multimethod ---------------------------------------------------

template <typename... TArgs, typename TImpl>
PossibleTypes<TArgs...>::Args<TImpl,TArgs...> method(TImpl& impl)
{
    Mixin<TImpl> wrappedImpl(impl);
    auto* emptyArgs =  new typename PossibleTypes<TArgs...>::
            template Implementation<Mixin<TImpl>>:: template ArgStart<TArgs...>(wrappedImpl);
    return typename PossibleTypes<TArgs...>::template Args<TImpl,TArgs...>(impl,emptyArgs);
}

} // namespace multi
#endif // MULITMETHODS_H

