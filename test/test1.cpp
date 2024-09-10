// Date: Sat Nov 18 11:57:58 2023
// Mail: lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "memory/compressed_pair.hpp"
#include <cstdio>
#include <memory.h>
#include <memory>

struct A {
    ~A() {
        printf("A is destructed\n");
    }
};

struct B {
};

struct C: A, B {
};

int main() {
    printf("%zd\n", sizeof(A));
    printf("%zd\n", sizeof(C));
    printf("%zd\n", sizeof(evo::compressed_pair<A, B>));
    // auto sc = evo::make_shared<C>(C{});
    // printf("%d\n", evo::is_convertible_v<C*, A*>);
}
