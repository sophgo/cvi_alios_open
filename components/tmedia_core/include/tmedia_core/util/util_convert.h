/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#pragma once


using namespace std;

template<typename E>
inline constexpr auto SafeEnumVal(E e) -> typename std::underlying_type<E>::type
{
   return static_cast<typename std::underlying_type<E>::type>(e);
}

