#ifndef _C4_LIST_HPP_
#define _C4_LIST_HPP_

#include "c4/config.hpp"
#include "c4/error.hpp"
#include "c4/storage/raw.hpp"

#include <iterator>

C4_BEGIN_NAMESPACE(c4)

template< class T, class I=C4_SIZE_TYPE >
using default_list_storage = stg::raw_paged_rt< T, I >;

template< class T, class I=C4_SIZE_TYPE, template< class T_, class I_ > class LinearStorage=default_list_storage >
class split_list;

template< class T, class I=C4_SIZE_TYPE, template< class T_, class I_ > class LinearStorage=default_list_storage >
class split_fwd_list;

template< class T, class I=C4_SIZE_TYPE, template< class T_, class I_ > class LinearStorage=default_list_storage >
class flat_list;

template< class T, class I=C4_SIZE_TYPE, template< class T_, class I_ > class LinearStorage=default_list_storage >
class flat_fwd_list;

template< class T, class I=C4_SIZE_TYPE > struct flat_list_elm;
template< class T, class I=C4_SIZE_TYPE > struct flat_fwd_list_elm;

//-----------------------------------------------------------------------------
template< class T, class I >
struct flat_list_elm
{
    T elm;
    I prev;
    I next;
};

template< class T, class I >
struct flat_fwd_list_elm
{
    T elm;
    I next;
};

//-----------------------------------------------------------------------------

template< class T, class List >
struct list_iterator : public std::iterator< std::bidirectional_iterator_tag, typename List::value_type >
{
    using I = typename List::size_type;

    List *list;
    I i;

    list_iterator(List *l, I i_) : list(l), i(i_) {}

    C4_ALWAYS_INLINE T& operator*  () { return  list->elm(i); }
    C4_ALWAYS_INLINE T* operator-> () { return &list->elm(i); }

    C4_ALWAYS_INLINE list_iterator& operator++ (   ) noexcept {                           i = list->next(i); return *this; }
    C4_ALWAYS_INLINE list_iterator& operator++ (int) noexcept { list_iterator it = *this; i = list->next(i); return    it; }

    C4_ALWAYS_INLINE list_iterator& operator-- (   ) noexcept {                           i = list->prev(i); return *this; }
    C4_ALWAYS_INLINE list_iterator& operator-- (int) noexcept { list_iterator it = *this; i = list->prev(i); return    it; }

    C4_ALWAYS_INLINE bool operator== (list_iterator const& that) const noexcept { return i == that.i && list == that.list; }
    C4_ALWAYS_INLINE bool operator!= (list_iterator const& that) const noexcept { return i != that.i || list != that.list; }
};

template< class T, class List >
struct fwd_list_iterator : public std::iterator< std::forward_iterator_tag, typename List::value_type >
{
    using I = typename List::size_type;

    List *list;
    I i;

    fwd_list_iterator(List *l, I i_) : list(l), i(i_) {}

    C4_ALWAYS_INLINE T* operator-> () { return &list->elm(i); }
    C4_ALWAYS_INLINE T& operator*  () { return  list->elm(i); }

    C4_ALWAYS_INLINE fwd_list_iterator& operator++ (   ) noexcept {                               i = list->next(i); return *this; }
    C4_ALWAYS_INLINE fwd_list_iterator& operator++ (int) noexcept { fwd_list_iterator it = *this; i = list->next(i); return    it; }

    C4_ALWAYS_INLINE bool operator== (fwd_list_iterator const& that) const noexcept { return i == that.i && list == that.list; }
    C4_ALWAYS_INLINE bool operator!= (fwd_list_iterator const& that) const noexcept { return i != that.i || list != that.list; }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// convenience defines; undefined below
#define _c4this  (static_cast<ListType      *>(this))
#define _c4cthis (static_cast<ListType const*>(this))

/** common code to implement doubly or singly linked lists */
template< class T, class I, class ListType >
class _list_crtp
{
public:

    void _init_initlist(std::initializer_list< T > il)
    {
        I sz = szconv< I >(il.size());
        if(sz)
        {
            _c4this->_set_head(0);
            _c4this->_set_tail(sz - 1);
            _c4this->_set_fhead(sz);
            _c4this->m_size = sz;
        }
        sz = 0;
        for(auto const& e : il)
        {
            c4::copy_construct(&(_c4this->elm(sz++)), e);
        }
    }

