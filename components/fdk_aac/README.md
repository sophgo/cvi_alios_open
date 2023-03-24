## 简介

FDK-AAC是一个开源的MPEG-4和MPEG-2 AAC解码器

官网地址： https://www.iis.fraunhofer.de/

## 如何使用

### libfdk-aac库的编译

- 下载fdk-aac-2.0.1.tar.gz并解压。
- 执行build脚本，编译并安装到当前目录output文件夹下。build脚本如下：

```c
CROSS_COMPILE=
ARCH=

TARGET_DIR=$(cd `dirname $0`; pwd)/output
CURRENT_DIR=$(cd `dirname $0`; pwd)

build_fdkaac() {
    cd fdk-aac-2.0.1
	autoreconf -fiv
    ./configure   --prefix=$TARGET_DIR \
		--enable-example \
        --disable-shared \
		--enable-silent-rules \
		CFLAGS="-g -O0 -fPIC " \
		CC=$CROSS_COMPILE\gcc \
		STRIP=$CROSS_COMPILE\strip \
		RANLIB=$CROSS_COMPILE\ranlib \
		OBJDUMP=$CROSS_COMPILE\objdump \
		NM=$CROSS_COMPILE\nm \
		AR=$CROSS_COMPILE\ar \
		AS=$CROSS_COMPILE\as ;
    make ; make install || exit 1
	make distclean 
    cd -
}

build_fdkaac


```

- 编译出的结果如下：

```c
yingc@yingc:~/thirdparty/fdk-aac/output$ tree
.
├── bin
│   ├── aac-enc
├── include
│   └── fdk-aac
│       ├── aacdecoder_lib.h
│       ├── aacenc_lib.h
│       ├── FDK_audio.h
│       ├── genericStds.h
│       ├── machine_type.h
│       └── syslib_channelMapDescr.h
├── lib
│   ├── libfdk-aac.a
│   ├── libfdk-aac.la
│   └── pkgconfig
│       └── fdk-aac.pc

```

### m4a文件解码示例

```

- 解码m4a文件(注意：示例为主要代码，g_indexes[i].size需要自行从m4a复用格式中算出来)。如下所示：

```c

int main()
{
	HANDLE_AACDECODER handle;
        // 打开aac解码器，采用raw格式
	handle = aacDecoder_Open(TT_MP4_RAW, 1);
	int conceal_method=2;//0 muting 1 noise 2 interpolation
        // raw解码参数，ASC(audio specific config)
	unsigned char extradata[]  = {0x13, 0x88,    0x56,    0xe5,    0xa5,    0x48,    0x80};
	int extradata_size    = 7;
	unsigned char *conf[] = { extradata };

	char *decdata = malloc(DEC_DATA_SIZE);
	printf("dec data ptr = %p\n", decdata);
        // 配置raw参数
	AAC_DECODER_ERROR err = aacDecoder_ConfigRaw(handle, conf, &extradata_size);
	aacDecoder_SetParam(handle, AAC_CONCEAL_METHOD,conceal_method);
        // 将音频输出配置为单声道
	aacDecoder_SetParam(handle,  AAC_PCM_MAX_OUTPUT_CHANNELS,1);
	aacDecoder_SetParam(handle,  AAC_PCM_MIN_OUTPUT_CHANNELS,1);

	unsigned int valid;
	UCHAR *data;
	for (i = 0; i < ARRAY_SIZE(g_indexes); i++) {
		size = g_indexes[i].size;
		offset = g_indexes[i].pos;
		//printf("======>size = %d, offset = %d\n", size, offset);

		valid = size;
		data = _s2_m4a + offset;
                // 填充aac编码数据到内部buffer中
		err = aacDecoder_Fill(handle, &data, &size, &valid);
                // 解码一帧数据，解码结果存放到decdata
		err = aacDecoder_DecodeFrame(handle, (INT_PCM *)decdata, DEC_DATA_SIZE/sizeof(INT_PCM), 0);
		if (!(err == 0 || err == AAC_DEC_NOT_ENOUGH_BITS)) {
			ret = -1;
			printf("aac decode fail. ");
			break;
		}
		CStreamInfo *info = aacDecoder_GetStreamInfo(handle);
		printf("==>>>err = 0x%x, %d\n", err, info ? info->sampleRate : -1);
#ifdef DEBUG_DUMP_DATA
		{
			if (info->frameSize) {
                                // 将解码后的数据dump到某片内存中
				int s = info->frameSize * info->numChannels * (16 / 8);
				if (pcm_offset + s < DUMP_DATA_SIZE) {
					memcpy(pcm_data + pcm_offset, decdata, s);
					pcm_offset += s;
				} else {
					break;
				}
			}
		}
#endif
	}
err:
        // 关闭aac句柄，销毁相关资源
	if (handle)
		aacDecoder_Close(handle);
	return 0;

}
 
```

