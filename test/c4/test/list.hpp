#ifndef _C4_LIST_TEST_HPP
#define _C4_LIST_TEST_HPP

#include "c4/list.hpp"

#include "c4/libtest/supprwarn_push.hpp"
#include "c4/test.hpp"
#include "c4/libtest/archetypes.hpp"

#ifdef __GNUC__
// this is needed in GCC for preventing warnings about varargs macros in pedantic mode
#   pragma GCC system_header
#endif

C4_BEGIN_NAMESPACE(c4)

#define _C4_DEFINE_LIST_TEST_TYPES(List)                            \
    using T = typename List::value_type;                            \
    using I = typename List::size_type;                             \
    using UI = typename List::difference_type;                      \
    using CT = Counting< T >;                                       \
    using proto = c4::archetypes::archetype_proto< T >;             \
    using C ## List = typename List::template rebind_type< CT >;    \
    using iltype = std::initializer_list< T >;                      \
    using ciltype = std::initializer_list< CT >;

//-----------------------------------------------------------------------------

template< class List >
struct is_dbl_list
    :
    public std::integral_constant
    <
        bool,
        c4::is_instance_of_tpl< c4::flat_list, List >::value
        ||
        c4::is_instance_of_tpl< c4::split_list, List >::value
    >
{};

//-----------------------------------------------------------------------------

template< class List >
void list_check_free_list(List const& li)
{
    C4_ASSERT(li.size() <= li.capacity());
    EXPECT_EQ(li.slack(), li.capacity()-li.size());
    using I = typename List::size_type;
    if(li.size() < li.capacity())
    {
        EXPECT_NE(li.m_fhead, List::npos);
    }
    else
    {
        EXPECT_EQ(li.m_fhead, List::npos); // this is not true, need to fix
    }

    if(li.capacity() == 0) return;
    I cursor = li.m_fhead;
    I count = 0;
    while(cursor != List::npos)
    {
        count++;
        cursor = li.next(cursor);
    }
    EXPECT_EQ(count, li.capacity()-li.size());

    list_check_free_list_prev(li, is_dbl_list< List >{});
}
template< class List >
void list_check_free_list_prev(List const& li, std::true_type /*is_dbl_list*/)
{
    using I = typename List::size_type;
    if(li.capacity() == 0) return;
    // get the last valid elm in the free list
    I last = li.m_fhead;
    if(last == List::npos) return;
    I cursor = li.next(last);
    while(cursor != List::npos)
    {
        last = li.next(last);
        cursor = li.next(cursor);
    }
    C4_ASSERT(last != List::npos);
    cursor = last;
    I count = 0;
    while(cursor != List::npos)
    {
        count++;
        cursor = li.prev(cursor);
    }
    EXPECT_EQ(count, li.capacity()-li.size());
}
template< class List >
void list_check_free_list_prev(List const& /*li*/, std::false_type /*is_dbl_list*/)
{
    // nothing to do - fwd lists have no prev elm
}

//-----------------------------------------------------------------------------

template< class List >
void list_test0_ctor_empty()
{
    _C4_DEFINE_LIST_TEST_TYPES(List);
    auto cch = CT::check_ctors_dtors(0, 0);
    {
        CList li;
        EXPECT_TRUE(li.empty());
        EXPECT_EQ(li.size(), 0);
        if(!List::storage_traits::fixed && !List::storage_traits::small)
        {
            EXPECT_EQ(li.capacity(), 0);
        }
        else
        {
            EXPECT_GT(li.capacity(), 0);
        }
        EXPECT_EQ(li.begin(), li.end());
        EXPECT_EQ(std::distance(li.begin(), li.end()), 0);
        list_check_free_list(li);
    }
}