    void _make_room_for(I how_many)
    {
        I cap = _c4cthis->capacity();
        I cap_next = _c4cthis->m_size + how_many;
        if(cap_next > cap)
        {
            _c4this->_growto(cap, cap_next);
        }
    }

    /** claim one element from the free list, growing the storage if necessary */
    I _claim() C4_NOEXCEPT_X
    {
        this->_make_room_for(I(1));
        C4_XASSERT(_c4cthis->m_fhead != ListType::npos);
        C4_XASSERT(_c4cthis->m_size + 1 < _c4cthis->capacity());
        I pos = _c4cthis->m_fhead;
        C4_XASSERT(pos != ListType::npos);
        I last = _c4cthis->next(pos);
        _c4this->_set_fhead(last);
        C4_XASSERT(pos != ListType::npos);
        return pos;
    }

    /** claim n elements from the free list, growing the storage if necessary */
    I _claim(I n) C4_NOEXCEPT_X
    {
        this->_make_foom_for(n);
        C4_XASSERT(_c4cthis->m_fhead != ListType::npos);
        C4_XASSERT(_c4cthis->m_size + n < _c4cthis->capacity());
        I pos = _c4cthis->m_fhead;
        I count = 0, last = pos;
        for(count = 0; count < n; ++count)
        {
            C4_XASSERT(last != ListType::npos);
            last = _c4cthis->next(last);
        }
        C4_XASSERT(count == n);
        _c4this->_set_fhead(last);
        C4_XASSERT(pos != ListType::npos);
        return pos;
    }

public:

    void push_front(T const& var)
    {
        C4_NOT_IMPLEMENTED();
    }

    void push_back(T const& var)
    {
        I pos = _c4this->_append();
        T *elm = &_c4this->elm(pos);
        c4::copy_construct(elm, var);
    }
    void push_back(T && var)
    {
        I pos = _c4this->_append();
        T *elm = &_c4this->elm(pos);
        c4::move_construct(elm, std::move(var));
    }

    template< class... Args >
    void emplace_back(Args&&... args)
    {
        I pos = _c4this->_append();
        T *elm = &_c4this->elm(pos);
        c4::construct(elm, std::forward< Args >(args)...);
    }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** common code to implement doubly linked lists */
template< class T, class I, class ListType >
class _doubly_list_crtp : public _list_crtp< T, I, ListType >
{
public:

    void _init_seq(I first, I last, I prev_ = ListType::npos, I next_ = ListType::npos)
    {
        C4_XASSERT(last >= first);
        if(last == first) return;
        _c4this->_set_prev(first, prev_);
        for(I i = first+1; i < last; ++i)
        {
            _c4this->_set_prev(i, i-1);
        }
        C4_XASSERT(last > 0);
        for(I i = first; i < last-1; ++i)
        {
            _c4this->_set_next(i, i+1);
        }
        _c4this->_set_next(last-1, next_);
    }

    I _append()
    {
        I pos = _claim();
        if(_c4this->m_size > 0)
        {
            _c4this->_set_prev(pos, _c4cthis->m_tail);
            _c4this->_set_next(_c4cthis->m_tail, pos);
        }
        else
        {
            _c4this->_set_head(0);
        }
        ++_c4this->m_size;
        _c4this->_set_tail(pos);
        return pos;
    }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** common code to implement forward linked lists */
template< class T, class I, class ListType >
class _fwd_list_crtp : public _list_crtp< T, I, ListType >
{
public:

    void _init_seq(I first, I last, I prev_ = ListType::npos, I next_ = ListType::npos)
    {
        C4_XASSERT(last >= first);
        if(last == first) return;
        C4_XASSERT(last > 0);
        for(I i = first; i < last-1; ++i)
        {
            _c4this->_set_next(i, i+1);
        }
        _c4this->_set_next(last-1, next_);
    }

