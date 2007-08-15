/*
 * Copyright (c) 1997-2004
 * Eric S. Albright
 * 
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Eric S. Albright makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * The binary trie implementation stems from a binary tree implementation
 * which bears the following copyright notices:
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 *
 * Copyright (c) 1996,1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 *
 */

////////////////////////////////////////////////////////
// Change History
// --------------
// Eric S. Albright 12/20/1997 fixed find algorithm
// Eric S. Albright 12/22/1997 more fixes to find algorithm
// Eric S. Albright 02/01/1998 reorganization for support of trie_map, 
//                              trie_multiset, trie_multimap
// Eric S. Albright 08/17/1998 removed binary read and write facility
// Eric S. Albright 08/27/1998 fixed bug with approximate read and
//                              edit distance
// Eric S. Albright 04/02/1999 fixed bug and improved performance of 
//                              approximate find and edit distance
// Eric S. Albright 10/04/1999 changed to be able to handle multi___
// Eric S. Albright 08/28/2002 fixed bug in upper_bound 
//                              adapted to version 3.3 of SGI STL
// Eric S. Albright 08/31/2002 added functionality to find, count,
//                              upper_bound, lower_bound, equal_range
//                              to be able to accept key iterators
// Eric S. Albright 09/27/2002 fixed bug with lower_bound and upper_bound
//                             added find_prefix
// Eric S. Albright 04/20/2004 made so would compile with VC.net 2003
// To do:
//
// - change approximate find to not use random_access of key
// - change approximate find to accept key iterators instead of just key
#pragma once
#ifndef __INTERNAL_TRIE_H
#define __INTERNAL_TRIE_H

#define TRIE_INFINITY 0x7fffffffu
#define TRIE_CUTOFF 0x8fffffffu
#include <wchar.h>
#include <cassert>
#if _DEBUG 
#include <windows.h>
  inline void _cdecl Trace(const wchar_t* lpszFormat, ...)
  {
	  va_list args;
	  va_start(args, lpszFormat);

	  int nBuf;
	  wchar_t szBuffer[512];

#pragma warning(suppress: 4996)
    nBuf = _vsnwprintf((wchar_t*)szBuffer, sizeof(szBuffer) / sizeof(wchar_t), lpszFormat, args);
	  assert(nBuf < sizeof(szBuffer));//Output truncated as it was > sizeof(szBuffer)

	  OutputDebugStringW(szBuffer);
	  va_end(args);
  }
  #define TRACE  Trace
#else
  inline void _cdecl Trace(const wchar_t , ...){}
  #define TRACE  1 ? (void)0 : Trace
#endif

/*
  Trie class, designed for use in implementing STL associative containers 
  (trie_set, trie_multiset, trie_map, and trie_multimap).
*/

/*
  tries are space efficient when prefixes are similar

  trie<char> myWordList;
  myWordList.insert("apple");
  myWordList.insert("ape");
  // produces:
  //         e
  // a - p /
  //       \ p - l - e

  trie<char>::iterator myWordIt;
  copy(myWordList.begin(), myWordList.end(), ostream_iterator<string> (cout, "\n"));
  // produces:
  // ape
  // apple
*/

/*
  The (non-binary) trie can be expressed using a binary tree:
  RLinks express siblings while LLinks express children.

          H                            H
       /  |  \                        /
     A    E    I                     A ----- E ------ I
    / \   | \   \                   /       /        /  
   D   V  $  R   S                 D - V   $ - R    S
   |   |     |   |                /   /       /    /
   $   E     $   $               $   E       $    $ 
       |                            /
       $                           $

  (for HAD, HAVE, HE, HER, HIS)

  The search in this binary tree proceeds by comparing a character
  of the argument to the character in the tree, and following RLINKS
  until finding a match; then the LLINK is taken and we treat the next 
  character of the argument in the same way.

  With such a binary tree, we are more or less doing a search by 
  comparison, with equal-unequal branching instead of less-greateer 
  branching. We must make at least log2N comparisons on the average 
  to distinguish between N keys.

  We make the distinction between two types of nodes - key and value.
  Value nodes are marked as being edges which allows them to contain 
  siblings as well as the data. 

  In order to gain the suitable ordering, the tree must be traversed in Preorder.

  Much of the theoretical and practical implementation for the approximate string
  matching components comes from the article:

  Heping Shang and T.H. Merrettal, "Tries for Approximate String Matching", IEEE
  Transactions on Knowledge and Data Engineering, Vol. 8, No. 4, August 1996.

  In order to handle multisets and multimaps, we have two types of nodes:
  Key and Value nodes. Both can be iterated over. The following represents the 
  trie for "had, have, have, he, her, his, his":

         H
        /
       A ----- E ------ I
      /       /        /  
     D - V   $ - R    S
    /   /       /    /
   $   E       $    $ - $
    /
   $ - $

*/ 

//#include <stl_algobase.h>
//#include <stl_algo.h>
//#include <stl_alloc.h>
//#include <stl_construct.h>
//#include <stl_function.h>
#include <utility>
#include <functional>
#include <memory>
#include <vector>

#define __STL_CLASS_PARTIAL_SPECIALIZATION
//#include <stl_vector.h>

namespace trie {
// TRIE_SPECIALIZATION

// Generalized Container functions allow for use of pointer to arrays as containers
template <class T, class I>
struct __container_begin : public std::unary_function<T, I> {
    I operator()(const T& __x) const { return __x.begin(); }
};

template <class T, class I>
struct __container_end : public std::unary_function<T, I> {
    I operator()(const T& __x) const { return __x.end(); }
};
template <class T, class I>
struct __container_size : public std::unary_function<T, I> {
    I operator()(const T& __x) const { return __x.size(); }
};

template <class _Pair>
struct _Select1st : public std::unary_function<_Pair, typename _Pair::first_type> {
  const typename _Pair::first_type& operator()(const _Pair& __x) const {
    return __x.first;
  }
};

template <class _Tp>
struct _Identity : public std::unary_function<_Tp,_Tp> {
  const _Tp& operator()(const _Tp& __x) const { return __x; }
};


// END TRIE_SPECIALIZATION


typedef bool _b_trie_edge_type;
const _b_trie_edge_type   _S_b_trie_non_edge = false;
const _b_trie_edge_type   _S_b_trie_edge = true;

struct _b_trie_node_base___
{
  typedef _b_trie_edge_type     _edge_type;
  typedef _b_trie_node_base___*    _base_ptr;

#ifdef _DEBUG
  _b_trie_edge_type             _debugfIsEdge;
#endif

  _base_ptr                     _M_parent;
  _base_ptr                     _M_left;
  _base_ptr                     _M_right;

  // the header's parent points to the root (so decrement will work)
  // the LLink points to the root also so it is unique (color can be algorithmically determined) (so increment will work) 
  // the RLink points to the rightmost as a convenient place to store that value.
  static bool _S_is_header(_base_ptr __x) {
    assert(__x != NULL);
    if (__x->_M_parent == __x->_M_left) {
      return true;
    }
    return false;
  }

  static _edge_type _S_edge(_base_ptr __x) {
    assert(__x != NULL);
    
    // if there is no LLink then there are no children and thus this is an edge
	  return(__x->_M_left == 0 || _S_is_header(__x));
  }

  // these are mis-named should be leftmost
  // and rightmost but we have name collision
  // with trie class which keeps a pointer to 
  // leftmost and rightmost in header for
  // performance reasons
  
  static _base_ptr _S_minimum(_base_ptr __x) {
    assert(__x != NULL);
    
    while (__x->_M_left != 0) {
      __x = __x->_M_left;
    }
    return __x;
  }

  static _base_ptr _S_maximum(_base_ptr __x) {
    assert(__x != NULL);
    while((__x->_M_right != 0 && __x->_M_right != __x) || __x->_M_left != 0) {
      while (__x->_M_right != 0) {
        __x = __x->_M_right;
      }
      if(__x->_M_left != 0) {
        __x = __x->_M_left;
      }
    }
    return __x;
  }

  // trie parent returns the "true" parent (as opposed to the binary parent)
  // The "true" parent is the node that would have been the parent if the tree
  // were not a binary tree.
  static _base_ptr _S_trie_parent(_base_ptr __n)
  {
    assert(__n != NULL);
    
    _base_ptr __x = __n;
    _base_ptr __y = __n->_M_parent;
    while (__x == __y->_M_right) {
      __x = __y;
      __y = __y->_M_parent;
    }
    return (__x == __y->_M_parent)? 0 : __y;
  }

};

// most of the nodes will simply hold a part of the key
// only the edge nodes will hold the value.
// the edge indicator will tell us which struct we are 
// pointing to (key or value);
template <class _SubKeyType>
struct _b_trie_key_node : public _b_trie_node_base___
{
public:
  typedef _b_trie_key_node<_SubKeyType>*   _key_link_type;

  typename _SubKeyType   _M_key_field;
};

template <class _Value>
struct _b_trie_value_node : public _b_trie_node_base___
{
public:
  typedef _b_trie_value_node<_Value>*    _value_link_type;
  typename _Value        _M_value_field;
};

struct _b_trie_base_iterator
{
  typedef std::bidirectional_iterator_tag                  iterator_category;
  typedef ptrdiff_t                                   difference_type;
  typedef _b_trie_node_base___::_base_ptr    _Base_ptr;

  _Base_ptr                                           _M_node;

  void _M_increment()
  { // preorder traversal
    // visit the node, traverse the left subtree, traverse the right subtree
    assert(_M_node != NULL);
/*
      header
       /
     $ - H
        /
       A ----- E ------ I
      /       /        /  
     D - V   $ - R    S
    /   /       /    /
   $   E       $    $ - $
      /
     $ - $

  starting with header -> $, H, A, D, $, V, E, $, $, E, $, R, $, I, S, $, $, header

*/
    
    if(_M_node->_M_left != 0) {
      _M_node = _M_node->_M_left;
    }
    else if(_M_node->_M_right != 0) {
      _M_node = _M_node->_M_right;
    }
    else {
      _Base_ptr __y = _M_node->_M_parent;
      assert(__y != NULL);
      while (__y->_M_right == 0 || __y->_M_right == _M_node) {
        _M_node = __y;
        assert(_M_node != NULL);
        __y = _M_node->_M_parent;
        // check for the case of the header
		    if (_b_trie_node_base___::_S_is_header(__y)) { 
          break;
        }
      }
      // check for the case of the header go back to the root
		  if(_b_trie_node_base___::_S_is_header(__y)) {
        _M_node = __y;
      }
      else
      {
        _M_node = __y->_M_right;
      }
    }
  }