//-----------------------------------------------------------------------------
template< class List >
void list_test0_ctor_with_capacity()
{
    _C4_DEFINE_LIST_TEST_TYPES(List);

    auto cch = CT::check_ctors_dtors(0, 0);
    {
        CList li(c4::with_capacity, 5);
        EXPECT_TRUE(li.empty());
        EXPECT_EQ(li.size(), 0);
        EXPECT_GE(li.capacity(), 5);
        EXPECT_EQ(li.begin(), li.end());
        EXPECT_EQ(std::distance(li.begin(), li.end()), 0);
        list_check_free_list(li);
    }
}
//-----------------------------------------------------------------------------

template< class List >
void _do_list_test0_small_reserve_to_long(stg::fixed_t)
{
    // nothing to do
}

template< class List, class TagType >
void _do_list_test0_small_reserve_to_long(TagType)
{
    using I = typename List::size_type;
    List li;
    list_check_free_list(li);
    for(size_t i = 0; i < 4; ++i)
    {
        if(List::storage_traits::paged && size_t(li.capacity()) == li.max_size())
        {
            return;
        }
        /* <jpmag>
         * g++-6 is erroring inside li.reserve() when optimizing because reserve()
         * does a comparison between the current and the asked capacity:
         *   void reserve(I cap)
         *   {
         *       I curr = _c4this->capacity();
         *       if(cap > curr)
         *       {
         *           _c4this->_growto(curr, cap);
         *       }
         *   }
         * The error message from gcc is "error: assuming signed overflow does
         * not occur when assuming that (X + c) >= X is always true
         * [-Werror=strict-overflow]". See
         * https://stackoverflow.com/questions/12984861/
         *
         * This probably means g++-6 inlines and elides and sees that cap==curr+8, so the
         * condition in the if turns to (curr+8 > curr). To prevent gcc from
         * optimizing, I'm making cap a volatile.
         */
	    volatile size_t cap = li.next_capacity(size_t(li.capacity()) + size_t(8)); // reserve more than the initial capacity
	    if(cap <= li.max_size())
	    {
		    li.reserve(szconv<I>(cap));
		    list_check_free_list(li); // the free list should be uninterrupted
	    }
	    else
	    {
		    return;
	    }
    }
}

template< class List >
void list_test0_small_reserve_to_long()
{
    _do_list_test0_small_reserve_to_long< List >(typename List::storage_traits::tag_type{});
}

//-----------------------------------------------------------------------------

template< class List >
void list_test0_ctor_with_initlist()
{
    _C4_DEFINE_LIST_TEST_TYPES(List);

    ciltype il = proto::cil();
    C4_ASSERT(il.size() > 0);
    {
        auto cpch = CT::check_copy_ctors(il.size());
        auto dtch = CT::check_dtors(il.size());
        {
            CList li(c4::aggregate, il);
            EXPECT_FALSE(li.empty());
            EXPECT_EQ(li.size(), il.size());
            EXPECT_GE(li.capacity(), il.size());
            EXPECT_NE(li.begin(), li.end());
            EXPECT_EQ(std::distance(li.begin(), li.end()), il.size());

            int pos = 0;
            for(auto const& v : li)
            {
                auto const& ref = proto::get(pos++);
                EXPECT_EQ(v, ref);
            }

            size_t iback = il.size() - 1;
            {
                EXPECT_EQ(li.front(), proto::get(0));
                EXPECT_EQ(li.back(), proto::get(iback));
            }
            {
                auto const& crli = li;
                EXPECT_EQ(crli.front(), proto::get(0));
                EXPECT_EQ(crli.back(), proto::get(iback));
            }

            list_check_free_list(li);
        }
    }
}

//-----------------------------------------------------------------------------

