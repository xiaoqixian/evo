/**********************************************
  > File Name		: declval.h
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Fri Apr 14 15:38:12 2023
  > Location        : Shanghai
  > Copyright@ https://github.com/xiaoqixian
 **********************************************/
#ifndef _DECLVAL_H
#define _DECLVAL_H

namespace evo {

/*
 * declval 通常用于获取可能没有构造器的类的成员函数的返回值类型推导
 */
template <typename T>
T&& _declval(int);

template <typename T>
decltype(_declval<T>(0)) declval();

}

#endif /* _DECLVAL_H*/
