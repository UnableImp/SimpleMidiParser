/******************************************************************************/
/*!
\file   BasicAllPass.hpp
\author Zack Krolikowksi
\date   12/22/2020
\breif
    A vary basic sounding plucked string filter
*/
/******************************************************************************/
#ifndef PLUCKEDSTRING_HPP
#define PLUCKEDSTRING_HPP

#include "Delay.hpp"
#include "BasicAllPass.hpp"
#include "BasicLowPass.hpp"

template<typename T>
class PluckedString : public Filter<T>
{
public:

    PluckedString(T R, T f, T decay) :  m_D(R/f), 
                                        m_L(std::floor(m_D-0.5)), 
                                        m_Delta((m_D - 0.5) - m_L),
                                        m_R(decay),
                                        m_R_L(std::pow(m_R, m_L)),
                                        m_Delay(m_L),
                                        m_AllPass( (std::sin(PI * f/R * (1 - m_Delta))) / (std::sin(PI * f/R * (1 + m_Delta)))),
                                        m_LowPass(m_R)
    {

    }

    T operator()(T x)
    {
        T y_l = m_Delay.pop();
        
        T y = x + (y_l * m_R_L);
        y = m_LowPass(y);
        y = m_AllPass(y);

        m_Delay.push(y);
        return y;

    }

private:
                       T m_D;       //!< Total delayed amount
                     int m_L;       //!< Length of delay filter
                       T m_Delta;   //!< ammount to shift by
                       T m_R;       //!< decay rate
                       T m_R_L;     //!< r^L
           Delay<double> m_Delay;   //!< Create the comb with about right freq peaks 
    BasicAllPass<double> m_AllPass; //!< All pass to shift value to right freq
    BasicLowPass<double> m_LowPass; //!< Basic lowpass as its going to be ran m_L times per sample
};
#endif