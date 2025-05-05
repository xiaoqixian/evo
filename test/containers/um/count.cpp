// Date:   Fri May 02 14:36:29 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "evo/unordered_map"
#include <string>
#include <gtest/gtest.h>

TEST(UnorderedMapTest, Count) {
    {
        typedef evo::unordered_map<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        const C c(std::begin(a), std::end(a));
        assert(c.count(30) == 1);
        assert(c.count(5) == 0);
    }

    // {
    //     typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
    //                         min_allocator<std::pair<const int, std::string>>> C;
    //     typedef std::pair<int, std::string> P;
    //     P a[] =
    //     {
    //         P(10, "ten"),
    //         P(20, "twenty"),
    //         P(30, "thirty"),
    //         P(40, "forty"),
    //         P(50, "fifty"),
    //         P(60, "sixty"),
    //         P(70, "seventy"),
    //         P(80, "eighty"),
    //     };
    //     const C c(std::begin(a), std::end(a));
    //     assert(c.count(30) == 1);
    //     assert(c.count(5) == 0);
    // }
}

