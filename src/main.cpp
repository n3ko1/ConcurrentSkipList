#include "skiplist.h"
#include "mingw.thread.h"
#include <iostream>
#include <assert.h>

constexpr int THREAD_COUNT = 32;
constexpr int ROUNDS = 100;

std::atomic<int> id(0);

int create_id()
{
    return id++;
}

void run(SkipList<int, std::string> &l)
{
    l.insert(create_id(), "");
}

void run_threaded_test()
{
    for (auto x = 0; x != ROUNDS; ++x)
    {
        SkipList<int, std::string> l = SkipList<int, std::string>();
        std::thread tA[THREAD_COUNT];
        for (auto j = 0; j != 100; ++j)
        {
            for (auto i = 0; i != THREAD_COUNT; ++i)
            {
                //std::cout << num << std::endl;
                tA[i] = std::thread(run, l);
            }

            for (auto i = 0; i != THREAD_COUNT; ++i)
            {
                tA[i].join();
            }

            // at this point, exactly THREAD_COUNT inserts should have happened
            int size = l.size();
            assert(size % THREAD_COUNT == 0);
        }

        std::cout << std::endl
                  << l.size();
    }
}

void run_simple_test()
{

    SkipList<int, std::string> l = SkipList<int, std::string>();
    l.insert(1, "Hello");
    l.insert(2, "World.");
    l.insert(3, "This");
    l.insert(4, "is");
    l.insert(5, "a");
    l.insert(6, "test");
    l.print(std::cout);
}

int main()
{
    run_simple_test();
    run_threaded_test();
    return 0;
}
