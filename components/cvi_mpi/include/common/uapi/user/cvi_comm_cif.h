#ifndef _U_CIF_UAPI_H_
#define _U_CIF_UAPI_H_

#if defined(CONFIG_KERNEL_RHINO)
#include <drv/gpio.h>
#endif

#define MIPI_LANE_NUM	4
#define WDR_VC_NUM	2
#define SYNC_CODE_NUM	4
#define BT_DEMUX_NUM	4
#define MIPI_DEMUX_NUM	4

// Struct for image dimensions (width and height in pixels).
struct img_size_s {
    unsigned int width;   // Width of the image
    unsigned int height;  // Height of the image
};


// Enum for RX MAC clock frequencies.
enum rx_mac_clk_e {
    RX_MAC_CLK_200M = 0,
    RX_MAC_CLK_300M,
    RX_MAC_CLK_400M,
    RX_MAC_CLK_500M,
    RX_MAC_CLK_600M,
    RX_MAC_CLK_BUTT,
};


// Enum for camera PLL frequency settings.
enum cam_pll_freq_e {
    CAMPLL_FREQ_NONE = 0,
    CAMPLL_FREQ_37P125M,
    CAMPLL_FREQ_25M,
    CAMPLL_FREQ_27M,
    CAMPLL_FREQ_24M,
    CAMPLL_FREQ_26M,
    CAMPLL_FREQ_NUM
};


// Structure for MCLK PLL configuration containing camera ID and frequency.
struct mclk_pll_s {
    unsigned int        cam;          // Camera identifier
    enum cam_pll_freq_e freq;         // PLL frequency setting
};


// Structure for DPHY configuration with enable flag and high-speed settle time.
struct dphy_s {
    unsigned char    enable;      // Enable flag for DPHY
    unsigned char    hs_settle;   // High-speed settle time
};



// Enumeration for different input modes supported by the system.
enum input_mode_e {
    INPUT_MODE_MIPI = 0,        // MIPI input mode
    INPUT_MODE_SUBLVDS,         // Sub-LVDS input mode
    INPUT_MODE_HISPI,           // HiSPI input mode
    INPUT_MODE_CMOS,            // CMOS input mode
    INPUT_MODE_BT1120,          // BT.1120 input mode
    INPUT_MODE_BT601,           // BT.601 input mode
    INPUT_MODE_BT656_9B,        // BT.656 (9-bit) input mode
    INPUT_MODE_CUSTOM_0,        // Custom input mode 0
    INPUT_MODE_BT_DEMUX,        // BT demultiplexed input mode
    INPUT_MODE_BUTT             // Placeholder for invalid input mode
};


enum raw_data_type_e {
	RAW_DATA_8BIT = 0,
	RAW_DATA_10BIT,
	RAW_DATA_12BIT,
	YUV422_8BIT,	/* MIPI-CSI only */
	YUV422_10BIT,   /* MIPI-CSI only*/
	RAW_DATA_BUTT
};

enum mipi_wdr_mode_e {
	CVI_MIPI_WDR_MODE_NONE = 0,
	CVI_MIPI_WDR_MODE_VC,
	CVI_MIPI_WDR_MODE_DT,
	CVI_MIPI_WDR_MODE_DOL,
	CVI_MIPI_WDR_MODE_MANUAL,  /* SOI case */
	CVI_MIPI_WDR_MODE_BUTT
};

// Enumeration for Wide Dynamic Range (WDR) modes available in the system.
enum wdr_mode_e {
    CVI_WDR_MODE_NONE = 0,      // No WDR mode
    CVI_WDR_MODE_2F,            // 2-frame WDR mode
    CVI_WDR_MODE_3F,            // 3-frame WDR mode
    CVI_WDR_MODE_DOL_2F,        // DOL 2-frame WDR mode
    CVI_WDR_MODE_DOL_3F,        // DOL 3-frame WDR mode
    CVI_WDR_MODE_DOL_BUTT       // Placeholder for invalid DOL WDR mode
};


// Enumeration for LVDS synchronization modes.
enum lvds_sync_mode_e {
    LVDS_SYNC_MODE_SOF = 0,     // Start of Frame (SOF) synchronization
    LVDS_SYNC_MODE_SAV,         // Start of Active Video (SAV) synchronization
    LVDS_SYNC_MODE_BUTT         // Placeholder for invalid sync mode
};


