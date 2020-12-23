/******************************************************************************/
/*!
\file   BasicLowPass.hpp
\author Zack Krolikowksi
\date   12/22/2020
\breif
    A simple low pass filter, usefull in filteres that have large delays,
    like the plucked string
*/
/******************************************************************************/
#ifndef BASICLOWPASS_HPP
#define BASICLOWPASS_HPP

#include "Filter.hpp"

template<typename T>
class BasicLowPass : public Filter<T>
{  
public:
    BasicLowPass(T  a1) : a1 (a1), x1(0) {}
    
    /*!************************************************************************
    \brief Get lowpassed value
    \param x - Current x value
    \return Returns the lowpassed value
    **************************************************************************/
    T operator()(T x) override
    {
        T out = 1/(static_cast<T>(2)) * (x + a1 * x1);

        x1 = x;

        return out; 
    }

private:
    T a1; //!< Decay *power*
    T x1; //!< last x
};
#endif