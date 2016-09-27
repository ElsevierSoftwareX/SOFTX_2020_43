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
#pragma once

#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/sequence/intrinsic/at.hpp>
#include <boost/fusion/view/zip_view.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/as_set.hpp>

#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/mpl/at.hpp>
#include <boost/fusion/include/size.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/container/vector/vector_fwd.hpp>
#include <boost/fusion/include/vector_fwd.hpp>
#include <boost/fusion/container/generation/make_vector.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/fusion/sequence/io.hpp>
#include <boost/fusion/include/io.hpp>

#include <common/array.hpp>
#include "../../common/gt_assert.hpp"
#include <boost/mpl/vector.hpp>
#include "location_type.hpp"
#include "common/array_addons.hpp"

namespace gridtools {

    // TODO this is duplicated below

    namespace {
        using cells = location_type< 0, 2 >;
        using edges = location_type< 1, 3 >;
        using vertexes = location_type< 2, 1 >;
    }

    template < typename T, typename ValueType >
    struct return_type {
        typedef array< ValueType, 0 > type;
    };

    // static triple dispatch
    template < typename Location1 >
    struct from {
        template < typename Location2 >
        struct to {
            template < typename Color >
            struct with_color;
        };
    };

    template < typename ValueType >
    struct return_type< from< cells >::template to< cells >, ValueType > {
        typedef array< ValueType, 3 > type;
    };

    template < typename ValueType >
    struct return_type< from< cells >::template to< edges >, ValueType > {
        typedef array< ValueType, 3 > type;
    };

    template < typename ValueType >
    struct return_type< from< cells >::template to< vertexes >, ValueType > {
        typedef array< ValueType, 3 > type;
    };

    template < typename ValueType >
    struct return_type< from< edges >::template to< edges >, ValueType > {
        typedef array< ValueType, 4 > type;
    };

    template < typename ValueType >
    struct return_type< from< edges >::template to< cells >, ValueType > {
        typedef array< ValueType, 2 > type;
    };

    template < typename ValueType >
    struct return_type< from< edges >::template to< vertexes >, ValueType > {
        typedef array< ValueType, 2 > type;
    };

    template < typename ValueType >
    struct return_type< from< vertexes >::template to< vertexes >, ValueType > {
        typedef array< ValueType, 6 > type;
    };

    template < typename ValueType >
    struct return_type< from< vertexes >::template to< cells >, ValueType > {
        typedef array< ValueType, 6 > type;
    };

    template < typename ValueType >
    struct return_type< from< vertexes >::template to< edges >, ValueType > {
        typedef array< ValueType, 6 > type;
    };

    template < uint_t SourceColor >
    struct get_connectivity_offset {

        template < int Idx >
        struct get_element {
            GT_FUNCTION
            constexpr get_element() {}

            template < typename Offsets >
            GT_FUNCTION constexpr static array< uint_t, 4 > apply(array< uint_t, 3 > const &i, Offsets offsets) {
                return {i[0] + offsets[Idx][0],
                    SourceColor + offsets[Idx][1],
                    i[1] + offsets[Idx][2],
                    i[2] + offsets[Idx][3]};
            }
        };
    };

    template < typename T >
    struct is_grid_topology;

    template < typename DestLocation, typename GridTopology, uint_t SourceColor >
    struct get_connectivity_index {

        GRIDTOOLS_STATIC_ASSERT((is_grid_topology< GridTopology >::value), "Error");
        GRIDTOOLS_STATIC_ASSERT((is_location_type< DestLocation >::value), "Error");

        template < int Idx >
        struct get_element {
            GT_FUNCTION
            constexpr get_element() {}

            template < typename Offsets >
            GT_FUNCTION static uint_t apply(
                GridTopology const &grid_topology, array< uint_t, 3 > const &i, Offsets offsets) {
                return boost::fusion::at_c< DestLocation::value >(grid_topology.virtual_storages())
                    .index(get_connectivity_offset< SourceColor >::template get_element< Idx >::apply(i, offsets));
            }
        };
    };

