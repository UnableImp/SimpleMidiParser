/******************************************************************************/
/*!
\file   BasicAllPass.hpp
\author Zack Krolikowksi
\date   12/22/2020
\breif
    A simple all pass filter for when the value of A is already known,
*/
/******************************************************************************/
#ifndef BASICALLPASS_HPP
#define BASICALLPASS_HPP

#include <cmath>
#include "Filter.hpp"

constexpr double PI = 3.141592653589793238462643;

template <typename T>
class BasicAllPass : public Filter<T>
{
public:
    BasicAllPass(T a) : a(a), x1(0), y1(0) {}

    /*!************************************************************************
    \brief Get value after all passed
    \param x - Current x value
    \return Returns the allpased value
    **************************************************************************/
    T operator()(T x) override
    {
        T y = a*(x - y1) + x1;
        x1 = x;
        y1 = y;
        return y;
    }

private:
    T a;  //!< Quaderture rotaion
    T x1; //!< Last x
    T y1; //!< Last y
};
#endif