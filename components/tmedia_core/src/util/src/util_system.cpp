/*
 * Copyright (C) 2021-2023 Alibaba Group Holding Limited
 */

#ifdef __linux__
#include <iostream>
#include <cstring>

#include <unistd.h>
#include <sys/stat.h>

#include <cxxabi.h>
#include <execinfo.h>
#include <elfutils/libdwfl.h>

#include <tmedia_core/util/util_system.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

using namespace std;
#define MAX_PATH_LEN 1024

const string TMUtilSystem::GetCurExePath()
{
    char *p = NULL;
    char arr_tmp[MAX_PATH_LEN] = {0};

#ifdef __APPLE__
    uint32_t bufLen = len;
    if (_NSGetExecutablePath(arr_tmp, &bufLen) != 0) {
        // Buffer size is too small.
        return std::string("");
    }
#endif

    readlink("/proc/self/exe", arr_tmp, MAX_PATH_LEN);
    if (NULL != (p = strrchr(arr_tmp,'/')))
        *p = '\0';
    else
        return std::string("");

    return std::string(arr_tmp);
}

const string TMUtilSystem::GetCurExeName()
{
    char *p = NULL;
    char arr_tmp[MAX_PATH_LEN] = {0};


#ifdef __APPLE__
    uint32_t bufLen = len;
    if (_NSGetExecutablePath(arr_tmp, &bufLen) != 0) {
        return std::string("");
    }
#else
    readlink("/proc/self/exe", arr_tmp, MAX_PATH_LEN);
#endif

    if (NULL != (p = strrchr(arr_tmp,'/')))
        return std::string(p+1);
    else
        return std::string("");
}

int TMUtilSystem::CreateDirectory(const char *path)
{
    uint32_t dirPathLen = strlen(path);
    if (dirPathLen > MAX_PATH_LEN)
    {
        return -1;
    }
    char tmpDirPath[MAX_PATH_LEN] = { 0 };
    for (uint32_t i = 0; i < dirPathLen; ++i)
    {
        tmpDirPath[i] = path[i];
        if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/')
        {
            if (access(tmpDirPath, 0) != 0)
            {
                int32_t ret = mkdir(tmpDirPath, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH);
                if (ret != 0)
                {
                    return ret;
                }
            }
        }
    }
    return 0;
}

void TMUtilSystem::PrintBackTrace()
{
    void* stack[512];
    int stack_size = backtrace(stack, sizeof(stack) / sizeof(*stack));
    std::cout << "Stack trace of " << stack_size << " frames:" << std::endl;
    for(int i = 0; i < stack_size; i++) {
        std::cout << i << "# ";
        PrintBackTraceByPC(stack[i]);
        std::cout << std::endl;
    }
}

const std::string TMUtilSystem::Demangle(const char* func_name)
{
    int status           = -1;
    char* func_real_name = abi::__cxa_demangle(func_name, 0, 0, &status);
    return (status == 0) ? func_real_name : func_name;
}

void TMUtilSystem::PrintBackTraceByPC(const void *pc)
{
    char* debuginfo_path = NULL;
    Dwfl_Callbacks callbacks = {
        .find_elf       = dwfl_linux_proc_find_elf,
        .find_debuginfo = dwfl_standard_find_debuginfo,
        .section_address= NULL,
        .debuginfo_path = &debuginfo_path,
    };

    Dwfl* dwfl = dwfl_begin(&callbacks);
    if (dwfl) {
        int ret1 = dwfl_linux_proc_report(dwfl, getpid());
        int ret2 = dwfl_report_end(dwfl, NULL, NULL);
        if ((ret1==0) && (ret2==0)) {
            Dwarf_Addr addr = (uintptr_t)pc;
            Dwfl_Module* module = dwfl_addrmodule(dwfl, addr);
            if (module) {
                const char* func_name = dwfl_module_addrname(module, addr);
                std::cout << std::hex << "0x" << addr << std::dec << " " << (func_name ? Demangle(func_name) : "<unknown>") << " ";

                Dwfl_Line* line = dwfl_getsrc(dwfl, addr);
                if (line) {
                    int nline;
                    Dwarf_Addr addr;
                    const char* file_name = dwfl_lineinfo(line, &addr, &nline, NULL, NULL, NULL);
                    std::cout << file_name << ":" << nline;
                } else {
                    std::cout << "<unknown>";
                }
            }
        }
        dwfl_end(dwfl);
    }
}

#endif