    /**
     * Following specializations provide all information about the connectivity of the icosahedral/ocahedral grid
     * While ordering is arbitrary up to some extent, if must respect some rules that user expect, and that conform
     * part of an API. Rules are the following:
     *   1. Flow variables on edges by convention are outward on downward cells (color 0) and inward on upward cells
     * (color 1)
     *      as depicted below
     @verbatim
              ^
              |                   /\
         _____|____              /  \
         \        /             /    \
          \      /             /      \
      <----\    /---->        /-->  <--\
            \  /             /     ^    \
             \/             /______|_____\
     @endverbatim
     *   2. Neighbor edges of a cell must follow the same convention than neighbor cells of a cell. I.e. the following
     *
     @verbatim
             /\
            1  2
           /_0__\
       imposes
          ____________
          \    /\    /
           \1 /  \2 /
            \/____\/
             \  0 /
              \  /
               \/
     @endverbatim
     *
     *   3. Cell neighbours of an edge, in the order 0 -> 1 follow the direction of the flow (N_t) on edges defined in
     * 1.
     *      This fixes the order of cell neighbors of an edge
     *
     *   4. Vertex neighbors of an edge, in the order 0 -> 1 defines a vector N_l which is perpendicular to N_t.
     *      This fixes the order of vertex neighbors of an edge
     *
     */
    template <>
    template <>
    template <>
    struct from< cells >::to< cells >::with_color< static_uint< 1 > > {

        template < typename ValueType >
        using return_t = typename return_type< from< cells >::to< cells >, ValueType >::type;

        /*
         * neighbors order
         *
         @verbatim
           ____________
           \    /\    /
            \1 /  \2 /
             \/____\/
              \  0 /
               \  /
                \/
         @endverbatim
         */
        GT_FUNCTION
        constexpr static return_t< array< int_t, 4 > > offsets() {
            return return_t< array< int_t, 4 > >{{{1, -1, 0, 0}, {0, -1, 0, 0}, {0, -1, 1, 0}}};
        }
    };

    template <>
    template <>
    template <>
    struct from< cells >::to< cells >::with_color< static_uint< 0 > > {

        template < typename ValueType >
        using return_t = typename return_type< from< cells >::to< cells >, ValueType >::type;

        /*
         * neighbors order
         *
         @verbatim
                 /\
                /0 \
               /____\
              /\    /\
             /2 \  /1 \
            /____\/____\
         @endverbatim
         */
        GT_FUNCTION
        constexpr static return_t< array< int_t, 4 > > offsets() {
            return return_t< array< int_t, 4 > >{{{-1, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, -1, 0}}};
        }
    };

    template <>
    template <>
    template <>
    struct from< vertexes >::to< vertexes >::with_color< static_uint< 0 > > {

        template < typename ValueType >
        using return_t = typename return_type< from< vertexes >::to< vertexes >, ValueType >::type;

        /*
         * neighbors order
         *
         @verbatim

                1____2
               /\    /\
              /  \  /  \
             0____\/____3
             \    /\    /
              \  /  \  /
               \5____4/

         @endverbatim
         */
        GT_FUNCTION
        constexpr static return_t< array< int_t, 4 > > offsets() {
            return return_t< array< int_t, 4 > >{
                {{0, 0, -1, 0}, {-1, 0, 0, 0}, {-1, 0, 1, 0}, {0, 0, 1, 0}, {1, 0, 0, 0}, {1, 0, -1, 0}}};
        }
    };

    template <>
    template <>
    template <>
    struct from< edges >::to< edges >::with_color< static_uint< 0 > > {

        template < typename ValueType >
        using return_t = typename return_type< from< edges >::to< edges >, ValueType >::type;

        /*
         * neighbors order
         *
         @verbatim

               __1___
              /\    /
             0  \  2
            /_3__\/

         @endverbatim
         */
        GT_FUNCTION
        constexpr static return_t< array< int_t, 4 > > offsets() {
            return return_t< array< int_t, 4 > >{{{0, 2, -1, 0}, {0, 1, 0, 0}, {0, 2, 0, 0}, {1, 1, -1, 0}}};
        }
    };

    template <>
    template <>
    template <>
    struct from< edges >::to< edges >::with_color< static_uint< 1 > > {

        template < typename ValueType >
        using return_t = typename return_type< from< edges >::to< edges >, ValueType >::type;

