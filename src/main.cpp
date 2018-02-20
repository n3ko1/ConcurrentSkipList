#include "skiplist.h"
#include <iostream>

int main()
{
    SkipList<int, std::string> l = SkipList<int, std::string>();
    l.print(std::cout);
    l.insert(1, "Hello");
    // l.insert(2, "world.");
    // l.insert(4, "Whatever.");
     l.print(std::cout);
    // l.print(std::cout);
    return 0;
}