    I _append()
    {
        I pos = _claim();
        if(_c4this->m_size > 0)
        {
            _c4this->_set_next(_c4cthis->m_tail, pos);
        }
        else
        {
            _c4this->_set_head(0);
        }
        ++_c4this->m_size;
        _c4this->_set_tail(pos);
        return pos;
    }

};

#define _c4this  (static_cast<ListType      *>(this))
#define _c4cthis (static_cast<ListType const*>(this))

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** an array-based doubly-linked list where the indices are interleaved
 * with the contained elements. Using paged raw storage, insertions are O(1). */
template< class T, class I, template< class T, class I > class RawStorage >
class flat_list : public _doubly_list_crtp< T, I, flat_list<T, I, RawStorage> >
{
public:

    RawStorage< flat_list_elm<T, I>, I > m_elms;

    I m_head;
    I m_tail;
    I m_size;
    I m_fhead;

public:

    enum : I { npos = static_cast< I >(-1) };

    using value_type = T;
    using size_type = I;
    using storage_type = RawStorage< flat_list_elm<T, I>, I >;
    using storage_traits = typename RawStorage< flat_list_elm<T, I>, I >::storage_traits;

    using iterator = list_iterator< T, flat_list >;
    using const_iterator = list_iterator< const T, const flat_list >;

public:

    flat_list() : m_elms(), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        _init_seq(0, capacity());
    }

    flat_list(c4::with_capacity_t, I cap) : m_elms(cap), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        _init_seq(0, capacity());
    }

    flat_list(c4::aggregate_t, std::initializer_list< T > il) : m_elms(szconv< I >(il.size())), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        _init_seq(0, capacity());
        _init_initlist(il);
    }

    void _growto(I cap, I next_cap)
    {
        //m_elms._raw_reserve(next_cap);
        m_elms._raw_resize(next_cap);
        _init_seq(cap, capacity());
    }

public:

    C4_ALWAYS_INLINE T      & elm(I i)       C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_elms[i].elm; }
    C4_ALWAYS_INLINE T const& elm(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_elms[i].elm; }

    C4_ALWAYS_INLINE I  prev(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_elms[i].prev; }
    C4_ALWAYS_INLINE I  next(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_elms[i].next; }

    C4_ALWAYS_INLINE void _set_prev(I i, I val) C4_NOEXCEPT_X { m_elms[i].prev = val; }
    C4_ALWAYS_INLINE void _set_next(I i, I val) C4_NOEXCEPT_X { m_elms[i].next = val; }

    C4_ALWAYS_INLINE void _set_head(I i) C4_NOEXCEPT_X { m_head = i; m_elms[i].prev = npos; }
    C4_ALWAYS_INLINE void _set_tail(I i) C4_NOEXCEPT_X { m_tail = i; m_elms[i].next = npos; }

    C4_ALWAYS_INLINE void _set_fhead(I i) C4_NOEXCEPT_X { m_fhead = i; m_elms[i].prev = npos; }

public:

    C4_ALWAYS_INLINE bool empty() const noexcept { return m_size == 0; }

    C4_ALWAYS_INLINE I size    () const noexcept { return m_size; }
    C4_ALWAYS_INLINE I capacity() const noexcept { return m_elms.capacity(); }

    C4_ALWAYS_INLINE iterator begin() noexcept { return iterator(this, m_head); }
    C4_ALWAYS_INLINE iterator end  () noexcept { return iterator(this, npos); }

    C4_ALWAYS_INLINE const_iterator begin() const noexcept { return const_iterator(this, m_head); }
    C4_ALWAYS_INLINE const_iterator end  () const noexcept { return const_iterator(this, npos); }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** an array-based doubly-linked list where the indices are stored in separate
 * arrays from the contained elements. Using paged raw storage, insertions
 * are O(1). */
template< class T, class I, template< class T, class I > class RawStorage >
class split_list : public _doubly_list_crtp< T, I, split_list<T, I, RawStorage> >
{
public:

    RawStorage< T, I > m_elm;
    RawStorage< I, I > m_prev;
    RawStorage< I, I > m_next;

    I m_head;
    I m_tail;
    I m_size;
    I m_fhead; //< the head of the free list (the list containing free elements)

public:

    enum : I { npos = static_cast< I >(-1) };

    using value_type = T;
    using size_type = I;
    using storage_type = RawStorage< T, I >;
    using storage_traits = typename RawStorage< T, I >::storage_traits;
    using index_storage_type = RawStorage< I, I >;
    using index_storage_traits = typename RawStorage< I, I >::storage_traits;

