#ifndef _PLAT_MBEDTLS_CONFIG_H_
#define _PLAT_MBEDTLS_CONFIG_H_

#ifdef HAVE_MBEDTLS_USER_CONFIG
#include "config_mbedtls_user.h"
#else
#if defined(CONFIG_CLOUDIO_ALICOAP) && defined(CONFIG_CLOUDIO_ALIMQTT)
    #include "config_yoc_aliiot.h"
#elif defined(CONFIG_CLOUDIO_ALICOAP)
    #include "config_yoc_alicoap.h"
#elif defined(CONFIG_CLOUDIO_ALIMQTT)
    #include "config_yoc_alimqtt.h"
#elif defined(CONFIG_USING_HTTP2)
    #include "config_yoc_http2.h"
#elif defined(CONFIG_USING_TLS)
    #include "config_yoc_tls.h"
#else
    #include "config_yoc_base.h"
#endif

#endif

#endif /* _PLAT_MBEDTLS_CONFIG_H_ */