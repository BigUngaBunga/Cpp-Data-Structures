#include <iostream>
#include "DynamicAllocator.hpp"
#include <chrono>
#include <cassert>

#define TestMemoryPool

int main()
{
#ifdef TestMemoryPool
    using IndexType = uint16_t;

    const IndexType N = (IndexType)-1 - 1;
    int Nmult = 1000;
    struct A { int a, b, c; 
        A(int a, int b, int c) : a(a), b(b), c(c) {}
    };
    A* ptrs[N];

    A a = { 1, 2, 3 };

    double memoryPoolTime = 0, newDeleteTime = 0;

    {
        // Start timings
        std::cout << "Testing Memory Pool..." << std::endl;
        auto start = std::chrono::high_resolution_clock::now();

        {
            DynamicAllocator<sizeof(A), IndexType> pool{ N };

            for (int n = 0; n < Nmult; n++)
            {
                for (int i = 0; i < N; i++)
                    ptrs[i] = pool.create<A>(0, 1, 2);
                for (int i = 0; i < N; i++)
                    pool.destroy(ptrs[i]);
            }
        }

        // Show timings
        auto end = std::chrono::high_resolution_clock::now();
        auto diff = end - start;
        memoryPoolTime = std::chrono::duration <double, std::milli>(diff).count();
        std::cout << memoryPoolTime << " ms" << std::endl;
    }

    {
        // Start timings
        std::cout << "Testing new/delete..." << std::endl;
        auto start = std::chrono::high_resolution_clock::now();

        for (int n = 0; n < Nmult; n++)
        {
            for (int i = 0; i < N; i++)
                ptrs[i] = new A{ 0, 1, 2 };
            for (int i = 0; i < N; i++)
                delete ptrs[i];
        }

        // Show timings
        auto end = std::chrono::high_resolution_clock::now();
        auto diff = end - start;
        newDeleteTime = std::chrono::duration <double, std::milli>(diff).count();
        std::cout << newDeleteTime << " ms" << std::endl;
    }
    auto difference = newDeleteTime - memoryPoolTime;
    std::cout << "Memory pool was " << difference << " ms faster" << std::endl;
    std::cout << "Memory pool was " << (newDeleteTime / memoryPoolTime) <<" times faster" << std::endl;
#endif // TestMemoryPool
}