template< class List >
void list_test0_push_back_copy()
{
    _C4_DEFINE_LIST_TEST_TYPES(List);

    auto arr = proto::carr();
    {
        auto cpch = CT::check_copy_ctors(arr.size());
        auto dtch = CT::check_dtors(arr.size());
        {
            CList li;
            for(auto const& elm : arr)
            {
                li.push_back(elm);
            }

            EXPECT_FALSE(li.empty());
            EXPECT_EQ(li.size(), arr.size());
            EXPECT_GE(li.capacity(), arr.size());
            EXPECT_NE(li.begin(), li.end());
            EXPECT_EQ(std::distance(li.begin(), li.end()), arr.size());

            int pos = 0;
            for(auto const& v : li)
            {
                auto const& ref = proto::get(pos++);
                EXPECT_EQ(v, ref);
            }

            size_t iback = arr.size() - 1;
            {
                EXPECT_EQ(li.front(), proto::get(0));
                EXPECT_EQ(li.back(), proto::get(iback));
            }
            {
                auto const& crli = li;
                EXPECT_EQ(crli.front(), proto::get(0));
                EXPECT_EQ(crli.back(), proto::get(iback));
            }

            list_check_free_list(li);
        }
    }
}

//-----------------------------------------------------------------------------

template< class List >
void list_test0_push_back_move()
{
    _C4_DEFINE_LIST_TEST_TYPES(List);

    ciltype il = proto::cil();
    {
        auto cpch = CT::check_move_ctors(il.size());
        auto dtch = CT::check_dtors(2 * il.size()); // 1 movedfrom + 1 movedto
        {
            CList li;
            list_check_free_list(li);
            li.reserve(szconv<I>(il.size())); // reserve the list to prevent moves
                                              // when it would be resized
            list_check_free_list(li);
            for(auto const& elm : il)
            {
                auto tmp = elm; // this will be copied here
                li.push_back(std::move(tmp)); // ... and moved here
            }

            EXPECT_FALSE(li.empty());
            EXPECT_EQ(li.size(), il.size());
            EXPECT_GE(li.capacity(), (typename List::size_type)il.size());
            EXPECT_NE(li.begin(), li.end());
            EXPECT_EQ(std::distance(li.begin(), li.end()), il.size());

            int pos = 0;
            for(auto const& v : li)
            {
                auto const& ref = proto::get(pos++);
                EXPECT_EQ(v, ref);
            }

            list_check_free_list(li);
        }
    }
}

//-----------------------------------------------------------------------------

template< class List >
void list_test0_grow_to_reallocate()
{
    _C4_DEFINE_LIST_TEST_TYPES(List);

    if(List::storage_traits::fixed) return;

    auto arr = proto::arr();
    {
        I target_size = 0;
        List li;
        list_check_free_list(li);
        li.push_back(arr[0]); // make it have at least some initial capacity
        list_check_free_list(li);
        I cap = li.capacity();

        // find a target size which will trigger allocations/deallocations
        if(List::storage_traits::paged)
        {
            target_size = 3 * stg::default_page_size< T, I >::value;
        }
        else
        {
            if(cap >= 32)
            {
                size_t tgt = 4 * cap; // use size_t to prevent overflow of I
                size_t mx = (size_t)std::numeric_limits< I >::max();
                target_size = (I)(tgt > mx ? mx : tgt);
            }
            else
            {
                target_size = std::is_same< I, int8_t >::value ? 64 : 128;
            }
        }
        C4_ASSERT(target_size > 0);
        target_size = std::min(target_size, szconv<I>(li.max_size()));

        // fill the list
        I pos = 1; // there's one already
        while(pos < target_size)
        {
            I ielm = pos++ % (I)arr.size();
            li.push_back(arr[ielm]);
        }
        C4_ASSERT_MSG(li.size() == target_size, "target=%zu sz=%zu max=%zu", (size_t)target_size, (size_t)li.size(), (size_t)li.max_size());
        C4_ASSERT(li.size() > cap);
        list_check_free_list(li);

        pos = 0;
        for(auto const& v : li)
        {
            I ielm = pos++ % (I)arr.size();
            auto const& ref = proto::get(ielm);
            EXPECT_EQ(v, ref);
        }

    }
}


//-----------------------------------------------------------------------------

