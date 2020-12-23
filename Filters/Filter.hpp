/******************************************************************************/
/*!
\file   Filter.hpp
\author Zack Krolikowksi
\date   12/22/2020
\breif
    A filter class, that all my other filters will be diriving from
*/
/******************************************************************************/
#ifndef FILTER_HPP
#define FILTER_HPP

template <typename T>
class Filter
{
public:
    virtual ~Filter() {};
    virtual T operator()(T x) {return x;};
};
#endif