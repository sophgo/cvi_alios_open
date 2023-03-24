#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <aos/cli.h>
#include <posix/timer.h>
#include <alsa/pcm.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include "board.h"
#include "aos/cli.h"
#include "vfs.h"

#include "cvi_aacdec.h"
#include "cvi_aacenc.h"

#define CVI_AAC_BUF_SIZE (2048 * 6)

/*samples per frame for AACLC and aacPlus */
#define AACLD_SAMPLES_PER_FRAME 512
#define AACLC_SAMPLES_PER_FRAME 1024
#define AACPLUS_SAMPLES_PER_FRAME 2048

typedef enum cviAAC_TYPE_E {
	AAC_TYPE_AACLC = 0,    /* AAC LC */
	AAC_TYPE_EAAC = 1,     /* eAAC  (HEAAC or AAC+  or aacPlusV1) */
	AAC_TYPE_EAACPLUS = 2, /* eAAC+ (AAC++ or aacPlusV2) */
	AAC_TYPE_AACLD = 3,
	AAC_TYPE_AACELD = 4,
	AAC_TYPE_BUTT,
} AAC_TYPE_E;

typedef struct _st_aac_dec {
    int sample_rate;
    int channel;
    int frame_byte;//ADTS+DATA
    unsigned char *aac_inputbuffer;
    unsigned  int aac_remainbytes;
    unsigned  int aac_bufferbyte;
    short *aac_dec_after_buf;
} ST_AAC_DEC;