// Enumeration for LVDS bit endianness modes.
enum lvds_bit_endian {
    LVDS_ENDIAN_LITTLE = 0,     // Little-endian bit order
    LVDS_ENDIAN_BIG,            // Big-endian bit order
    LVDS_ENDIAN_BUTT            // Placeholder for invalid endianness
};

// Enumeration for LVDS VSYNC modes.
enum lvds_vsync_type_e {
	LVDS_VSYNC_NORMAL = 0,		// VSYNC normal mode
	LVDS_VSYNC_SHARE,			// VSYNC share mode
	LVDS_VSYNC_HCONNECT,		// VSYNC hconnect mode
	LVDS_VSYNC_BUTT				// Placeholder for invalid endianness
};

// Enumeration for LVDS Frame ID (FID) types.
enum lvds_fid_type_e {
    LVDS_FID_NONE = 0,          // No Frame ID
    LVDS_FID_IN_SAV,           // Frame ID included in Start of Active Video (SAV)
    LVDS_FID_BUTT               // Placeholder for invalid FID type
};


// Structure representing the Frame ID (FID) type for LVDS.
struct lvds_fid_type_s {
    enum lvds_fid_type_e fid;  // Frame ID type
};


// Structure representing the vertical sync (VSYNC) configuration for LVDS.
struct lvds_vsync_type_s {
    enum lvds_vsync_type_e sync_type; // Type of vertical sync
    unsigned short hblank1;            // Horizontal blanking interval 1
    unsigned short hblank2;            // Horizontal blanking interval 2
};


// Structure representing the attributes of an LVDS device configuration.
struct lvds_dev_attr_s {
    enum wdr_mode_e wdr_mode;               // Wide Dynamic Range mode
    enum lvds_sync_mode_e sync_mode;        // LVDS synchronization mode
    enum raw_data_type_e raw_data_type;     // Type of raw data
    enum lvds_bit_endian data_endian;       // Data endianness
    enum lvds_bit_endian sync_code_endian;  // Sync code endianness
    short lane_id[MIPI_LANE_NUM + 1];       // Lane IDs for MIPI
    short sync_code[MIPI_LANE_NUM][WDR_VC_NUM + 1][SYNC_CODE_NUM]; // Sync codes
/*
 * sublvds:
 * sync_code[x][0][0] sync_code[x][0][1] sync_code[x][0][2] sync_code[x][0][3]
 *	n0_lef_sav	   n0_lef_eav	      n1_lef_sav	 n1_lef_eav
 * sync_code[x][1][0] sync_code[x][1][1] sync_code[x][1][2] sync_code[x][1][3]
 *	n0_sef_sav	   n0_sef_eav	      n1_sef_sav	 n1_sef_eav
 * sync_code[x][2][0] sync_code[x][2][1] sync_code[x][2][2] sync_code[x][2][3]
 *	n0_lsef_sav	   n0_lsef_eav	      n1_lsef_sav	 n1_lsef_eav
 *
 * hispi:
 * sync_code[x][0][0] sync_code[x][0][1] sync_code[x][0][2] sync_code[x][0][3]
 *	t1_sol		   tl_eol	      t1_sof		 t1_eof
 * sync_code[x][1][0] sync_code[x][1][1] sync_code[x][1][2] sync_code[x][1][3]
 *	t2_sol		   t2_eol	      t2_sof		 t2_eof
 */
    struct lvds_vsync_type_s vsync_type;    // Vertical sync configuration
    struct lvds_fid_type_s fid_type;        // Frame ID configuration
    char pn_swap[MIPI_LANE_NUM + 1];        // Pin swap configuration
};


// Structure containing configuration information for MIPI demultiplexer.
struct mipi_demux_info_s {
    unsigned int demux_en;                   // Enable flag for demultiplexer
    unsigned char vc_mapping[MIPI_DEMUX_NUM]; // Virtual channel mapping for demultiplexer
};


// Structure representing attributes for a MIPI device configuration.
struct mipi_dev_attr_s {
    enum raw_data_type_e raw_data_type;        // Type of raw data
    short lane_id[MIPI_LANE_NUM + 1];          // Lane IDs for MIPI
    enum mipi_wdr_mode_e wdr_mode;              // Wide Dynamic Range mode
    short data_type[WDR_VC_NUM];                // Data types for each virtual channel
    char pn_swap[MIPI_LANE_NUM + 1];            // Pin swap configuration
    struct dphy_s dphy;                          // D-PHY configuration
    struct mipi_demux_info_s demux;             // Demultiplexer configuration
};