  void _M_decrement()
  { // reverse preorder traversal
    // traverse right subtree, traverse left subtree, visit the node
    assert(_M_node != NULL);
/*
      header
       /
     $ - H
        /
       A ----- E ------ I
      /       /        /  
     D - V   $ - R    S
    /   /       /    /
   $   E       $    $ - $
      /
     $ - $

  starting with header -> $, $, S, I, $, R, $, E, $, $, E, V, $, D, A, H, $, header
*/

    // check for the case of the header go to _M_rightmost
		if(_b_trie_node_base___::_S_is_header(_M_node)) {
      _M_node = _M_node->_M_right;
    }
    else {
     
      _Base_ptr __y = _M_node;
      _M_node = _M_node->_M_parent;
      assert(_M_node != NULL);
    
      if(_M_node->_M_left != __y && _M_node->_M_left != 0) {
        while(_M_node->_M_right != 0 || _M_node->_M_left != 0) {
          if(_M_node->_M_right != 0 && _M_node->_M_right != __y) {
            __y = _M_node;
            _M_node = _M_node->_M_right;
            assert(_M_node != NULL);
          }
          else {
            __y = _M_node;
            _M_node = _M_node->_M_left;
            assert(_M_node != NULL);
          }
        }
      }
    }
  }

  // trie_increment increments through the iterators until the 
  // iterator is an edge (a terminal node)
  void _M_trie_increment()
  {
    do {_M_increment();} 
    while(_b_trie_node_base___::_S_edge(_M_node) == _S_b_trie_non_edge);
    assert(_M_node->_debugfIsEdge == true);
  }

  // trie_decrement decrements through the iterators until the 
  // iterator is an edge (a terminal node)
  void _M_trie_decrement()
  {
    do {_M_decrement();}
    while(_b_trie_node_base___::_S_edge(_M_node) == _S_b_trie_non_edge);
    assert(_M_node->_debugfIsEdge == true);
  }
};

template <class _Value, class _Ref, class _Ptr>
struct _b_trie_iterator : public _b_trie_base_iterator
{
public:
  typedef _Value           value_type;
  typedef _Ref             reference;
  typedef _Ptr             pointer;

  typedef _b_trie_iterator<_Value, _Value&, _Value*>             iterator;
  typedef _b_trie_iterator<_Value, const _Value&, const _Value*> const_iterator;

  typedef _b_trie_iterator<_Value, _Ref, _Ptr>                   _Self;

  typedef _b_trie_value_node<_Value>*                            _value_link_type;

  _b_trie_iterator() {}
  _b_trie_iterator(_Base_ptr __x) { _M_node = __x; }
  _b_trie_iterator(_value_link_type __x) { _M_node = __x; }
  _b_trie_iterator(const iterator& __it) { _M_node = __it._M_node; }

  typename reference operator*() const { 
    assert(_M_node != NULL);
    assert(_M_node->_debugfIsEdge == _S_b_trie_edge);
    assert(_value_link_type(_M_node) != NULL);
    return _value_link_type(_M_node)->_M_value_field;
  }

//#ifndef __STL_NO_ARROW_OPERATOR
  pointer operator->() const {
    return &(operator*());
  }
//#endif // __STL_NO_ARROW_OPERATOR

  _Self& operator++() { _M_trie_increment(); return *this; }
  _Self operator++(int) {
    _Self __tmp = *this;
    _M_trie_increment();
    assert(_M_node != NULL);
    assert(_M_node->_debugfIsEdge == _S_b_trie_edge);
    return __tmp;
  }
    
  _Self& operator--() { _M_trie_decrement(); return *this; }
  _Self operator--(int) {
    _Self __tmp = *this;
    _M_trie_decrement();
    assert(_M_node != NULL);
    assert(_M_node->_debugfIsEdge == _S_b_trie_edge);
    return __tmp;
  }

};
inline bool operator==(const _b_trie_base_iterator& __x,
                       const _b_trie_base_iterator& __y) {
  return __x._M_node == __y._M_node;
}


inline bool operator!=(const _b_trie_base_iterator& __x,
                       const _b_trie_base_iterator& __y) {
  return !(__x._M_node == __y._M_node);
}

#ifndef __STL_CLASS_PARTIAL_SPECIALIZATION

inline bidirectional_iterator_tag
iterator_category(const _b_trie_base_iterator&) {
  return bidirectional_iterator_tag();
}

inline _b_trie_base_iterator::difference_type*
distance_type(const _b_trie_base_iterator&) {
  return (_b_trie_base_iterator::difference_type*) 0;
}

template <class _Value, class _Ref, class _Ptr>
inline _Value* value_type(const _b_trie_iterator<_Value, _Ref, _Ptr>&) {
  return (_Value*) 0;
}

#endif // __STL_CLASS_PARTIAL_SPECIALIZATION




// _Base for general standard-conforming allocators.
template <class _SubKeyType, class _Value, class _Alloc>
class _b_trie_alloc_base {
public:
  typedef typename _Alloc  _allocator_type;

  typedef _b_trie_key_node<_SubKeyType>*    _key_link_type;
  typedef _b_trie_value_node<_Value>*       _value_link_type;

  typedef _b_trie_key_node<_SubKeyType>     _key_node_type;
  typedef _b_trie_value_node<_Value>        _value_node_type;

  typedef _Value        _value_type;
  typedef _SubKeyType   _sub_key_type;


  _allocator_type get_allocator() const { return _M_allocator; }
  
  _b_trie_alloc_base(const _Alloc& __a)
    : _M_allocator(__a),
      _M_header(0) {}

protected:
  _key_link_type _M_header;

  _key_link_type _M_allocate_key_node() { 
    return _allocator_type::rebind<_key_node_type>::other(_M_allocator).allocate(1);
  }
  _value_link_type _M_allocate_value_node() { 
    return _allocator_type::rebind<_value_node_type>::other(_M_allocator).allocate(1);
  }

  void _M_deallocate_key_node(_key_link_type __k) {
    _allocator_type::rebind<_key_node_type>::other(_M_allocator).deallocate(__k, 1);
  }
  void _M_deallocate_value_node(_value_link_type __v) { 
    _allocator_type::rebind<_value_node_type>::other(_M_allocator).deallocate(__v, 1);
  }

  void _M_construct_key(_key_link_type __x, _sub_key_type __k) {
    _allocator_type::rebind<_sub_key_type>::other(_M_allocator).construct(&__x->_M_key_field, __k);
  }
  void _M_construct_value(_value_link_type __x, _value_type __v) {
    _allocator_type::rebind<_value_type>::other(_M_allocator).construct(&__x->_M_value_field, __v);
  }

  void _M_destroy_key(_key_link_type __k) {
    _allocator_type::rebind<_sub_key_type>::other(_M_allocator).destroy(&__k->_M_key_field);
  }
  void _M_destroy_value(_value_link_type __v) {
    _allocator_type::rebind<_value_type>::other(_M_allocator).destroy(&__v->_M_value_field);
  }

private:    
  typename _allocator_type    _M_allocator;
};

template <class _SubKeyType, class _Value, class _Alloc>
struct _b_trie_base
  : public _b_trie_alloc_base<_SubKeyType, _Value, _Alloc>
{
  typedef _b_trie_alloc_base<_SubKeyType, _Value, _Alloc>
          _Base;
  typedef typename _Base::_allocator_type    allocator_type;

  _b_trie_base(const allocator_type& __a) 
    : _Base(__a) { _M_header = _M_allocate_key_node(); 
          assert(_M_header != NULL);
          _M_header->_M_parent = 0;
#if _DEBUG
        _M_header->_debugfIsEdge = true;
#endif
}
  ~_b_trie_base() { _M_deallocate_key_node(_M_header); }

};



#define TRIE_TEMPLATE template <class _Key, class _Value, class _SubKeyType, class _KeyOfValue, class _KeyIterator, class _KeySizeType, class _KeyBegin, class _KeyEnd, class _KeySize, class _Compare, class _Alloc>
#define B_TRIE _b_trie<_Key, _Value, _SubKeyType, _KeyOfValue, _KeyIterator, _KeySizeType, _KeyBegin, _KeyEnd, _KeySize, _Compare, _Alloc>

template <class _Key, 
          class _Value, 
          class _SubKeyType, 
          class _KeyOfValue, 
          class _KeyIterator,
          class _KeySizeType,
          class _KeyBegin,
          class _KeyEnd,
          class _KeySize,
          class _Compare, 
          class _Alloc = allocator<_Value> >
class _b_trie: protected _b_trie_base<_SubKeyType, _Value, _Alloc> {
    typedef _b_trie_base<_SubKeyType, _Value, _Alloc> _Base;
private:
    typedef B_TRIE   _self;

protected:
    typedef _b_trie_node_base___*                      _base_ptr;
    typedef _b_trie_key_node<_SubKeyType>              _b_trie_key_node;
    typedef _b_trie_value_node<_Value>                 _b_trie_value_node;
    typedef _b_trie_edge_type                          _edge_type;

public:
    typedef _Key                key_type;
    typedef _Value              value_type;
    typedef _SubKeyType         sub_key_type;
    
    typedef value_type*         pointer;
    typedef const value_type*   const_pointer;
    typedef value_type&         reference;
    typedef const value_type&   const_reference;

    typedef _b_trie_key_node*   _key_link_type;
    typedef _b_trie_value_node* _value_link_type;
    typedef size_t              size_type;
    typedef ptrdiff_t           difference_type;

    typedef typename _Base::allocator_type    allocator_type;
    
