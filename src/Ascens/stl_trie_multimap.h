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
 * The trie_multimap implementation stems from a multimap implementation
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

#pragma once

#ifndef __TRIE_INTERNAL_MULTIMAP_H
#define __TRIE_INTERNAL_MULTIMAP_H

//#include <concept_checks.h>

namespace trie {

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1174
#pragma set woff 1375
#endif

#if _DEBUG
#if !defined(__FIRST_OF_PAIR_LESS)
#define __FIRST_OF_PAIR_LESS
template <class _T, class _Y>
struct first_of_pair_less_func : public _STD binary_function<_STD pair<_T, _Y>, _STD pair<_T, _Y>, _T::size_type> {
  _T::size_type operator()(const _STD pair<_T, _Y>& x, const _STD pair<_T, _Y>& y) const {
    return x.first < y.first;
  }
};
#endif
#endif

#define TRIE_MULTIMAP trie_multimap<_Key, _Type, _SubKeyType, _KeyIterator, _KeySizeType, _KeyBegin, _KeyEnd, _KeySize, _Compare, _Alloc>
#define TRIE_MULTIMAP_TEMPLATE template <class _Key, class _Type, class _SubKeyType, class _KeyIterator, class _KeySizeType, class _KeyBegin, class _KeyEnd, class _KeySize, class _Compare, class _Alloc>

// Forward declarations of operators == and <, needed for friend declarations.
template <class _Key, 
          class _Type, 
          class _SubKeyType = _Key::value_type, 
          class _KeyIterator = _Key::const_iterator,
          class _KeySizeType = _Key::size_type,
          class _KeyBegin = __container_begin<typename _Key, typename _KeyIterator>,
          class _KeyEnd = __container_end<typename _Key, typename _KeyIterator>,
          class _KeySize = __container_size<typename _Key, typename _KeySizeType>,
          class _Compare = std::less<typename _SubKeyType>, 
          class _Alloc = std::allocator<_Type> >
class trie_multimap;


TRIE_MULTIMAP_TEMPLATE 
inline bool operator==(const TRIE_MULTIMAP& __x,
                       const TRIE_MULTIMAP& __y);

TRIE_MULTIMAP_TEMPLATE 
inline bool operator<(const TRIE_MULTIMAP& __x,
                       const TRIE_MULTIMAP& __y);

TRIE_MULTIMAP_TEMPLATE 
class trie_multimap {
  // requirements:

//  __STL_CLASS_REQUIRES(_Type, _Assignable);
//  __STL_CLASS_BINARY_FUNCTION_CHECK(_Compare, bool, _Key, _Key);

public:

// typedefs:

  typedef _Key                 key_type;
  typedef _Type                   data_type;
  typedef _Type                   mapped_type;
  typedef std::pair<const _Key, _Type>  value_type;
  typedef _Compare             key_compare;

  typedef _KeyIterator  key_iterator;
  typedef _KeySizeType  key_size_type;
  typedef _KeyBegin     key_begin;
  typedef _KeyEnd       key_end;
  typedef _KeySize      key_size;
  
  typedef _SubKeyType          sub_key_type;

  class value_compare 
    : public std::binary_function<value_type, value_type, bool> {
    friend class TRIE_MULTIMAP;
    protected:
        _Compare comp;
        value_compare(_Compare __c) : comp(__c) {}
    public:
        bool operator()(const value_type& __x, const value_type& __y) const {
            return comp(__x.first, __y.first);
        }
  };

private:
  typedef _b_trie<key_type, value_type, sub_key_type, _Select1st<value_type>, 
        key_iterator, key_size_type, key_begin, key_end, key_size,
        key_compare, _Alloc> rep_type;

  rep_type _M_t;  // binary trie representing multimap
public:
  typedef typename rep_type::pointer                  pointer;
  typedef typename rep_type::const_pointer            const_pointer;
  typedef typename rep_type::reference                reference;
  typedef typename rep_type::const_reference          const_reference;
  typedef typename rep_type::iterator                 iterator;
  typedef typename rep_type::const_iterator           const_iterator; 
  typedef typename rep_type::reverse_iterator         reverse_iterator;
  typedef typename rep_type::const_reverse_iterator   const_reverse_iterator;
  typedef typename rep_type::size_type                size_type;
  typedef typename rep_type::difference_type          difference_type;
  typedef typename rep_type::allocator_type		        allocator_type;

  // allocation/deallocation

  trie_multimap() 
    : _M_t(key_compare(), allocator_type()) 
  {}

  explicit trie_multimap(
    const key_compare& __comp,
    const allocator_type& __a = allocator_type()) 
    : _M_t(__comp, __a)
  {}

#if 1 /*def __STL_MEMBER_TEMPLATES  */
  template <class _InputIterator>
  trie_multimap(_InputIterator __first, _InputIterator __last)
    : _M_t(_Compare(), allocator_type())
  { _M_t.insert_equal(__first, __last); }