#define _C4_LIST_BASIC_TESTS(listtestname, listtype)            \
TEST(listtestname, ctor_empty)                                  \
{                                                               \
    list_test0_ctor_empty< listtype >();                        \
}                                                               \
TEST(listtestname, ctor_empty_const)                            \
{                                                               \
    list_test0_ctor_empty< const listtype >();                  \
}                                                               \
TEST(listtestname, ctor_with_capacity)                          \
{                                                               \
    list_test0_ctor_with_capacity< listtype >();                \
}                                                               \
TEST(listtestname, ctor_with_initlist)                          \
{                                                               \
    list_test0_ctor_with_initlist< listtype >();                \
}                                                               \
TEST(listtestname, small_reserve_to_long)                       \
{                                                               \
    list_test0_small_reserve_to_long< listtype >();             \
}                                                               \
TEST(listtestname, push_back_copy)                              \
{                                                               \
    list_test0_push_back_copy< listtype >();                    \
}                                                               \
TEST(listtestname, push_back_move)                              \
{                                                               \
    list_test0_push_back_move< listtype >();                    \
}                                                               \
TEST(listtestname, grow_to_reallocate)                          \
{                                                               \
    list_test0_grow_to_reallocate< listtype >();                \
}


//-----------------------------------------------------------------------------

#define _C4_CALL_LIST_TESTS(list_type_name, list_type,                  \
                            containee_type_name, containee_type,        \
                            sztype_name, sztype,                        \
                            storage_type_name, storage_type_macro,      \
                            ...)                                        \
_C4_LIST_BASIC_TESTS                                                    \
(                                                                       \
    list_type_name##__##containee_type_name##__##sztype_name##__##storage_type_name, \
    list_type< containee_type C4_COMMA sztype C4_COMMA storage_type_macro(containee_type, sztype, ## __VA_ARGS__) >   \
)


//-----------------------------------------------------------------------------

#ifndef C4_QUICKTEST

#define _C4_CALL_LIST_TESTS_FOR_ALL_SIZE_TYPES(list_tyname, list_ty,    \
                                               containee_type_name, containee_type, \
                                               storage_type_name, storage_type_macro, \
                                               ...)                        \
    _C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type, u64, uint64_t, storage_type_name, storage_type_macro, ## __VA_ARGS__) \
    _C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type, i64,  int64_t, storage_type_name, storage_type_macro, ## __VA_ARGS__) \
    _C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type, u32, uint32_t, storage_type_name, storage_type_macro, ## __VA_ARGS__) \
    _C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type, i32,  int32_t, storage_type_name, storage_type_macro, ## __VA_ARGS__) \
    _C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type, u16, uint16_t, storage_type_name, storage_type_macro, ## __VA_ARGS__) \
    _C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type, i16,  int16_t, storage_type_name, storage_type_macro, ## __VA_ARGS__) \
    _C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type, u8 ,  uint8_t, storage_type_name, storage_type_macro, ## __VA_ARGS__) \
    _C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type, i8 ,   int8_t, storage_type_name, storage_type_macro, ## __VA_ARGS__)

#else // C4_QUICKTEST

#define _C4_CALL_LIST_TESTS_FOR_ALL_SIZE_TYPES(list_tyname, list_ty,    \
                                               containee_type_name, containee_type, \
                                               storage_type_name, storage_type_macro, \
                                               ...)                     \
    _C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type, size_t,  size_t, storage_type_name, storage_type_macro, ## __VA_ARGS__) \
    _C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type,    i32, int32_t, storage_type_name, storage_type_macro, ## __VA_ARGS__)

#endif // ! C4_QUICKTEST

//-----------------------------------------------------------------------------

