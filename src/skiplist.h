#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <vector>
#include <string>

class SkipList
{
  public:
    SkipList();
    ~SkipList(){};

    std::string *search(const int searchKey) const;
    void insert(const int key, const std::string &val);
    void remove(const int key);

    void print(std::ostream &os) const;

  private:
    struct SkipNode
    {
        SkipNode(const int k, const std::string &v, const int forward_size) : key(k), value(v) { forward.reserve(forward_size); }
        ~SkipNode() {}
        int key;
        std::string value;

        // Array of forward nodes
        std::vector<SkipNode *> forward;
    };

    int random_level() const;
    SkipNode *find_node(const int searchKey) const;
    SkipNode *head;
    SkipNode *NIL;

    const int max_levels;
    const float probability;
};

#endif