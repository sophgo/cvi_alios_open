#include <string.h>
#include <stdio.h>
#include <aos/kernel.h>
#include <aos/cli.h>
#include "cvi_rpmsg.h"

#define CVI_RPMSG_TASK_STACK  8192

#define ECHO_TEST_EPT_NAME    "rpmsg-echo-test"
#define USER_EPT_NAME    "rpmsg-user"


static int cvi_rpmsg_echo_test_cb(cvi_rpmsg_ept_t *ept, const void *data,
    size_t len, uint32_t src, void *priv)
{
    (void)priv;

    if (!ept || !data || len == 0)
    return -1;

    /* Echo back to the actual sender (src).  Using cvi_rpmsg_sendto(src)
    * instead of cvi_rpmsg_send(dest_addr) keeps each sender's response
    * routed to the right endpoint — required when multiple Linux peers
    * talk to this one endpoint concurrently. */
    return cvi_rpmsg_sendto(ept, data, (int)len, src);
}

static int cvi_rpmsg_user_cb(cvi_rpmsg_ept_t *ept, const void *data,
    size_t len, uint32_t src, void *priv)
{
    (void)priv;

    if (!ept || !data || len == 0)
    return -1;

    /* Reply to the sender rather than a single cached dest_addr. */
    return cvi_rpmsg_sendto(ept, data, (int)len, src);
}

/*
 * ns_bind callback: invoked if the host announces a service we haven't bound.
 * Normally we announce first (see cvi_rpmsg_echo_test_run), so this is a
 * fallback that binds on demand.
 */
 static void cvi_rpmsg_echo_ns_bind(const char *name, uint32_t remote_addr)
 {
    cvi_rpmsg_ept_t *ept = NULL;

	 if (!name)
		 return;

    printf("[cvi_rpmsg] ns bind, name:%s remote_addr:%d\n", name, remote_addr);

    if (!strcmp(name, ECHO_TEST_EPT_NAME)) {
        ept = cvi_rpmsg_create_ept(name, CVI_RPMSG_ADDR_ANY, remote_addr,
            cvi_rpmsg_echo_test_cb, NULL, NULL);
         if (ept) 
            printf("[cvi_rpmsg] echo ept bound via ns_cb (dest=0x%x)\n", remote_addr);
    } else if (!strcmp(name, USER_EPT_NAME)) {
        ept = cvi_rpmsg_create_ept(name, CVI_RPMSG_ADDR_ANY, remote_addr,
            cvi_rpmsg_user_cb, NULL, NULL);
         if (ept) 
            printf("[cvi_rpmsg] user ept bound via ns_cb (dest=0x%x)\n", remote_addr);
    }

 }

static void cvi_rpmsg_run(void *arg)
{
	int ret;

	(void)arg;
	/* Pass ns_bind_cb as a fallback; the primary path below creates the
	 * ept and announces proactively. */
	ret = cvi_rpmsg_init(cvi_rpmsg_echo_ns_bind);
	if (ret) {
		printf("[cvi_rpmsg] init failed: %d\n", ret);
		return;
	}

	/* Announce the echo service so the Linux host creates the channel and
	 * /dev/rpmsg_ctrl0 appears.  src/dest = ANY: the local rpmsg device
	 * assigns src; dest is set by the NS handshake when Linux creates its
	 * endpoint. */
	cvi_rpmsg_create_ept(ECHO_TEST_EPT_NAME,
						CVI_RPMSG_ADDR_ANY,
						CVI_RPMSG_ADDR_ANY,
						cvi_rpmsg_echo_test_cb, NULL, NULL);
    printf("[cvi_rpmsg] rpmsg bus up; service \"%s\" ready\n", ECHO_TEST_EPT_NAME);

    cvi_rpmsg_create_ept(USER_EPT_NAME,
                        CVI_RPMSG_ADDR_ANY,
                        CVI_RPMSG_ADDR_ANY,
                        cvi_rpmsg_user_cb, NULL, NULL);

    printf("[cvi_rpmsg] rpmsg bus up; service \"%s\" ready\n", USER_EPT_NAME);
}

int cvi_rpmsg_service_start(void)
{
	static int started = 0;

	if (started)
		return 0;
	started = 1;

	return aos_task_new("cvi_rpmsg_run", cvi_rpmsg_run, NULL,CVI_RPMSG_TASK_STACK);
}