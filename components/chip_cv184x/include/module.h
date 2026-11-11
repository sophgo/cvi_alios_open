/**
 * @file module.h
 * @brief Static driver function registration and one-click execution
 *
 * Usage:
 *   1. Define a driver function:  int my_driver_init(void) { ... }
 *   2. Register it statically:    ALIOS_MODULE_REGISTER(my_driver_init);
 *   3. Run all registered:        module_init();
 */

#ifndef __MODULE_H__
#define __MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define SECTION(x) __attribute__((section(x)))
#define USED       __attribute__((used))

typedef int (*driver_func)(void);

/**
 * @brief Statically register a driver function into ModuleDriver section.
 *        All registered functions will be executed by cvi_module_init().
 */
#define ALIOS_MODULE_REGISTER(func) \
    USED const driver_func __module_drv_##func SECTION("ModuleDriver") = func

/** Begin marker of the ModuleDriver section (defined in linker script) */
extern const driver_func _module_driver_begin[];

/** End marker of the ModuleDriver section (defined in linker script) */
extern const driver_func _module_driver_end[];

/**
 * @brief Run all driver functions registered via ALIOS_MODULE_REGISTER.
 */
void module_init(void);

#ifdef __cplusplus
}
#endif

#endif /* CVI_MODULE_H */
