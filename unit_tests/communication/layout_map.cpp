#ifndef __host__
#define __host__
#endif

#ifndef __device__
#define __device__
#endif

#include <iostream>
#include <common/layout_map.h>

int main () {

  if (gridtools::layout_map<2>::at<0>() == 2)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,3>::at<0>() == 1)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,3>::at<1>() == 3)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,3,-3>::at<0>() == 1)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,3,-3>::at<1>() == 3)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,3,-3>::at<2>() == -3)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,3,-3,5>::at<0>() == 1)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,3,-3,5>::at<1>() == 3)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,3,-3,5>::at<2>() == -3)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,3,-3,5>::at<3>() == 5)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  ////////////////////////////////////////////////////////////////////
  if (gridtools::layout_map<2>()[0] == 2)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,3>()[0] == 1)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,3>()[1] == 3)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,3,-3>()[0] == 1)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,3,-3>()[1] == 3)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,3,-3>()[2] == -3)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,3,-3,5>()[0] == 1)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,3,-3,5>()[1] == 3)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,3,-3,5>()[2] == -3)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,3,-3,5>()[3] == 5)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";


  typedef gridtools::layout_transform<gridtools::layout_map<0,1>, gridtools::layout_map<0,1> >::type transf0;

  if (transf0::at<0>() == 0)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";
  
  if (transf0::at<1>() == 1)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  typedef gridtools::layout_transform<gridtools::layout_map<0,1>, gridtools::layout_map<1,0> >::type transf01;

  if (transf01::at<0>() == 1)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";
  
  if (transf01::at<1>() == 0)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  typedef gridtools::layout_transform<gridtools::layout_map<1,0>, gridtools::layout_map<1,0> >::type transf02;

  if (transf02::at<0>() == 0)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";
  
  if (transf02::at<1>() == 1)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  typedef gridtools::layout_transform<gridtools::layout_map<2,0,1>, gridtools::layout_map<2,1,0> >::type transf;

  if (transf::at<0>() == 1)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";
  
  if (transf::at<1>() == 0)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (transf::at<2>() == 2)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  typedef gridtools::layout_transform<gridtools::layout_map<1,2,0>, gridtools::layout_map<0,1,2> >::type transf2;

  if (transf2::at<0>() == 1)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";
  
  if (transf2::at<1>() == 2)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (transf2::at<2>() == 0 )
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  int a=10,b=100,c=1000;

  if (gridtools::layout_map<2,0,1>::select<0>(a,b,c) == c)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<2,0,1>::select<1>(a,b,c) == a)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<2,0,1>::select<2>(a,b,c) == b)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,2,0>::select<0>(a,b,c) == b)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,2,0>::select<1>(a,b,c) == c)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<1,2,0>::select<2>(a,b,c) == a)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED\n";

  if (gridtools::layout_map<2,0,1>::find<0>(a,b,c) == b)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED " << gridtools::layout_map<2,0,1>::find<0>(a,b,c) << " != " << b << "\n";

  if (gridtools::layout_map<2,0,1>::find<1>(a,b,c) == c)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED " << gridtools::layout_map<2,0,1>::find<1>(a,b,c) << " != " << c << "\n";

  if (gridtools::layout_map<2,0,1>::find<2>(a,b,c) == a)
    std::cout << "PASSED\n";
  else
    std::cout << "FAILED " << gridtools::layout_map<2,0,1>::find<2>(a,b,c) << " != " << a << "\n";


  ////// TESTING FIND_VAL
  if (gridtools::layout_map<2,0,1>::find_val<0,int,666>(a,b,c) == b)
      std::cout << "PASSED\n";
  else
      std::cout << "FAILED " << gridtools::layout_map<2,0,1>::find_val<0,int,666>(a,b,c) << " != " << b << "\n";

  if (gridtools::layout_map<2,0,1>::find_val<1,int,666>(a,b,c) == c)
      std::cout << "PASSED\n";
  else
      std::cout << "FAILED " << gridtools::layout_map<2,0,1>::find_val<1,int,666>(a,b,c) << " != " << c << "\n";

  if (gridtools::layout_map<2,0,1>::find_val<2,int,666>(a,b,c) == a)
      std::cout << "PASSED\n";
  else
      std::cout << "FAILED " << gridtools::layout_map<2,0,1>::find_val<2,int,666>(a,b,c) << " != " << a << "\n";

  if (gridtools::layout_map<2,0,1>::find_val<3,int,666>(a,b,c) == 666)
      std::cout << "PASSED\n";
  else
      std::cout << "FAILED " << gridtools::layout_map<2,0,1>::find_val<3,int,666>(a,b,c) << " != " << 666 << "\n";

  return 0;
}