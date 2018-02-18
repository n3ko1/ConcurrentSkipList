#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <string>
#include <iostream>
#include <limits>
#include <random>
#include <atomic>

#define OUT_PARAM

template <class T>
class AtomicMarkableReference
{
private:
  struct MarkableReference
  {
    T *val;
    bool marked;

    MarkableReference() : val(nullptr), marked(false) {}
    MarkableReference(T *value, bool mark) : val(value), marked(mark) {}

    bool operator==(const MarkableReference &other)
    {
      return (val == other.val && marked == other.marked);
    }
  };

  // atomically holds marked reference
  std::atomic<MarkableReference *> marked_ref;

public:
  AtomicMarkableReference()
  {
    marked_ref.store(new MarkableReference());
  }
  AtomicMarkableReference(T *value, bool mark)
  {
    marked_ref.store(new MarkableReference(value, mark));
  }
  T *get_reference()
  {
    return marked_ref.load()->next;
  }

  // Stores the value of this references marked flag in reference
  T *get(OUT_PARAM bool &mark)
  {
    MarkableReference *temp = marked_ref.load();
    mark = temp->marked;
    return temp->val;
  }

  void set(T *value, bool mark)
  {
    MarkableReference *curr = marked_ref.load();
    if (value != curr->val || mark != curr->marked)
    {
      marked_ref.store(new MarkableReference(value, mark));
    }
  }

  // Atomically sets the value of both the reference and mark to the given update values
  // if the current reference is equal to the expected reference and the current mark is equal to the expected mark.
  // returns true on success
  bool compare_and_swap(T *expected_value, T *new_value, bool expected_mark, bool new_mark)
  {
    MarkableReference *curr = marked_ref.load();
    return (expected_value == curr->val && expected_mark == curr->marked &&
            ((new_value == curr->val && new_mark == curr->marked) ||                                 // if already equal, return true by shortcircuiting
             marked_ref.compare_exchange_strong(curr, new MarkableReference(new_value, new_mark)))); // otherwise, attempt compare and swap
  }
};

// BEGIN SKIPLIST IMPLEMENTATION
#define SKIPLIST_TEMPLATE_ARGS \
  template <typename KeyType, typename ValueType>

#define SKIPLIST_TYPE \
  SkipList<KeyType, ValueType>

SKIPLIST_TEMPLATE_ARGS
class SkipList
{
private:
  struct SkipNode
  {
    SkipNode(const KeyType k, const ValueType &v, const int forward_size) : key(k), value(v), top_level(forward_size)
    {
      intialize_forward(forward_size);
    }
    SkipNode(const KeyType k, const int forward_size) : key(k), value(nullptr), top_level(forward_size)
    {
      intialize_forward(forward_size);
    }
    ~SkipNode() { delete forward; }

    void intialize_forward(const int forward_size)
    {
      forward = new AtomicMarkableReference<SkipNode> *[forward_size];
      for (auto i = 0; i != forward_size; ++i)
      {
        forward[i] = AtomicMarkableReference<SkipNode>(nullptr, false);
      }
    }

    KeyType key;
    ValueType value;

    int top_level;

    // Array of atomic forward nodes
    AtomicMarkableReference<SkipNode> *forward;

    int node_level() const;
  };

public:
  SkipList();
  ~SkipList(){};

  ValueType *search(const KeyType search_key) const;
  bool find(const KeyType search_key, SkipNode **preds, SkipNode **succs); // not const, will delete marked nodes

  void insert(const KeyType key, const ValueType &val);
  void remove(const KeyType key);

  void print(std::ostream &os) const;

private:
  int random_level() const;
  SkipNode *head;
  SkipNode *NIL;

  static const int max_levels = 16;
  const float probability;
};

SKIPLIST_TEMPLATE_ARGS
SKIPLIST_TYPE::SkipList() : probability(0.5)
{
  head = new SkipNode(std::numeric_limits<KeyType>::min(), max_levels);
  NIL = new SkipNode(std::numeric_limits<KeyType>::max(), max_levels);
  for (auto i = 0; i != max_levels; ++i)
  {
    head->forward[i] = AtomicMarkableReference<SkipNode>(NIL, false);
  }
}

