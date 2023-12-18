/* boost random.hpp header file
 *
 * Copyright Jens Maurer 2000-2001
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/random for documentation.
 *
 * $Id$
 *
 * Revision history
 *  2000-02-18  portability fixes (thanks to Beman Dawes)
 *  2000-02-21  shuffle_output, inversive_congruential_schrage,
 *              generator_iterator, uniform_smallint
 *  2000-02-23  generic modulus arithmetic helper, removed *_schrage classes,
 *              implemented Streamable and EqualityComparable concepts for 
 *              generators, added Bernoulli distribution and Box-Muller
 *              transform
 *  2000-03-01  cauchy, lognormal, triangle distributions; fixed 
 *              uniform_smallint; renamed gaussian to normal distribution
 *  2000-03-05  implemented iterator syntax for distribution functions
 *  2000-04-21  removed some optimizations for better BCC/MSVC compatibility
 *  2000-05-10  adapted to BCC and MSVC
 *  2000-06-13  incorporated review results
 *  2000-07-06  moved basic templates from namespace detail to random
 *  2000-09-23  warning removals and int64 fixes (Ed Brey)
 *  2000-09-24  added lagged_fibonacci generator (Matthias Troyer)
 *  2001-02-18  moved to individual header files
 */

#ifndef BOOST_RANDOM_HPP
#define BOOST_RANDOM_HPP

// generators
#include <LocalBoost/boost/random/additive_combine.hpp>
#include <LocalBoost/boost/random/discard_block.hpp>
#include <LocalBoost/boost/random/independent_bits.hpp>
#include <LocalBoost/boost/random/inversive_congruential.hpp>
#include <LocalBoost/boost/random/lagged_fibonacci.hpp>
#include <LocalBoost/boost/random/linear_congruential.hpp>
#include <LocalBoost/boost/random/linear_feedback_shift.hpp>
#include <LocalBoost/boost/random/mersenne_twister.hpp>
#include <LocalBoost/boost/random/mixmax.hpp>
#include <LocalBoost/boost/random/ranlux.hpp>
#include <LocalBoost/boost/random/shuffle_order.hpp>
#include <LocalBoost/boost/random/shuffle_output.hpp>
#include <LocalBoost/boost/random/subtract_with_carry.hpp>
#include <LocalBoost/boost/random/taus88.hpp>
#include <LocalBoost/boost/random/xor_combine.hpp>

// misc
#include <LocalBoost/boost/random/generate_canonical.hpp>
#include <LocalBoost/boost/random/seed_seq.hpp>
#include <LocalBoost/boost/random/random_number_generator.hpp>
#include <LocalBoost/boost/random/variate_generator.hpp>

// distributions
#include <LocalBoost/boost/random/bernoulli_distribution.hpp>
#include <LocalBoost/boost/random/beta_distribution.hpp>
#include <LocalBoost/boost/random/binomial_distribution.hpp>
#include <LocalBoost/boost/random/cauchy_distribution.hpp>
#include <LocalBoost/boost/random/chi_squared_distribution.hpp>
#include <LocalBoost/boost/random/discrete_distribution.hpp>
#include <LocalBoost/boost/random/exponential_distribution.hpp>
#include <LocalBoost/boost/random/extreme_value_distribution.hpp>
#include <LocalBoost/boost/random/fisher_f_distribution.hpp>
#include <LocalBoost/boost/random/gamma_distribution.hpp>
#include <LocalBoost/boost/random/geometric_distribution.hpp>
#include <LocalBoost/boost/random/hyperexponential_distribution.hpp>
#include <LocalBoost/boost/random/laplace_distribution.hpp>
#include <LocalBoost/boost/random/lognormal_distribution.hpp>
#include <LocalBoost/boost/random/negative_binomial_distribution.hpp>
#include <LocalBoost/boost/random/non_central_chi_squared_distribution.hpp>
#include <LocalBoost/boost/random/normal_distribution.hpp>
#include <LocalBoost/boost/random/piecewise_constant_distribution.hpp>
#include <LocalBoost/boost/random/piecewise_linear_distribution.hpp>
#include <LocalBoost/boost/random/poisson_distribution.hpp>
#include <LocalBoost/boost/random/student_t_distribution.hpp>
#include <LocalBoost/boost/random/triangle_distribution.hpp>
#include <LocalBoost/boost/random/uniform_01.hpp>
#include <LocalBoost/boost/random/uniform_int.hpp>
#include <LocalBoost/boost/random/uniform_int_distribution.hpp>
#include <LocalBoost/boost/random/uniform_on_sphere.hpp>
#include <LocalBoost/boost/random/uniform_real.hpp>
#include <LocalBoost/boost/random/uniform_real_distribution.hpp>
#include <LocalBoost/boost/random/uniform_smallint.hpp>
#include <LocalBoost/boost/random/weibull_distribution.hpp>

#include <LocalBoost/boost/random/generate_canonical.hpp>

#endif // BOOST_RANDOM_HPP