int _cvi_aac_decode(CVIAACDecoder Handle, ST_AAC_DEC *pst_aac_dec, unsigned char *pInputBuf, short *pOutbuf, int byte_left,
		    int *frame_byte)
{
	//int sampling_frequency_index;
	int s32Ret;
	int s32TotalSizeBytes = byte_left;
	int poutbuf_byte = 0;

	AACFrameInfo  aacFrameInfo;

	if (byte_left > (int) pst_aac_dec->aac_bufferbyte) {
		printf("byte_left[%d] > aac_bufferbyte[%d]\n", byte_left, (int) pst_aac_dec->aac_bufferbyte);
		return -1;
	}
	if (pst_aac_dec->aac_remainbytes != 0) {
		if ((pst_aac_dec->aac_remainbytes + byte_left) >
		    pst_aac_dec->aac_bufferbyte) {

			printf("remain[%d] + input[%d] > bufsize[%d]",
				  (int)pst_aac_dec->aac_remainbytes,
				  byte_left,
				  (int)pst_aac_dec->aac_bufferbyte);
			printf("force flush the buffer\n");
			memset(pst_aac_dec->aac_inputbuffer, 0, pst_aac_dec->aac_bufferbyte);
			memmove(pst_aac_dec->aac_inputbuffer, pInputBuf, byte_left);
			s32TotalSizeBytes = byte_left;
			pst_aac_dec->aac_remainbytes = s32TotalSizeBytes;
		} else {
			memmove(pst_aac_dec->aac_inputbuffer + pst_aac_dec->aac_remainbytes, pInputBuf, byte_left);
			s32TotalSizeBytes = byte_left + pst_aac_dec->aac_remainbytes;
			//printf("has data s32TotalSizeBytes = %d\n", s32TotalSizeBytes);
		}

	} else {
		memmove(pst_aac_dec->aac_inputbuffer, pInputBuf, byte_left);
		s32TotalSizeBytes = byte_left;
		//printf("[%s][%d] inputlen[%d]\n", __func__, __LINE__, byte_left);
	}

	pst_aac_dec->aac_remainbytes = s32TotalSizeBytes;

	while (1) {
		if (pst_aac_dec->aac_remainbytes > 7) {

			if ((pst_aac_dec->aac_inputbuffer[0] == 0xff) &&
			    ((pst_aac_dec->aac_inputbuffer[1] & 0xf0) == 0xf0)) {//adts

				//sampling_frequency_index = (pst_aac_dec->aac_inputbuffer[2] & 0x3C) >> 2;
				//pst_aac_dec->sample_rate = get_aac_sample_rate(sampling_frequency_index);
				pst_aac_dec->channel = ((pst_aac_dec->aac_inputbuffer[2] & 0x01) << 2) |
						       ((pst_aac_dec->aac_inputbuffer[3] & 0xC0) >> 6);

				pst_aac_dec->frame_byte = ((pst_aac_dec->aac_inputbuffer[3] & 0x03) << 11) |
					(pst_aac_dec->aac_inputbuffer[4] << 3) | (pst_aac_dec->aac_inputbuffer[5] >> 5);

				//printf("packer_byte = %d,channel = %d\n",
				//	  pst_aac_dec->frame_byte,
				//	  pst_aac_dec->channel);

				if (pst_aac_dec->aac_remainbytes < (unsigned int)pst_aac_dec->frame_byte) {
					//printf(" ADTS_byte(7) <remainbytes < aac frame_byte\n");
					break;
				}

				int output_bytes = 0;
				int vail = pst_aac_dec->frame_byte;

				s32Ret = AACDecodeFrame(Handle, (CVI_U8 **)(&pst_aac_dec->aac_inputbuffer),
					(CVI_S32 *)&vail, (CVI_S16 *)pst_aac_dec->aac_dec_after_buf);
				if (s32Ret != 0) {
					printf("aac one frame decode faile\n");
					return -1;
				}

				AACGetLastFrameInfo(Handle, &aacFrameInfo);
				aacFrameInfo.nChans = ((aacFrameInfo.nChans != 0) ? aacFrameInfo.nChans : 1);

				/* samples per frame of one sound track*/
				int output_Samples = aacFrameInfo.outputSamps;

				if ((output_Samples != AACLC_SAMPLES_PER_FRAME) &&
					(output_Samples != AACPLUS_SAMPLES_PER_FRAME) &&
					(output_Samples != AACLD_SAMPLES_PER_FRAME)) {
					printf("aac decoder failed! [%d]\n", output_Samples);
					return -1;
				}

				output_bytes = output_Samples * sizeof(CVI_U16) * aacFrameInfo.nChans;
				//int totalbytepass =  aacFrameInfo.bytespassDec;

				pst_aac_dec->aac_remainbytes = pst_aac_dec->aac_remainbytes -
							       pst_aac_dec->frame_byte;

				memmove(pst_aac_dec->aac_inputbuffer,
				       pst_aac_dec->aac_inputbuffer + pst_aac_dec->frame_byte,
				       pst_aac_dec->aac_remainbytes);

				memmove(pOutbuf + poutbuf_byte/sizeof(short),
					pst_aac_dec->aac_dec_after_buf, output_bytes);
				poutbuf_byte += output_bytes;
				*frame_byte = poutbuf_byte;

				//printf("output_bytes = %d, poutbuf_byte = %d, totalbytepass = %d, bitrate = %d\n",
				//	  output_bytes, poutbuf_byte, totalbytepass, aacFrameInfo.bitRate);

			} else {
				memmove(pst_aac_dec->aac_inputbuffer, pst_aac_dec->aac_inputbuffer + 1,
						pst_aac_dec->aac_remainbytes - 1);
				pst_aac_dec->aac_remainbytes -= 1;
				//printf("aac_inputbuffer is not ADTS\n");
			}

		} else {
			//printf("remainbytes = < ADTS_byte(7)\n");
			break;
		}

	}

	return 0;
}