SKIPLIST_TEMPLATE_ARGS
int SKIPLIST_TYPE::random_level() const
{
  int new_level = 1;

  while (((double)rand() / (RAND_MAX)) < probability)
  {
    ++new_level;
  }
  return (new_level > max_levels ? max_levels : new_level);
}

SKIPLIST_TEMPLATE_ARGS
void SKIPLIST_TYPE::print(std::ostream &os) const
{
  auto x = head;
  while (x->forward[0]->key != std::numeric_limits<KeyType>::max())
  {
    x = x->forward[0]; // traverse to the right
    os << "Key: " << x->key << " Value: " << x->value << " Level: " << x->node_level() << std::endl;
  }
}

SKIPLIST_TEMPLATE_ARGS
bool SKIPLIST_TYPE::find(const KeyType search_key, SkipNode **preds, SkipNode **succs)
{
  bool marked = false;
  bool snip;

  SkipNode *pred, curr, succ;

RETRY:
  pred = head;
  for (auto level = max_levels; level >= 0; --level)
  {
    curr = pred->forward[level].get_reference();
    while (true)
    {
      // delete marked nodes
      succ = curr->forward[level].get(marked);
      while (marked)
      {
        snip = pred->forward[level].compare_and_swap(curr, succ, false, false);
        if (!snip)
          goto RETRY; // CAS failed, try again
        curr = pred->forward[level].get_reference();
        succ = curr->forward[level].get(marked);
      }
      if (curr->key < search_key)
      {
        pred = curr;
        curr = succ;
      }
      else
      {
        break;
      }
    }
    preds[level] = pred;
    succs[level] = curr;
  }
  return (curr.key == search_key);
}

SKIPLIST_TEMPLATE_ARGS
ValueType *SKIPLIST_TYPE::search(const KeyType search_key) const
{
  auto x = head;
  // traverse from top of head. Forward size of head is list level
  for (int i = head->node_level() - 1; i >= 0; --i)
  {
    while (x->forward[i] && x->forward[i]->key < search_key)
    {
      x = x->forward[i]; // traverse to the right
    }
  }

  x = x->forward[0];
  return (x->key == search_key ? &x->value : nullptr);
}

SKIPLIST_TEMPLATE_ARGS
void SKIPLIST_TYPE::insert(const KeyType key, const ValueType &val)
{
  int top_level = random_level();
  auto preds = new SkipNode *[max_levels];
  auto succs = new SkipNode *[max_levels];
  while (true)
  {
    bool found = find(key, preds, succs);
    if (found)
    {
      // TODO support multi-value
      return;
    }
    else
    {
      auto new_node = new SkipNode(key, val, top_level);
      for (auto level = 0; level <= top_level; ++level)
      {
        auto succ = succs[level];
        new_node->forward[level].set(succ, false);
      }
      auto pred = preds[0];
      auto succ = preds[0];
      new_node->forward[0].set(succ, false);
      if (!pred->forward[0].compare_and_swap(succ, new_node, false, false))
      {
        continue; // CAS failed, try again
      }
      for (auto level = 1; level <= top_level; ++level)
      {
        while (true)
        {
          pred = preds[level];
          succ = succs[level];
          if (pred->forward[level].compare_and_swap(succ, new_node, false, false))
            break;
          find(key, preds, succs); // CAS failed for upper level, search node to update preds and succs
        }
      }
      return;
    }
  }
}

SKIPLIST_TEMPLATE_ARGS
void SKIPLIST_TYPE::remove(const KeyType key)
{
  auto x = head;
  auto update = new SkipNode *[max_levels];

  // traverse from top of head. Forward size of head is list level
  for (int i = head->node_level() - 1; i >= 0; --i)
  {
    while (x->forward[i] && x->forward[i]->key < key)
    {
      x = x->forward[i]; // traverse to the right
    }
    update[i] = x; // store last forward pointer
  }
  x = x->forward[0];
  if (x->key == key)
  {
    for (int i = 0; i != head->node_level() - 1; ++i)
    {
      if (update[i]->forward[i] != x)
        break;
      update[i]->forward[i] = x->forward[i];
    }
    x = nullptr;
  }
  delete update;
  return;
}

#endif