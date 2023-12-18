
#ifndef BOOST_MPL_ZIP_VIEW_HPP_INCLUDED
#define BOOST_MPL_ZIP_VIEW_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2010
// Copyright David Abrahams 2000-2002
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id$
// $Date$
// $Revision$

#include <LocalBoost/boost/mpl/transform.hpp>
#include <LocalBoost/boost/mpl/begin_end.hpp>
#include <LocalBoost/boost/mpl/iterator_tags.hpp>
#include <LocalBoost/boost/mpl/next.hpp>
#include <LocalBoost/boost/mpl/lambda.hpp>
#include <LocalBoost/boost/mpl/deref.hpp>
#include <LocalBoost/boost/mpl/aux_/na_spec.hpp>

namespace boost { namespace mpl {

template< typename IteratorSeq >
struct zip_iterator
{
    typedef forward_iterator_tag category;
    typedef typename transform1<
          IteratorSeq
        , deref<_1>
        >::type type;

    typedef zip_iterator<
          typename transform1<
                IteratorSeq
              , mpl::next<_1>
            >::type
        > next;
};

template<
      typename BOOST_MPL_AUX_NA_PARAM(Sequences)
    >
struct zip_view
{
 private:
    typedef typename transform1< Sequences, mpl::begin<_1> >::type first_ones_;
    typedef typename transform1< Sequences, mpl::end<_1> >::type last_ones_;
    
 public:
    typedef nested_begin_end_tag tag;
    typedef zip_view type;
    typedef zip_iterator<first_ones_> begin;
    typedef zip_iterator<last_ones_> end;
};

BOOST_MPL_AUX_NA_SPEC(1, zip_view)

}}

#endif // BOOST_MPL_ZIP_VIEW_HPP_INCLUDED