    allocator_type get_allocator() const { return _Base::get_allocator(); }

protected:
    using _Base::_M_allocate_key_node;
    using _Base::_M_allocate_value_node;
    using _Base::_M_deallocate_key_node;
    using _Base::_M_deallocate_value_node;
    using _Base::_M_destroy_key;
    using _Base::_M_destroy_value;
    using _Base::_M_construct_key;
    using _Base::_M_construct_value;
    using _Base::_M_header;

protected:
    _key_link_type _M_create_key_node(const sub_key_type& __x) {
      _key_link_type __tmp = _M_allocate_key_node();
#if _DEBUG
      assert(__tmp != NULL);
      __tmp->_debugfIsEdge = false;
#endif
       try {
        _M_construct_key(_key_link_type(__tmp), __x);
      }
      catch(...) { 
        (_M_deallocate_key_node(__tmp));
        throw; 
      }
      return __tmp;
    }

    _value_link_type _M_create_value_node(const value_type& __v) {
      _value_link_type __tmp = _M_allocate_value_node();
#if _DEBUG
      assert(__tmp != NULL);
      __tmp->_debugfIsEdge = true;
#endif
       try {
        _M_construct_value(_value_link_type(__tmp), __v);
      }
      catch(...) { 
        (_M_deallocate_value_node(__tmp));
        throw; 
      }
      return __tmp;
    }

    _base_ptr _M_clone_node(_base_ptr __x) {
      _base_ptr __tmp;
      if(_S_edge(__x) == _S_b_trie_edge) {
        assert(__x != NULL);
        assert(__x->_debugfIsEdge == true);
        __tmp = _M_create_value_node(_S_value(__x));
      }
      else {
        assert(__x != NULL);
        assert(__x->_debugfIsEdge == false);
        __tmp = _M_create_key_node(_S_key(__x));
      }
      assert(__tmp != NULL);
      __tmp->_M_left = 0;
      __tmp->_M_right = 0;
      return __tmp;
    }

    // I have to tell it whether it is an edge since the links have been cleared out already
    void destroy_node(bool __fIsEdge, _base_ptr __p) {
#if _DEBUG
      assert(__p != NULL);
      assert(__fIsEdge == __p->_debugfIsEdge);
#endif

      if(__fIsEdge) {
        --_M_node_count;
        _M_destroy_value(_value_link_type(__p));
        _M_deallocate_value_node(_value_link_type(__p));
      }
      else {
        _M_destroy_key(_key_link_type(__p));
        _M_deallocate_key_node(_key_link_type(__p));
      }
    }

protected:
    size_type      _M_node_count;  // keeps track of number of elements contained within trie
    _base_ptr      _M_bp_leftmost; // allows header to be distinct from all other nodes during iteration

    _Compare       _M_key_compare;

    _key_link_type _M_set_root(_key_link_type __x) {
      assert(_M_header != NULL); 
      _M_header->_M_parent=(_base_ptr)__x;
      _M_header->_M_left=(_base_ptr)__x;
      return __x;
    }
    _key_link_type& _M_get_root() const { 
      assert(_M_header != NULL); 
      assert(_M_header->_M_parent == _M_header->_M_left);
      return (_key_link_type&) _M_header->_M_left;
    }
    _key_link_type& _M_leftmost()   const { return (_key_link_type&) _M_bp_leftmost; }
    _key_link_type& _M_rightmost()  const {
        assert(_M_header != NULL);
        return (_key_link_type&) _M_header->_M_right;
    }
    
    _base_ptr _M_set_root(_base_ptr __x) {
      assert(_M_header != NULL); 
      _M_header->_M_parent = __x;
      _M_header->_M_left=__x;
      return __x;
    }
    _base_ptr& _M_get_root()      { 
      assert(_M_header != NULL); 
      assert(_M_header->_M_parent == _M_header->_M_left);
      return (_base_ptr&) _M_header->_M_left; 
    }
    _base_ptr& _M_leftmost()  { return (_base_ptr&) _M_bp_leftmost; }
    _base_ptr& _M_rightmost() { assert(_M_header != NULL); return (_base_ptr&) _M_header->_M_right; }

    static _key_link_type&   _S_left(_key_link_type __x)    { assert(__x != NULL); return (_key_link_type&)(__x->_M_left); }
    static _key_link_type&   _S_right(_key_link_type __x)   { assert(__x != NULL); return (_key_link_type&)(__x->_M_right); }
    static _key_link_type&   _S_parent(_key_link_type __x)  { assert(__x != NULL); return (_key_link_type&)(__x->_M_parent); }
    static sub_key_type&     _S_key(_key_link_type __x)     { assert(__x != NULL); return __x->_M_key_field; }
    static const _edge_type  _S_edge(_key_link_type __x)    { assert(__x != NULL); return (const _edge_type)(_b_trie_node_base___::_S_edge(__x)); }

    static _value_link_type& _S_left(_value_link_type __x)   { assert(__x != NULL); return (_value_link_type&)(__x->_M_left); }
    static _value_link_type& _S_right(_value_link_type __x)  { assert(__x != NULL); return (_value_link_type&)(__x->_M_right); }
    static _value_link_type& _S_parent(_value_link_type __x) { assert(__x != NULL); return (_value_link_type&)(__x->_M_parent); }
    static reference         _S_value(_value_link_type __x)  { assert(__x != NULL); return __x->_M_value_field; }
    static const _edge_type  _S_edge(_value_link_type __x)   { assert(__x != NULL); return (const _edge_type)(_b_trie_node_base___::_S_edge(__x)); }

    static _base_ptr&        _S_left(_base_ptr __x)   { assert(__x != NULL); return (_base_ptr&)(__x->_M_left); }
    static _base_ptr&        _S_right(_base_ptr __x)  { assert(__x != NULL); return (_base_ptr&)(__x->_M_right); }
    static _base_ptr&        _S_parent(_base_ptr __x) { assert(__x != NULL); return (_base_ptr&)(__x->_M_parent); }
    static reference         _S_value(_base_ptr __x)  { assert(__x != NULL); return ((_value_link_type)__x)->_M_value_field; }
    static sub_key_type&     _S_key(_base_ptr __x)    { assert(__x != NULL); return ((_key_link_type)__x)->_M_key_field;} 
    static const _edge_type  _S_edge(_base_ptr __x)   { assert(__x != NULL); return (const _edge_type)(_b_trie_node_base___::_S_edge(__x)); }

    static _key_link_type _S_trie_parent(_key_link_type __x) {
      return (_key_link_type) (_b_trie_node_base___::_S_trie_parent(__x)); 
    }
    static _key_link_type _S_trie_parent(_base_ptr __x) {
      return (_key_link_type) (_b_trie_node_base___::_S_trie_parent(__x)); 
    }
    
    static _key_link_type _S_minimum(_key_link_type __x) { 
        return (_key_link_type)  _b_trie_node_base___::_S_minimum(__x);
    }
    static _key_link_type _S_minimum(_value_link_type __x) { 
        return (_key_link_type)  _b_trie_node_base___::_S_minimum(__x);
    }
    static _key_link_type _S_minimum(_base_ptr __x) { 
        return (_key_link_type)  _b_trie_node_base___::_S_minimum(__x);
    }
    
    static _key_link_type _S_maximum(_key_link_type __x) {
        return (_key_link_type) _b_trie_node_base___::_S_maximum(__x);
    }
    static _key_link_type _S_maximum(_value_link_type __x) {
        return (_key_link_type) _b_trie_node_base___::_S_maximum(__x);
    }
    static _key_link_type _S_maximum(_base_ptr __x) {
        return (_key_link_type) _b_trie_node_base___::_S_maximum(__x);
    }

public:
    typedef _b_trie_iterator<value_type, reference, pointer>                   
        iterator;
    typedef _b_trie_iterator<value_type, const_reference, const_pointer> 
        const_iterator;

#ifdef __STL_CLASS_PARTIAL_SPECIALIZATION
    typedef std::reverse_iterator<const_iterator>    const_reverse_iterator;
    typedef std::reverse_iterator<iterator>          reverse_iterator;
#else  // __STL_CLASS_PARTIAL_SPECIALIZATION
    typedef std::reverse_bidirectional_iterator<iterator, value_type, reference, difference_type>
        reverse_iterator; 
    typedef std::reverse_bidirectional_iterator<const_iterator, value_type, const_reference, difference_type>
        const_reverse_iterator;
#endif // __STL_CLASS_PARTIAL_SPECIALIZATION

private:
    
    void _M_erase(bool fIsEdge, _base_ptr __x);

    iterator _M_k_insert(bool __fInsertLeft, _base_ptr __y, 
                       const sub_key_type& __k);
    iterator _M_v_insert(_base_ptr __y, const value_type& __v);
    void _M_balance(bool __fInsertLeft, _base_ptr __y, 
                   _base_ptr __z);

    _base_ptr _M_copy(_base_ptr __x, _base_ptr __p);

    enum __partial_find_type {
      __lower_bound,
      __upper_bound,
      __find,
      __find_if_prefix,
    };
#ifdef __STL_MEMBER_TEMPLATES  
    template <class _InputIterator>
    iterator _M_partial_find(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd, enum __partial_find_type __fType) const;
#else
    iterator _M_partial_find(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd,  enum __partial_find_type __fType) const;
#endif

    _STD pair<iterator,bool> _M_insert(bool __fInsertUnique, const value_type& __x);

public:
                                // allocation/deallocation
    _b_trie() 
        : _Base(allocator_type()), 
          _M_node_count(0),
          _M_key_compare()
    { _empty_initialize(); }

    _b_trie(const _Compare& __comp)
      : _Base(allocator_type()), 
        _M_node_count(0), 
        _M_key_compare(__comp)
      { _empty_initialize(); }


    _b_trie(const _Compare& __comp, 
           const allocator_type __a)
      : _Base(__a), 
        _M_node_count(0), 
        _M_key_compare(__comp)
      { _empty_initialize(); }

    _b_trie(const _self& __x) 
      : _Base(__x.get_allocator()), 
        _M_node_count(0),
        _M_key_compare(__x._M_key_compare)
      { 
        if (__x._M_get_root() == 0) {
            _empty_initialize();
        }
        else {
          _M_set_root(_M_copy(__x._M_get_root(), _M_header));
          _M_leftmost() = _S_minimum(_M_get_root());
          _M_rightmost() = _S_maximum(_M_get_root());
        }
        _M_node_count = __x._M_node_count;
    }

    ~_b_trie() {
        clear();
    }

    _self& operator=(const _self& __x);

private:
    void _empty_initialize() {
        _M_set_root(_base_ptr(0));
        _M_leftmost() = _M_header;
        _M_rightmost() = _M_header;
    }

public:    
                                // accessors:
    _Compare key_comp() const          { return _M_key_compare; }
    
