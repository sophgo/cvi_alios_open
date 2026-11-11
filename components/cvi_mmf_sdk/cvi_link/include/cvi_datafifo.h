#ifndef __CVI_DATAFIFO_H
#define __CVI_DATAFIFO_H
#include "cvi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


typedef unsigned long CVI_DATAFIFO_HANDLE;
#define CVI_DATAFIFO_INVALID_HANDLE (-1)

/** Datafifo Error number base */
#define CVI_DATAFIFO_ERRNO_BASE 0x1A00
/** Parameter is invalid */
#define CVI_DATAFIFO_ERR_EINVAL_PAEAMETER	 (CVI_DATAFIFO_ERRNO_BASE + 1)
/** Null pointer*/
#define CVI_DATAFIFO_ERR_NULL_PTR			 (CVI_DATAFIFO_ERRNO_BASE + 2)
/** failure caused by malloc memory */
#define CVI_DATAFIFO_ERR_NOMEM				 (CVI_DATAFIFO_ERRNO_BASE + 3)
/** failure caused by device operation */
#define CVI_DATAFIFO_ERR_DEV_OPT			 (CVI_DATAFIFO_ERRNO_BASE + 4)
/** operation is not permitted, Reader to write or Writer to read */
#define CVI_DATAFIFO_ERR_NOT_PERM			 (CVI_DATAFIFO_ERRNO_BASE + 5)
/** data buffer is empty, no data to read*/
#define CVI_DATAFIFO_ERR_NO_DATA			 (CVI_DATAFIFO_ERRNO_BASE + 6)
/** data buffer is full, no space to write*/
#define CVI_DATAFIFO_ERR_NO_SPACE			 (CVI_DATAFIFO_ERRNO_BASE + 7)
/** read error*/
#define CVI_DATAFIFO_ERR_READ				 (CVI_DATAFIFO_ERRNO_BASE + 8)
/** write error*/
#define CVI_DATAFIFO_ERR_WRITE				 (CVI_DATAFIFO_ERRNO_BASE + 9)


/**
 * @brief Stream release callback. when bDataReleaseByWriter is CVI_TRUE,
 * The writer should register this function and do release in this function.
 */
typedef void (*CVI_DATAFIFO_RELEASESTREAM_FN_PTR)(void *pStream);

/** Role of caller*/
typedef enum cviDATAFIFO_OPEN_MODE_E {
	DATAFIFO_READER, /**<WRITER*/
	DATAFIFO_WRITER /**<READER*/
} CVI_DATAFIFO_OPEN_MODE_E;

/** DATAFIFO parameters */
typedef struct cviDATAFIFO_PARAMS_S {
	CVI_U32 u32EntriesNum; /**< The number of items in the ring buffer*/
	CVI_U32 u32CacheLineSize; /**< Item size*/
	CVI_BOOL bDataReleaseByWriter; /**<Whether the data buffer release by writer*/
	CVI_DATAFIFO_OPEN_MODE_E enOpenMode; /**<READER or WRITER*/
} CVI_DATAFIFO_PARAMS_S;

/** DATAFIFO advanced function */
typedef enum cviDATAFIFO_CMD_E {
	DATAFIFO_CMD_GET_PHY_ADDR, /**<Get the physic address of ring buffer*/
	/**<When the read buffer read over, the reader should
		call this function to notify the writer*/
	DATAFIFO_CMD_READ_DONE,
	DATAFIFO_CMD_WRITE_DONE, /**<When the writer buffer is write done, the writer should call this function*/
	/**<When bDataReleaseByWriter is CVI_TRUE, the writer should call this
		to register release callback*/
	DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK,
	DATAFIFO_CMD_GET_AVAIL_WRITE_LEN, /**<Get available write length*/
	DATAFIFO_CMD_GET_AVAIL_READ_LEN, /**<Get available read length*/
	DATAFIFO_CMD_SHOW_POINTER
} CVI_DATAFIFO_CMD_E;

/**
 * @brief This function malloc ring buffer and initialize DATAFIFO module.
 * when one side call this, the other side should call ::CVI_DATAFIFO_OpenByAddr.
 * @param[out] handle Handle of DATAFIFO.
 * @param[in] pstParams Parameters of DATAFIFO.
 * @return CVI_SUCCESS Initialize DATAFIFO success.
 * @return CVI_FAILURE Initialize DATAFIFO fail.
 */
CVI_S32 CVI_DATAFIFO_Open(CVI_DATAFIFO_HANDLE *Handle, CVI_DATAFIFO_PARAMS_S *pstParams);

/**
 * @brief This function map the ring buffer physic address to its virtue address and initialize DATAFIFO module.
 * This function should be called after the other side call ::CVI_DATAFIFO_Open
   because it need physic address of ring buffer.
 * @param[out] handle Handle of DATAFIFO.
 * @param[in] pstParams Parameters of DATAFIFO.
 * @param[in] u32PhyAddr Physic address of ring buffer. Get it from the other side.
 * @return CVI_SUCCESS Initialize DATAFIFO success.
 * @return CVI_FAILURE Initialize DATAFIFO fail.
 */
CVI_S32 CVI_DATAFIFO_OpenByAddr(CVI_DATAFIFO_HANDLE *Handle,
	CVI_DATAFIFO_PARAMS_S *pstParams, CVI_U64 u64PhyAddr);

/**
 * @brief This function will free or unmap ring buffer and deinitialize DATAFIFO.
 * @param[in] handle Handle of DATAFIFO.
 * @return CVI_SUCCESS Close success.
 * @return CVI_FAILURE Close fail.
 */
CVI_S32 CVI_DATAFIFO_Close(CVI_DATAFIFO_HANDLE Handle);

/**
 * @brief Read data from ring buffer and save it to ppData.
 * every read buffer size is ::u32CacheLineSize
 * @param[in] handle Handle of DATAFIFO.
 * @param[out] ppData Item read.
 * @return CVI_SUCCESS Read success.
 * @return CVI_FAILURE Read fail.
 */
CVI_S32 CVI_DATAFIFO_Read(CVI_DATAFIFO_HANDLE Handle, CVI_VOID **ppData);

/**
 * @brief Write data to ring buffer. data size should be ::u32CacheLineSize
 * @param[in] handle Handle of DATAFIFO.
 * @param[in] pData Item to write.
 * @return CVI_SUCCESS Write success.
 * @return CVI_FAILURE Write fail.
 */
CVI_S32 CVI_DATAFIFO_Write(CVI_DATAFIFO_HANDLE Handle, CVI_VOID *pData);

/**
 * @brief Advanced function. see ::CVI_DATAFIFO_CMD_E
 * @param[in] handle Handle of DATAFIFO.
 * @param[in] enCMD Command.
 * @param[in,out] arg Input or output argument.
 * @return CVI_SUCCESS Call function success.
 * @return CVI_FAILURE Call function fail.
 */
CVI_S32 CVI_DATAFIFO_CMD(CVI_DATAFIFO_HANDLE Handle, CVI_DATAFIFO_CMD_E enCMD, CVI_VOID *pArg);

/** @}*/  /** <!-- ==== DATAFIFO End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