#define _C4_CALL_FLAT_LIST_TESTS_FOR_STORAGE(tyname, ty, storage_type_name, storage_type_macro, ...) \
_C4_CALL_LIST_TESTS_FOR_ALL_SIZE_TYPES(flat_list, flat_list, tyname, ty, storage_type_name, storage_type_macro, ## __VA_ARGS__)

#define _C4_CALL_SPLIT_LIST_TESTS_FOR_STORAGE(tyname, ty, storage_type_name, storage_type_macro, ...) \
_C4_CALL_LIST_TESTS_FOR_ALL_SIZE_TYPES(split_list, split_list, tyname, ty, storage_type_name, storage_type_macro, ## __VA_ARGS__)

#define _C4_CALL_FLAT_FWD_LIST_TESTS_FOR_STORAGE(tyname, ty, storage_type_name, storage_type_macro, ...) \
_C4_CALL_LIST_TESTS_FOR_ALL_SIZE_TYPES(flat_fwd_list, flat_fwd_list, tyname, ty, storage_type_name, storage_type_macro, ## __VA_ARGS__)

#define _C4_CALL_SPLIT_FWD_LIST_TESTS_FOR_STORAGE(tyname, ty, storage_type_name, storage_type_macro, ...) \
_C4_CALL_LIST_TESTS_FOR_ALL_SIZE_TYPES(split_fwd_list, split_fwd_list, tyname, ty, storage_type_name, storage_type_macro, ## __VA_ARGS__)

//-----------------------------------------------------------------------------

template< class I > struct raw_fixed_max_size             { enum { value = 1024, value_pot = 1024 }; };
template<         > struct raw_fixed_max_size< uint8_t >  { enum { value =  255, value_pot =  128 }; };
template<         > struct raw_fixed_max_size<  int8_t >  { enum { value =  127, value_pot =   64 }; };
template<         > struct raw_fixed_max_size<    char >  { enum { value =  127, value_pot =   64 }; };

//-----------------------------------------------------------------------------

#define _C4_RAW_FIXED(ty, sz, N) stg::raw_fixed<ty, (N < raw_fixed_max_size<sz>::value ? N : raw_fixed_max_size<sz>::value), sz>
#define _C4_RAW_SMALL(ty, sz, N) stg::raw_small<ty, sz, N>
#define _C4_RAW(ty, sz)          stg::raw         <ty, sz>
#define _C4_RAW_PAGED(ty, sz)    stg::raw_paged   <ty, sz>
#define _C4_RAW_PAGED_RT(ty, sz) stg::raw_paged_rt<ty, sz>

#define _C4_RAW_FIXED_FLAT_LIST(ty, sz, N) stg::raw_fixed   <flat_list_elm<ty, sz>, (N < raw_fixed_max_size<sz>::value ? N : raw_fixed_max_size<sz>::value), sz>
#define _C4_RAW_SMALL_FLAT_LIST(ty, sz, N) stg::raw_small   <flat_list_elm<ty, sz>, sz, N>
#define _C4_RAW_FLAT_LIST(ty, sz)          stg::raw         <flat_list_elm<ty, sz>, sz>
#define _C4_RAW_PAGED_FLAT_LIST(ty, sz)    stg::raw_paged   <flat_list_elm<ty, sz>, sz>
#define _C4_RAW_PAGED_RT_FLAT_LIST(ty, sz) stg::raw_paged_rt<flat_list_elm<ty, sz>, sz>

#define _C4_RAW_FIXED_FLAT_FWD_LIST(ty, sz, N) stg::raw_fixed   <flat_fwd_list_elm<ty, sz>, (N < raw_fixed_max_size<sz>::value ? N : raw_fixed_max_size<sz>::value), sz>
#define _C4_RAW_SMALL_FLAT_FWD_LIST(ty, sz, N) stg::raw_small   <flat_fwd_list_elm<ty, sz>, sz, N>
#define _C4_RAW_FLAT_FWD_LIST(ty, sz)          stg::raw         <flat_fwd_list_elm<ty, sz>, sz>
#define _C4_RAW_PAGED_FLAT_FWD_LIST(ty, sz)    stg::raw_paged   <flat_fwd_list_elm<ty, sz>, sz>
#define _C4_RAW_PAGED_RT_FLAT_FWD_LIST(ty, sz) stg::raw_paged_rt<flat_fwd_list_elm<ty, sz>, sz>

C4_END_NAMESPACE(c4)

#endif // _C4_LIST_TEST_HPP

#include "c4/libtest/supprwarn_pop.hpp"
