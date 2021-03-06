#ifndef _C4_STORAGE_GROWTH_HPP_
#define _C4_STORAGE_GROWTH_HPP_

/** @file growth.hpp Storage growth policies for dynamic containers. */

#include "c4/config.hpp"
#include "c4/error.hpp"

C4_BEGIN_NAMESPACE(c4)
C4_BEGIN_NAMESPACE(stg)

/** @defgroup storage_growth_policies Storage growth policies
 * These are policies which can be used in dynamic growth containers. */

//-----------------------------------------------------------------------------
/** Grow by the least possible amount.
 * @ingroup storage_growth_policies */
struct growth_least
{
    C4_ALWAYS_INLINE static size_t next_size(size_t /*elm_size*/, size_t /*curr*/, size_t at_least) noexcept
    {
        return at_least;
    }
};
/** growth_least with watermark.
 * @ingroup storage_growth_policies */
struct growth_least_wm
{
    C4_ALWAYS_INLINE static size_t next_size(size_t /*elm_size*/, size_t curr, size_t at_least) noexcept
    {
        if(at_least < curr) return curr;
        return at_least;
    }
};


//-----------------------------------------------------------------------------
/** Grow to the double of the current size if it is bigger than at_least;
 * if not, then just to at_least.
 * @ingroup storage_growth_policies  */
struct growth_two
{
    C4_ALWAYS_INLINE static size_t next_size(size_t /*elm_size*/, size_t curr, size_t at_least) noexcept
    {
        size_t nxt = (curr << 1);
        return nxt > at_least ? nxt : at_least;
    }
};
/** growth_two with watermark.
 * @ingroup storage_growth_policies  */
struct growth_two_wm
{
    C4_ALWAYS_INLINE static size_t next_size(size_t elm_size, size_t curr, size_t at_least) noexcept
    {
        size_t ns = growth_two::next_size(elm_size, curr, at_least);
        if(ns <= curr) return curr;
        return ns;
    }
};


//-----------------------------------------------------------------------------
/** Grow by the Fibonacci ratio if the result is bigger than at_least;
 * if not, then just to at_least.
 * @warning do not use a number larger than phi; see http://lokiastari.com/blog/2016/03/25/resizemaths/
 * @ingroup storage_growth_policies  */
struct growth_phi
{
    C4_ALWAYS_INLINE static size_t next_size(size_t /*elm_size*/, size_t curr, size_t at_least) noexcept
    {
        if(at_least <= curr) return curr;
        size_t nxt = size_t(float(curr) * 1.618f);
        nxt = nxt > 0 ? nxt : 1;
        return nxt > at_least ? nxt : at_least;
    }
};
/** growth_phi with watermark.
 * @ingroup storage_growth_policies  */
struct growth_phi_wm
{
    C4_ALWAYS_INLINE static size_t next_size(size_t elm_size, size_t curr, size_t at_least) noexcept
    {
        size_t ns = growth_phi::next_size(elm_size, curr, at_least);
        if(ns <= curr) return curr;
        return ns;
    }
};


//-----------------------------------------------------------------------------
/** Grow another growth policy in fixed chunk sizes; useful for SIMD buffers. The chunk size must be a power of two.
 * @ingroup storage_growth_policies  */
template< class GrowthPolicy, size_t PowerOfTwoChunkSize >
struct growth_chunks
{
    C4_STATIC_ASSERT(PowerOfTwoChunkSize > 1);
    C4_STATIC_ASSERT_MSG((PowerOfTwoChunkSize & (PowerOfTwoChunkSize - 1)) == 0, "chunk size must be a power of two");

    using growth_policy = GrowthPolicy;
    enum { chunk_size = PowerOfTwoChunkSize };

    C4_ALWAYS_INLINE static size_t next_size(size_t /*elm_size*/, size_t curr, size_t at_least) noexcept
    {
        size_t next = GrowthPolicy::next_size(curr, at_least);
        size_t rem = (next & (PowerOfTwoChunkSize - 1));
        next += rem ? PowerOfTwoChunkSize - rem : 0;
        C4_ASSERT((next % PowerOfTwoChunkSize) == 0);
        C4_ASSERT(next >= at_least);
        return next;
    }
};

/** growth_chunks with watermark.
 * @ingroup storage_growth_policies  */
template< class GrowthPolicy, size_t PowerOfTwoChunkSize >
struct growth_chunks_wm
{
    C4_ALWAYS_INLINE static size_t next_size(size_t elm_size, size_t curr, size_t at_least) noexcept
    {
        size_t ns = growth_chunks< GrowthPolicy, PowerOfTwoChunkSize >::next_size(elm_size, curr, at_least);
        if(ns <= curr) return curr;
        return ns;
    }
};


//-----------------------------------------------------------------------------
/** Grow by powers of 2 until a threshold of 1024 bytes, then Fibonacci (golden) ratio
 * @ingroup storage_growth_policies */
struct growth_default
{
    enum { threshold = 1024 };
    C4_ALWAYS_INLINE static size_t next_size(size_t elm_size, size_t curr, size_t at_least) noexcept
    {
        if(at_least*elm_size <= threshold)
            return growth_two_wm::next_size(elm_size, curr, at_least);
        else
            return growth_phi_wm::next_size(elm_size, curr, at_least);
    }
};


//-----------------------------------------------------------------------------
/** @ingroup storage_growth_policies */
template< class SmallGrowthPolicy, class LargeGrowthPolicy, size_t BytesThreshold >
struct growth_composed
{
    using small_policy = SmallGrowthPolicy;
    using large_policy = LargeGrowthPolicy;
    enum { threshold = BytesThreshold };

    C4_ALWAYS_INLINE static size_t next_size(size_t elm_size, size_t curr, size_t at_least) noexcept
    {
        if(at_least*elm_size <= BytesThreshold)
            return SmallGrowthPolicy::next_size(elm_size, curr, at_least);
        else
            return LargeGrowthPolicy::next_size(elm_size, curr, at_least);
    }
};

/** growth_composed with watermark.
 * @ingroup storage_growth_policies  */
template< class SmallGrowthPolicy, class LargeGrowthPolicy, size_t BytesThreshold >
struct growth_composed_wm
{
    C4_ALWAYS_INLINE static size_t next_size(size_t elm_size, size_t curr, size_t at_least) noexcept
    {
        using gc = growth_composed< SmallGrowthPolicy, LargeGrowthPolicy, BytesThreshold >;
        size_t ns = gc::next_size(elm_size, curr, at_least);
        if(ns <= curr) return curr;
        return ns;
    }
};


//-----------------------------------------------------------------------------
/** @ingroup storage_growth_policies */
template< class GrowthPolicy >
struct growth_watermark
{
    C4_ALWAYS_INLINE static size_t next_size(size_t elm_size, size_t curr, size_t at_least) noexcept
    {
        size_t ns = GrowthPolicy::next_size(elm_size, curr, at_least);
        if(ns <= curr) return curr;
        return ns;
    }
};

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)

#endif // _C4_STORAGE_GROWTH_HPP_
