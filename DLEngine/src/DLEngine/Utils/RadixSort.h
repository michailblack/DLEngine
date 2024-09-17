#pragma once
#include <functional>
#include <vector>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

namespace DLEngine
{
#define PREFETCH 1

#if PREFETCH
#include <xmmintrin.h>	// for prefetch
#define pfval	64
#define pfval2	128
#define pf(x)	_mm_prefetch(reinterpret_cast<const char*>(x + i + pfval), 0)
#define pf2(x)	_mm_prefetch(reinterpret_cast<const char*>(x + i + pfval2), 0)
#else
#define pf(x)
#define pf2(x)
#endif

    namespace Utils
    {
        // ================================================================================================
        // flip a float for sorting
        //  finds SIGN of fp number.
        //  if it's 1 (negative float), it flips all bits
        //  if it's 0 (positive float), it flips the sign only
        // ================================================================================================
        __forceinline uint32_t FloatFlip(uint32_t f)
        {
            uint32_t mask = -int32_t(f >> 31) | 0x80000000;
            return f ^ mask;
        }

        __forceinline void FloatFlipX(uint32_t& f)
        {
            uint32_t mask = -int32_t(f >> 31) | 0x80000000;
            f ^= mask;
        }

        // ================================================================================================
        // flip a float back (invert FloatFlip)
        //  signed was flipped from above, so:
        //  if sign is 1 (negative), it flips the sign bit back
        //  if sign is 0 (positive), it flips all bits back
        // ================================================================================================
        __forceinline uint32_t IFloatFlip(uint32_t f)
        {
            uint32_t mask = ((f >> 31) - 1) | 0x80000000;
            return f ^ mask;
        }

        // ---- utils for accessing 11-bit quantities
#define _0(x)	(x & 0x7FF)
#define _1(x)	(x >> 11 & 0x7FF)
#define _2(x)	(x >> 22 )

#if 0
        template <typename T>
        void FloatRadixSort(const std::vector<T>& arr, std::vector<T>& sortedArr, const std::function<float(const T&)>& floatKeyExtractor)
        {
            const uint32_t elementsCount{ static_cast<uint32_t>(arr.size()) };
            sortedArr.reserve(elementsCount);
            sortedArr.clear();

            std::vector<uint32_t> sortedKeys(elementsCount);
            std::vector<uint32_t> keys(elementsCount);
            std::unordered_map<float, uint32_t> keyToIndexMap;
            keyToIndexMap.reserve(elementsCount);

            for (uint32_t i = 0; i < elementsCount; ++i)
            {
                float key{ floatKeyExtractor(arr[i]) };
         
                keyToIndexMap[key] = i;
                keys[i] = reinterpret_cast<uint32_t&>(key);
            }

            // 3 histograms on the stack:
            constexpr uint32_t kHist{ 2048u };
            uint32_t b0[kHist * 3u]{};

            uint32_t* b1{ b0 + kHist };
            uint32_t* b2{ b1 + kHist };

            memset(b0, 0u, kHist * 3u * sizeof(uint32_t));

            // 1. parallel histogramming pass
            //
            for (uint32_t i{ 0u }; i < elementsCount; ++i)
            {
                pf(keys.data());

                uint32_t fi{ Utils::FloatFlip(keys[i]) };

                b0[_0(fi)]++;
                b1[_1(fi)]++;
                b2[_2(fi)]++;
            }

            // 2. Sum the histograms -- each histogram entry records the number of values preceding itself.
            {
                uint32_t sum0{ 0u }, sum1{ 0u }, sum2{ 0u };
                uint32_t tsum;
                for (uint32_t i{ 0u }; i < kHist; ++i)
                {

                    tsum = b0[i] + sum0;
                    b0[i] = sum0 - 1;
                    sum0 = tsum;

                    tsum = b1[i] + sum1;
                    b1[i] = sum1 - 1;
                    sum1 = tsum;

                    tsum = b2[i] + sum2;
                    b2[i] = sum2 - 1;
                    sum2 = tsum;
                }
            }

            // byte 0: floatflip entire value, read/write histogram, write out flipped
            for (uint32_t i{ 0u }; i < elementsCount; ++i)
            {

                uint32_t fi{ keys[i] };
                Utils::FloatFlipX(fi);
                uint32_t pos{ _0(fi) };

                pf2(keys.data());
                sortedKeys[++b0[pos]] = fi;
            }

            // byte 1: read/write histogram, copy
            //   sorted -> array
            for (uint32_t i{ 0u }; i < elementsCount; ++i)
            {
                uint32_t si{ sortedKeys[i] };
                uint32_t pos{ _1(si) };
            
                pf2(sortedKeys.data());
                keys[++b1[pos]] = si;
            }

            // byte 2: read/write histogram, copy & flip out
            //   array -> sorted
            for (uint32_t i{ 0u }; i < elementsCount; ++i)
            {
                uint32_t ai{ keys[i] };
                uint32_t pos{ _2(ai) };

                pf2(keys.data());
                sortedKeys[++b2[pos]] = Utils::IFloatFlip(ai);
            }

            for (uint32_t i{ 0u }; i < elementsCount; ++i)
                sortedArr.push_back(arr[keyToIndexMap[reinterpret_cast<float&>(sortedKeys[i])]]);
        }
#endif
        static void RadixSort11(float* farray, float* sorted, uint32_t elements)
        {
            uint32_t i;
            uint32_t* sort = (uint32_t*)sorted;
            uint32_t* array = (uint32_t*)farray;

            // 3 histograms on the stack:
            const uint32_t kHist = 2048;
            uint32_t b0[kHist * 3];

            uint32_t* b1 = b0 + kHist;
            uint32_t* b2 = b1 + kHist;

            for (i = 0; i < kHist * 3; i++) {
                b0[i] = 0;
            }
            //memset(b0, 0, kHist * 12);

            // 1.  parallel histogramming pass
            //
            for (i = 0; i < elements; i++) {

                pf(array);

                uint32_t fi = FloatFlip((uint32_t&)array[i]);

                b0[_0(fi)]++;
                b1[_1(fi)]++;
                b2[_2(fi)]++;
            }

            // 2.  Sum the histograms -- each histogram entry records the number of values preceding itself.
            {
                uint32_t sum0 = 0, sum1 = 0, sum2 = 0;
                uint32_t tsum;
                for (i = 0; i < kHist; i++) {

                    tsum = b0[i] + sum0;
                    b0[i] = sum0 - 1;
                    sum0 = tsum;

                    tsum = b1[i] + sum1;
                    b1[i] = sum1 - 1;
                    sum1 = tsum;

                    tsum = b2[i] + sum2;
                    b2[i] = sum2 - 1;
                    sum2 = tsum;
                }
            }

            // byte 0: floatflip entire value, read/write histogram, write out flipped
            for (i = 0; i < elements; i++) {

                uint32_t fi = array[i];
                FloatFlipX(fi);
                uint32_t pos = _0(fi);

                pf2(array);
                sort[++b0[pos]] = fi;
            }

            // byte 1: read/write histogram, copy
            //   sorted -> array
            for (i = 0; i < elements; i++) {
                uint32_t si = sort[i];
                uint32_t pos = _1(si);
                pf2(sort);
                array[++b1[pos]] = si;
            }

            // byte 2: read/write histogram, copy & flip out
            //   array -> sorted
            for (i = 0; i < elements; i++) {
                uint32_t ai = array[i];
                uint32_t pos = _2(ai);

                pf2(array);
                sort[++b2[pos]] = IFloatFlip(ai);
            }

            // to write original:
            memcpy(array, sorted, elements * 4);
        }
    }
}