        /*
         * neighbors order
         *
         @verbatim

             /\
            0  1
           /____\
           \    /
            3  2
             \/

         @endverbatim
         */
        GT_FUNCTION
        constexpr static return_t< array< int_t, 4 > > offsets() {
            return return_t< array< int_t, 4 > >{{{-1, 1, 0, 0}, {-1, -1, 1, 0}, {0, 1, 0, 0}, {0, -1, 0, 0}}};
        }
    };

    template <>
    template <>
    template <>
    struct from< edges >::to< edges >::with_color< static_uint< 2 > > {

        template < typename ValueType >
        using return_t = typename return_type< from< edges >::to< edges >, ValueType >::type;

        /*
         * neighbors order
         *
         @verbatim

           __1___
           \    /\
            0  /  2
             \/_3__\

         @endverbatim
         */
        GT_FUNCTION
        constexpr static return_t< array< int_t, 4 > > offsets() {
            return return_t< array< int_t, 4 > >{{{0, -2, 0, 0}, {0, -1, 0, 0}, {0, -2, 1, 0}, {1, -1, 0, 0}}};
        }
    };

    template <>
    template <>
    template <>
    struct from< cells >::to< edges >::with_color< static_uint< 1 > > {

        template < typename ValueType = int_t >
        using return_t = typename return_type< from< cells >::to< edges >, ValueType >::type;

