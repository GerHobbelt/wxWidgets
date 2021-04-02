// shmem_test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <boost/interprocess/managed_shared_memory.hpp>

using namespace boost::interprocess;

int main()
{
   managed_shared_memory mshm(create_only, "mshm", 65536, (void *)(size_t(1) << 40));
   void * ptr = mshm.allocate(100, std::nothrow);

   auto handle = mshm.get_handle_from_address(ptr);
}
