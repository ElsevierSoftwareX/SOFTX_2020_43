#pragma once
#include "../grid_traits_backend_fwd.hpp"
#include "../../run_functor_arguments_fwd.hpp"
#include "execute_kernel_functor_host_fwd.hpp"

namespace gridtools {

    namespace icgrid {
        template <>
        struct grid_traits_arch< enumtype::Host > {
            template < typename RunFunctorArguments >
            struct kernel_functor_executer {
                GRIDTOOLS_STATIC_ASSERT((is_run_functor_arguments< RunFunctorArguments >::value), "Error");
                typedef execute_kernel_functor_host< RunFunctorArguments > type;
            };
        };
    }
}
