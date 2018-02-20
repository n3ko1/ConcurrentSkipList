#include "skiplist.h"
#include "mingw.thread.h"
#include <iostream>

constexpr int THREAD_COUNT = 4;
constexpr int ROUNDS = 10;

std::atomic<int> id(0);

int create_id() {
    return id++;
}

void run(SkipList<int, std::string> &l)
{
    l.insert(create_id(), "");
}

int main()
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
        }
        //l.print(std::cout);
        std::cout << std::endl
                  << l.size();
    }
    return 0;
}