    iterator begin()                      { return _M_leftmost(); }
    const_iterator begin() const          { return _M_leftmost(); }
    
    iterator end()                        { return _M_header; }
    const_iterator end() const            { return _M_header; }
    
    reverse_iterator rbegin()             { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    
    reverse_iterator rend()               { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const   { return const_reverse_iterator(begin()); } 
    
    bool empty() const                    { return _M_node_count == 0; }
    size_type size() const                { return _M_node_count; }
    size_type max_size() const            { return size_type(-1); }

    void swap(_self& __t) {
      std::swap(_M_header, __t._M_header);
      std::swap(_M_node_count, __t._M_node_count);
      std::swap(_M_key_compare, __t._M_key_compare);
      std::swap(_M_bp_leftmost, __t._M_bp_leftmost);
    }
    
public:
                                // insert/erase
    std::pair<iterator,bool> insert_unique(const value_type& __x);
    iterator insert_equal(const value_type& __x);
    
#ifdef __STL_MEMBER_TEMPLATES  
    template <class _InputIterator>
    void insert_unique(_InputIterator __first, _InputIterator __last);
    
    template <class _InputIterator>
    void insert_equal(_InputIterator __first, _InputIterator __last);
#else // __STL_MEMBER_TEMPLATES
    void insert_unique(const_iterator __first, const_iterator __last);
    void insert_unique(const value_type* __first, const value_type* __last);
    
    void insert_equal(const_iterator __first, const_iterator __last);
    void insert_equal(const value_type* __first, const value_type* __last);
#endif // __STL_MEMBER_TEMPLATES

    void erase(iterator __position);
    size_type erase(const key_type& __x);
    void erase(iterator __first, iterator __last);
    void erase(const key_type* __first, const key_type* __last);
    
    void clear() {
      if (_M_node_count != 0) {
        _M_erase((_S_edge(_M_get_root())==_S_b_trie_edge),_M_get_root());
        _M_set_root((_base_ptr)0);
        _M_leftmost() = _M_header;
        _M_rightmost() = _M_header;
        _M_node_count = 0;
      }
    }      

public:
                                // set operations:
    iterator find(const key_type& __x);
#ifdef __STL_MEMBER_TEMPLATES
    template <class _InputIterator>
    iterator find(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd);
#else
    iterator find(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd);
#endif

    const_iterator find(const key_type& __x) const;
#ifdef __STL_MEMBER_TEMPLATES
    template <class _InputIterator>
    const_iterator find(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd) const;
#else
    const_iterator find(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd) const;
#endif

    size_type count(const key_type& __x) const;
#ifdef __STL_MEMBER_TEMPLATES
    template <class _InputIterator>
    size_type count(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd) const;
#else
    size_type count(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd) const;
#endif

    iterator lower_bound(const key_type& __x);
#ifdef __STL_MEMBER_TEMPLATES
    template <class _InputIterator>
    iterator lower_bound(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd);
#else
    iterator lower_bound(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd);
#endif

    const_iterator lower_bound(const key_type& __x) const;
#ifdef __STL_MEMBER_TEMPLATES
    template <class _InputIterator>
    const_iterator lower_bound(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd) const;
#else
    const_iterator lower_bound(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd) const;
#endif

    iterator upper_bound(const key_type& __x);
#ifdef __STL_MEMBER_TEMPLATES
    template <class _InputIterator>
    iterator upper_bound(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd);
#else
    iterator upper_bound(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd);
#endif

    const_iterator upper_bound(const key_type& __x) const;
#ifdef __STL_MEMBER_TEMPLATES
    template <class _InputIterator>
    const_iterator upper_bound(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd) const;
#else
    const_iterator upper_bound(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd) const;
#endif

    std::pair<iterator,iterator> equal_range(const key_type& __x);
#ifdef __STL_MEMBER_TEMPLATES
    template <class _InputIterator>
    std::pair<iterator,iterator> equal_range(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd);
#else
    std::pair<iterator,iterator> equal_range(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd);
#endif

    std::pair<const_iterator, const_iterator> equal_range(const key_type& __x) const;
#ifdef __STL_MEMBER_TEMPLATES
    template <class _InputIterator>
    std::pair<const_iterator, const_iterator> equal_range(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd) const;
#else
    std::pair<const_iterator, const_iterator> equal_range(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd) const;
#endif


    iterator find_if_prefix(const key_type& __x);
#ifdef __STL_MEMBER_TEMPLATES
    template <class _InputIterator>
    iterator find_if_prefix(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd);
#else
    iterator find_if_prefix(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd);
#endif

    const_iterator find_if_prefix(const key_type& __x) const;
#ifdef __STL_MEMBER_TEMPLATES
    template <class _InputIterator>
    const_iterator find_if_prefix(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd) const;
#else
    const_iterator find_if_prefix(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd) const;
#endif


    // approximate string matching
public:
    std::vector< iterator > approximate_find(const key_type& __x, 
                                               unsigned int __k=1);
    std::vector< const_iterator > approximate_find(const key_type& __x, 
                                                     unsigned int __k=1) const;
    
    std::vector< iterator > best_find(const key_type& __x, 
                                        unsigned int __k=TRIE_INFINITY);
    std::vector< const_iterator > best_find(const key_type& __x, 
                                              unsigned int __k=TRIE_INFINITY) const;

private:
    unsigned int __edit_distance(const _Key& __P, 
                                 _key_link_type __W, 
                                 std::vector< std::vector<unsigned int> >& __DT,
                                 std::vector<unsigned int>& __rgCe, 
                                 std::vector<unsigned int>& __rgCb,
                                 unsigned int __i /*level*/, 
                                 unsigned int __k /*cutoff _S_value*/) const;
#ifdef _DEBUG
    unsigned int __edit_distance(const _Key& __P, 
                                 const std::vector<sub_key_type>& __W,
                                  std::vector< std::vector<unsigned int> > & __DT) const;
#endif

    std::vector< _key_link_type > __approximate_match(const key_type& __P, 
                                bool __fBestCase, unsigned int __k) const;
};



TRIE_TEMPLATE 
inline bool 
operator==(const B_TRIE& __x, 
           const B_TRIE& __y) 
{
  return  __x.size() == __y.size() && 
          equal(__x.begin(), __x.end(), __y.begin());
}

TRIE_TEMPLATE 
inline bool 
operator<(const B_TRIE& __x, 
          const B_TRIE& __y) 
{
  return lexicographical_compare(__x.begin(), __x.end(),
                                 __y.begin(), __y.end());
}

#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER
TRIE_TEMPLATE 
inline bool
operator!=(B_TRIE& __x,
           B_TRIE& __y) {
  return !(__x == __y);
}

TRIE_TEMPLATE 
inline bool
operator>(B_TRIE& __x,
           B_TRIE& __y) {
  return __y < __x;
}

TRIE_TEMPLATE 
inline bool
operator<=(B_TRIE& __x,
           B_TRIE& __y) {
  return !(__y < __x);
}


TRIE_TEMPLATE 
inline bool
operator>=(B_TRIE& __x,
           B_TRIE& __y) {
  return !(__x < __y);
}

TRIE_TEMPLATE 
inline void 
swap(B_TRIE& __x, 
     B_TRIE& __y) 
{
  __x.swap(__y);
}

#endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */

TRIE_TEMPLATE 
B_TRIE& B_TRIE::operator=(const B_TRIE& __x) 
{
  if (this != &__x) {
                                // Note that Key may be a constant type
    clear();
    _M_node_count = 0;
    _M_key_compare = __x._M_key_compare;
    if (__x._M_get_root() == 0) {
      _M_set_root(_base_ptr(0));
      _M_leftmost() = _M_header;
      _M_rightmost() = _M_header;
    } else {
      _M_set_root(_M_copy(__x._M_get_root(), _M_header));
      _M_leftmost() = _S_minimum(_M_get_root());
      _M_rightmost() = _S_maximum(_M_get_root());
      _M_node_count = __x._M_node_count;
    }
  }
  return *this;
}

TRIE_TEMPLATE 
typename B_TRIE::iterator
B_TRIE::_M_k_insert(bool __fInsertLeft, _base_ptr __y, const sub_key_type& __k) 
{
  _key_link_type __z = _M_create_key_node(__k);
  
  _M_balance(__fInsertLeft, __y, __z);

  return iterator(__z);
}

TRIE_TEMPLATE 
typename B_TRIE::iterator 
B_TRIE::_M_v_insert(_base_ptr __y, const value_type& __v) 
{
  _value_link_type __z = _M_create_value_node(__v);
  ++_M_node_count;
  _M_balance(true, __y, __z);
  return iterator(__z);
}


TRIE_TEMPLATE 
void
B_TRIE::_M_balance(bool __fInsertLeft/*is z to be the left node of the parent*/, 
                  _base_ptr __y/*parent of z*/, 
                  _base_ptr __z/*inserted node*/)
{
// potential combinations:
//  y - a         here left(z) = 0; right(z) = left(y); parent(left(y)) = z; parent(z) = y; left(y) = z;
//  |
//  z - b         (fInsertLeft == true) leftmost may have changed, rightmost may have changed (if there was a single left branch)
//
// or
//
//  y - z - a     (fInsertLeft == false) rightmost may have changed, leftmost didn't
//  |
//  b             here left(z) = 0; right(z) = right(y); parent(right(y)) = z; parent(z) = y; right(y) = z;
//
// the case of the header is a special case of (fInsertLeft == true)

  _base_ptr __x; // a or b depending on whether inserting left or right.
  
  assert(__y != 0);
  assert(__z != 0);
  
  
  // y has a new child
  if(__fInsertLeft || __y == _M_header) {
    __x = _S_left(__y);
  }
  else {
    __x = _S_right(__y);
  }

  if(__fInsertLeft || __y == _M_header) {
    _S_left(__y) = __z;
    if (__y == _M_header) {
      _M_set_root(__z);
      _M_leftmost() = __z; 
      if(_M_rightmost() == _M_header) {
        _M_rightmost() = __z;
      }
    }
  }
  else {
    _S_right(__y) = __z;
  }

  
  // z is a new node
  _S_parent(__z) = __y; // new node's parent is always y.
  _S_left(__z) = 0;     // new nodes are always inserted at the edge (this is an assumption based on the working of trie)
  _S_right(__z) = __x;  // if fInsertLeft then this is y's old left otherwise it is y's old right.

  // x has a new _S_parent
  if(__x != 0) {
    _S_parent(__x) = __z;
  }

  // make sure leftmost and rightmost point to the correct node.
  // this can only be optimized by using minimum and maximum so we might as well use brute force
  if (__y != _M_header) {
    assert(_M_get_root() != 0);
    if (__fInsertLeft == true) {
      _M_leftmost() = _S_minimum(_M_get_root());
    }
    _M_rightmost() = _S_maximum(_M_get_root());
  }
  assert(_M_leftmost() == _S_minimum(_M_get_root()));
}

TRIE_TEMPLATE 
typename B_TRIE::iterator
B_TRIE::insert_equal(const _Value& __v)
{
  _STD pair<iterator, bool> __pitb = _M_insert(false, __v);  
  
  assert(__pitb.second); // the value should always be inserted
  return __pitb.first;
}

TRIE_TEMPLATE 
std::pair<typename B_TRIE::iterator, 
            bool>
B_TRIE::insert_unique(const _Value& __v)
{
  return _M_insert(true, __v);  
}

TRIE_TEMPLATE 
std::pair< typename B_TRIE::iterator, 
             bool>
B_TRIE::_M_insert(bool __fInsertUnique /*allow only one value per key?*/, 
                 const _Value& __v)
{
  _base_ptr __y = _M_header;
  _base_ptr __x = _M_get_root();
  bool __inserted = false;
  iterator __j=end();
  
  _KeyIterator __itKey = _KeyBegin()(_KeyOfValue()(__v));
  _KeyIterator __itKeyEnd = _KeyEnd()(_KeyOfValue()(__v));
  _KeySizeType __depth = _KeySize()(_KeyOfValue()(__v));
  --__depth;

  if(__itKey == __itKeyEnd) {
    // check to see if there is already a value at this place
    if ((__fInsertUnique == false) || __x==0 || _S_edge(__x) == _S_b_trie_non_edge ) {
      // add the value
      __j = _M_v_insert(__y, __v);
      __inserted = true;
    }
  }
  else {
    for(;__itKey != __itKeyEnd; ++__itKey, --__depth)  {
      bool __fInsertLeft = true;
      // go right until you go past where the sub key of this level would be inserted
      // if it is a value keep going right until we hit a key
      while (__x != 0 && ((_S_edge(__x) == _S_b_trie_edge) || (_M_key_compare(_S_key(__x), *__itKey)))) {
        __y = __x;
        __x = _S_right(__x);
        __fInsertLeft = false;
      }
      // check to see if the sub key has a match at this level
      if (__x==0 || _M_key_compare(*__itKey, _S_key(__x))) {
        // it does not have a match so insert the key.
        __j = _M_k_insert(__fInsertLeft, __y, *__itKey);
        // if this is the end of our "key string" then we need to insert 
        // the value as well
        if(__depth == 0) { // the value is only inserted at end of the "key string"
          __j = _M_v_insert(__j._M_node, __v);
        }
        __inserted = true;
      }
      else {
        __j = iterator(__x);
        // it has a match at this level.
        if (__depth == 0){
          // if we are at the end of the key and have followed it down the trie 
          // successfully, check to see if there is already a value at this place
          assert(_S_left(__x) != 0); // we can assume there is always a value or a further key.
          if ((__fInsertUnique == false) || _S_edge(_S_left(__x)) == _S_b_trie_non_edge ) {
            // add the value
            __j = _M_v_insert(__x, __v);
            __inserted = true;
          }
          else {
            assert(_S_edge(_S_left(__x)) == _S_b_trie_edge);
            __j = iterator(_S_left(__x));
          }
        }
      }
      __y = _key_link_type(__j._M_node);
      __x = _S_left(__y); // descend
    }
  }

  return _STD pair<iterator, bool> (__j, __inserted);
}

#ifdef __STL_MEMBER_TEMPLATES  

TRIE_TEMPLATE 
template<class _InputIterator>
void 
B_TRIE::insert_equal(_InputIterator __first, _InputIterator __last) 
{
  for ( ; __first != __last; ++__first) {
    insert_equal(*__first);
  }
}

TRIE_TEMPLATE 
template<class _InputIterator>
void 
B_TRIE::insert_unique(_InputIterator __first, _InputIterator __last) 
{
  for ( ; __first != __last; ++__first) {
    insert_unique(*__first);
  }
}

#else //__STL_MEMBER_TEMPLATES
TRIE_TEMPLATE 
void 
B_TRIE::insert_equal(const _Value* __first, const _Value* __last) 
{
  for(; __first != __last; ++__first) {
    insert_equal(*__first);
  }
}


TRIE_TEMPLATE 
void 
B_TRIE::insert_equal(const_iterator __first, const_iterator __last) 
{
  for(; __first != __last; ++__first) {
    insert_equal(*__first);
  }
}

TRIE_TEMPLATE 
void 
B_TRIE::insert_unique(const _Value* __first, const _Value* __last) 
{
  for(; __first != __last; ++__first) {
    insert_unique(*__first);
  }
}


TRIE_TEMPLATE 
void 
B_TRIE::insert_unique(const_iterator __first, const_iterator __last) 
{
  for(; __first != __last; ++__first) {
    insert_unique(*__first);
  }
}

#endif // __STL_MEMBER_TEMPLATES
         
TRIE_TEMPLATE 
inline void
B_TRIE::erase(iterator __position) 
{
  _base_ptr __nodeT;
  bool __fErasableNode;
  bool __fIsEdge = true;

  if(_S_left(__position._M_node) != 0) {
    assert(false); // this should be the value so this should never happen!
  }
  else {
    if(_S_right(__position._M_node) == 0) { // has no siblings
      do {
        __fErasableNode = true;
        if(_S_left(_S_parent(__position._M_node)) == __position._M_node) {
          _S_left(_S_parent(__position._M_node)) = 0;
        }
        else {
          _S_right(_S_parent(__position._M_node)) = 0;
          __fErasableNode = false;
        }
      
        if(__position._M_node == _M_leftmost()) {
          _M_leftmost() = _S_parent(__position._M_node);
        }
        else if(__position._M_node == _M_rightmost()) {
          _M_rightmost() = _S_maximum(_S_parent(__position._M_node));
        }
        __nodeT = __position._M_node;
        
        __position._M_node = _S_trie_parent(__position._M_node);
      
        _M_erase(__fIsEdge, _key_link_type(__nodeT));
        __fIsEdge = false;
      } while(__position._M_node != 0 && _S_left(__position._M_node) == 0 && _S_right(__position._M_node) == 0 && __fErasableNode);
    }
  
    if (__position._M_node == 0) {
      // we now have an empty trie
      assert(_M_node_count == 0);
      _M_set_root((_base_ptr)0);
      _M_leftmost() = _M_header;
      _M_rightmost() = _M_header;
    }
    else if(_S_left(__position._M_node) == 0 && _S_right(__position._M_node) != 0)
    {
      _S_parent(_S_right(__position._M_node)) = _S_parent(__position._M_node);
      // rebalance
      if(_S_parent(_S_parent(__position._M_node)) == __position._M_node) { // the special case for the root
        assert(_S_parent(__position._M_node) == _M_header);
        _M_set_root(_S_right(__position._M_node));
      }
      else if(_S_right(_S_parent(__position._M_node)) == __position._M_node) {
        _S_right(_S_parent(__position._M_node)) = _S_right(__position._M_node);
      }
      else {
        _S_left(_S_parent(__position._M_node)) = _S_right(__position._M_node);
      }

      if(__position._M_node == _M_leftmost()) {
        _M_leftmost() = _S_minimum(_S_right(__position._M_node));
      }
    
      _S_right(__position._M_node) = 0;
      _M_erase(__fIsEdge, _key_link_type(__position._M_node));
    }
  }
  assert(_M_get_root() == 0 || _M_leftmost() == _S_minimum(_M_get_root()));
  assert(_M_get_root() == 0 || _M_rightmost() == _S_maximum(_M_get_root()));
  assert(_M_get_root() != 0 || _M_leftmost() == _M_header);
  assert(_M_get_root() != 0 || _M_rightmost() == _M_header);
}

TRIE_TEMPLATE 
typename B_TRIE::size_type 
B_TRIE::erase(const key_type& __x) 
{
  std::pair<iterator, iterator> __p = equal_range(__x);
  size_type __n = std::distance(__p.first, __p.second);
  erase(__p.first, __p.second);
  return __n;
}

TRIE_TEMPLATE 
typename B_TRIE::_base_ptr 
B_TRIE::_M_copy(_base_ptr __x, _base_ptr __p) 
{
   // structural copy. x and p must be non-null
  assert(__x != 0);
  assert(__p != 0);

  _base_ptr __top = _M_clone_node(__x);
  assert(__top != NULL);
  __top->_M_parent = __p;

   try {
    if(_S_right(__x)) {
      _S_right(__top) = _M_copy(_S_right(__x), __top);
    }
    __p = __top;
    __x = _S_left(__x);

    while (__x != 0) {
      _base_ptr __y = _M_clone_node(__x);
      _S_left(__p) = __y;
      _S_parent(__y) = __p;
      if (_S_right(__x)) {
        _S_right(__y) = _M_copy(_S_right(__x), __y);
      }
      __p = __y;
      __x = _S_left(__x);
    }
  }
  catch(...) { 
    (_M_erase((_S_edge(__top)==_S_b_trie_edge), __top));
    throw; 
  }

  return __top;
}

TRIE_TEMPLATE 
void 
B_TRIE::_M_erase(bool __fIsEdge, _base_ptr __x) 
{
  // erases all children 
  while (__x != 0) {
    assert(__x != NULL);
    assert(__x->_debugfIsEdge == __fIsEdge);
    _M_erase(((_S_right(__x)!=0)?(_S_edge(_S_right(__x))==_S_b_trie_edge):false),_S_right(__x));
    _base_ptr __y = _S_left(__x);
    destroy_node(__fIsEdge, __x);
    __x = __y;
    if (__x!=0) {
      __fIsEdge = _S_edge(__x);
    }
  }
}

TRIE_TEMPLATE 
void 
B_TRIE::erase(iterator __first, iterator __last) 
{
  if (__first == begin() && __last == end()) {
    clear();
  }
  else {
    while (__first != __last) {
      erase(__first++);
    }
  }
}

TRIE_TEMPLATE 
void 
B_TRIE::erase(const key_type* __first, const key_type* __last) 
{
  while (__first != __last) {
    erase(*__first++);
  }
}

TRIE_TEMPLATE 
typename B_TRIE::iterator 
B_TRIE::find(const key_type& __k) 
{
  typename _KeyIterator __itKeyBegin = _KeyBegin()(__k);
  typename _KeyIterator __itKeyEnd = _KeyEnd()(__k);

  return find(__itKeyBegin, __itKeyEnd);
}

TRIE_TEMPLATE 
typename B_TRIE::const_iterator 
B_TRIE::find(const key_type& __k) const 
{
  typename _KeyIterator __itKeyBegin = _KeyBegin()(__k);
  typename _KeyIterator __itKeyEnd = _KeyEnd()(__k);

  return find(__itKeyBegin, __itKeyEnd);
}

TRIE_TEMPLATE 
#ifdef __STL_MEMBER_TEMPLATES
  template<class _InputIterator>
  typename B_TRIE::iterator 
  B_TRIE::find(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd) 
#else
  typename B_TRIE::iterator 
  B_TRIE::find(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd) 
#endif
{
  return _M_partial_find(__itKeyBegin, __itKeyEnd, __find);
}


TRIE_TEMPLATE 
#ifdef __STL_MEMBER_TEMPLATES
  template<class _InputIterator>
  typename B_TRIE::const_iterator 
  B_TRIE::find(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd)  const 
#else
  typename B_TRIE::const_iterator 
  B_TRIE::find(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd)  const 
#endif
{
  return const_iterator(_M_partial_find(__itKeyBegin, __itKeyEnd, __find));
}

TRIE_TEMPLATE 
typename B_TRIE::iterator 
B_TRIE::find_if_prefix(const key_type& __k) 
{
  typename _KeyIterator __itKeyBegin = _KeyBegin()(__k);
  typename _KeyIterator __itKeyEnd = _KeyEnd()(__k);

  return find_if_prefix(__itKeyBegin, __itKeyEnd);
}

TRIE_TEMPLATE 
typename B_TRIE::const_iterator 
B_TRIE::find_if_prefix(const key_type& __k) const 
{
  typename _KeyIterator __itKeyBegin = _KeyBegin()(__k);
  typename _KeyIterator __itKeyEnd = _KeyEnd()(__k);

  return find_if_prefix(__itKeyBegin, __itKeyEnd);
}

TRIE_TEMPLATE 
#ifdef __STL_MEMBER_TEMPLATES
  template<class _InputIterator>
  typename B_TRIE::iterator 
  B_TRIE::find_if_prefix(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd) 
#else
  typename B_TRIE::iterator 
  B_TRIE::find_if_prefix(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd) 
#endif
{
  return _M_partial_find(__itKeyBegin, __itKeyEnd, __find_if_prefix);
}

TRIE_TEMPLATE 
#ifdef __STL_MEMBER_TEMPLATES
  template<class _InputIterator>
  typename B_TRIE::const_iterator 
  B_TRIE::find_if_prefix(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd)  const 
#else
  typename B_TRIE::const_iterator 
  B_TRIE::find_if_prefix(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd)  const 
#endif
{
  return const_iterator(_M_partial_find(__itKeyBegin, __itKeyEnd, __find_if_prefix));
}

TRIE_TEMPLATE 
typename B_TRIE::size_type 
B_TRIE::count(const key_type& __k) const 
{
  std::pair<const_iterator, const_iterator> __p = equal_range(__k);
  size_type __n = std::distance(__p.first, __p.second);

  return __n;
}

TRIE_TEMPLATE 
#ifdef __STL_MEMBER_TEMPLATES
  template<class _InputIterator>
  typename B_TRIE::size_type 
  B_TRIE::count(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd) const 
#else
  typename B_TRIE::size_type 
  B_TRIE::count(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd) const 
#endif
{
  std::pair<const_iterator, const_iterator> __p = equal_range(__itKeyBegin, __itKeyEnd);
  size_type __n = 0;
  distance(__p.first, __p.second, __n);

  return __n;
}

TRIE_TEMPLATE 
typename B_TRIE::iterator 
B_TRIE::lower_bound(const key_type& __k) 
{
  typename _KeyIterator __itKeyBegin = _KeyBegin()(__k);
  typename _KeyIterator __itKeyEnd = _KeyEnd()(__k);

  return lower_bound(__itKeyBegin, __itKeyEnd);
}

TRIE_TEMPLATE 
typename B_TRIE::const_iterator 
B_TRIE::lower_bound(const key_type& __k) const 
{
  typename _KeyIterator __itKeyBegin = _KeyBegin()(__k);
  typename _KeyIterator __itKeyEnd = _KeyEnd()(__k);

  return lower_bound(__itKeyBegin, __itKeyEnd);
}

TRIE_TEMPLATE 
#ifdef __STL_MEMBER_TEMPLATES
  template<class _InputIterator>
  typename B_TRIE::iterator 
  B_TRIE::lower_bound(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd) 
#else
  typename B_TRIE::iterator 
  B_TRIE::lower_bound(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd) 
#endif
{
  return _M_partial_find(__itKeyBegin, __itKeyEnd, __lower_bound);
}

TRIE_TEMPLATE 
#ifdef __STL_MEMBER_TEMPLATES
  template<class _InputIterator>
  typename B_TRIE::const_iterator 
  B_TRIE::lower_bound(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd) const 
#else
  typename B_TRIE::const_iterator 
  B_TRIE::lower_bound(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd) const 
#endif
{
  return const_iterator(_M_partial_find(__itKeyBegin, __itKeyEnd, __lower_bound));
}


TRIE_TEMPLATE 
typename B_TRIE::iterator 
B_TRIE::upper_bound(const key_type& __k) 
{
  typename _KeyIterator __itKeyBegin = _KeyBegin()(__k);
  typename _KeyIterator __itKeyEnd = _KeyEnd()(__k);

  return upper_bound(__itKeyBegin, __itKeyEnd);
}

TRIE_TEMPLATE 
typename B_TRIE::const_iterator 
B_TRIE::upper_bound(const key_type& __k) const 
{
  typename _KeyIterator __itKeyBegin = _KeyBegin()(__k);
  typename _KeyIterator __itKeyEnd = _KeyEnd()(__k);

  return upper_bound(__itKeyBegin, __itKeyEnd);
}

TRIE_TEMPLATE 
#ifdef __STL_MEMBER_TEMPLATES
  template<class _InputIterator>
  typename B_TRIE::iterator 
  B_TRIE::upper_bound(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd) 
#else
  typename B_TRIE::iterator 
  B_TRIE::upper_bound(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd) 
#endif
{
  return _M_partial_find(__itKeyBegin, __itKeyEnd, __upper_bound);
}

TRIE_TEMPLATE 
#ifdef __STL_MEMBER_TEMPLATES
  template<class _InputIterator>
  typename B_TRIE::const_iterator 
  B_TRIE::upper_bound(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd) const 
#else
  typename B_TRIE::const_iterator 
  B_TRIE::upper_bound(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd) const 
#endif
{
  return const_iterator(_M_partial_find(__itKeyBegin, __itKeyEnd, __upper_bound));
}

TRIE_TEMPLATE 
inline 
std::pair<typename B_TRIE::iterator, typename B_TRIE::iterator>
B_TRIE::equal_range(const key_type& __k) 
{
  return std::pair<iterator, iterator>(lower_bound(__k), upper_bound(__k));
}

TRIE_TEMPLATE 
inline 
std::pair<typename B_TRIE::const_iterator, typename B_TRIE::const_iterator>
B_TRIE::equal_range(const key_type& __k) const 
{
  return std::pair<const_iterator,const_iterator>(lower_bound(__k), upper_bound(__k));
}

TRIE_TEMPLATE 
#ifdef __STL_MEMBER_TEMPLATES
  template<class _InputIterator>
  inline 
  std::pair<typename B_TRIE::iterator, typename B_TRIE::iterator>
  B_TRIE::equal_range(const _InputIterator& __itKeyBegin, const _InputIterator& __itKeyEnd) 
#else
  inline 
  std::pair<typename B_TRIE::iterator, typename B_TRIE::iterator>
  B_TRIE::equal_range(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd) 
#endif
{
  return std::pair<iterator, iterator>(lower_bound(__itKeyBegin, __itKeyEnd), 
                                         upper_bound(__itKeyBegin, __itKeyEnd));
}

TRIE_TEMPLATE 
#ifdef __STL_MEMBER_TEMPLATES
  template<class _InputIterator>
  inline 
  std::pair<typename B_TRIE::const_iterator, typename B_TRIE::const_iterator>
  B_TRIE::equal_range(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd) const 
#else
  inline 
  std::pair<typename B_TRIE::const_iterator, typename B_TRIE::const_iterator>
  B_TRIE::equal_range(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd) const 
#endif
{
  return std::pair<iterator, iterator>(lower_bound(__itKeyBegin, __itKeyEnd), 
                                         upper_bound(__itKeyBegin, __itKeyEnd));
}

TRIE_TEMPLATE 
#ifdef __STL_MEMBER_TEMPLATES
  template<class _InputIterator>
  typename B_TRIE::iterator
  B_TRIE::_M_partial_find(const _InputIterator __itKeyBegin, const _InputIterator __itKeyEnd,  __partial_find_type __fType) const 
#else
  typename B_TRIE::iterator
  B_TRIE::_M_partial_find(const _KeyIterator& __itKeyBegin, const _KeyIterator& __itKeyEnd,  __partial_find_type __fType) const 
#endif
{
 
  _key_link_type __z = _M_header; /* Last node with a value node which is a prefix of k. */
  _key_link_type __y = _M_header; /* Last node which is not greater than k. */
  _key_link_type __x = _M_get_root(); /* Current node. */
  bool __fFound = false;
  iterator __j = iterator(__y); // same as except without const end();
#ifdef __STL_MEMBER_TEMPLATES
  typename _InputIterator __itKey = __itKeyBegin;
#else
  _KeyIterator __itKey = __itKeyBegin;
#endif

  if(__itKey == __itKeyEnd) {
    // check to see if there is an empty value 
    if (__x!=0 && _S_edge(__x) == _S_b_trie_edge ) {
      __j = iterator(__x);
      __fFound = true; // an exact match was found.
    }
  }
  else {

    for(; __itKey != __itKeyEnd; ++__itKey) {
      __fFound = false;                          // default to not found ESA 12/22/97
      if(__x != 0) {
        while (__x!= 0 && 
          ((_S_edge(__x) == _S_b_trie_edge) || (_M_key_compare(_S_key(__x),*__itKey)))) { // less than subkey
          __y = __x;
          __x = _S_right(__x);
        }
        if (__x == 0) {
          //__j = ++iterator(__y);
        }
        else if (_M_key_compare(*__itKey, _S_key(__x))){ // greater than subkey
          __j = iterator(__x);
          break;
        }
        else // equal to subkey
        {
          assert(_S_left(__x) != 0); // we can assume there is always a value or a further key.
          if(_S_edge(_S_left(__x)) == _S_b_trie_edge) {   // it is only found if it is an edge. ESA 12/22/97
            __x = _S_left(__x); // descend
            if(__fType == __upper_bound) {
              while (_S_right(__x)!= 0 && (_S_edge(_S_right(__x)) == _S_b_trie_edge)) {
                __x = _S_right(__x);
              }
            }
            assert(__x != NULL);
            assert(__x->_debugfIsEdge == true);
            __j = iterator(__x);   
            __z = __x;
          }
          else {
            __j = iterator(__x);   
            __x = _S_left(__x); // descend
          }
          __fFound = true; // an exact match was found.
        }
      }
      else
      {
        // if the path found in the trie is smaller than the key then we did not
        // find a match.                  // ESA fixed this bug 12/20/97
        // follow the rightmost path of this subtree.
        __j = iterator(_S_maximum(__y)); // ESA fixed this bug 9/29/2002
        break;
      }
    }
  }
  switch (__fType) {
    case __find:
      assert(__j._M_node != NULL);
      if (__fFound == true && _S_edge(__j._M_node) == _S_b_trie_edge) { /* if found by partial find routine, may only have matched the _S_key that was there. Make sure that there is a _S_value to go along with it.*/
        assert(__j._M_node->_debugfIsEdge == true);
      }
      else {
        __j = iterator(_M_header); //end();
      }
      break;
    case __find_if_prefix:
      __j = iterator(__z);   
            
      if(_S_edge(__j._M_node) != _S_b_trie_edge) {
        __j = iterator(_M_header); //end();
      }
      break;
    case __lower_bound:
      if ((__fFound == false && __j != iterator(_M_header)/*end()*/)) {
        ++__j;
      }
      break;
    case __upper_bound:
      if (__fFound == true || __j != iterator(_M_header)/*end()*/) {
        ++__j;
      }
      break;
    default:
      assert(false);
  }
  return __j;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Approximate string matching
//

TRIE_TEMPLATE 
std::vector< typename B_TRIE::iterator > 
B_TRIE::approximate_find(const key_type& __P, unsigned int __k)
{
  std::vector<_key_link_type>                      __rgNodes;
  typename std::vector<_key_link_type>::iterator   __itrgNodes;
  std::vector<iterator>                           __rgIt;

  __rgNodes = __approximate_match(__P, false, __k); 
  if(!__rgNodes.empty()) {
    for(__itrgNodes = __rgNodes.begin(); __itrgNodes != __rgNodes.end(); ++__itrgNodes) {
      __rgIt.push_back(iterator(*__itrgNodes));
    }
  }  
  return __rgIt;
}

TRIE_TEMPLATE 
std::vector< typename B_TRIE::const_iterator > 
B_TRIE::approximate_find(const key_type& __P, unsigned int __k) const
{
  _STD vector<_key_link_type>						          __rgNodes;
  typename _STD vector<_key_link_type>::iterator   __itrgNodes;
  _STD vector<const_iterator>						          __rgIt;

  __rgNodes = __approximate_match(__P, false, __k); 
  if(!__rgNodes.empty()) {
    for(__itrgNodes = __rgNodes.begin(); __itrgNodes != __rgNodes.end(); ++__itrgNodes) {
      __rgIt.push_back(const_iterator(*__itrgNodes));
    }
  }  
  return __rgIt;
}

TRIE_TEMPLATE 
std::vector< typename B_TRIE::iterator >
B_TRIE::best_find(const key_type& __x, unsigned int __k)
{
  std::vector<_key_link_type>                      __rgNodes;
  typename std::vector<_key_link_type>::iterator   __itrgNodes;
  std::vector<iterator>                           __rgIt;

  // best_case match
  __rgNodes = __approximate_match(__x, true, __k); 
  if(!__rgNodes.empty()) {
    for(__itrgNodes = __rgNodes.begin(); __itrgNodes != __rgNodes.end(); ++__itrgNodes) {
      __rgIt.push_back(iterator(*__itrgNodes));
    }
  }  
  return __rgIt;
}

TRIE_TEMPLATE 
std::vector< typename B_TRIE::const_iterator >
B_TRIE::best_find(const key_type& __x, unsigned int __k) const
{
  std::vector<_key_link_type>                        __rgNodes;
  typename std::vector<_key_link_type>::iterator     __itrgNodes;
  std::vector<const_iterator>                       __rgIt;

  // best_case match
  __rgNodes = __approximate_match(__x, true, __k); 
  if(!__rgNodes.empty()) {
    for(__itrgNodes = __rgNodes.begin(); __itrgNodes != __rgNodes.end(); ++__itrgNodes) {
      __rgIt.push_back(const_iterator(*__itrgNodes));
    }
  }  
  return __rgIt;
}


TRIE_TEMPLATE 
std::vector< typename B_TRIE::_key_link_type > 
B_TRIE::__approximate_match(const key_type& __P, bool __fBestCase, unsigned int __k /*cutoff*/) const
{
  std::vector<_key_link_type>                __rgNodes;
  std::vector< _STD vector<unsigned int> >  __DT;
  std::vector<unsigned int>                 __rgCe, __rgCb;
  unsigned int __level = 0;
  unsigned int __d; // edit distance

  bool __fFollowTrie = true;

  __DT.resize(_KeySize()(__P)+1);
  __rgCe.resize(_KeySize()(__P)+1);
  __rgCb.resize(_KeySize()(__P)+1);
  
  _key_link_type __node = _M_get_root(); /* Current node. */

  while ((__node != NULL) && (_S_edge(__node) == _S_b_trie_edge)) {
    assert(__node != NULL);
    __node = (_key_link_type) __node->_M_right;
  }

  if (__node != NULL) {
  
    // initialize first column
    assert(__node->_debugfIsEdge == false);

    __d = __edit_distance(__P, __node, __DT, __rgCe, __rgCb, __level, __k);

    __level = 1;
    if(__DT.size() < __level+1) {
      __DT.resize(__level+1);
      __rgCe.resize(__level+1);
      __rgCb.resize(__level+1);
    }
  
    while (__node != _M_header) {
      // we can only perform an edit_distance operation on keys
      if (_S_edge(__node) == _S_b_trie_non_edge) {
        assert(__node->_debugfIsEdge == false);
        
        // visit the node
        __d = __edit_distance(__P, __node, __DT, __rgCe, __rgCb, __level, __k);

#ifdef _DEBUG
        {
          std::vector<sub_key_type> W_DEBUG;
          if(W_DEBUG.size() < __level) {
            W_DEBUG.resize(__level);
          }
          _key_link_type node_DEBUG = __node;
          for(unsigned int level_DEBUG = __level; level_DEBUG > 0; --level_DEBUG) {
            assert(level_DEBUG <= W_DEBUG.size());
            assert(node_DEBUG != 0);
            assert(_S_edge(node_DEBUG) != _S_b_trie_edge); // is key valid on next line
            W_DEBUG[level_DEBUG-1] = _S_key(node_DEBUG);
            node_DEBUG = _S_trie_parent(node_DEBUG);
          }
      
          std::vector< std::vector<unsigned int> > DT_DEBUG;
          unsigned int d_DEBUG = __edit_distance(__P, W_DEBUG, DT_DEBUG);

#if 0
          for(unsigned int i_DEBUG=0; i_DEBUG != _KeySize()(__P); ++i_DEBUG) {
            TRACE(L"%c",__P[i_DEBUG]);
          }
          TRACE(L" : ");
          for(std::vector<sub_key_type>::iterator i = W_DEBUG.begin(); i!= W_DEBUG.end(); ++i){
            TRACE(L"%c",*i);
          }
          TRACE(L" : %d\n",__d);
#endif

#ifdef __DISPLAY_DP // Display DP
          TRACE(L"\nEdit Distance Algorithm\n");
          TRACE(L" k: %d\n",  __k);
          TRACE(L"Optimized algorithm distance: %d\n", __d);
          {for(int i_DEBUG=0; i_DEBUG <= _KeySize()(__P); i_DEBUG++) {
            if(i_DEBUG == 0) {
              TRACE(L"\t\t");
              for(unsigned int j_DEBUG=0; j_DEBUG < W_DEBUG.size(); ++j_DEBUG)
              {
                assert(W_DEBUG.size() > j_DEBUG);
                TRACE(L"%c\t", W_DEBUG[j_DEBUG]);
              }
              TRACE(L"\n\t");
            }
            else {
              assert(__P.size() > i_DEBUG-1);
              TRACE(L"%c\t", __P[i_DEBUG-1]);
            }
            for(unsigned int j_DEBUG=0; j_DEBUG <= W_DEBUG.size(); ++j_DEBUG) {
              assert(__DT.size() > j_DEBUG);
              assert(__DT[j_DEBUG].size() > i_DEBUG);

              if(__DT[j_DEBUG][i_DEBUG] != TRIE_INFINITY) {
                TRACE(L"%d", __DT[j_DEBUG][i_DEBUG]);
              }
              TRACE(L"\t");
            }
            TRACE(L"\n");
          }}

          assert(rgCe.size() > i_DEBUG);
          TRACE(L"rgCe[level]:\n");
          {for(unsigned int i_DEBUG=0; i_DEBUG <= __level; ++i_DEBUG) {
            TRACE(L"%d\n", __rgCe[i_DEBUG]);
          }}

          TRACE(L"\nBrute-force algorithm distance: %d\n", d_DEBUG);
          {for(unsigned int i_DEBUG=0; i_DEBUG <= _KeySize()(__P); i_DEBUG++) {
            if(i_DEBUG == 0) {
              TRACE(L"\t\t");
              for(unsigned int j_DEBUG=0; j_DEBUG < W_DEBUG.size(); ++j_DEBUG)
              {
                assert(W_DEBUG.size() > j_DEBUG);
                TRACE(L"%c\t", W_DEBUG[j_DEBUG]);
              }
              TRACE(L"\n\t");
            }
            else {
              assert(__P.size() > i_DEBUG-1);
              TRACE(L"%c\t", __P[i_DEBUG-1]);
            }
            for(unsigned int j_DEBUG=0; j_DEBUG <= W_DEBUG.size(); ++j_DEBUG) {
              assert(DT_DEBUG.size() > j_DEBUG);
              assert(DT_DEBUG[j_DEBUG].size() > i_DEBUG);

              if(DT_DEBUG[j_DEBUG][i_DEBUG] != TRIE_INFINITY) {
                TRACE(L"%d", DT_DEBUG[j_DEBUG][i_DEBUG]);
              }
              TRACE(L"\t");
            }
            TRACE(L"\n");
          }}
#endif
          if (__d != d_DEBUG) {
            assert(__rgCe.size() > __level);
            if ((__rgCe[__level] == 0) || (d_DEBUG > __k)) {
              d_DEBUG = TRIE_INFINITY;
            }
          }
          assert(__d == d_DEBUG);

        }
#endif
      }
      // if lowest possible distance is greater than cut off
      // (the end is 0 in other words don't begin)
#if 0
      size_type __crgCe__ = __rgCe.size();
      size_type __crgCb__ = __rgCb.size();
      size_type __cDT__ = __DT.size();
#endif

      assert(__rgCe.size() > __level);
      if(__rgCe[__level] == TRIE_CUTOFF && _S_edge(__node) == _S_b_trie_non_edge) {
        assert(__node != NULL);
        assert(__node->_debugfIsEdge == false);
        // cut off this subtrie
        assert(__d==TRIE_INFINITY || __d > __k);
        __fFollowTrie = false;
      }
      else {
        // if we have a word (signaled by word end) whose edit distance is within cut off
        if(_S_edge(__node) == _S_b_trie_edge && __d <= __k) {
          assert(__node != NULL);
          assert(__node->_debugfIsEdge == true);

          // has our cutoff changed?
          if (__fBestCase && __d < __k) {
            __k = __d;
            __rgNodes.clear();
          }
          __rgNodes.push_back(__node);
        }
        __fFollowTrie = true;
      }
      assert(__node != NULL);
        
      if(__node->_M_left && __fFollowTrie) { // traverse the left subtree
        ++__level;
        if(__DT.size() < __level+1) {
          __DT.resize(__level+1);
          __rgCe.resize(__level+1);
          __rgCb.resize(__level+1);
        }
        __node = (_key_link_type) __node->_M_left;
      }
      else if(__node->_M_right) { // traverse the left subtree
        __node = (_key_link_type) __node->_M_right;
      } 
      else { // move back up traversing the right subtrees
        _base_ptr __y = __node->_M_parent;
        assert(__y != NULL);
        while ((__y->_M_right == 0 || __y->_M_right == __node) && (__y != _M_header)) {
          if(__y->_M_right == 0) {
            --__level;
          }
          __node = (_key_link_type) __y;
          __y = __y->_M_parent;
          assert(__y != NULL);
        }
        if (__y == _M_header) {
          __node = (_key_link_type) __y;
        }
        else {
          --__level;
          __node = (_key_link_type) __y->_M_right;
        }
      }
    }  
  }
  return __rgNodes;
}

TRIE_TEMPLATE 
inline unsigned int
B_TRIE::__edit_distance(const key_type& P, _key_link_type W, std::vector< std::vector<unsigned int> >& DT,
                std::vector<unsigned int>& rgCe, std::vector<unsigned int>& rgCb,
                unsigned int i /*level*/, unsigned int k /*cutoff*/) const
{
  unsigned int Ce, CeT, CeO; //end cutoff
  unsigned int Cb, CbT, CbO; //begin cutoff
  unsigned int dT, diT, ddT, drT, dtT; // distance, insertion, deletion, replace, transpose
  unsigned int s;
  
  assert(W != NULL);

  assert(_S_edge(W) != _S_b_trie_edge); // is key valid on next line
  sub_key_type wT = _S_key(W);
  sub_key_type wpT;
  if (i > 1) {
    assert(_S_trie_parent(W) != 0);
    assert(_S_edge(_S_trie_parent(W)) != _S_b_trie_edge); // is key valid on next line
    wpT = _S_key(_S_trie_parent(W));
  }
  assert(DT.size() > i);
  DT[i].clear();
  DT[i].resize(_KeySize()(P) + 1, TRIE_INFINITY);

  assert((i>0)?rgCe.size() > i-1:true);
  assert((i>0)?rgCb.size() > i-1:true);
  Ce  = (i>0) ? rgCe[i-1] : k;
  Cb  = (i>0) ? rgCb[i-1] : 0;
  CeO = (i>1) ? rgCe[i-2] : k;
  CbO = (i>1) ? rgCb[i-2] : 0;
  CeT = TRIE_CUTOFF;
  CbT = Ce+1;
  unsigned int mac = std::min<unsigned int>(Ce+1, _KeySize()(P));

  for(unsigned int j=Cb; j <= mac; ++j) {
    if(Ce != TRIE_CUTOFF) {  // need special case for when cutoff is 0
      assert(DT.size() > i);
      assert(DT[i].size() > j);

      if(i == 0 || j == 0) {
        DT[i][j] = i+j;
      }
      else {
        assert((unsigned int)_KeySize()(P) > j-1);
        sub_key_type pT = P[j-1];
        s = (!_M_key_compare(pT, wT) && !_M_key_compare(wT, pT)) ? 0 : 1;
        assert(DT[i].size() > j-1);
        assert(DT[i-1].size() > j);
        ddT = (j == Cb)   ? TRIE_INFINITY : (DT[i][j-1]) + 1;    // delete distance
        diT = (j > CeO+1) ? TRIE_INFINITY : (DT[i-1][j]) + 1;    // insert distance
        drT = (j <= CbO)  ? TRIE_INFINITY : (DT[i-1][j-1]) + s;  // replace distance
        dT = std::min<unsigned int>(drT, std::min<unsigned int>(diT, ddT));
      
        assert((i>1 && j>1)? (unsigned int)_KeySize()(P) > j-1-1:true);
        if(i < 2 || j < 2 ||
             _M_key_compare(P[j-1-1], wT) || _M_key_compare(wT, P[j-1-1]) ||
             _M_key_compare(pT, wpT) || _M_key_compare(wpT, pT)){
          DT[i][j] = dT;
        }
        else {
            assert(DT[i-2].size() > j-2);
            dtT = (DT[i-2][j-2])+1;                             // transpose distance
            DT[i][j] =  std::min<unsigned int>(dT, dtT);
        }
      }

      if(DT[i][j] <= k) {
        if(j < CbT) {
          CbT = j;
        }
        CeT = j;
      }
      else if((_b_trie_node_base___::_S_edge(W) == _S_b_trie_edge) && (j > Ce)) {
        assert(W != NULL);
        assert(W->_debugfIsEdge == true);
        break;
      }
    }
  }
  assert(rgCe.size() > i);
  assert(rgCb.size() > i);
  rgCe[i] = CeT;
  rgCb[i] = CbT;

  assert(DT.size() > i);
  assert(DT[i].size() > (unsigned int)_KeySize()(P));
  return DT[i][_KeySize()(P)];
}

#ifdef _DEBUG 
// brute force algorithm
TRIE_TEMPLATE 
inline unsigned int
B_TRIE::__edit_distance(const key_type& P, const std::vector<sub_key_type>& W, std::vector< std::vector<unsigned int> > &DT) const
{
  DT.resize(W.size() + 1, std::vector<unsigned int>());
  for(unsigned int i=0; i <= W.size(); i++) {
    assert(DT.size() > i);
    DT[i].resize(_KeySize()(P) + 1);
    for(unsigned int j=0; j <= (unsigned int) _KeySize()(P); j++) {
      unsigned int s = 1,
                   r = TRIE_INFINITY;
      if(i > 0 && j > 0) {
        assert((unsigned int)_KeySize()(P) > j-1);
        assert(W.size() > i-1);
        if(P[j-1] == W[i-1]) {
          s = 0;
        }
        if(i > 1 && j > 1) {
          assert((unsigned int)_KeySize()(P) > j-1);
          assert(W.size() > i-1);
          if(P[j-1-1] == W[i-1] && P[j-1] == W[i-1-1]) {
            r = 1;
          }
          assert(DT.size() > i);
          assert(DT[i].size() > j);
          assert(DT[i-1].size() > j);
          assert(DT[i-2].size() > j-2);
          DT[i][j] = std::min<unsigned int>((DT[i][j-1]) + 1, 
                      std::min<unsigned int>((DT[i-1][j]) + 1,
                       std::min<unsigned int>((DT[i-1][j-1]) + s,
                            (DT[i-2][j-2]) + r)));
        }
        else {
          assert(DT.size() > i);
          assert(DT[i].size() > j);
          assert(DT[i-1].size() > j);
          DT[i][j] = std::min<unsigned int>((DT[i][j-1]) + 1, 
                      std::min<unsigned int>((DT[i-1][j]) + 1,
                           (DT[i-1][j-1]) + s));
        }
      }
      else {
        assert(DT.size() > i);
        assert(DT[i].size() > j);
        DT[i][j] = i+j;
      }
    }
  }
  assert(DT.size() > W.size());
  assert(DT[W.size()].size() > (unsigned int)_KeySize()(P));
  return DT[W.size()][_KeySize()(P)];
}
#endif // #ifdef _DEBUG


} // end namespace

#endif // #ifndef __INTERNAL_TRIE_H


