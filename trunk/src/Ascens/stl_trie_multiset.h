/*
 * Copyright (c) 1998-2002 
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
 * The trie_multiset implementation stems from a multiset implementation
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

/* NOTE: This is an internal header file, included by other STL headers.
 *   You should not attempt to use it directly.
 */

#pragma once

#ifndef __TRIE_INTERNAL_MULTISET_H
#define __TRIE_INTERNAL_MULTISET_H

//#include <concept_checks.h>

namespace trie {

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1174
#pragma set woff 1375
#endif


#define TRIE_MULTISET trie_multiset<_Key, _SubKeyType, _KeyIterator, _KeySizeType, _KeyBegin, _KeyEnd, _KeySize, _Compare, _Alloc>
#define TRIE_MULTISET_TEMPLATE template <class _Key, class _SubKeyType, class _KeyIterator, class _KeySizeType, class _KeyBegin, class _KeyEnd, class _KeySize, class _Compare, class _Alloc>

// Forward declarations of operators == and <, needed for friend declarations.
template <class _Key, 
          class _SubKeyType = _Key::value_type, 
          class _KeyIterator = _Key::const_iterator,
          class _KeySizeType = _Key::size_type,
          class _KeyBegin = __container_begin<typename _Key, typename _KeyIterator>,
          class _KeyEnd = __container_end<typename _Key, typename _KeyIterator>,
          class _KeySize = __container_size<typename _Key, typename _KeySizeType>,
          class _Compare =std::less<typename _SubKeyType>, 
          class _Alloc = std::allocator<_Key> >
class trie_multiset;

TRIE_MULTISET_TEMPLATE 
inline bool operator==(const TRIE_MULTISET& __x,
                       const TRIE_MULTISET& __y);

TRIE_MULTISET_TEMPLATE 
inline bool operator<(const TRIE_MULTISET& __x,
                       const TRIE_MULTISET& __y);

TRIE_MULTISET_TEMPLATE 
class trie_multiset {
  // requirements:
//  __STL_CLASS_REQUIRES(_Key, _Assignable);
//  __STL_CLASS_BINARY_FUNCTION_CHECK(_Compare, bool, _Key, _Key);
public:
  // typedefs:

  typedef _Key           key_type;
  typedef _Key           value_type;
  typedef _Compare       key_compare;
  typedef _Compare       value_compare;

  typedef _KeyIterator  key_iterator;
  typedef _KeySizeType  key_size_type;
  typedef _KeyBegin     key_begin;
  typedef _KeyEnd       key_end;
  typedef _KeySize      key_size;
  
  typedef _SubKeyType    sub_key_type;

private:
  typedef _b_trie<key_type, value_type, sub_key_type, _Identity<value_type>, 
        key_iterator, key_size_type, key_begin, key_end, key_size,
        key_compare, 
        _Alloc> rep_type;
  
  rep_type t;  // binary trie representing multiset
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

  trie_multiset()
    : t(key_compare(), allocator_type()) 
  {}

  explicit trie_multiset(const key_begin& __kb, 
    const key_end& __ke,
    const key_size& __ks,
    const key_compare& __comp,
    const allocator_type& __a = allocator_type())
    : t(__kb, __ke, __ks, __comp, __a) 
  {}

#if 1 //def __STL_MEMBER_TEMPLATES
  template <class _InputIterator>
  trie_multiset(_InputIterator first, _InputIterator last)
    : t(_Compare(), allocator_type()) 
  { t.insert_equal(first, last); }
  template <class _InputIterator>
  trie_multiset(_InputIterator first, _InputIterator last,
    const _Compare& __comp,
    const allocator_type& __a = allocator_type())
    : t(__comp, __a)
  { t.insert_equal(first, last); }
#else
  trie_multiset(const value_type* first, const value_type* last)
    : t(_Compare(), allocator_type()) 
  { t.insert_equal(first, last); }
  trie_multiset(const value_type* first, const value_type* last,
    const _Compare& __comp,
    const allocator_type& __a = allocator_type())
    : t(__comp, __a)
  { t.insert_equal(first, last); }

  trie_multiset(const_iterator first, const_iterator last)
    : t(_Compare(), allocator_type()) 
  { t.insert_equal(first, last); }
  trie_multiset(const_iterator first, const_iterator last, 
    const _Compare& __comp,
    const allocator_type& __a = allocator_type())
    : t(__comp, __a)
  { t.insert_equal(first, last); }
#endif // __STL_MEMBER_TEMPLATES

  trie_multiset(const TRIE_MULTISET& x) : t(x.t) {}
  TRIE_MULTISET& operator=(const TRIE_MULTISET& x) {
    t = x.t; 
    return *this;
  }

  // accessors:

  key_compare key_comp() const { return t.key_comp(); }
  value_compare value_comp() const { return t.key_comp(); }
  allocator_type get_allocator() const { return t.get_allocator(); }

  iterator begin() const { return t.begin(); }
  iterator end() const { return t.end(); }
  reverse_iterator rbegin() const { return t.rbegin(); } 
  reverse_iterator rend() const { return t.rend(); }
  bool empty() const { return t.empty(); }
  size_type size() const { return t.size(); }
  size_type max_size() const { return t.max_size(); }
  void swap(TRIE_MULTISET& x) { t.swap(x.t); }

