// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "c4/sstream.hpp"
#include "c4/string.hpp"
#include "c4/type_name.hpp"
#include <iostream>

#include "c4/libtest/supprwarn_push.hpp"
#include "c4/test.hpp"
#include "c4/libtest/archetypes.hpp"

C4_BEGIN_NAMESPACE(c4)

//-----------------------------------------------------------------------------
template <class T>
struct RoundTripTest_c4string : public ::testing::Test
{
    sstream< c4::string > ss;
};
template <class T>
struct RoundTripTest_c4wstring : public ::testing::Test
{
    sstream< c4::wstring > ss;
};
template <class T>
struct RoundTripTest_c4substring : public ::testing::Test
{
    c4::string str;
    sstream< c4::substring > ss;
    RoundTripTest_c4substring() : str(512), ss(str.data(), str.size())
    {
    }
};
template <class T>
struct RoundTripTest_c4wsubstring : public ::testing::Test
{
    c4::wstring str;
    sstream< c4::wsubstring > ss;
    RoundTripTest_c4wsubstring() : str(512), ss(str.data(), str.size())
    {
    }
};
template <class T>
struct RoundTripTest_stdstring : public ::testing::Test
{
    sstream< std::string > ss;
};
template <class T>
struct RoundTripTest_stdwstring : public ::testing::Test
{
    sstream< std::wstring > ss;
};
TYPED_TEST_CASE_P(RoundTripTest_c4string);
TYPED_TEST_CASE_P(RoundTripTest_c4wstring);
TYPED_TEST_CASE_P(RoundTripTest_c4substring);
TYPED_TEST_CASE_P(RoundTripTest_c4wsubstring);
TYPED_TEST_CASE_P(RoundTripTest_stdstring);
TYPED_TEST_CASE_P(RoundTripTest_stdwstring);

//-----------------------------------------------------------------------------
#define _testrtrip3(which, strtype)                                 \
                                                                    \
    which<strtype, TypeParam>(this->ss, 65, 66, 67);                \
    this->ss.reset();                                               \
                                                                    \
    which<strtype, TypeParam>(this->ss, 97, 98, 99);                \
    this->ss.reset();                                               \
                                                                    \
    which<strtype, TypeParam>(this->ss, 123, 124, 125);             \
    this->ss.reset();                                               \
                                                                    \
    which<strtype, archetypes::exvec3<TypeParam>>(this->ss, {65,66,67},{68,69,70},{71,72,73}); \
    this->ss.reset();

//-----------------------------------------------------------------------------