    using iterator = list_iterator< T, split_list >;
    using const_iterator = list_iterator< const T, const split_list >;

public:

    split_list() : m_elm(), m_prev(), m_next(), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        _init_seq(0, capacity());
    }

    split_list(c4::with_capacity_t, I cap) : m_elm(cap), m_prev(cap), m_next(cap), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        _init_seq(0, capacity());
    }

    split_list(c4::aggregate_t, std::initializer_list< T > il) : m_elm(szconv< I >(il.size())), m_prev(szconv< I >(il.size())), m_next(szconv< I >(il.size())), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        _init_seq(0, capacity());
        _init_initlist(il);
    }

    void _growto(I curr_cap, I next_cap)
    {
        m_elm._raw_reserve(next_cap);
        m_prev._raw_reserve(next_cap);
        m_next._raw_reserve(next_cap);
        _init_seq(curr_cap, next_cap);
    }

public:

    C4_ALWAYS_INLINE T      & elm(I i)       C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_elm[i]; }
    C4_ALWAYS_INLINE T const& elm(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_elm[i]; }

    C4_ALWAYS_INLINE I prev(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_prev[i]; }
    C4_ALWAYS_INLINE I next(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_next[i]; }

    C4_ALWAYS_INLINE void _set_prev(I i, I val) C4_NOEXCEPT_X { m_prev[i] = val; }
    C4_ALWAYS_INLINE void _set_next(I i, I val) C4_NOEXCEPT_X { m_next[i] = val; }

    C4_ALWAYS_INLINE void _set_head(I i) C4_NOEXCEPT_X { m_head = i; m_prev[i] = npos; }
    C4_ALWAYS_INLINE void _set_tail(I i) C4_NOEXCEPT_X { m_tail = i; m_next[i] = npos; }

    C4_ALWAYS_INLINE void _set_fhead(I i) C4_NOEXCEPT_X { m_fhead = i; m_prev[i] = npos; }

public:
    
    C4_ALWAYS_INLINE bool empty() const noexcept { return m_size == 0; }

    C4_ALWAYS_INLINE I size() const noexcept { return m_size; }
    C4_ALWAYS_INLINE I capacity() const noexcept { return m_elm.capacity(); }

    C4_ALWAYS_INLINE iterator begin() noexcept { return iterator(this, m_head); }
    C4_ALWAYS_INLINE iterator end  () noexcept { return iterator(this, npos); }

    C4_ALWAYS_INLINE const_iterator begin() const noexcept { return const_iterator(this, m_head); }
    C4_ALWAYS_INLINE const_iterator end  () const noexcept { return const_iterator(this, npos); }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** an array-based forward-linked list where the indices are interleaved
 * with the contained elements. Using paged raw storage, insertions are O(1). */
template< class T, class I, template< class T, class I > class RawStorage >
class flat_fwd_list : public _fwd_list_crtp< T, I, flat_fwd_list<T, I, RawStorage> >
{
public:

    RawStorage< flat_fwd_list_elm<T, I>, I > m_elms;

    I m_head;
    I m_tail;
    I m_size;
    I m_fhead;

public:

    enum : I { npos = static_cast< I >(-1) };

    using value_type = T;
    using size_type = I;
    using storage_type = RawStorage< flat_fwd_list_elm<T, I>, I >;
    using storage_traits = typename RawStorage< flat_fwd_list_elm<T, I>, I >::storage_traits;

    using iterator = list_iterator< T, flat_fwd_list >;
    using const_iterator = list_iterator< const T, const flat_fwd_list >;

public:

    flat_fwd_list() : m_elms(), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        _init_seq(0, capacity());
    }

    flat_fwd_list(c4::with_capacity_t, I cap) : m_elms(cap), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        _init_seq(0, capacity());
    }

    flat_fwd_list(c4::aggregate_t, std::initializer_list< T > il) : m_elms(szconv< I >(il.size())), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        _init_seq(0, capacity());
        _init_initlist(il);
    }