// Structure for manual Wide Dynamic Range (WDR) attributes configuration.
struct manual_wdr_attr_s {
    unsigned int manual_en;                // Enable flag for manual WDR
    unsigned short l2s_distance;           // Distance from L2 to S
    unsigned short lsef_length;            // Length of LSEF
    unsigned int discard_padding_lines;    // Number of padding lines to discard
    unsigned int update;                   // Update flag for configuration changes
};


// Enumeration for TTL pin functions used in data and control signaling.
enum ttl_pin_func_e {
    TTL_PIN_FUNC_VS,     // Vertical Sync
    TTL_PIN_FUNC_HS,     // Horizontal Sync
    TTL_PIN_FUNC_VDE,    // Vertical Data Enable
    TTL_PIN_FUNC_HDE,    // Horizontal Data Enable
    TTL_PIN_FUNC_D0,     // Data pin 0
    TTL_PIN_FUNC_D1,     // Data pin 1
    TTL_PIN_FUNC_D2,     // Data pin 2
    TTL_PIN_FUNC_D3,     // Data pin 3
    TTL_PIN_FUNC_D4,     // Data pin 4
    TTL_PIN_FUNC_D5,     // Data pin 5
    TTL_PIN_FUNC_D6,     // Data pin 6
    TTL_PIN_FUNC_D7,     // Data pin 7
    TTL_PIN_FUNC_D8,     // Data pin 8
    TTL_PIN_FUNC_D9,     // Data pin 9
    TTL_PIN_FUNC_D10,    // Data pin 10
    TTL_PIN_FUNC_D11,    // Data pin 11
    TTL_PIN_FUNC_D12,    // Data pin 12
    TTL_PIN_FUNC_D13,    // Data pin 13
    TTL_PIN_FUNC_D14,    // Data pin 14
    TTL_PIN_FUNC_D15,    // Data pin 15
    TTL_PIN_FUNC_NUM,     // Total number of pin functions
};


// Enumeration for TTL video input sources.
enum ttl_src_e {
    TTL_VI_SRC_VI0 = 0,  // Video input source 0
    TTL_VI_SRC_VI1,      // Video input source 1
    TTL_VI_SRC_VI2,      // Video input source 2 (BT demux)
    TTL_VI_SRC_NUM       // Total number of video input sources
};


// Enumeration for TTL format types.
enum ttl_fmt_e {
    TTL_SYNC_PAT = 0,    // Synchronization pattern
    TTL_VHS_11B,         // VHS format 11 bits
    TTL_VHS_19B,         // VHS format 19 bits
    TTL_VDE_11B,         // VDE format 11 bits
    TTL_VDE_19B,         // VDE format 19 bits
    TTL_VSDE_11B,        // VSDE format 11 bits
    TTL_VSDE_19B,        // VSDE format 19 bits
};


// Enumeration for BT demux modes.
enum bt_demux_mode_e {
    BT_DEMUX_DISABLE = 0, // Disable BT demux
    BT_DEMUX_2,           // BT demux mode with 2 channels
    BT_DEMUX_3,           // BT demux mode with 3 channels
    BT_DEMUX_4,           // BT demux mode with 4 channels
};


// Structure for BT demux synchronization status.
struct bt_demux_sync_s {
    unsigned char sav_vld; // Start of Active Video (SAV) valid flag
    unsigned char sav_blk;  // Start of Active Video (SAV) block count
    unsigned char eav_vld;  // End of Active Video (EAV) valid flag
    unsigned char eav_blk;  // End of Active Video (EAV) block count
};

// Structure for BT demux attributes and configuration.
struct bt_demux_attr_s {
    signed char func[TTL_PIN_FUNC_NUM];        // Functionality array for TTL pin functions
    unsigned short v_fp;                        // Vertical front porch
    unsigned short h_fp;                        // Horizontal front porch
    unsigned short v_bp;                        // Vertical back porch
    unsigned short h_bp;                        // Horizontal back porch
    enum bt_demux_mode_e mode;                 // BT demux mode
    unsigned char sync_code_part_A[3];         // Sync code parts 0 to 2
    struct bt_demux_sync_s sync_code_part_B[BT_DEMUX_NUM]; // Sync code part 3
    char yc_exchg;                              // Y/C exchange flag
};


// Structure for TTL device attributes and configuration.
struct ttl_dev_attr_s {
    enum ttl_src_e vi;                     // Video input source
    enum ttl_fmt_e ttl_fmt;                // TTL format type
    enum raw_data_type_e raw_data_type;    // Raw data type
    signed char func[TTL_PIN_FUNC_NUM];    // Functionality array for TTL pin functions
    unsigned short v_bp;                    // Vertical back porch
    unsigned short h_bp;                    // Horizontal back porch
};