static int audio_debug_aac_cmd(int argc, char *argv[])
{
    int fdin = -1;
    int fdout = -1;
    int channel = 2;
    int sample_rate = 48000;
    int inFrameByte = 0;
    int outFrameByte = 0;
    int s32Readbyte = 0;
    int gAACType = 0;
    char *datain;
    char *dataout;
    int enorDe;
    int enBitRate;
    int s32Ret;

    printf("***********************************************\n"
           "***Audio AAC TEST[internal test] ***\n"
           "usage	: <input.raw> <output.raw> <sample rate> <channel num> <AAC type> <encode or decode>\n"
           "sample rate, 8K~96K\n"
           "channel num: 1 or 2\n"
           "AAC type, 0:aaclc, 1:eaac, 2: heaac\n"
           "encode or decode, 0:encode, 1:decode\n"
           "bitRate\n"
           "***********************************************\n");
    printf("***********************************************\n"
           "***For example ***\n"
           "encode aac: audio_test_aac /mnt/sd/QA_16k.qa_pcm  /mnt/sd/QA_16k.qa_pcm.aac 16000 2 0 0 64000\n"
           "decode aac: audio_test_aac /mnt/sd/QA_16k.qa_pcm.aac  /mnt/sd/QA_16k.qa_pcm.aac.raw 16000 2 0 1 64000\n"
           "***********************************************\n");
    if (argc < 8) {
        printf("[Error]Please check the usage\n");
        printf("[Error]Input argument is not enough!!!\n");
        return -1;
    }

    sample_rate = atoi(argv[3]);

    channel = atoi(argv[4]);
    gAACType = atoi(argv[5]);
    enorDe = atoi(argv[6]); //0, encode, 1, decode
    enBitRate = atoi(argv[7]);

    fdin = aos_open(argv[1], O_RDONLY);
    fdout = aos_open(argv[2], O_CREAT | O_TRUNC | O_RDWR);
    if (fdin <= 0 || fdout <= 0) {
        printf("can't open file [%d, %d] \n", fdin, fdout);
        return -1;
    }

    printf("sample_rate=%d, channel=%d, aacType=%d, enorDe=%d, bitRate=%d\n",
        sample_rate, channel, gAACType, enorDe, enBitRate);

    printf("process start\n");

    if (enorDe == 0) {
        int s32framesize = 0;
        printf("encode aac: \n");

        AACENC_CONFIG config;
        AAC_ENCODER_S *pstAACState;

        AACInitDefaultConfig(&config);

        config.coderFormat = (AuEncoderFormat)gAACType;
        config.bitRate = enBitRate;
        config.bitsPerSample = 16;
        config.sampleRate = sample_rate;
        config.bandWidth = sample_rate/2;
        config.transtype = AACENC_ADTS;
        printf("[OpenAACEncoder_][%d][%d][%d][%d][%d][%d]\n",
            config.coderFormat,
            config.bitRate,
            config.bitsPerSample,
            config.sampleRate,
            config.bandWidth,
            config.transtype);
        if (channel == 1 && gAACType != AAC_TYPE_EAACPLUS) {
            config.nChannelsIn = 1;
            config.nChannelsOut = 1;
        } else {
            config.nChannelsIn = 2;
            config.nChannelsOut = 2;
        }

        config.quality = AU_QualityHigh;

        /* create encoder */
        s32Ret = AACEncoderOpen(&pstAACState, &config);
        if (s32Ret) {
            printf("[Func]:%s [Line]:%d s32Ret:0x%x.#########\n", __func__, __LINE__, s32Ret);
            return s32Ret;
        }

        if (config.coderFormat == (AuEncoderFormat)AAC_TYPE_AACLC)
			s32framesize = AACLC_SAMPLES_PER_FRAME;
		else if (config.coderFormat == (AuEncoderFormat)AAC_TYPE_EAAC ||
				config.coderFormat == (AuEncoderFormat)AAC_TYPE_EAACPLUS)
			s32framesize = AACPLUS_SAMPLES_PER_FRAME;
		else if (config.coderFormat == (AuEncoderFormat)AAC_TYPE_AACLD ||
				config.coderFormat == (AuEncoderFormat)AAC_TYPE_AACELD)
			s32framesize = AACLD_SAMPLES_PER_FRAME;

		if (channel ==  2)
			s32framesize = s32framesize * 2;

        datain = malloc(s32framesize * sizeof(short));
        dataout = malloc(s32framesize * sizeof(short));
        if (datain == NULL || dataout == NULL) {
            printf("datain or dataout malloc error\n");
            return -1;
        }
        inFrameByte = s32framesize * sizeof(short);
        outFrameByte = inFrameByte;
        printf("Encode each frame with samples size[%d]\n", s32framesize);
        CVI_S32 s32Cnt = 0;
        while (1) {
            s32Cnt++;
            s32Readbyte = aos_read(fdin, datain, inFrameByte);
            if (s32Readbyte == 0 || s32Readbyte < 0) {
                break;
            }
            //do the aac encode frame by frame
            s32Ret = AACEncoderFrame(pstAACState, (short *)datain, (unsigned char *)dataout,
                        inFrameByte, &outFrameByte);

            if (s32Ret != 0) {
                printf("[Func]:%s [Line]:%d [Info]:%s32framesize[%d]\n",
                        __func__,
                        __LINE__,
                        "AAC encode failed",
                        s32framesize);
                return s32Ret;
            } else{
                 //printf("fwrite inFrameByte[%d], outFrameByte[%d] line[%d]Cnt[%d]\n", inFrameByte, outFrameByte, __LINE__, s32Cnt);
                 aos_write(fdout, dataout, outFrameByte);
            }

        }
        AACEncoderClose(pstAACState);
        free(datain);
        free(dataout);

    } else if (enorDe == 1) {
        printf("decode aac: \n");
        ST_AAC_DEC *pst_aac_dec = (ST_AAC_DEC *)malloc(sizeof(ST_AAC_DEC));
		if (!pst_aac_dec) {
			printf("malloc error\n");
			return -1;
		}
        memset(pst_aac_dec, 0, sizeof(ST_AAC_DEC));
        pst_aac_dec->sample_rate = 0;
	    pst_aac_dec->channel = 0;
		pst_aac_dec->frame_byte = 0;
		pst_aac_dec->aac_remainbytes = 0;
		pst_aac_dec->aac_bufferbyte = CVI_AAC_BUF_SIZE;
		pst_aac_dec->aac_inputbuffer = (unsigned char *)malloc(CVI_AAC_BUF_SIZE);
		pst_aac_dec->aac_dec_after_buf = (short *)malloc(CVI_AAC_BUF_SIZE);
        if (!pst_aac_dec->aac_inputbuffer
			|| !pst_aac_dec->aac_dec_after_buf) {
			printf("malloc error, aac_inputbuffer[%p], aac_dec_after_buf[%p]\n",
					pst_aac_dec->aac_inputbuffer,
					pst_aac_dec->aac_dec_after_buf);
			return CVI_FAILURE;
		}
        memset(pst_aac_dec->aac_inputbuffer, 0, CVI_AAC_BUF_SIZE);
		memset(pst_aac_dec->aac_dec_after_buf, 0, CVI_AAC_BUF_SIZE);

        AACENCTransportType enTranType = AACDEC_ADTS;
        CVIAACDecoder decHandle = AACInitDecoder(enTranType);

        int s32framesize = 1024;
        if (gAACType == AAC_TYPE_AACLC)
			s32framesize = AACLC_SAMPLES_PER_FRAME;
		else if (gAACType == AAC_TYPE_EAAC ||
				gAACType == AAC_TYPE_EAACPLUS)
			s32framesize = AACPLUS_SAMPLES_PER_FRAME;
		else if (gAACType == AAC_TYPE_AACLD ||
				gAACType == AAC_TYPE_AACELD)
			s32framesize = AACLD_SAMPLES_PER_FRAME;

        if (channel ==  2)
		s32framesize = s32framesize * 2;

        datain = malloc(s32framesize * sizeof(short));
        dataout = malloc(s32framesize * sizeof(short) * 120);
        if (datain == NULL || dataout == NULL) {
            printf("datain or dataout malloc error\n");
            return -1;
        }
        inFrameByte = s32framesize * sizeof(short);
        outFrameByte = inFrameByte;

        CVI_S32 s32Cnt = 0;
        while (1) {
            s32Cnt++;
            s32Readbyte = aos_read(fdin, datain, inFrameByte);
            if (s32Readbyte == 0 || s32Readbyte < 0) {
                break;
            }
            //do the aac decode frame by frame
            s32Ret = _cvi_aac_decode(decHandle, pst_aac_dec,
					 (unsigned char *)datain, (short *)dataout,
					 inFrameByte, &outFrameByte);

            if (s32Ret != 0) {
                printf("[Func]:%s [Line]:%d [Info]:%s32framesize[%d]\n",
                        __func__,
                        __LINE__,
                        "AAC dncode failed",
                        s32framesize);
                return s32Ret;
            } else{
                 //printf("fwrite inFrameByte[%d], outFrameByte[%d] line[%d]Cnt[%d]\n", inFrameByte, outFrameByte, __LINE__, s32Cnt);
                 aos_write(fdout, dataout, outFrameByte);
            }

        }

        AACFreeDecoder(decHandle);
        free(datain);
        free(dataout);
		printf("AAC decode end\n");
    } else {
        printf("encode or decode type wrong\n");
        return -1;
    }

    aos_close(fdout);
    aos_close(fdin);
    printf("process finish\n");
    return 0;
}


ALIOS_CLI_CMD_REGISTER(audio_debug_aac_cmd, audio_test_aac, audio aac test);

