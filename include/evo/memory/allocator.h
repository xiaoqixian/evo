/********************************************
 > File Name       : allocator.h
 > Author          : lunar
 > Email           : lunar_ubuntu@qq.com
 > Created Time    : Thu May 25 16:50:12 2023
 > Copyright@ https://github.com/xiaoqixian
********************************************/

#pragma once

#include <memory>
namespace evo {

template <typename T>
using allocator = std::allocator<T>;

} // namespace evo
