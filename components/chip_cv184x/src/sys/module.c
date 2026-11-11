#include "module.h"


void module_init(void)
{
    const driver_func *fn;

    for (fn = _module_driver_begin; fn < _module_driver_end; fn++) {
        if (*fn) {
            (*fn)();
        }
    }
}
