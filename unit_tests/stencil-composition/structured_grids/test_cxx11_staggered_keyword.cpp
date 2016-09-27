/*
  GridTools Libraries

  Copyright (c) 2016, GridTools Consortium
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  For information: http://eth-cscs.github.io/gridtools/
*/
#include "gtest/gtest.h"
#include "stencil-composition/stencil-composition.hpp"

#ifdef __CUDACC__
#define BACKEND backend< Cuda, GRIDBACKEND, Block >
#else
#ifdef BACKEND_BLOCK
#define BACKEND backend< Host, GRIDBACKEND, Block >
#else
#define BACKEND backend< Host, GRIDBACKEND, Naive >
#endif
#endif

namespace test_staggered_keyword {
    using namespace gridtools;
    using namespace enumtype;

    typedef gridtools::interval< level< 0, -2 >, level< 1, 1 > > axis;
    typedef gridtools::interval< level< 0, -1 >, level< 1, -1 > > x_interval;

    struct functor {
        static uint_t ok_i;
        static uint_t ok_j;

        typedef accessor< 0, gridtools::enumtype::inout > p_i;
        typedef accessor< 1 > p_j;
        typedef boost::mpl::vector< p_i, p_j > arg_list;
        template < typename Evaluation >
        GT_FUNCTION static void Do(Evaluation const &eval, x_interval) {
            // std::cout<<"i: "<< eval(p_i(-5,-5,0)) <<", j: "<<eval(p_j(-5,-5,0))<< std::endl;
            if (eval(p_i(-5, -5, 0)) == 5)
                ok_i++;
            if (eval(p_j(-5, -5, 0)) == 5)
                ok_j++;
        }
    };
    uint_t functor::ok_i = 0;
    uint_t functor::ok_j = 0;

    bool test() {

        typedef gridtools::layout_map< 0, 1, 2 > layout_t;
        typedef gridtools::BACKEND::storage_info< 0, layout_t > meta_t;
        typedef gridtools::BACKEND::storage_type< uint_t, meta_t >::type storage_type;

        meta_t meta_((uint_t)30, (uint_t)20, (uint_t)1);
        storage_type i_data(meta_);
        storage_type j_data(meta_);

        auto lam_i = [](uint_t const &i_, uint_t const &j_, uint_t const &k_) -> uint_t { return i_; };
        auto lam_j = [](uint_t const &i_, uint_t const &j_, uint_t const &k_) -> uint_t { return j_; };

        i_data.initialize(lam_i);
        j_data.initialize(lam_j);

        uint_t di[5] = {0, 0, 5, 30 - 1, 30};
        uint_t dj[5] = {0, 0, 5, 20 - 1, 20};

        gridtools::grid< axis > grid(di, dj);
        grid.value_list[0] = 0;
        grid.value_list[1] = 1 - 1;

        typedef arg< 0, storage_type > p_i_data;
        typedef arg< 1, storage_type > p_j_data;
        typedef boost::mpl::vector< p_i_data, p_j_data > accessor_list;

        aggregator_type< accessor_list > domain(boost::fusion::make_vector(&i_data, &j_data));
        auto comp = gridtools::make_computation< gridtools::BACKEND >(
            domain,
            grid,
            gridtools::make_multistage(execute< forward >(),
                gridtools::make_stage< functor, staggered< 5, 5, 5, 5 > >(p_i_data(), p_j_data())));

        comp->ready();
        comp->steady();
        comp->run();

        return (functor::ok_i && functor::ok_j);
    }
} // namespace test_staggered_keyword

TEST(stencil, test_staggered_keyword) { EXPECT_TRUE(test_staggered_keyword::test()); }