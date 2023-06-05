#define DATA_LEN	128

typedef struct {
    unsigned int evt:14;
    unsigned int len:16;
    unsigned int type:2;
} spi_register_t;

typedef void *(*spi_service_call)(void *);
typedef struct {
    spi_service_call rxPrepare;
    spi_service_call rxDone;
    spi_service_call txPrepare;
    spi_service_call txDone;
    spi_service_call dataHandle;
} spi_service_func_t;

#define ES_SPI_CMD_READ  0x0B
#define ES_SPI_CMD_WRITE 0x51
#define ES_SPI_CMD_STATE 0x05
#define ES_SPI_CMD_DUMMY 0x00
#define ES_SPI_CONTROL_READ  0x00
#define ES_SPI_CONTROL_WRITE 0x01
#define ES_SPI_STATE_TIMEOUT 10000
#define BIT16 (1<<16)
#define BIT15 (1<<15)
#define SPI_SERVICE_CONTROL_LEN 0x8000
#define SPI_SERVICE_CONTROL_INT 0x9000
#define SPI_SERVICE_CONTROL_MSG 0xA000
#define SPI_SERVICE_MSG_MAX_LEN 0x1000


typedef enum {
    SPI_SERVICE_TYPE_INFO,
    SPI_SERVICE_TYPE_HTOS = 0x100,
    SPI_SERVICE_TYPE_STOH = 0x200,
    SPI_SERVICE_TYPE_OTA = 0x300,
    SPI_SERVICE_TYPE_MSG = 0x500,
    SPI_SERVICE_TYPE_INT,
} spi_service_type_e;


enum
{
    SPI_EVENT_WIFI_LINK,
    SPI_EVENT_INT_REPORT,
    SPI_EVENT_MSG_DATA,
};

#define SPI_IOC_RD_IP           	0
#define SPI_IOC_RD_MASK         	1
#define SPI_IOC_RD_MACADDR0     	2
#define SPI_IOC_RD_MACADDR1     	3
#define SPI_IOC_RD_GW0          	4
#define SPI_IOC_RD_GW1          	5
#define SPI_IOC_RD_GW2          	6
#define SPI_IOC_RD_GW3          	7
#define SPI_IOC_RD_DNS0         	8
#define SPI_IOC_RD_DNS1         	9
#define SPI_IOC_RD_DNS2         	10
#define SPI_IOC_RD_DNS3         	11
#define SPI_IOC_INVALID_PORT_WR 	12
#define SPI_IOC_INVALID_PORT_RD 	13
#define SPI_IOC_FE_VERSION     		14
#define SPI_IOC_RXBUFF         		15
#define SPI_IOC_TXBUFF          	16
#define SPI_IOC_STATUS         		17
#define SPI_IOC_OTA					18  

int spi_service_init(void);
int es_spi_get_info_data(unsigned int len, unsigned char *skb);



