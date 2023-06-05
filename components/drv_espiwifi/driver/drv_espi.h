#ifndef __DRV_ESPI_H__
#define __DRV_ESPI_H__

typedef struct {
    unsigned int evt:14;
    unsigned int len:16;
    unsigned int type:2;
} drv_espi_cfg_t;

typedef enum {
    DRV_ESPI_TYPE_INFO,
    DRV_ESPI_TYPE_HTOS = 0x100,
    DRV_ESPI_TYPE_STOH = 0x200,
    DRV_ESPI_TYPE_OTA = 0x300,
    DRV_ESPI_TYPE_MSG = 0x500,
    DRV_ESPI_TYPE_INT,
} drv_espi_type_e;

#define DRV_ESSPI_CMD_LEN 2
#define DRV_ESSPI_DATA_OFFSET DRV_ESSPI_CMD_LEN

int drv_espi_read_info(int offset, unsigned char *rbuff, unsigned int len);
int drv_espi_write_info(int offset, unsigned char *wbuff, unsigned int len);
int drv_espi_sendto_peer(unsigned char *data, unsigned int len);
int drv_espi_service_init(void);
int drv_espi_send_type_data(drv_espi_type_e type, unsigned char *msg, unsigned int len);

#endif