template< class String, class T >
void do_round_trip_chevron(sstream<String> &ss, T const& val1, T const& val2, T const& val3)
{
    C4_LOGP("roundtrip chevron AQUI 0: sz{} p={} g={} cap={}\n", ss.size(), ss.tellp(), ss.tellg(), ss.capacity());
    csubstring sn = c4::type_name< String >();
    csubstring tn = c4::type_name< T >();
    C4_LOGF("roundtrip chevron< %.*s, %.*s >: AQUI 1\n", (int)sn.size(), sn.data(), (int)tn.size(), tn.data());
    using char_type = typename sstream< String >::char_type;
    T v1, v2, v3;
    ss << val1 << char_type(' ') << val2 << char_type(' ') << val3;
    C4_LOGF("roundtrip chevron< %.*s, %.*s >: AQUI 2\n", (int)sn.size(), sn.data(), (int)tn.size(), tn.data());
    char_type c;
    ss >> v1 >> c >> v2 >> c >> v3;
    C4_LOGF("roundtrip chevron< %.*s, %.*s >: AQUI 3\n", (int)sn.size(), sn.data(), (int)tn.size(), tn.data());
    EXPECT_EQ(v1, val1) << C4_PRETTY_FUNC;
    EXPECT_EQ(v2, val2) << C4_PRETTY_FUNC;
    EXPECT_EQ(v3, val3) << C4_PRETTY_FUNC;
    C4_LOGF("roundtrip chevron< %.*s, %.*s >: AQUI 4\n", (int)sn.size(), sn.data(), (int)tn.size(), tn.data());
    C4_LOGF("roundtrip chevron< %.*s, %.*s >: AQUI 5 sz={} p={} g={} cap={}\n",
            (int)sn.size(), sn.data(), (int)tn.size(), tn.data(), ss.size(), ss.tellp(), ss.tellg(), ss.capacity());
}
TYPED_TEST_P(RoundTripTest_c4string, chevron)
{
    _testrtrip3(do_round_trip_chevron, c4::string);
}
TYPED_TEST_P(RoundTripTest_c4wstring, chevron)
{
    _testrtrip3(do_round_trip_chevron, c4::wstring);
}
TYPED_TEST_P(RoundTripTest_c4substring, chevron)
{
    _testrtrip3(do_round_trip_chevron, c4::substring);
}
TYPED_TEST_P(RoundTripTest_c4wsubstring, chevron)
{
    _testrtrip3(do_round_trip_chevron, c4::wsubstring);
}
TYPED_TEST_P(RoundTripTest_stdstring, chevron)
{
    _testrtrip3(do_round_trip_chevron, std::string);
}
TYPED_TEST_P(RoundTripTest_stdwstring, chevron)
{
    _testrtrip3(do_round_trip_chevron, std::wstring);
}

//-----------------------------------------------------------------------------
template< class String, class T >
void do_round_trip_printp(sstream<String> &ss, T const& val1, T const& val2, T const& val3)
{
    using char_type = typename sstream< String >::char_type;

    T v1, v2, v3;
    ss.printp(C4_TXTTY("{} {} {}", char_type), val1, val2, val3);
    ss.scanp(C4_TXTTY("{} {} {}", char_type), v1, v2, v3);
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);

    ss.reset(); v1 = v2 = v3 = {};
    ss.printp(C4_TXTTY("{} aaaaaaaaa {} bbbb {} ccc", char_type), val1, val2, val3);
    ss.scanp(C4_TXTTY("{} aaaaaaaaa {} bbbb {} ccc", char_type), v1, v2, v3);
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);

    ss.reset(); v1 = v2 = v3 = {};
    ss.printp(C4_TXTTY("{} aaaaaaaaa_{} bbbb_{} ccc", char_type), val1, val2, val3);
    ss.scanp(C4_TXTTY("{} aaaaaaaaa_{} bbbb_{} ccc", char_type), v1, v2, v3);
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);
}
TYPED_TEST_P(RoundTripTest_c4string, printp)
{
    _testrtrip3(do_round_trip_printp, c4::string);
}
TYPED_TEST_P(RoundTripTest_c4wstring, printp)
{
    _testrtrip3(do_round_trip_printp, c4::wstring);
}
TYPED_TEST_P(RoundTripTest_c4substring, printp)
{
    _testrtrip3(do_round_trip_printp, c4::substring);
}
TYPED_TEST_P(RoundTripTest_c4wsubstring, printp)
{
    _testrtrip3(do_round_trip_printp, c4::wsubstring);
}
TYPED_TEST_P(RoundTripTest_stdstring, printp)
{
    _testrtrip3(do_round_trip_printp, std::string);
}
TYPED_TEST_P(RoundTripTest_stdwstring, printp)
{
    _testrtrip3(do_round_trip_printp, std::wstring);
}

