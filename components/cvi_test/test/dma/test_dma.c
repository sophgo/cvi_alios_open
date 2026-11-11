#include <cvi_dma.h>
#include <aos/cli.h>
#include <drv/dma.h>
#include <aos/kernel.h>

#define DMA_MEM2MEM_TEST_SIZE  (1024)
#define DMA_MEM2MEM_TEST_VALUE (0x5A)

static aos_sem_t dma_sem;

static void dma_test_callback(csi_dma_ch_t *dma_ch, csi_dma_event_t event,
                              void *arg)
{
    if (event == DMA_EVENT_TRANSFER_DONE) {
        aos_sem_signal(&dma_sem);
    }
}

void dma_mem2mem_test(int argc, char **argv)
{
    void               *src_addr  = NULL;
    void               *dst_addr  = NULL;
    int                 ret       = 0;
    bool                test_pass = true;
    csi_dma_ch_t        dma_ch;
    csi_dma_ch_config_t config;

    if (aos_sem_new(&dma_sem, 0) != 0) {
        dma_err("Failed to create semaphore.\n");
        return;
    }

    src_addr = aos_malloc(DMA_MEM2MEM_TEST_SIZE);
    if (!src_addr) {
        dma_err("Failed to allocate source memory.\n");
        goto free_sem;
    }

    dst_addr = aos_malloc(DMA_MEM2MEM_TEST_SIZE);
    if (!dst_addr) {
        dma_err("Failed to allocate destination memory.\n");
        aos_free(src_addr);
        goto free_sem;
    }
	dma_err("src_addr=%p, dst_addr=%p\r\n", src_addr, dst_addr);
    memset(src_addr, DMA_MEM2MEM_TEST_VALUE, DMA_MEM2MEM_TEST_SIZE);
    memset(dst_addr, 0, DMA_MEM2MEM_TEST_SIZE);

	printf("--- Source Buffer (first 32 bytes) ---\n");
    for (int i = 0; i < 32; i++) {
        printf("%02x ", ((uint8_t *)src_addr)[i]);
    }
    printf("\n");

    printf("--- Destination Buffer (first 32 bytes) ---\n");
    for (int i = 0; i < 32; i++) {
        printf("%02x ", ((uint8_t *)dst_addr)[i]);
    }
    printf("\n");

    ret = csi_dma_ch_alloc(&dma_ch, 4, 0);
    if (ret != 0) {
        dma_err("Failed to allocate DMA channel.\n");
        test_pass = false;
        goto free_mem;
    }

    config.src_inc   = DMA_ADDR_INC;
    config.dst_inc   = DMA_ADDR_INC;
    config.src_tw    = DMA_DATA_WIDTH_32_BITS;
    config.dst_tw    = DMA_DATA_WIDTH_32_BITS;
    config.trans_dir = DMA_MEM2MEM;
    config.group_len = 16;
    config.handshake = 0;
    config.src_reload_en = 0;
    config.dst_reload_en = 0;
    config.half_int_en   = 0;
    config.lli_src_en    = 0;

    csi_dma_ch_config(&dma_ch, &config);
    csi_dma_ch_attach_callback(&dma_ch, dma_test_callback, NULL);
    csi_dma_ch_start(&dma_ch, src_addr, dst_addr, DMA_MEM2MEM_TEST_SIZE);

    if (aos_sem_wait(&dma_sem, 5000) != 0) {
        dma_err("DMA transfer timeout.\n");
        csi_dma_ch_stop(&dma_ch);
        test_pass = false;
    }

    if (test_pass && memcmp(src_addr, dst_addr, DMA_MEM2MEM_TEST_SIZE) != 0) {
        dma_err("Memory comparison failed.\n");
        test_pass = false;
    }

    printf("--- end transfer ---\n");
    printf("--- Source Buffer (first 32 bytes) ---\n");
    for (int i = 0; i < 32; i++) {
        printf("%02x ", ((uint8_t *)src_addr)[i]);
    }
    printf("\n");

    printf("--- Destination Buffer (first 32 bytes) ---\n");
    for (int i = 0; i < 32; i++) {
        printf("%02x ", ((uint8_t *)dst_addr)[i]);
    }
    printf("\n");
    csi_dma_ch_detach_callback(&dma_ch);
    csi_dma_ch_free(&dma_ch);

free_mem:
    aos_free(src_addr);
    aos_free(dst_addr);
free_sem:
    aos_sem_free(&dma_sem);

    if (test_pass) {
        dma_err("DMA mem2mem test PASSED.\n");
    } else {
        dma_err("DMA mem2mem test FAILED.\n");
    }
}

ALIOS_CLI_CMD_REGISTER(dma_mem2mem_test, dma_mem2mem_test, DMA memory to memory test);