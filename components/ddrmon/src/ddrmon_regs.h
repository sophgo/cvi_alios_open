/* SPDX-License-Identifier: Apache-2.0 */

#ifndef CVI_DDRMON_REGS_H_
#define CVI_DDRMON_REGS_H_

// Per-port block offsets (relative to AXIMON base)
#define AXIMON_M1_WRITE   0x000
#define AXIMON_M1_READ    0x080
#define AXIMON_M2_WRITE   0x100
#define AXIMON_M2_READ    0x180
#define AXIMON_M3_WRITE   0x200
#define AXIMON_M3_READ    0x280
#define AXIMON_M4_WRITE   0x300
#define AXIMON_M4_READ    0x380
#define AXIMON_M5_WRITE   0x400
#define AXIMON_M5_READ    0x480
#define AXIMON_M6_WRITE   0x500
#define AXIMON_M6_READ    0x580

// Register offsets inside each port block (aligned with Linux AXI monitor driver)
#define AXIMON_OFFSET_CONTROL         0x00  // Control register
#define AXIMON_OFFSET_STATUS          0x04  // Status register
#define AXIMON_OFFSET_ID              0x08  // ID filter
#define AXIMON_OFFSET_ADDR_MIN        0x0C  // Address range min
#define AXIMON_OFFSET_ADDR_MAX        0x10  // Address range max
#define AXIMON_OFFSET_ADDR_MASK       0x14  // Address mask
#define AXIMON_OFFSET_LEN_MIN         0x18  // Length range min
#define AXIMON_OFFSET_LEN_MAX         0x1C  // Length range max
#define AXIMON_OFFSET_LEN_MASK        0x20  // Length mask
#define AXIMON_OFFSET_CYCLE           0x24  // Cycle counter
#define AXIMON_OFFSET_HITCNTS         0x28  // Hit counter
#define AXIMON_OFFSET_BYTECNTS        0x2C  // Byte counter
#define AXIMON_OFFSET_MAXLATCNTS      0x30  // Max latency counter
#define AXIMON_OFFSET_LATENCYCNTS     0x34  // Latency counter
#define AXIMON_OFFSET_REQOUTSTANDCNTS 0x38  // Outstanding request counter
#define AXIMON_OFFSET_BRESPCNTS       0x3C  // BRESP counter
#define AXIMON_OFFSET_RRESPCNTS       0x40  // RRESSP counter
#define AXIMON_OFFSET_LAT_BIN_SIZE_SEL 0x50 // Latency histogram bin size
#define AXIMON_OFFSET_LATENCY_HIS_0   0x54  // Latency histogram[0]
#define AXIMON_OFFSET_LATENCY_HIS_1   0x58  // Latency histogram[1]
#define AXIMON_OFFSET_LATENCY_HIS_2   0x5C  // Latency histogram[2]
#define AXIMON_OFFSET_LATENCY_HIS_3   0x60  // Latency histogram[3]

// Control register magic values
#define AXIMON_START_REGVALUE         0x30001  // Start monitoring
#define AXIMON_STOP_REGVALUE          0x30002  // Stop monitoring
#define AXIMON_SNAPSHOT_REGVALUE_1    0x40004  // Snapshot step 1 (lock counters)
#define AXIMON_SNAPSHOT_REGVALUE_2    0x40000  // Snapshot step 2 (read enabled)

// Clock selection value
#define AXIMON_SELECT_CLK             0x01000100

static inline unsigned int port_write_base(int idx)
{
	static const unsigned int tbl[6] = {
		AXIMON_M1_WRITE, AXIMON_M2_WRITE, AXIMON_M3_WRITE,
		AXIMON_M4_WRITE, AXIMON_M5_WRITE, AXIMON_M6_WRITE
	};
	return (idx >= 0 && idx < 6) ? tbl[idx] : 0;
}

static inline unsigned int port_read_base(int idx)
{
	static const unsigned int tbl[6] = {
		AXIMON_M1_READ, AXIMON_M2_READ, AXIMON_M3_READ,
		AXIMON_M4_READ, AXIMON_M5_READ, AXIMON_M6_READ
	};
	return (idx >= 0 && idx < 6) ? tbl[idx] : 0;
}

#endif // CVI_DDRMON_REGS_H_
