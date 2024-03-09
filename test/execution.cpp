// Date: Wed Jan 24 21:57:06 2024
// Mail: lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include <execution>
#include <variant>

int main() {
    std::variant<int, double, char> a('c');
    char c = std::get<char>(a);
}