  template <class _InputIterator>
  trie_multimap(_InputIterator __first, _InputIterator __last, 
    const _Compare& __comp,
    const allocator_type& __a = allocator_type()) 
    : _M_t(__comp, __a) 
  { _M_t.insert_equal(__first, __last); }

#else
  trie_multimap(const value_type* __first, const value_type* __last)
    : _M_t(_Compare(), allocator_type()) 
  { _M_t.insert_equal(__first, __last); }

  trie_multimap(const value_type* __first, const value_type* __last,
    const _Compare& __comp,
    const allocator_type& __a = allocator_type())
    : _M_t(__comp, __a) 
    { _M_t.insert_equal(__first, __last); }

  trie_multimap(const_iterator __first, const_iterator __last)
    : _M_t(_Compare(), allocator_type())
  { _M_t.insert_equal(__first, __last); }

  trie_multimap(const_iterator __first, const_iterator __last,
    const _Compare& __comp,
    const allocator_type& __a = allocator_type())
    : _M_t(__comp, __a) 
  { _M_t.insert_equal(__first, __last); }
#endif // __STL_MEMBER_TEMPLATES

  trie_multimap(const TRIE_MULTIMAP& __x) : _M_t(__x._M_t) { }
  TRIE_MULTIMAP& operator=(const TRIE_MULTIMAP& __x) {
    _M_t = __x._M_t;
    return *this; 
  }

  // accessors:

  key_compare key_comp() const { return _M_t.key_comp(); }
  value_compare value_comp() const { return value_compare(_M_t.key_comp()); }
  allocator_type get_allocator() const { return _M_t.get_allocator(); }
  
  iterator begin() { return _M_t.begin(); }
  const_iterator begin() const { return _M_t.begin(); }
  iterator end() { return _M_t.end(); }
  const_iterator end() const { return _M_t.end(); }
  reverse_iterator rbegin() { return _M_t.rbegin(); }
  const_reverse_iterator rbegin() const { return _M_t.rbegin(); }
  reverse_iterator rend() { return _M_t.rend(); }
  const_reverse_iterator rend() const { return _M_t.rend(); }
  bool empty() const { return _M_t.empty(); }
  size_type size() const { return _M_t.size(); }
  size_type max_size() const { return _M_t.max_size(); }
  void swap(TRIE_MULTIMAP& __x) { _M_t.swap(__x._M_t); }

  // insert/erase

  iterator insert(const value_type& __x) { return _M_t.insert_equal(__x); }
  iterator insert(iterator __position, const value_type& __x) {
    return _M_t.insert_equal(__position, __x);
  }
#if 1 /*def __STL_MEMBER_TEMPLATES  */
  template <class _InputIterator>
  void insert(_InputIterator __first, _InputIterator __last) {
    _M_t.insert_equal(__first, __last);
  }
#else
  void insert(const value_type* __first, const value_type* __last) {
    _M_t.insert_equal(__first, __last);
  }
  void insert(const_iterator __first, const_iterator __last) {
    _M_t.insert_equal(__first, __last);
  }
#endif // __STL_MEMBER_TEMPLATES
  void erase(iterator __position) { _M_t.erase(__position); }
  size_type erase(const key_type& __x) { return _M_t.erase(__x); }
  void erase(iterator __first, iterator __last) { _M_t.erase(__first, __last); }
  void clear() { _M_t.clear(); }

  // multimap operations:

  iterator find(const key_type& __x) { return _M_t.find(__x); }
  const_iterator find(const key_type& __x) const { return _M_t.find(__x); }
  iterator find_if_prefix(const key_type& __x) { return _M_t.find_if_prefix(__x); }
  const_iterator find_if_prefix(const key_type& __x) const { return _M_t.find_if_prefix(__x); }
  size_type count(const key_type& __x) const { return _M_t.count(__x); }
  iterator lower_bound(const key_type& __x) {
    assert(_M_t.lower_bound(__x) == std::lower_bound(begin(), end(), 
                                value_type(__x, data_type()),
                                first_of_pair_less_func<const key_type, data_type>()));
    return _M_t.lower_bound(__x); 
  }
  const_iterator lower_bound(const key_type& __x) const {
    assert(_M_t.lower_bound(__x) == std::lower_bound(begin(), end(), 
                                value_type(__x, data_type()),
                                first_of_pair_less_func<const key_type, data_type>()));
    return _M_t.lower_bound(__x); 
  }
  iterator upper_bound(const key_type& __x) {
    assert(_M_t.upper_bound(__x) == std::upper_bound(begin(), end(), 
                                      value_type(__x, data_type()),
                                      first_of_pair_less_func<const key_type, data_type>()));
    return _M_t.upper_bound(__x); 
  }
  const_iterator upper_bound(const key_type& __x) const {
    assert(_M_t.upper_bound(__x) == std::upper_bound(begin(), end(), 
                                      value_type(__x, data_type()),
                                      first_of_pair_less_func<const key_type, data_type>()));
    return _M_t.upper_bound(__x); 
  }
  std::pair<iterator,iterator> equal_range(const key_type& __x) {
    assert(_M_t.equal_range(__x) == std::equal_range(begin(), end(), 
                                      value_type(__x, data_type()),
                                      first_of_pair_less_func<const key_type, data_type>()));
    return _M_t.equal_range(__x);
  }
  std::pair<const_iterator,const_iterator> equal_range(const key_type& __x) const {
    assert(_M_t.equal_range(__x) == std::equal_range(begin(), end(), 
                                      value_type(__x, data_type()),
                                      first_of_pair_less_func<const key_type, data_type>()));
    return _M_t.equal_range(__x);
  }



