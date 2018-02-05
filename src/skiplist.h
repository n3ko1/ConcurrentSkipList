#ifndef SKIPLIST_H
#define SKIPLIST_H

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
    SkipNode(const int k, const std::string &v, const int forward_size) : key(k), value(v)
    {
      forward = new SkipNode *[forward_size];
      for (auto i = 0; i != forward_size; ++i)
      {
        forward[i] = nullptr;
      }
    }
    ~SkipNode() { delete forward; }
    int key;
    std::string value;

    // Array of forward nodes
    SkipNode **forward;

    int node_level() const;
  };

  int random_level() const;
  SkipNode *find_node(const int searchKey) const;
  SkipNode *head;
  SkipNode *NIL;

  static const int max_levels = 16;
  const float probability;
};

#endif