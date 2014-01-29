/** This code tests a solution to have clone objects on GPUs. The objects can have references to
    data members that must be initialized on GPU with references on the device

    Authors: Mauro Bianco, Ugo Varetto

    This version uses a gpu enabled boost::fusion library
*/


#define BOOST_NO_CXX11_RVALUE_REFERENCES

#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/zip_view.hpp>
#include <boost/fusion/include/for_each.hpp>

#include <stdio.h>
#include <stdlib.h>

#include <gpu_clone.h>

/********************************************************
    GENERIC CODE THAW WORKS WITH ANY (almost POD) OBJECT
*********************************************************/


/********************************************************
    SPECIFIC CODE WITH AN OBJECT THAT HAS REFERENCES
    BUT NEED TO BE CLONED ON GPU
*********************************************************/

struct A: public gridtools::clonable_to_gpu<A> {
    typedef boost::fusion::vector<int, double> v_type;
    v_type v1;
    v_type v2;

    typedef boost::fusion::vector<v_type&, v_type&> support_t;
    typedef boost::fusion::zip_view<support_t> zip_view_t;

    zip_view_t zip_view;

    A(v_type const & a, v_type const& b)
        : v1(a)
        , v2(b)
        , zip_view(support_t(v1, v2))
    {
    }

    __device__
    A(A const& a) 
        : v1(a.v1)
        , v2(a.v2)
        , zip_view(support_t(v1, v2))
    { }

    ~A() { }

    void update_gpu_copy() const {
        clone_to_gpu();
    }

    __host__ __device__
    void out() const {
        printf("v1:  ");
        boost::fusion::for_each(v1, print_elements());
        printf("\n");

        printf("v2:  ");
        boost::fusion::for_each(v2, print_elements());
        printf("\n");

        printf("zip: ");
        boost::fusion::for_each(zip_view, print_zip());
        printf("\n");
    }

private:
    struct print_elements {
        __host__ __device__
        void operator()(int u) const {
            printf("%d, ", u);
        }

        __host__ __device__
        void operator()(double u) const {
            printf("%e, ", u);
        }
    };

    struct print_zip {
        template <typename V>
        __host__ __device__
        void operator()(V const & v) const {
            boost::fusion::for_each(v, print_elements());
            printf("\n");
        }
    };

};

/** class to test gpu_clonable data-members
 */
struct B: public gridtools::clonable_to_gpu<B> {
    A a;

    B(typename A::v_type const& v1, typename A::v_type const& v2) 
        : a(v1, v2)
    {
        //        clone_to_gpu();
    }

    __device__
    B(B const& b) 
        : a(b.a)
    {}

    __host__ __device__
    void out() const {
        a.out();
    }
};

__global__
void print_on_gpu(A * a) {
    a->out();
}

__global__
void print_on_gpu(B * b) {
    b->out();
}

struct minus1 {
    template <typename T>
    __host__ __device__ // Avoid warning
    void operator()(T & x) const {
        x -= 1;
    }
};

int main(int argc, char** argv) {

    if (argc != 2) {
        printf("Multiplicator is needed\n");
        return 1;
    }

    int m = atoi(argv[1]);

    typename A::v_type w1(m*1, m*3.1415926);
    typename A::v_type w2(m*2, m*2.7182818);

    A a(w1, w2);
    a.update_gpu_copy();

    a.out();

    printf("Performing the same operation on GPU on cloned object\n");

    print_on_gpu<<<1,1>>>(a.gpu_object_ptr);

    cudaDeviceSynchronize();

    printf("Updating the object with -1\n");

    boost::fusion::for_each(a.v1, minus1());
    boost::fusion::for_each(a.v2, minus1());
    a.update_gpu_copy();

    a.out();

    printf("Performing the same operation on GPU on cloned object\n");

    print_on_gpu<<<1,1>>>(a.gpu_object_ptr);

    cudaDeviceSynchronize();

    printf("\nTesting data clonable data members of clonable classes\n");

    typename A::v_type bw1(m*8, m*1.23456789);
    typename A::v_type bw2(m*7, m*9.87654321);

    B b(bw1, bw2);

    b.out();

    printf("Now doing the same on GPU");

    print_on_gpu<<<1,1>>>(b.gpu_object_ptr);

    boost::fusion::for_each(b.a.v1, minus1());
    boost::fusion::for_each(b.a.v2, minus1());
    b.clone_to_gpu();

    return 0;
}