// Structure for combo device attributes, supporting multiple input types.
struct combo_dev_attr_s {
    enum input_mode_e input_mode;                   // Input mode selection
    enum rx_mac_clk_e mac_clk;                       // MAC clock configuration
    struct mclk_pll_s mclk;                          // Master clock PLL settings
    union {                                          // Union for different device attributes
        struct mipi_dev_attr_s mipi_attr;           // MIPI device attributes
        struct lvds_dev_attr_s lvds_attr;           // LVDS device attributes
        struct ttl_dev_attr_s ttl_attr;             // TTL device attributes
        struct bt_demux_attr_s bt_demux_attr;       // BT demux attributes
    };
    unsigned int devno;                              // Device number
    struct img_size_s img_size;                      // Image size configuration
    struct manual_wdr_attr_s wdr_manu;              // Manual WDR attributes
};


// Enumeration for clock edge types.
enum clk_edge_e {
    CLK_UP_EDGE = 0,  // Rising clock edge
    CLK_DOWN_EDGE,    // Falling clock edge
    CLK_EDGE_BUTT     // Placeholder for edge count
};


// Structure for clock edge configuration associated with a device.
struct clk_edge_s {
    unsigned int devno;             // Device number
    enum clk_edge_e edge;          // Clock edge type
};


// Enumeration for output MSB (Most Significant Bit) configurations.
enum output_msb_e {
    OUTPUT_NORM_MSB = 0,  // Normal MSB order
    OUTPUT_REVERSE_MSB,   // Reversed MSB order
    OUTPUT_MSB_BUTT       // Placeholder for MSB count
};

// Structure for device MSB (Most Significant Bit) configuration.
struct msb_s {
    unsigned int devno;        // Device number
    enum output_msb_e msb;    // Output MSB type
};


// Structure for cropping configuration of a device.
struct crop_top_s {
    unsigned int devno;      // Device number
    unsigned int crop_top;   // Top crop value
    unsigned int update;     // Update flag
};


// Structure for manual Wide Dynamic Range (WDR) configuration.
struct manual_wdr_s {
    unsigned int devno;                // Device number
    struct manual_wdr_attr_s attr;     // Manual WDR attributes
};


// Structure for vertical sync (VSYNC) generation configuration.
struct vsync_gen_s {
    unsigned int devno;        // Device number
    unsigned int distance_fp;  // Distance from front porch
};


// Enumeration for BT format output types.
enum bt_fmt_out_e {
    BT_FMT_OUT_CBYCRY,  // Cb, Y, Cr, Y format
    BT_FMT_OUT_CRYCBY,  // Cr, Y, Cb, Y format
    BT_FMT_OUT_YCBYCR,  // Y, Cb, Y, Cr format
    BT_FMT_OUT_YCRYCB,  // Y, Cr, Y, Cb format
};


// Structure for BT format output configuration.
struct bt_fmt_out_s {
    unsigned int devno;           // Device number
    enum bt_fmt_out_e fmt_out;    // BT format output type
};


// Structure for CIF (Common Intermediate Format) cropping window configuration.
struct cif_crop_win_s {
    unsigned int devno;  // Device number
    unsigned int enable; // Enable cropping flag
    unsigned int x;      // X-coordinate of the crop window
    unsigned int y;      // Y-coordinate of the crop window
    unsigned int w;      // Width of the crop window
    unsigned int h;      // Height of the crop window
};


// Structure for CIF YUV swap configuration.
struct cif_yuv_swap_s {
    unsigned int devno;   // Device number
    unsigned int uv_swap; // UV channel swap flag
    unsigned int yc_swap; // Y and C channel swap flag
};


// Structure representing the reset GPIO configuration for a sensor.
// Contains GPIO settings and parameters for resetting the sensor.
struct snsr_rst_gpio_s {
#if defined(CONFIG_KERNEL_RHINO)
	csi_gpio_t snsr_rst_gpio;    // GPIO object for the sensor reset control
	uint32_t snsr_rst_port_idx;   // Index of the GPIO port used for sensor reset
	uint32_t snsr_rst_pin;        // Pin number for the sensor reset GPIO
	uint32_t snsr_rst_pol;        // Polarity setting for the sensor reset (active high/low)
#endif
};

