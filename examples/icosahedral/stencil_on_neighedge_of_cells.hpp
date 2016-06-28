#include "gtest/gtest.h"
#include <boost/mpl/equal.hpp>
#include <stencil-composition/stencil-composition.hpp>
#include "tools/verifier.hpp"
#include "unstructured_grid.hpp"

using namespace gridtools;
using namespace enumtype;

namespace soneoc {

#ifdef __CUDACC__
#define BACKEND backend< Cuda, GRIDBACKEND, Block >
#else
#ifdef BACKEND_BLOCK
#define BACKEND backend< Host, GRIDBACKEND, Block >
#else
#define BACKEND backend< Host, GRIDBACKEND, Naive >
#endif
#endif

    using backend_t = BACKEND;
    using icosahedral_topology_t = ::gridtools::icosahedral_topology< backend_t >;

    typedef gridtools::interval< level< 0, -1 >, level< 1, -1 > > x_interval;
    typedef gridtools::interval< level< 0, -2 >, level< 1, 1 > > axis;

    template < uint_t Color >
    struct test_on_edges_functor {
        typedef in_accessor< 0, icosahedral_topology_t::edges, extent< 1 > > in;
        typedef inout_accessor< 1, icosahedral_topology_t::cells > out;
        typedef boost::mpl::vector< in, out > arg_list;

        template < typename Evaluation >
        GT_FUNCTION static void Do(Evaluation const &eval, x_interval) {
            auto ff = [](const double _in, const double _res) -> double { return _in + _res; };

            /**
               This interface checks that the location types are compatible with the accessors
             */
            eval(out()) = eval(on_edges(ff, 0.0, in()));
        }
    };

    bool test(uint_t x, uint_t y, uint_t z, uint_t t_steps, bool verify) {

        uint_t d1 = x;
        uint_t d2 = y;
        uint_t d3 = z;

        using cell_storage_type = typename backend_t::storage_t< icosahedral_topology_t::cells, double >;
        using edge_storage_type = typename backend_t::storage_t< icosahedral_topology_t::edges, double >;

        const uint_t halo_nc = 1;
        const uint_t halo_mc = 1;
        const uint_t halo_k = 0;

        icosahedral_topology_t icosahedral_grid(d1, d2, d3);

        auto in_edges = icosahedral_grid.make_storage< icosahedral_topology_t::edges, double >("in_edge");
        auto out_cells = icosahedral_grid.make_storage< icosahedral_topology_t::cells, double >("out");
        auto ref_on_edges = icosahedral_grid.make_storage< icosahedral_topology_t::cells, double >("ref_on_edges");

        for (int i = 1; i < d1 - 1; ++i) {
            for (int c = 0; c < icosahedral_topology_t::edges::n_colors::value; ++c) {
                for (int j = 1; j < d2 - 1; ++j) {
                    for (int k = 0; k < d3; ++k) {
                        in_edges(i, c, j, k) =
                            in_edges.meta_data().index(array< uint_t, 4 >{(uint_t)i, (uint_t)c, (uint_t)j, (uint_t)k});
                    }
                }
            }
        }

        out_cells.initialize(0.0);
        ref_on_edges.initialize(0.0);

        typedef arg< 0, edge_storage_type > p_in_edges;
        typedef arg< 1, cell_storage_type > p_out_cells;

        typedef boost::mpl::vector< p_in_edges, p_out_cells > accessor_list_edges_t;

        gridtools::domain_type< accessor_list_edges_t > domain_edges(boost::fusion::make_vector(&in_edges, &out_cells));

        array< uint_t, 5 > di = {halo_nc, halo_nc, halo_nc, d1 - halo_nc - 1, d1};
        array< uint_t, 5 > dj = {halo_mc, halo_mc, halo_mc, d2 - halo_mc - 1, d2};

        gridtools::grid< axis, icosahedral_topology_t > grid_(icosahedral_grid, di, dj);
        grid_.value_list[0] = 0;
        grid_.value_list[1] = d3 - 1;

        auto stencil_edges = gridtools::make_computation< backend_t >(
            domain_edges,
            grid_,
            gridtools::make_mss // mss_descriptor
            (execute< forward >(),
                gridtools::make_esf< test_on_edges_functor, icosahedral_topology_t, icosahedral_topology_t::cells >(
                    p_in_edges(), p_out_cells())));

        stencil_edges->ready();
        stencil_edges->steady();
        stencil_edges->run();

#ifdef __CUDACC__
        out_cells.d2h_update();
#endif

        bool result = true;
        if (verify) {
            unstructured_grid ugrid(d1, d2, d3);
            for (uint_t i = halo_nc; i < d1 - halo_nc; ++i) {
                for (uint_t c = 0; c < icosahedral_topology_t::cells::n_colors::value; ++c) {
                    for (uint_t j = halo_mc; j < d2 - halo_mc; ++j) {
                        for (uint_t k = 0; k < d3; ++k) {
                            auto neighbours =
                                ugrid.neighbours_of< icosahedral_topology_t::cells, icosahedral_topology_t::edges >(
                                    {i, c, j, k});
                            for (auto iter = neighbours.begin(); iter != neighbours.end(); ++iter) {
                                ref_on_edges(i, c, j, k) += in_edges(*iter);
                            }
                        }
                    }
                }
            }

            verifier ver(1e-10);
            array< array< uint_t, 2 >, 4 > halos = {{{halo_nc, halo_nc}, {0, 0}, {halo_mc, halo_mc}, {halo_k, halo_k}}};

            result = ver.verify(grid_, ref_on_edges, out_cells, halos);
        }
#ifdef BENCHMARK
        for (uint_t t = 1; t < t_steps; ++t) {
            stencil_edges->run();
        }
        stencil_edges->finalize();
        std::cout << stencil_edges->print_meter() << std::endl;

#endif

        return result;
    }

} // namespace soc
