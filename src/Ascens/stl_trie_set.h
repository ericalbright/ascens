/*
 * Copyright (c) 1997-2002
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
 * The trie_set implementation stems from a set implementation
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
 */

#pragma once

#ifndef __INTERNAL_TRIE_SET_H
#define __INTERNAL_TRIE_SET_H

#ifdef _DEBUG
#include <algorithm> // for std::find used in assert
#endif

//#include <concept_checks.h>

namespace trie {

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1174
#pragma set woff 1375
#endif


#define TRIE_SET trie_set<_Key, _SubKeyType, _KeyIterator, _KeySizeType, _KeyBegin, _KeyEnd, _KeySize, _Compare, _Alloc>
#define TRIE_SET_TEMPLATE template <class _Key, class _SubKeyType, class _KeyIterator, class _KeySizeType, class _KeyBegin, class _KeyEnd, class _KeySize, class _Compare, class _Alloc>

// Forward declarations of operators == and <, needed for friend declarations.
template <class _Key, 
          class _SubKeyType = _Key::value_type, 
          class _KeyIterator = _Key::const_iterator,
          class _KeySizeType = _Key::size_type,
          class _KeyBegin = __container_begin<typename _Key, typename _KeyIterator>,
          class _KeyEnd = __container_end<typename _Key, typename _KeyIterator>,
          class _KeySize = __container_size<typename _Key, typename _KeySizeType>,
          class _Compare = std::less<typename _SubKeyType>, 
          class _Alloc = std::allocator<_Key> >
class trie_set;

TRIE_SET_TEMPLATE 
inline bool operator==(const TRIE_SET& __x,
                       const TRIE_SET& __y);

TRIE_SET_TEMPLATE 
inline bool operator<(const TRIE_SET& __x,
                       const TRIE_SET& __y);

TRIE_SET_TEMPLATE 
class trie_set {
  // requirements:
//  __STL_CLASS_REQUIRES(_Key, _Assignable);
//  __STL_CLASS_BINARY_FUNCTION_CHECK(_Compare, bool, _Key, _Key);

public:
  // typedefs:

  typedef _Key          key_type;
  typedef _Key          value_type;
  typedef _Compare      key_compare;
  typedef _Compare      value_compare;

  typedef _KeyIterator  key_iterator;
  typedef _KeySizeType  key_size_type;
  typedef _KeyBegin     key_begin;
  typedef _KeyEnd       key_end;
  typedef _KeySize      key_size;
  
  typedef _SubKeyType    sub_key_type;

private:
  typedef _b_trie<key_type, value_type, sub_key_type, _Identity<value_type>,  
    key_iterator, key_size_type, key_begin, key_end, key_size,
    key_compare, _Alloc > rep_type;

  rep_type _M_t;  // binary trie representing set
public:
  typedef typename rep_type::const_pointer                pointer;
  typedef typename rep_type::const_pointer                const_pointer;
  typedef typename rep_type::const_reference              reference;
  typedef typename rep_type::const_reference              const_reference;
  typedef typename rep_type::const_iterator               iterator;
  typedef typename rep_type::const_iterator               const_iterator;
  typedef typename rep_type::const_reverse_iterator       reverse_iterator;
  typedef typename rep_type::const_reverse_iterator       const_reverse_iterator;
  typedef typename rep_type::size_type                    size_type;
  typedef typename rep_type::difference_type              difference_type;
  typedef typename rep_type::allocator_type	              allocator_type;

  // allocation/deallocation

  trie_set() 
    : _M_t(key_compare(), allocator_type()) 
  {}

  explicit trie_set(const key_compare& __comp,
    const allocator_type& __a = allocator_type())
    : _M_t(__comp, __a) 
  {}

#if 1 //def __STL_MEMBER_TEMPLATES
  template <class _InputIterator>
  trie_set(_InputIterator __first, _InputIterator __last)
    : _M_t(_Compare(), allocator_type()) 
  { _M_t.insert_unique(__first, __last); }