        /*
         * neighbors order
         *
         @verbatim

              /\
             1  2
            /_0__\

         @endverbatim
         */
        GT_FUNCTION
        constexpr static return_t< array< int_t, 4 > > offsets() {
            return return_t< array< int_t, 4 > >{{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, -1, 1, 0}}};
        }
    };

    template <>
    template <>
    template <>
    struct from< cells >::to< edges >::with_color< static_uint< 0 > > {

        template < typename ValueType >
        using return_t = typename return_type< from< cells >::to< edges >, ValueType >::type;

        /*
         * neighbors order
         *
         @verbatim

           __0___
           \    /
            2  1
             \/

         @endverbatim
         */
        GT_FUNCTION
        constexpr static return_t< array< int_t, 4 > > offsets() {
            return return_t< array< int_t, 4 > >{{{0, 1, 0, 0}, {0, 2, 0, 0}, {0, 0, 0, 0}}};
        }
    };

    template <>
    template <>
    template <>
    struct from< cells >::to< vertexes >::with_color< static_uint< 0 > > {

        template < typename ValueType >
        using return_t = typename return_type< from< cells >::to< vertexes >, ValueType >::type;

        /*
         * neighbors order
         *
         @verbatim

          1______2
           \    /
            \  /
             \/
             0

         @endverbatim
         */
        GT_FUNCTION
        constexpr static return_t< array< int_t, 4 > > offsets() {
            return return_t< array< int_t, 4 > >{{{1, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 1, 0}}};
        }
    };

    template <>
    template <>
    template <>
    struct from< cells >::to< vertexes >::with_color< static_uint< 1 > > {

        template < typename ValueType >
        using return_t = typename return_type< from< cells >::to< vertexes >, ValueType >::type;

        /*
         * neighbors order
         *
         @verbatim

              0
              /\
             /  \
            /____\
           2      1

         @endverbatim
         */
        GT_FUNCTION
        constexpr static return_t< array< int_t, 4 > > offsets() {
            return return_t< array< int_t, 4 > >{{{0, -1, 1, 0}, {1, -1, 1, 0}, {1, -1, 0, 0}}};
        }
    };

    template <>
    template <>
    template <>
    struct from< edges >::to< cells >::with_color< static_uint< 0 > > {

        template < typename ValueType >
        using return_t = typename return_type< from< edges >::to< cells >, ValueType >::type;

        /*
         * neighbors order
         *
         @verbatim
               ______
              /\  1 /
             /0 \  /
            /____\/

         @endverbatim
         */
        GT_FUNCTION
        constexpr static return_t< array< int_t, 4 > > offsets() {
            return return_t< array< int_t, 4 > >{{{0, 1, -1, 0}, {0, 0, 0, 0}}};
        }
    };

    template <>
    template <>
    template <>
    struct from< edges >::to< cells >::with_color< static_uint< 1 > > {

        template < typename ValueType >
        using return_t = typename return_type< from< edges >::to< cells >, ValueType >::type;

        /*
         * neighbors order
         *
         @verbatim

              /\
             / 0\
            /____\
            \    /
             \1 /
              \/

         @endverbatim
         */
        GT_FUNCTION
        constexpr static return_t< array< int_t, 4 > > offsets() {
            return return_t< array< int_t, 4 > >{{{-1, 0, 0, 0}, {0, -1, 0, 0}}};
        }
    };

    template <>
    template <>
    template <>
    struct from< edges >::to< cells >::with_color< static_uint< 2 > > {

        template < typename ValueType >
        using return_t = typename return_type< from< edges >::to< cells >, ValueType >::type;

        /*
         * neighbors order
         *
         @verbatim
           ______
           \ 1  /\
            \  / 0\
             \/____\

         @endverbatim
         */
        GT_FUNCTION
        constexpr static return_t< array< int_t, 4 > > offsets() {
            return return_t< array< int_t, 4 > >{{{0, -1, 0, 0}, {0, -2, 0, 0}}};
        }
    };

    template <>
    template <>
    template <>
    struct from< edges >::to< vertexes >::with_color< static_uint< 0 > > {

        template < typename ValueType >
        using return_t = typename return_type< from< edges >::to< vertexes >, ValueType >::type;

        /*
         * neighbors order
         *
         @verbatim

              1______
              /\    /
             /  \  /
            /____\/
                  0

         @endverbatim
         */
        GT_FUNCTION
        constexpr static return_t< array< int_t, 4 > > offsets() {
            return return_t< array< int_t, 4 > >{{{1, 0, 0, 0}, {0, 0, 0, 0}}};
        }
    };

    template <>
    template <>
    template <>
    struct from< edges >::to< vertexes >::with_color< static_uint< 1 > > {

        template < typename ValueType >
        using return_t = typename return_type< from< edges >::to< vertexes >, ValueType >::type;

        /*
         * neighbors order
         *
         @verbatim

              /\
             /  \
           0/____\1
            \    /
             \  /
              \/

         @endverbatim
         */
        GT_FUNCTION
        constexpr static return_t< array< int_t, 4 > > offsets() {
            return return_t< array< int_t, 4 > >{{{0, -1, 0, 0}, {0, -1, 1, 0}}};
        }
    };

    template <>
    template <>
    template <>
    struct from< edges >::to< vertexes >::with_color< static_uint< 2 > > {

        template < typename ValueType >
        using return_t = typename return_type< from< edges >::to< vertexes >, ValueType >::type;

        /*
         * neighbors order
         *
         @verbatim

           ______0
           \    /\
            \  /  \
             \/____\
             1

         @endverbatim
         */
        GT_FUNCTION
        constexpr static return_t< array< int_t, 4 > > offsets() {
            return return_t< array< int_t, 4 > >{{{0, -2, 1, 0}, {1, -2, 0, 0}}};
        }
    };

    template <>
    template <>
    template <>
    struct from< vertexes >::to< cells >::with_color< static_uint< 0 > > {

        template < typename ValueType >
        using return_t = typename return_type< from< vertexes >::to< cells >, ValueType >::type;

        /*
         * neighbors order
         *
         @verbatim
               ______
              /\ 1  /\
             /0 \  / 2\
            /____\/____\
            \ 5  /\ 3  /
             \  /4 \  /
              \/____\/

         @endverbatim
         */
        GT_FUNCTION
        constexpr static return_t< array< int_t, 4 > > offsets() {
            return return_t< array< int_t, 4 > >{
                {{-1, 1, -1, 0}, {-1, 0, 0, 0}, {-1, 1, 0, 0}, {0, 0, 0, 0}, {0, 1, -1, 0}, {0, 0, -1, 0}}};
        }
    };

    template <>
    template <>
    template <>
    struct from< vertexes >::to< edges >::with_color< static_uint< 0 > > {

        template < typename ValueType >
        using return_t = typename return_type< from< vertexes >::to< edges >, ValueType >::type;

        /*
         * neighbors order
         *
         @verbatim
               ______
              /\    /\
             /  1  2  \
            /__0_\/__3_\
            \    /\    /
             \  5  4  /
              \/____\/

         @endverbatim
         */
        GT_FUNCTION
        constexpr static return_t< array< int_t, 4 > > offsets() {
            return return_t< array< int_t, 4 > >{
                {{0, 1, -1, 0}, {-1, 0, 0, 0}, {-1, 2, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}, {0, 2, -1, 0}}};
        }
    };

    template < typename SrcLocation, typename DestLocation, uint_t Color >
    struct connectivity {

        GRIDTOOLS_STATIC_ASSERT((is_location_type< SrcLocation >::value), "Error: unknown src location type");
        GRIDTOOLS_STATIC_ASSERT((is_location_type< DestLocation >::value), "Error: unknown dst location type");

        GRIDTOOLS_STATIC_ASSERT(
            (!boost::is_same< SrcLocation, cells >::value || Color < 2), "Error: Color index beyond color length");
        GRIDTOOLS_STATIC_ASSERT(
            (!boost::is_same< SrcLocation, edges >::value || Color < 3), "Error: Color index beyond color length");
        GRIDTOOLS_STATIC_ASSERT(
            (!boost::is_same< SrcLocation, vertexes >::value || Color < 1), "Error: Color index beyond color length");

        GT_FUNCTION
        constexpr static
            typename return_type< typename from< SrcLocation >::template to< DestLocation >, array< int_t, 4 > >::type
            offsets() {
            return from< SrcLocation >::template to< DestLocation >::template with_color<
                static_uint< Color > >::offsets();
        }
    };

    /**
    */
    template < typename Backend >
    class icosahedral_topology : public clonable_to_gpu< icosahedral_topology< Backend > > {
      public:
        using cells = location_type< 0, 2 >;
        using edges = location_type< 1, 3 >;
        using vertexes = location_type< 2, 1 >;
        using layout_map_t = typename Backend::layout_map_t;
        using type = icosahedral_topology< Backend >;

        template < typename LocationType >
        using meta_storage_t = typename Backend::template storage_info_t< LocationType >;

        template < typename LocationType, typename ValueType >
        using storage_t = typename Backend::template storage_t< LocationType, ValueType >;

        const gridtools::array< uint_t, 2 > m_dims; // Sizes as cells in a multi-dimensional Cell array

        using grid_meta_storages_t =
            boost::fusion::vector3< meta_storage_t< cells >, meta_storage_t< edges >, meta_storage_t< vertexes > >;

        grid_meta_storages_t m_virtual_storages;

      public:
        using n_locations = static_uint< boost::mpl::size< grid_meta_storages_t >::value >;
        template < typename LocationType >
        GT_FUNCTION uint_t size(LocationType location) {
            return boost::fusion::at_c< LocationType::value >(m_virtual_storages).size();
        }

        icosahedral_topology() = delete;

      public:
        template < typename... UInt >
        GT_FUNCTION icosahedral_topology(uint_t first_, uint_t second_, UInt... dims)
            : m_dims{second_, first_},
              m_virtual_storages(meta_storage_t< cells >(array< uint_t, meta_storage_t< cells >::space_dimensions >{
                                     first_, cells::n_colors::value, second_, dims...}),
                  meta_storage_t< edges >(array< uint_t, meta_storage_t< edges >::space_dimensions >{
                      first_, edges::n_colors::value, second_, dims...}),
                  // here we assume by convention that the dual grid (vertexes) have one more grid point
                  meta_storage_t< vertexes >(array< uint_t, meta_storage_t< vertexes >::space_dimensions >{
                      first_, vertexes::n_colors::value, second_ + 1, dims...})) {}

        __device__ icosahedral_topology(icosahedral_topology const &other)
            : m_dims(other.m_dims), m_virtual_storages(boost::fusion::at_c< cells::value >(other.m_virtual_storages),
                                        boost::fusion::at_c< edges::value >(other.m_virtual_storages),
                                        boost::fusion::at_c< vertexes::value >(other.m_virtual_storages)) {}

        GT_FUNCTION
        grid_meta_storages_t const &virtual_storages() const { return m_virtual_storages; }

        // TODOMEETING move semantic
        template < typename LocationType, typename ValueType >
        GT_FUNCTION storage_t< LocationType, double > make_storage(char const *name) const {
            return storage_t< LocationType, ValueType >(
                boost::fusion::at_c< LocationType::value >(m_virtual_storages), name);
        }

        template < typename LocationType >
        GT_FUNCTION array< int_t, 4 > ll_indices(array< int_t, 3 > const &i, LocationType) const {
            auto out = array< int_t, 4 >{i[0],
                i[1] % static_cast< int_t >(LocationType::n_colors::value),
                i[1] / static_cast< int >(LocationType::n_colors::value),
                i[2]};
            return array< int_t, 4 >{i[0],
                i[1] % static_cast< int_t >(LocationType::n_colors::value),
                i[1] / static_cast< int >(LocationType::n_colors::value),
                i[2]};
        }

        template < typename LocationType >
        GT_FUNCTION int_t ll_offset(array< uint_t, 4 > const &i, LocationType) const {
            return boost::fusion::at_c< LocationType::value >(m_virtual_storages).index(i);
        }

        /**
          * function to extract the absolute index of all neighbours of current position. This is used to find position
         * of
          * neighbours when the neighbours are not in the
          * same location as the location type of the iteration space (otherwise connectivity table providing 4D arrays
         * position
          * offsets is recommended, since they are compute at compile time)
          * @return an array (over neighbours) of unsinged integers (indices of position).
          *     Dimension of the array depends on the number of neighbours of the location type
          * @i indexes of current position in the iteration space
          */
        template < typename Location1, typename Location2, typename Color >
        GT_FUNCTION typename return_type< typename from< Location1 >::template to< Location2 >, uint_t >::type
            connectivity_index(Location1, Location2, Color, array< uint_t, 3 > const &i) const {

            using return_type_t =
                typename return_type< typename from< Location1 >::template to< Location2 >, uint_t >::type;

            using n_neighbors_t = static_int< return_type_t::n_dimensions >;

            // Note: offsets have to be extracted here as a constexpr object instead of passed inline to the apply fn
            // Otherwise constexpr of the array is lost
            constexpr const auto offsets =
                from< Location1 >::template to< Location2 >::template with_color< Color >::offsets();

            using seq = gridtools::apply_gt_integer_sequence<
                typename gridtools::make_gt_integer_sequence< int, n_neighbors_t::value >::type >;
            return seq::template apply< return_type_t,
                get_connectivity_index< Location2, type, Color::value >::template get_element >(*this, i, offsets);
        }

        template < typename Location2 > // Works for cells or edges with same code
        GT_FUNCTION
            typename return_type< typename from< cells >::template to< Location2 >, uint_t >::type neighbors_indices_3(
                array< uint_t, 4 > const &i, cells, Location2) const {
            switch (i[1] % cells::n_colors::value) {
            case 0: {
                return connectivity_index(cells(), Location2(), static_int< 0 >(), {i[0], i[2], i[3]});
            }
            case 1: {
                return connectivity_index(cells(), Location2(), static_int< 1 >(), {i[0], i[2], i[3]});
            }
            default: {
                GTASSERT(false);
                return typename return_type< typename from< cells >::template to< Location2 >, uint_t >::type();
            }
            }
        }

        template < typename Location2 > // Works for cells or edges with same code
        GT_FUNCTION
            typename return_type< typename from< edges >::template to< Location2 >, uint_t >::type neighbors_indices_3(
                array< uint_t, 4 > const &i, edges, Location2) const {
            switch (i[1] % edges::n_colors::value) {
            case 0: {
                return connectivity_index(edges(), Location2(), static_int< 0 >(), {i[0], i[2], i[3]});
            }
            case 1: {
                return connectivity_index(edges(), Location2(), static_int< 1 >(), {i[0], i[2], i[3]});
            }
            case 2: {
                return connectivity_index(edges(), Location2(), static_int< 2 >(), {i[0], i[2], i[3]});
            }
            default: {
                GTASSERT(false);
                return typename return_type< typename from< edges >::template to< Location2 >, uint_t >::type();
            }
            }
        }

        template < typename Location2 > // Works for cells or edges with same code
        GT_FUNCTION typename return_type< typename from< vertexes >::template to< Location2 >, uint_t >::type
            neighbors_indices_3(array< uint_t, 4 > const &i, vertexes, Location2) const {
            return connectivity_index(vertexes(), Location2(), static_int< 0 >(), {i[0], i[2], i[3]});
        }
    };

    template < typename T >
    struct is_grid_topology : boost::mpl::false_ {};

    template < typename Backend >
    struct is_grid_topology< icosahedral_topology< Backend > > : boost::mpl::true_ {};

} // namespace gridtools