// Structure representing the reset GPIO configuration for a sensor.
// Contains GPIO settings and parameters for resetting the sensor.
enum of_gpio_flags {
    OF_GPIO_ACTIVE_LOW = 0x1,      // GPIO pin is active when the signal is low
    OF_GPIO_SINGLE_ENDED = 0x2,     // Single-ended GPIO, supports only one signal level
    OF_GPIO_OPEN_DRAIN = 0x4,       // Open-drain mode, allows multiple devices to share the same pin by pulling it low
    OF_GPIO_TRANSITORY = 0x8,       // Transitory GPIO, indicates that the pin state may be temporary
    OF_GPIO_PULL_UP = 0x10,         // Pull-up resistor, ensures the pin is high when not driven
    OF_GPIO_PULL_DOWN = 0x20,       // Pull-down resistor, ensures the pin is low when not driven
};


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

void vip_clk_en(void);
void vip_csi_bdg_dma_cfg(unsigned int devno, unsigned int hdrMode);
int proc_cif_show(void);
int cvi_cif_reset_snsr_gpio_init(int devno, struct snsr_rst_gpio_s *snsr_gpio);
#if defined(CONFIG_KERNEL_RHINO)
int cif_reset_mipi(uint32_t devno);
int cif_enable_snsr_clk(uint32_t devno, uint8_t on);
int cif_reset_snsr_gpio(unsigned int devno, uint8_t on);
int cif_enable_ahd_clk(uint32_t devno, uint8_t on, uint8_t mclkFreq);
#endif
int cif_set_dev_attr(struct combo_dev_attr_s *attr);
int cvi_cif_init(void);
int cvi_cif_exit(void);


/**
 * @brief Initializes the CIF (Common Intermediate Format) module.
 *
 * This function sets up the necessary resources for the CIF module
 * and prepares it for use.
 *
 * @return int Returns 0 on success, or a negative error code on failure.
 */



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

/* mipi_rx ioctl commands related definition */
#define CVI_MIPI_IOC_MAGIC		'm'

/* Support commands */
#if 0
#define CVI_MIPI_SET_DEV_ATTR		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x01, struct combo_dev_attr_s)
#define CVI_MIPI_SET_OUTPUT_CLK_EDGE	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x02, struct clk_edge_s)
#define CVI_MIPI_RESET_SENSOR		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x05, unsigned int)
#define CVI_MIPI_UNRESET_SENSOR		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x06, unsigned int)
#define CVI_MIPI_RESET_MIPI		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x07, unsigned int)
#define CVI_MIPI_ENABLE_SENSOR_CLOCK	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x10, unsigned int)
#define CVI_MIPI_DISABLE_SENSOR_CLOCK	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x11, unsigned int)
#define CVI_MIPI_SET_CROP_TOP		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x20, struct crop_top_s)
#define CVI_MIPI_SET_WDR_MANUAL		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x21, struct manual_wdr_s)
#define CVI_MIPI_SET_LVDS_FP_VS		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x22, struct vsync_gen_s)
#define CVI_MIPI_RESET_LVDS		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x23, unsigned int)
#define CVI_MIPI_SET_BT_FMT_OUT		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x24, struct bt_fmt_out_s)
#define CVI_MIPI_GET_CIF_ATTR		_IOWR(CVI_MIPI_IOC_MAGIC, \
						0x25, struct cif_attr_s)
#define CVI_MIPI_SET_SENSOR_CLOCK	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x26, struct mclk_pll_s)
#define CVI_MIPI_SET_MAX_MAC_CLOCK	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x27, unsigned int)
#define CVI_MIPI_SET_CROP_WINDOW	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x28, struct cif_crop_win_s)
#define CVI_MIPI_SET_YUV_SWAP		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x29, struct cif_yuv_swap_s)
/* Unsupport commands */
#define CVI_MIPI_SET_PHY_CMVMODE	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x04, unsigned int)
#define CVI_MIPI_UNRESET_MIPI		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x08, unsigned int)
#define CVI_MIPI_RESET_SLVS		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x09, unsigned int)
#define CVI_MIPI_UNRESET_SLVS		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x0A, unsigned int)
#define CVI_MIPI_SET_HS_MODE		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x0B, unsigned int)
#define CVI_MIPI_ENABLE_MIPI_CLOCK	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x0C, unsigned int)
#define CVI_MIPI_DISABLE_MIPI_CLOCK	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x0D, unsigned int)
#define CVI_MIPI_ENABLE_SLVS_CLOCK	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x0E, unsigned int)
#define CVI_MIPI_DISABLE_SLVS_CLOCK	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x0F, unsigned int)
#endif
#endif // _U_CVI_VIP_CIF_H_
