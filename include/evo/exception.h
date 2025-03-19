/********************************************
 > File Name       : exception.h
 > Author          : lunar
 > Email           : lunar_ubuntu@qq.com
 > Created Time    : Thu May 18 22:05:47 2023
 > Copyright@ https://github.com/xiaoqixian
********************************************/

#ifndef _EXCEPTION_H
#define _EXCEPTION_H

namespace evo {

class exception {
public:
    exception() noexcept = default;
    exception(exception const&) noexcept = default; 
    virtual const char* what() const noexcept = 0;
};

class logic_error: public exception {
public:
    //TODO logic_error constructor accepts string
    explicit logic_error(const char*);
    logic_error() noexcept;
    logic_error(logic_error const&) noexcept;
    virtual const char* what() const noexcept;
};

class length_error: public logic_error {
public:
    inline explicit length_error(const char* s): logic_error(s) {}
    //TODO length_error constructor accepts string

    length_error(length_error const&) noexcept = default;
};

}

#endif 
