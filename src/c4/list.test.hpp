#ifndef _C4_LIST_TEST_HPP
#define _C4_LIST_TEST_HPP

#include "c4/list.hpp"

#include "c4/test.hpp"
#include "c4/archetypes.hpp"

C4_BEGIN_NAMESPACE(c4)

template< class List >
void list_test0_empty_ctor()
{
    List li;
    EXPECT_TRUE(li.empty());
    EXPECT_EQ(li.size(), 0);
    EXPECT_EQ(li.capacity(), 0);
    EXPECT_EQ(li.begin(), li.end());
    EXPECT_EQ(std::distance(li.begin(), li.end()), 0);
}
template< class List >
void list_test0_with_capacity()
{
    List li(c4::with_capacity, 5);
    EXPECT_TRUE(li.empty());
    EXPECT_EQ(li.size(), 0);
    EXPECT_GE(li.capacity(), (typename List::size_type)5);
    EXPECT_EQ(li.begin(), li.end());
    EXPECT_EQ(std::distance(li.begin(), li.end()), 0);
}

#define _C4_TEST_LIST_BASIC_TESTS(listtestname, listtype)       \
TEST(listtestname, ctor_empty)                                  \
{                                                               \
    list_test0_empty_ctor< listtype >();                        \
}                                                               \
TEST(listtestname, ctor_empty_const)                            \
{                                                               \
    list_test0_empty_ctor< const listtype >();                  \
}                                                               \
TEST(listtestname, ctor_with_capacity)                          \
{                                                               \
    list_test0_with_capacity< listtype >();                     \
}

#define _C4_CALL_LIST_TESTS(list_type_name, list_type, containee_type_name, containee_type, sztype_name, sztype) \
_C4_TEST_LIST_BASIC_TESTS                                               \
(                                                                       \
    list_type_name##__##containee_type_name##__##sztype_name,           \
    list_type< containee_type C4_COMMA sztype >                         \
)

#define _C4_CALL_LIST_TESTS_FOR_ALL_SIZE_TYPES(list_tyname, list_ty, containee_type_name, containee_type) \
_C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type, uint64_t, uint64_t) \
_C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type,  int64_t,  int64_t) \
_C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type, uint32_t, uint32_t) \
_C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type,  int32_t,  int32_t) \
_C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type, uint16_t, uint16_t) \
_C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type,  int16_t,  int16_t) \
_C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type,  uint8_t,  uint8_t) \
_C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type,   int8_t,   int8_t)

#define _C4_CALL_FLAT_LIST_TESTS_ADAPTOR(tyname, ty) \
_C4_CALL_LIST_TESTS_FOR_ALL_SIZE_TYPES(flat_list, flat_list, tyname, ty)

#define _C4_CALL_SPLIT_LIST_TESTS_ADAPTOR(tyname, ty) \
_C4_CALL_LIST_TESTS_FOR_ALL_SIZE_TYPES(split_list, split_list, tyname, ty)

#define _C4_CALL_FLAT_FWD_LIST_TESTS_ADAPTOR(tyname, ty) \
_C4_CALL_LIST_TESTS_FOR_ALL_SIZE_TYPES(flat_fwd_list, flat_fwd_list, tyname, ty)

#define _C4_CALL_SPLIT_FWD_LIST_TESTS_ADAPTOR(tyname, ty) \
_C4_CALL_LIST_TESTS_FOR_ALL_SIZE_TYPES(split_fwd_list, split_fwd_list, tyname, ty)

C4_END_NAMESPACE(c4)

#endif // _C4_LIST_TEST_HPP
