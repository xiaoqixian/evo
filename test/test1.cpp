// Date: Sat Nov 18 11:57:58 2023
// Mail: lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "expected.hpp"
#include "type_traits.h"
#include <iostream>
#include <type_traits>

template <typename T, typename... Args, 
    decltype(T(declval<Args>()...))>
static evo::true_type test_dc(int);

template <typename T, typename... Args>
static evo::false_type test_dc(...);

struct My {
    //explicit My() noexcept = delete;
    explicit My() noexcept = default;
    explicit My(My const& other) noexcept = default;
};

void test() {
    bool is_r = evo::is_default_constructible<My>::value;
    std::cout << is_r << std::endl;
}

int main() {
    test();
    bool a = std::is_constructible<void>::value;
    auto e = evo::expected<My, void>();
    auto c = evo::expected<My, void>(e);
}
