/******************************************************************************/
/*!
\file   Delay.hpp
\author Zack Krolikowksi
\date   12/22/2020
\breif
    A queue that gets inited with correct size for delays 
*/
/******************************************************************************/
#ifndef DELAY_HPP
#define DELAY_HPP

#include <queue>
#include "Filter.hpp"

template<typename T>
class Delay : public Filter<T>
{
public:
    
    /*!************************************************************************
    \brief Init delay
    \param delayLength - Lenght of the delay in samples
    **************************************************************************/
    Delay(int delayLength)
    {
        for(int i = 0; i < delayLength; ++i)
            m_delay.push(0);
    }

    /*!************************************************************************
    \brief Get delayed value
    \param x - Current x value
    \return Returns the delayed value
    **************************************************************************/
    T operator()(T x) override
    {
        m_delay.push(x);
        T out = m_delay.front();
        return out;
    }

    T pop()
    {
        T out = m_delay.front();
        m_delay.pop();
        return out;
    }

    void push(T x)
    {
        m_delay.push(x);
    }

private:
    std::queue<T> m_delay; //!< delay
};
#endif