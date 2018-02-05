#include "skiplist.h"
#include <iostream>

int main()
{
    SkipList<std::string> l = SkipList<std::string>();
    l.insert(1, "Hello");
    l.insert(2, "world.");
    l.insert(4, "Whatever.");
    l.print(std::cout);
    l.remove(2);
    l.print(std::cout);

    SkipList<float> l2 = SkipList<float>();
    l2.insert(1, 5.5);
    l2.insert(2, 3.2);
    l2.insert(4, 6);
    l2.print(std::cout);
    l2.remove(2);
    l2.print(std::cout);
    return 0;
}