  template <class _InputIterator>
  trie_set(_InputIterator __first, _InputIterator __last,
    const _Compare& __comp,
    const allocator_type& __a = allocator_type())
    : _M_t(__comp, __a)
  { _M_t.insert_unique(__first, __last); }

#else
  trie_set(const value_type* __first, const value_type* __last) 
    : _M_t(_Compare(), allocator_type()) 
  { _M_t.insert_unique(__first, __last); }

  trie_set(const value_type* __first, const value_type* __last,
    const _Compare& __comp,
    const allocator_type& __a = allocator_type())
    : _M_t(__comp, __a)
  { _M_t.insert_unique(__first, __last); }

  trie_set(const_iterator __first, const_iterator __last)
    : _M_t(_Compare(), allocator_type())
  { _M_t.insert_unique(__first, __last); }

  trie_set(const_iterator __first, const_iterator __last,
    const _Compare& __comp,
    const allocator_type& __a = allocator_type())
    : _M_t(__comp, __a)
  { _M_t.insert_unique(__first, __last); }
#endif // __STL_MEMBER_TEMPLATES

  trie_set(const TRIE_SET& __x) : _M_t(__x._M_t) {}
  TRIE_SET& operator=(const TRIE_SET& __x) { 
    _M_t = __x._M_t; 
    return *this;
  }

  // accessors:

  key_compare key_comp() const { return _M_t.key_comp(); }
  value_compare value_comp() const { return _M_t.key_comp(); }
  allocator_type get_allocator() const { return _M_t.get_allocator(); }
  
  iterator begin() const { return _M_t.begin(); }
  iterator end() const { return _M_t.end(); }
  reverse_iterator rbegin() const { return _M_t.rbegin(); } 
  reverse_iterator rend() const { return _M_t.rend(); }
  bool empty() const { return _M_t.empty(); }
  size_type size() const { return _M_t.size(); }
  size_type max_size() const { return _M_t.max_size(); }
  void swap(TRIE_SET& __x) { _M_t.swap(__x._M_t); }

  // insert/erase
  std::pair<iterator, bool> insert(const value_type& __x) { 
    std::pair<typename rep_type::iterator, bool> __p = _M_t.insert_unique(__x); 
    return std::pair<iterator, bool>(__p.first, __p.second);
  }
  iterator insert(iterator __position, const value_type& __x) {
    _M_t.insert_unique(__x);
    return __position;
  }
#if 1 //def __STL_MEMBER_TEMPLATES
  template <class _InputIterator>
  void insert(_InputIterator first, _InputIterator last) {
    _M_t.insert_unique(first, last);
  }
#else
  void insert(const_iterator first, const_iterator last) {
    _M_t.insert_unique(first, last);
  }
  void insert(const value_type* first, const value_type* last) {
    _M_t.insert_unique(first, last);
  }
#endif // __STL_MEMBER_TEMPLATES

  void erase(iterator position) { 
    _M_t.erase((typename rep_type::iterator&)position); 
  }
  size_type erase(const key_type& __x) { 
    return _M_t.erase(__x); 
  }
  void erase(iterator first, iterator last) { 
    _M_t.erase((typename rep_type::iterator&)first, 
            (typename rep_type::iterator&)last); 
  }
  void clear() { _M_t.clear(); }

  // set operations:

