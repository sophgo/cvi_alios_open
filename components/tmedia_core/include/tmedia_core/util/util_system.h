/*
 * Copyright (C) 2021 Alibaba Group Holding Limited
 */

#pragma once

#include <string>

using namespace std;

class TMUtilSystem
{
public:
    static const string GetCurExePath();
    static const string GetCurExeName();

    static void PrintBackTrace();

private:
    static const string Demangle(const char* func_name);
    static void PrintBackTraceByPC(const void *pc);
};