  iterator find(const key_iterator& __begin, const key_iterator& __end) { 
    return _M_t.find(__begin, __end); 
  }
  const_iterator find(const key_iterator& __begin, const key_iterator& __end) const { 
    return _M_t.find(__begin, __end); 
  }
  iterator find_if_prefix(const key_iterator& __begin, const key_iterator& __end) { 
    return _M_t.find_if_prefix(__begin, __end); 
  }
  const_iterator find_if_prefix(const key_iterator& __begin, const key_iterator& __end) const { 
    return _M_t.find_if_prefix(__begin, __end); 
  }
  size_type count(const key_iterator& __begin, const key_iterator& __end) const { 
    return _M_t.count(__begin, __end); 
  }
  iterator lower_bound(const key_iterator& __begin, const key_iterator& __end) {
    return _M_t.lower_bound(__begin, __end); 
  }
  const_iterator lower_bound(const key_iterator& __begin, const key_iterator& __end) const {
    return _M_t.lower_bound(__begin, __end); 
  }
  iterator upper_bound(const key_iterator& __begin, const key_iterator& __end) {
    return _M_t.upper_bound(__begin, __end); 
  }
  const_iterator upper_bound(const key_iterator& __begin, const key_iterator& __end) const {
    return _M_t.upper_bound(__begin, __end); 
  }
  std::pair<iterator,iterator> equal_range(const key_iterator& __begin, const key_iterator& __end) {
    return _M_t.equal_range(__begin, __end);
  }
  std::pair<const_iterator,const_iterator> equal_range(const key_iterator& __begin, const key_iterator& __end) const {
    return _M_t.equal_range(__begin, __end);
  }
#if 1 /*def __STL_TEMPLATE_FRIENDS*/
  TRIE_MULTIMAP_TEMPLATE
  friend bool operator== <> (const TRIE_MULTIMAP&, const TRIE_MULTIMAP&);
  TRIE_MULTIMAP_TEMPLATE
  friend bool operator< <> (const TRIE_MULTIMAP&, const TRIE_MULTIMAP&);
#else /* __STL_TEMPLATE_FRIENDS */
  friend bool std:: operator== <> (const trie_multimap&, const trie_multimap&);
  friend bool std:: operator< <> (const trie_multimap&, const trie_multimap&);
#endif /* __STL_TEMPLATE_FRIENDS */

  // trie operations:

  std::vector<iterator> approximate_find(const key_type& __x, unsigned int __k = 1) 
  { return _M_t.approximate_find(__x, __k); }

  std::vector<const_iterator> approximate_find(const key_type& __x, unsigned int __k = 1) const 
  { return _M_t.approximate_find(__x, __k); }

  std::vector<iterator> best_find(const key_type& __x, unsigned int __k = TRIE_INFINITY) 
  { return _M_t.best_find(__x, __k); }

  std::vector<const_iterator> best_find(const key_type& __x, unsigned int __k = TRIE_INFINITY) const 
  { return _M_t.best_find(__x, __k); }

};

TRIE_MULTIMAP_TEMPLATE
inline bool operator==(const TRIE_MULTIMAP& x, 
                       const TRIE_MULTIMAP& y) {
  return x._M_t == y._M_t;
}

TRIE_MULTIMAP_TEMPLATE
inline bool operator<(const TRIE_MULTIMAP& x, 
                      const TRIE_MULTIMAP& y) {
  return x._M_t < y._M_t;
}
#if 1 //def __STL_FUNCTION_TMPL_PARTIAL_ORDER

TRIE_MULTIMAP_TEMPLATE
inline bool operator!=(const TRIE_MULTIMAP& __x, 
                       const TRIE_MULTIMAP& __y) {
  return !(__x == __y);
}

TRIE_MULTIMAP_TEMPLATE
inline bool operator>(const TRIE_MULTIMAP& __x, 
                      const TRIE_MULTIMAP& __y) {
  return __y < __x;
}


TRIE_MULTIMAP_TEMPLATE
inline bool operator<=(const TRIE_MULTIMAP& __x, 
                       const TRIE_MULTIMAP& __y) {
  return !(__y < __x);
}


TRIE_MULTIMAP_TEMPLATE
inline bool operator>=(const TRIE_MULTIMAP& __x, 
                       const TRIE_MULTIMAP& __y) {
  return !(__x < __y);
}


TRIE_MULTIMAP_TEMPLATE
inline void swap(TRIE_MULTIMAP& x, 
                 TRIE_MULTIMAP& y) {
  x.swap(y);
}

#endif // __STL_FUNCTION_TMPL_PARTIAL_ORDER

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1174
#pragma reset woff 1375
#endif

} // trie namespace

#endif // #ifndef __TRIE_INTERNAL_MULTIMAP_H

