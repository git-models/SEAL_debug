// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "seal/memorymanager.h"
#include "seal/util/iterator.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include "gtest/gtest.h"

using namespace seal;
using namespace seal::util;
using namespace std;

namespace sealtest
{
    namespace util
    {
        TEST(IteratorTest, CoeffIterator)
        {
            ASSERT_FALSE(CoeffIterator::value_type_is_seal_iterator_type::value);
            ASSERT_FALSE(ConstCoeffIterator::value_type_is_seal_iterator_type::value);

            array<uint64_t, 10> arr{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            CoeffIterator ci(arr.data());
            ConstCoeffIterator cci(arr.data());

            for_each(arr.begin(), arr.end(), [ci](auto a) mutable { ASSERT_EQ(a, **ci++); });
            for_each(arr.begin(), arr.end(), [cci](auto a) mutable { ASSERT_EQ(a, **cci++); });

            ASSERT_EQ(arr.data(), static_cast<uint64_t *>(ci));
            ASSERT_EQ(arr.data(), static_cast<const uint64_t *>(cci));

            auto ci2 = ci++;
            auto cci2 = cci++;
            ASSERT_EQ(arr[1], **ci);
            ASSERT_EQ(arr[1], **cci);
            ASSERT_EQ(arr[0], **ci2);
            ASSERT_EQ(arr[0], **cci2);

            ci2 = ++ci;
            cci2 = ++cci;
            ASSERT_EQ(arr[2], **ci);
            ASSERT_EQ(arr[2], **cci);
            ASSERT_EQ(arr[2], **ci2);
            ASSERT_EQ(arr[2], **cci2);

            ASSERT_TRUE(ci == ci2);
            ASSERT_TRUE(cci == cci2);
        }

        TEST(IteratorTest, RNSIterator)
        {
            ASSERT_TRUE(RNSIterator::value_type_is_seal_iterator_type::value);
            ASSERT_TRUE(ConstRNSIterator::value_type_is_seal_iterator_type::value);

            array<uint64_t, 12> arr{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
            RNSIterator ri(arr.data(), 4);
            ConstRNSIterator cri(arr.data(), 4);

            {
                vector<uint64_t> values;
                for_each_n(ri, arr.size() / ri.poly_modulus_degree(), [&](auto ci) {
                    for_each_n(ci, ri.poly_modulus_degree(), [&](auto c) { values.push_back(*c); });
                });
                ASSERT_TRUE(equal(arr.begin(), arr.end(), values.begin()));
            }
            {
                vector<uint64_t> values;
                for_each_n(cri, arr.size() / cri.poly_modulus_degree(), [&](auto cci) {
                    for_each_n(cci, cri.poly_modulus_degree(), [&](auto c) { values.push_back(*c); });
                });
                ASSERT_TRUE(equal(arr.begin(), arr.end(), values.begin()));
            }

            ASSERT_EQ(arr[0], ***ri++);
            ASSERT_EQ(arr[4], ***ri++);
            ASSERT_EQ(arr[8], ***ri);
            ASSERT_EQ(arr[0], ***cri++);
            ASSERT_EQ(arr[4], ***cri++);
            ASSERT_EQ(arr[8], ***cri);

            ASSERT_EQ(arr.data() + 8, static_cast<uint64_t *>(ri));
            ASSERT_EQ(arr.data() + 8, static_cast<const uint64_t *>(cri));

            ASSERT_EQ(arr[4], ***--ri);
            ASSERT_EQ(arr[0], ***--ri);
            ASSERT_EQ(arr[4], ***--cri);
            ASSERT_EQ(arr[0], ***--cri);

            ASSERT_TRUE(ri == cri);
            ASSERT_TRUE(ri == arr.data());
        }

        TEST(IteratorTest, PolyIterator)
        {
            ASSERT_TRUE(PolyIterator::value_type_is_seal_iterator_type::value);
            ASSERT_TRUE(ConstPolyIterator::value_type_is_seal_iterator_type::value);

            array<uint64_t, 12> arr{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
            PolyIterator pi(arr.data(), 3, 2);
            ConstPolyIterator cpi(arr.data(), 3, 2);

            {
                vector<uint64_t> values;
                for_each_n(pi, arr.size() / (pi.poly_modulus_degree() * pi.coeff_modulus_count()), [&](auto ri) {
                    for_each_n(ri, pi.coeff_modulus_count(), [&](auto ci) {
                        for_each_n(ci, pi.poly_modulus_degree(), [&](auto c) { values.push_back(*c); });
                    });
                });
                ASSERT_TRUE(equal(arr.begin(), arr.end(), values.begin()));
            }
            {
                vector<uint64_t> values;
                for_each_n(cpi, arr.size() / (cpi.poly_modulus_degree() * cpi.coeff_modulus_count()), [&](auto cri) {
                    for_each_n(cri, cpi.coeff_modulus_count(), [&](auto cci) {
                        for_each_n(cci, cpi.poly_modulus_degree(), [&](auto c) { values.push_back(*c); });
                    });
                });
                ASSERT_TRUE(equal(arr.begin(), arr.end(), values.begin()));
            }

            ASSERT_EQ(arr[0], ****pi++);
            ASSERT_EQ(arr[6], ****pi++);
            ASSERT_EQ(arr[0], ****cpi++);
            ASSERT_EQ(arr[6], ****cpi++);

            ASSERT_EQ(arr[6], ****--pi);
            ASSERT_EQ(arr[0], ****--pi);
            ASSERT_EQ(arr[6], ****--cpi);
            ASSERT_EQ(arr[0], ****--cpi);

            ASSERT_TRUE(pi == cpi);
            ASSERT_TRUE(pi == arr.data());
        }

        TEST(IteratorTest, IteratorWrapper)
        {
            ASSERT_FALSE(IteratorWrapper<int>::value_type_is_seal_iterator_type::value);

            array<int32_t, 5> int_arr{ 0, 1, 2, 3, 4 };
            array<char, 5> char_arr{ 'a', 'b', 'c', 'd', 'e' };

            IteratorWrapper<int32_t *> int_iter(int_arr.data());
            IteratorWrapper<char *> char_iter(char_arr.data());

            {
                vector<int32_t> values;
                for_each_n(int_iter, int_arr.size(), [&](auto int_ptr) { values.push_back(*int_ptr); });
                ASSERT_TRUE(equal(int_arr.begin(), int_arr.end(), values.begin()));
            }
            {
                vector<char> values;
                for_each_n(char_iter, char_arr.size(), [&](auto char_ptr) { values.push_back(*char_ptr); });
                ASSERT_TRUE(equal(char_arr.begin(), char_arr.end(), values.begin()));
            }

            ASSERT_EQ(*int_iter, int_arr.data());
            ASSERT_EQ(*char_iter, char_arr.data());
        }

        TEST(IteratorTest, ReverseIterator)
        {
            ASSERT_FALSE(ReverseIterator<CoeffIterator>::value_type_is_seal_iterator_type::value);
            ASSERT_TRUE(ReverseIterator<RNSIterator>::value_type_is_seal_iterator_type::value);

            array<uint64_t, 10> arr{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            CoeffIterator ci(arr.data());
            advance(ci, arr.size() - 1);
            ReverseIterator<CoeffIterator> rci(ci);
            ConstCoeffIterator cci(arr.data());
            advance(cci, arr.size() - 1);
            ReverseIterator<ConstCoeffIterator> rcci(cci);

            for_each(arr.rbegin(), arr.rend(), [rci](auto a) mutable { ASSERT_EQ(a, **rci++); });
            for_each(arr.rbegin(), arr.rend(), [rcci](auto a) mutable { ASSERT_EQ(a, **rcci++); });

            ASSERT_EQ(arr.data() + arr.size() - 1, static_cast<uint64_t *>(rci));
            ASSERT_EQ(arr.data() + arr.size() - 1, static_cast<const uint64_t *>(rcci));

            ++rci;
            ASSERT_EQ(**rci, arr[8]);
            --rci;
            ASSERT_EQ(**rci, arr[9]);
            rci++;
            ASSERT_EQ(**rci, arr[8]);
            rci--;
            ASSERT_EQ(**rci, arr[9]);

            array<int32_t, 5> int_arr{ 0, 1, 2, 3, 4 };
            array<char, 5> char_arr{ 'a', 'b', 'c', 'd', 'e' };

            IteratorWrapper<int32_t *> int_iter(int_arr.data() + int_arr.size() - 1);
            IteratorWrapper<char *> char_iter(char_arr.data() + char_arr.size() - 1);

            {
                vector<int32_t> values;
                for_each_n(ReverseIterator<IteratorWrapper<int32_t *>>(int_iter), int_arr.size(), [&](auto int_ptr) {
                    values.push_back(*int_ptr);
                });

                auto values_it = values.begin();
                for_each(
                    int_arr.rbegin(), int_arr.rend(), [&values_it](auto a) mutable { ASSERT_EQ(a, *values_it++); });
            }
            {
                vector<char> values;
                for_each_n(ReverseIterator<IteratorWrapper<char *>>(char_iter), char_arr.size(), [&](auto char_ptr) {
                    values.push_back(*char_ptr);
                });

                auto values_it = values.begin();
                for_each(
                    char_arr.rbegin(), char_arr.rend(), [&values_it](auto a) mutable { ASSERT_EQ(a, *values_it++); });
            }
        }

        TEST(IteratorTest, IteratorTuple)
        {
            array<uint64_t, 12> arr{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
            CoeffIterator ci(arr.data());
            RNSIterator ri(arr.data(), 3);
            PolyIterator pi(arr.data(), 3, 2);

            IteratorTuple<CoeffIterator, PolyIterator> it1(ci, pi);
            IteratorTuple<RNSIterator, PolyIterator> it2(ri, pi);
            ASSERT_TRUE(decltype(it1)::value_type_is_seal_iterator_type::value);
            ASSERT_TRUE(decltype(it2)::value_type_is_seal_iterator_type::value);

            ASSERT_EQ(0, **get<0>(*it1));
            ASSERT_EQ(0, ***get<1>(*it1));
            ASSERT_EQ(0, **get<0>(*it2));
            ASSERT_EQ(0, ***get<1>(*it2));

            ++it1;
            ++it2;
            ASSERT_EQ(1, **get<0>(*it1));
            ASSERT_EQ(6, ***get<1>(*it1));
            ASSERT_EQ(3, **get<0>(*it2));
            ASSERT_EQ(6, ***get<1>(*it2));

            --it1;
            --it2;
            ASSERT_EQ(0, **get<0>(*it1));
            ASSERT_EQ(0, ***get<1>(*it1));
            ASSERT_EQ(0, **get<0>(*it2));
            ASSERT_EQ(0, ***get<1>(*it2));

            IteratorTuple<CoeffIterator, RNSIterator, PolyIterator> it3(ci, ri, pi);
            ASSERT_TRUE(decltype(it3)::value_type_is_seal_iterator_type::value);

            ASSERT_EQ(0, **get<0>(*it3));
            ASSERT_EQ(0, **get<1>(*it3));
            ASSERT_EQ(0, ***get<2>(*it3));

            ++it3;
            ASSERT_EQ(1, **get<0>(*it3));
            ASSERT_EQ(3, **get<1>(*it3));
            ASSERT_EQ(6, ***get<2>(*it3));

            --it3;
            ASSERT_EQ(0, **get<0>(*it3));
            ASSERT_EQ(0, **get<1>(*it3));
            ASSERT_EQ(0, ***get<2>(*it3));

            IteratorTuple<
                IteratorTuple<CoeffIterator, RNSIterator, PolyIterator>, IteratorTuple<RNSIterator, PolyIterator>>
                it4(it3, it2);
            ASSERT_TRUE(decltype(it4)::value_type_is_seal_iterator_type::value);
            auto it5 = it4;
            it5++;
            it5--;
            ASSERT_TRUE(it5 == it4);
        }
    } // namespace util
} // namespace sealtest
