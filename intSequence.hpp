/*
 * intSequence.hpp
 *
 *  Created on: May 31, 2019
 *      Author: edroi
 */

#ifndef INTSEQUENCE_HPP_
#define INTSEQUENCE_HPP_


namespace seq
{
// Int sequence generator
template<int ...> struct seq {};

template<int N, int ...S> struct gens : gens<N - 1, N - 1, S...> { };

template<int ...S> struct gens<0, S...>{ typedef seq<S...> type; };
};



#endif /* INTSEQUENCE_HPP_ */