  // insert/erase
  iterator insert(const value_type& x) { 
    return t.insert_equal(x);
  }
  iterator insert(iterator position, const value_type& x) {
    t.insert_equal(x);
    return position;
  }

#if 1 //def __STL_MEMBER_TEMPLATES  
  template <class _InputIterator>
  void insert(_InputIterator first, _InputIterator last) {
    t.insert_equal(first, last);
  }
#else
  void insert(const value_type* first, const value_type* last) {
    t.insert_equal(first, last);
  }
  void insert(const_iterator first, const_iterator last) {
    t.insert_equal(first, last);
  }
#endif // __STL_MEMBER_TEMPLATES

  void erase(iterator position) { 
    t.erase((typename rep_type::iterator&)position); 
  }
  size_type erase(const key_type& x) { 
    return t.erase(x); 
  }
  void erase(iterator first, iterator last) { 
    t.erase((typename rep_type::iterator&)first, 
            (typename rep_type::iterator&)last); 
  }
  void clear() { t.clear(); }

  // multiset operations:

  iterator find(const key_type& __x) const { 
    assert(t.find(__x) == std::find(begin(), end(), __x));
    return t.find(__x); 
  }
  iterator find_if_prefix(const key_type& __x) const { 
    return t.find_if_prefix(__x); 
  }
  size_type count(const key_type& x) const { 
#if _DEBUG
    size_type __s = std::count(begin(), end(), x);
    assert(__s == t.count(x));
#endif
    return t.count(x); }
  iterator lower_bound(const key_type& x) const {
    assert(t.lower_bound(x) == std::lower_bound(begin(), end(), x));
    return t.lower_bound(x);
  }
  iterator upper_bound(const key_type& x) const {
    assert(t.upper_bound(x) == std::upper_bound(begin(), end(), x));
    return t.upper_bound(x); 
  }
  std::pair<iterator,iterator> equal_range(const key_type& x) const {
    assert(t.equal_range(x) == std::equal_range(begin(), end(), x));
    return t.equal_range(x);
  }


  iterator find(const key_iterator& __begin, const key_iterator& __end) const { 
    return t.find(__begin, __end); 
  }
  iterator find_if_prefix(const key_iterator& __begin, const key_iterator& __end) const { 
    return t.find_if_prefix(__begin, __end); 
  }
  size_type count(const key_iterator& __begin, const key_iterator& __end) const { 
    return t.count(__begin, __end); 
  }
  iterator lower_bound(const key_iterator& __begin, const key_iterator& __end) const {
    return t.lower_bound(__begin, __end);
  }
  iterator upper_bound(const key_iterator& __begin, const key_iterator& __end) const {
    return t.upper_bound(__begin, __end); 
  }
  std::pair<iterator,iterator> equal_range(const key_iterator& __begin, const key_iterator& __end) const {
    return t.equal_range(__begin, __end);
  }

#if 1 /*def __STL_TEMPLATE_FRIENDS*/
  TRIE_MULTISET_TEMPLATE
  friend bool operator== <> (const TRIE_MULTISET&, const TRIE_MULTISET&);
  TRIE_MULTISET_TEMPLATE
  friend bool operator< <> (const TRIE_MULTISET&, const TRIE_MULTISET&);
#else /* __STL_TEMPLATE_FRIENDS */
  friend bool std:: operator== <> (const trie_multiset&, const trie_multiset&);
  friend bool std:: operator< <> (const trie_multiset&, const trie_multiset&);
#endif /* __STL_TEMPLATE_FRIENDS */

  // trie operations:
  std::vector<iterator> approximate_find(const key_type& x, unsigned int __k=1) const 
  { return t.approximate_find(x, __k); }

  std::vector<iterator> best_find(const key_type& x, unsigned int __k=TRIE_INFINITY) const 
  { return t.best_find(x, __k); }
  
};

TRIE_MULTISET_TEMPLATE
inline bool operator==(const TRIE_MULTISET& x, 
                       const TRIE_MULTISET& y) {
  return x.t == y.t;
}

TRIE_MULTISET_TEMPLATE
inline bool operator<(const TRIE_MULTISET& x, 
                      const TRIE_MULTISET& y) {
  return x.t < y.t;
}

#if 1 //def __STL_FUNCTION_TMPL_PARTIAL_ORDER


TRIE_MULTISET_TEMPLATE
inline bool operator!=(const TRIE_MULTISET& __x, 
                       const TRIE_MULTISET& __y) {
  return !(__x == __y);
}


TRIE_MULTISET_TEMPLATE
inline bool operator>(const TRIE_MULTISET& __x, 
                      const TRIE_MULTISET& __y) {
  return __y < __x;
}


TRIE_MULTISET_TEMPLATE
inline bool operator<=(const TRIE_MULTISET& __x, 
                       const TRIE_MULTISET& __y) {
  return !(__y < __x);
}


TRIE_MULTISET_TEMPLATE
inline bool operator>=(const TRIE_MULTISET& __x, 
                       const TRIE_MULTISET& __y) {
  return !(__x < __y);
}

TRIE_MULTISET_TEMPLATE
inline void swap(TRIE_MULTISET& x, 
                 TRIE_MULTISET& y) {
  x.swap(y);
}

#endif // __STL_FUNCTION_TMPL_PARTIAL_ORDER

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1174
#pragma reset woff 1375
#endif

}//trie namespace
#endif // #ifndef __TRIE_INTERNAL_MULTISET_H