  iterator find(const key_type& __x) const { 
    assert(_M_t.find(__x) == std::find(begin(), end(), __x));
    return _M_t.find(__x); 
  }
  iterator find_if_prefix(const key_type& __x) const { 
    return _M_t.find_if_prefix(__x); 
  }
  size_type count(const key_type& __x) const {
    assert(_M_t.find(__x) == std::find(begin(), end(), __x));
    return (_M_t.find(__x) == _M_t.end()) ? 0 : 1; 
  }
  iterator lower_bound(const key_type& __x) const {
    assert(_M_t.lower_bound(__x) == std::lower_bound(begin(), end(), __x));
    return _M_t.lower_bound(__x);
  }
  iterator upper_bound(const key_type& __x) const {
    assert(_M_t.upper_bound(__x) == std::upper_bound(begin(), end(), __x));
    return _M_t.upper_bound(__x); 
  }
  std::pair<iterator,iterator> equal_range(const key_type& __x) const {
    assert(_M_t.equal_range(__x) == std::equal_range(begin(), end(), __x));
    return _M_t.equal_range(__x);
  }


  iterator find(const key_iterator& __begin, const key_iterator& __end) const { 
    return _M_t.find(__begin, __end); 
  }
  iterator find_if_prefix(const key_iterator& __begin, const key_iterator& __end) const { 
    return _M_t.find_if_prefix(__begin, __end); 
  }
  size_type count(const key_iterator& __begin, const key_iterator& __end) const {
    return (_M_t.find(__begin, __end) == _M_t.end()) ? 0 : 1; 
  }
  iterator lower_bound(const key_iterator& __begin, const key_iterator& __end) const {
    return _M_t.lower_bound(__begin, __end);
  }
  iterator upper_bound(const key_iterator& __begin, const key_iterator& __end) const {
    return _M_t.upper_bound(__begin, __end); 
  }
  std::pair<iterator,iterator> equal_range(const key_iterator& __begin, const key_iterator& __end) const {
    return _M_t.equal_range(__begin, __end);
  }

#if 1/*def __STL_TEMPLATE_FRIENDS*/
  TRIE_SET_TEMPLATE
  friend bool operator== <> (const TRIE_SET&, const TRIE_SET&);
  TRIE_SET_TEMPLATE
  friend bool operator< <> (const TRIE_SET&, const TRIE_SET&);
#else /* __STL_TEMPLATE_FRIENDS */
  friend bool std:: operator== <> (const trie_set&, const trie_set&);
  friend bool std:: operator< <> (const trie_set&, const trie_set&);
#endif /* __STL_TEMPLATE_FRIENDS */

  // trie operations:
  std::vector<iterator> approximate_find(const key_type& __x, unsigned int k=1) const 
  { return _M_t.approximate_find(__x, k); }

  std::vector<iterator> best_find(const key_type& __x, unsigned int k=TRIE_INFINITY) const 
  { return _M_t.best_find(__x, k); }
 
};

TRIE_SET_TEMPLATE
inline bool operator==(const TRIE_SET& __x, const TRIE_SET& __y) {
  return __x._M_t == __y._M_t;
}

TRIE_SET_TEMPLATE
inline bool operator<(const TRIE_SET& __x, const TRIE_SET& __y) {
  return __x._M_t < __y._M_t;
}

#if 1 //def __STL_FUNCTION_TMPL_PARTIAL_ORDER
TRIE_SET_TEMPLATE
inline bool operator!=(const TRIE_SET& __x, 
                       const TRIE_SET& __y) {
  return !(__x == __y);
}


TRIE_SET_TEMPLATE
inline bool operator>(const TRIE_SET& __x, 
                      const TRIE_SET& __y) {
  return __y < __x;
}


TRIE_SET_TEMPLATE
inline bool operator<=(const TRIE_SET& __x, 
                       const TRIE_SET& __y) {
  return !(__y < __x);
}


TRIE_SET_TEMPLATE
inline bool operator>=(const TRIE_SET& __x, 
                       const TRIE_SET& __y) {
  return !(__x < __y);
}

TRIE_SET_TEMPLATE
inline void swap(TRIE_SET& __x, TRIE_SET& __y) {
  __x.swap(__y);
}

#endif //__STL_FUNCTION_TMPL_PARTIAL_ORDER

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1174
#pragma reset woff 1375
#endif

} // trie namespace

#endif // #ifndef __INTERNAL_TRIE_SET_H