//-----------------------------------------------------------------------------
template< class String, class T >
void do_round_trip_cat(sstream<String> &ss, T const& val1, T const& val2, T const& val3)
{
    using char_type = typename sstream< String >::char_type;
    T v1, v2, v3;
    ss.cat(val1, char_type(' '), val2, char_type(' '), val3);
    char_type c;
    ss.uncat(v1, c, v2, c, v3);
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);
}
TYPED_TEST_P(RoundTripTest_c4string, cat)
{
    _testrtrip3(do_round_trip_cat, c4::string);
}
TYPED_TEST_P(RoundTripTest_c4wstring, cat)
{
    _testrtrip3(do_round_trip_cat, c4::wstring);
}
TYPED_TEST_P(RoundTripTest_c4substring, cat)
{
    _testrtrip3(do_round_trip_cat, c4::substring);
}
TYPED_TEST_P(RoundTripTest_c4wsubstring, cat)
{
    _testrtrip3(do_round_trip_cat, c4::wsubstring);
}
TYPED_TEST_P(RoundTripTest_stdstring, cat)
{
    _testrtrip3(do_round_trip_cat, std::string);
}
TYPED_TEST_P(RoundTripTest_stdwstring, cat)
{
    _testrtrip3(do_round_trip_cat, std::wstring);
}


//-----------------------------------------------------------------------------
template< class String, class T >
void do_round_trip_catsep(sstream<String> &ss, T const& val1, T const& val2, T const& val3)
{
    using char_type = typename sstream< String >::char_type;
    T v1, v2, v3;
    ss.catsep(char_type(' '), val1, val2, val3);
    ss.uncatsep(char_type(' '), v1, v2, v3);
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);
}
TYPED_TEST_P(RoundTripTest_c4string, catsep)
{
    _testrtrip3(do_round_trip_catsep, c4::string);
}
TYPED_TEST_P(RoundTripTest_c4wstring, catsep)
{
    _testrtrip3(do_round_trip_catsep, c4::wstring);
}
TYPED_TEST_P(RoundTripTest_c4substring, catsep)
{
    _testrtrip3(do_round_trip_catsep, c4::substring);
}
TYPED_TEST_P(RoundTripTest_c4wsubstring, catsep)
{
    _testrtrip3(do_round_trip_catsep, c4::wsubstring);
}
TYPED_TEST_P(RoundTripTest_stdstring, catsep)
{
    _testrtrip3(do_round_trip_catsep, std::string);
}
TYPED_TEST_P(RoundTripTest_stdwstring, catsep)
{
    _testrtrip3(do_round_trip_catsep, std::wstring);
}

REGISTER_TYPED_TEST_CASE_P(RoundTripTest_c4string,     chevron, printp, cat, catsep);
REGISTER_TYPED_TEST_CASE_P(RoundTripTest_c4wstring,    chevron, printp, cat, catsep);
REGISTER_TYPED_TEST_CASE_P(RoundTripTest_c4substring,  chevron, printp, cat, catsep);
REGISTER_TYPED_TEST_CASE_P(RoundTripTest_c4wsubstring, chevron, printp, cat, catsep);
REGISTER_TYPED_TEST_CASE_P(RoundTripTest_stdstring,    chevron, printp, cat, catsep);
REGISTER_TYPED_TEST_CASE_P(RoundTripTest_stdwstring,   chevron, printp, cat, catsep);

INSTANTIATE_TYPED_TEST_CASE_P(sstream, RoundTripTest_c4string,     c4::archetypes::scalars);
INSTANTIATE_TYPED_TEST_CASE_P(sstream, RoundTripTest_c4wstring,    c4::archetypes::scalars);
INSTANTIATE_TYPED_TEST_CASE_P(sstream, RoundTripTest_c4substring,  c4::archetypes::scalars);
INSTANTIATE_TYPED_TEST_CASE_P(sstream, RoundTripTest_c4wsubstring, c4::archetypes::scalars);
INSTANTIATE_TYPED_TEST_CASE_P(sstream, RoundTripTest_stdstring,    c4::archetypes::scalars);
INSTANTIATE_TYPED_TEST_CASE_P(sstream, RoundTripTest_stdwstring,   c4::archetypes::scalars);

C4_END_NAMESPACE(c4)

#include "c4/libtest/supprwarn_pop.hpp"