public:

    C4_ALWAYS_INLINE T      & elm(I i)       C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_elms[i].elm; }
    C4_ALWAYS_INLINE T const& elm(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_elms[i].elm; }

    C4_ALWAYS_INLINE I  next(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_elms[i].next; }

    C4_ALWAYS_INLINE void _set_next(I i, I val) C4_NOEXCEPT_X { m_elms[i].next = val; }

    C4_ALWAYS_INLINE void _set_head(I i) C4_NOEXCEPT_X { m_head = i; }
    C4_ALWAYS_INLINE void _set_tail(I i) C4_NOEXCEPT_X { m_tail = i; m_elms[i].next = npos; }

    C4_ALWAYS_INLINE void _set_fhead(I i) C4_NOEXCEPT_X { m_fhead = i; }

public:
    
    C4_ALWAYS_INLINE bool empty() const noexcept { return m_size == 0; }

    C4_ALWAYS_INLINE I size() const noexcept { return m_size; }
    C4_ALWAYS_INLINE I capacity() const noexcept { return m_elms.capacity(); }

    C4_ALWAYS_INLINE iterator begin() noexcept { return iterator(this, m_head); }
    C4_ALWAYS_INLINE iterator end  () noexcept { return iterator(this, npos); }

    C4_ALWAYS_INLINE const_iterator begin() const noexcept { return const_iterator(this, m_head); }
    C4_ALWAYS_INLINE const_iterator end  () const noexcept { return const_iterator(this, npos); }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** an array-based forward-linked list where the indices are stored in a
 * separate array from the contained elements. Using paged raw storage,
 * insertions are O(1). */
template< class T, class I, template< class T, class I > class RawStorage >
class split_fwd_list : public _fwd_list_crtp< T, I, split_fwd_list<T, I, RawStorage> >
{
public:

    RawStorage< T, I > m_elm;
    RawStorage< I, I > m_next;

    I m_head;
    I m_tail;
    I m_size;
    I m_fhead;

public:

    enum : I { npos = static_cast< I >(-1) };

    using value_type = T;
    using size_type = I;
    using storage_type = RawStorage< T, I >;
    using storage_traits = typename RawStorage< T, I >::storage_traits;
    using index_storage_type = RawStorage< I, I >;
    using index_storage_traits = typename RawStorage< I, I >::storage_traits;

    using iterator = list_iterator< T, split_fwd_list >;
    using const_iterator = list_iterator< const T, const split_fwd_list >;

public:

    split_fwd_list() : m_elm(), m_next(), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        _init_seq(0, capacity());
    }

    split_fwd_list(c4::with_capacity_t, I cap) : m_elm(cap), m_next(cap), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        _init_seq(0, capacity());
    }

    split_fwd_list(c4::aggregate_t, std::initializer_list< T > il) : m_elm(szconv< I >(il.size())), m_next(szconv< I >(il.size())), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        _init_seq(0, capacity());
        _init_initlist(il);
    }

public:

    C4_ALWAYS_INLINE T      & elm(I i)       C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_elm[i]; }
    C4_ALWAYS_INLINE T const& elm(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_elm[i]; }

    C4_ALWAYS_INLINE I next(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_next[i]; }

    C4_ALWAYS_INLINE void _set_next(I i, I val) C4_NOEXCEPT_X { m_next[i] = val; }

    C4_ALWAYS_INLINE void _set_head(I i) C4_NOEXCEPT_X { m_head = i; }
    C4_ALWAYS_INLINE void _set_tail(I i) C4_NOEXCEPT_X { m_tail = i; m_next[i] = npos; }

    C4_ALWAYS_INLINE void _set_fhead(I i) C4_NOEXCEPT_X { m_fhead = i; }

public:

    C4_ALWAYS_INLINE bool empty() const noexcept { return m_size == 0; }

    C4_ALWAYS_INLINE I size() const noexcept { return m_size; }
    C4_ALWAYS_INLINE I capacity() const noexcept { return m_elm.capacity(); }

    C4_ALWAYS_INLINE iterator begin() noexcept { return iterator(this, m_head); }
    C4_ALWAYS_INLINE iterator end  () noexcept { return iterator(this, npos); }

    C4_ALWAYS_INLINE const_iterator begin() const noexcept { return const_iterator(this, m_head); }
    C4_ALWAYS_INLINE const_iterator end  () const noexcept { return const_iterator(this, npos); }

};

C4_END_NAMESPACE(c4)

#endif /* _C4_LIST_HPP_ */
