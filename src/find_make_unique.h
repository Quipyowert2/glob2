#pragma once
#ifndef INCLUDED_FIND_MAKE_UNIQUE_HPP
#define INCLUDED_FIND_MAKE_UNIQUE_HPP

#if __cplusplus >= 201402L
#include <memory>
using std::make_unique;
#else
#if BOOST_VERSION >= 107500
#include <boost/smart_ptr/make_unique.hpp>
#elif BOOST_VERSION >= 106300
#include <boost/make_unique.hpp>
#elif BOOST_VERSION >= 105700
#include <boost/move/make_unique.hpp>
#else
#error "Can't make_unique when there's no Boost and C++ standard is earlier than C++14"
#endif
using boost::make_unique;
#endif // __cplusplus

#endif