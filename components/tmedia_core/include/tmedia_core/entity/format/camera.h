/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#ifndef TM_FORMAT_CAMERA_H
#define TM_FORMAT_CAMERA_H

#include <string>
#ifdef __linux__
#include <tmedia_config.h>
#endif

#include <tmedia_core/common/common_inc.h>
#include <tmedia_core/entity/format/format_demuxer.h>
#include <tmedia_core/entity/format/format_info.h>
#include <tmedia_core/entity/format/format_io.h>

/*
 * Camera logical parts are below, including: input sensor and output channel
 *  @------------------------------@
 *  |    Camera Settings      _____|______   Channel Settings
 *  |  ===================   |            | =====================
 *  |      Exposure          | channel[0] | configuration: width/height, pix_fmt
 *  |      R/G/B Gain        |_____ ______| ---> Channel Events
 *  |      HDR mode           _____|______
 *  |        ......          |            |
 *  |                        | channel[1] | configuration: width/height, pix_fmt
 *  |                        |_____ ______| ---> Channel Events
 *  |                         _____|______
 *  |                        |            |
 *  |                        | channel[n] | configuration: width/height, pix_fmt
 *  |                        |_____ ______| ---> Channel Events
 *  |        CAMERA                |
 *  @------------------------------@        ===> Camera Events (sensor, error, warning...)
 */

using namespace std;

typedef struct {
    int count;
    struct {
        int mode_id;    // must support mode_id=0 as default mode
        char description[128];
    } modes[8];
} TMCameraModes_s;

typedef struct {
    int   mode_id;
    char *calibriation;	/* set NULL to use default in system  */
    char *lib3a;		/* set NULL to use default in system  */
} TMCameraModeCfg_s;

typedef enum {
	TM_CAMERA_CHANNEL_0 = 0,
	TM_CAMERA_CHANNEL_1,
	TM_CAMERA_CHANNEL_2,
	TM_CAMERA_CHANNEL_3,
	TM_CAMERA_CHANNEL_4,
	TM_CAMERA_CHANNEL_5,
	TM_CAMERA_CHANNEL_6,
	TM_CAMERA_CHANNEL_7,
	TM_CAMERA_CHANNEL_MAX_COUNT
} TMCameraChannelID_e;

typedef enum {
    TMCAMERA_CAPTURE_NONE  = 0,
    TMCAMERA_CAPTURE_VIDEO = 1 << 0,
    TMCAMERA_CAPTURE_AUDIO = 1 << 1,
} TMCameraCaptureType_e;

typedef enum {
    TMCAMERA_MODE_NONE,
    TMCAMERA_COLOR_MODE,
    TMCAMERA_MONO_MODE,
} TMCameraWorkMode_e;

#define CAMERA_NAME_RGB0 "camera_rgb0"
#define CAMERA_NAME_IR0  "camera_ir0"

class TMCamera : public TMVideoInput
{
public:
    enum class PropID : uint32_t
    {
        // Camera property IDs            Data Type
        CAMERA_HFLIP,                   // int, 0: no horizontal flip; 1: horizontal flip
        CAMERA_VFLIP,                   // int, 0: no vertical flip; 1: vertical flip
        CAMERA_ROTATE,                  // int, 0: no rotage; 90, 180, 270
        CAMERA_WORK_MODE,               // TMCameraWorkMode_e

        // Channel property IDs
        CHANNEL_CAPTURE_TYPE,           // int, bitmask by TMCameraCaptureType_e
        CHANNEL_OUTPUT_WIDTH,           // int, <0 means use camera origin width
        CHANNEL_OUTPUT_HEIGHT,          // int, <0 means use camera origin height
        CHANNEL_OUTPUT_PIXEL_FORMAT,    // TMImageInfo::PixelFormat
    };

    TMCamera() {InitDefaultPropertyList();}
    virtual ~TMCamera() {}

    // TMSrcEntity interface
    virtual TMSrcPad *GetSrcPad(int padID = 0) = 0;

    // TMVideoInput interface
    virtual int Open(string deviceName, TMPropertyList *propList = NULL) = 0;
    virtual int Close()                                                  = 0;
    virtual int SetConfig(TMPropertyList &propList)                      = 0;
    virtual int GetConfig(TMPropertyList &propList)                      = 0;
    virtual int Start()                                                  = 0;
    virtual int Stop()                                                   = 0;
    virtual int RecvFrame(TMVideoFrame &frame, int timeout)              = 0;
    virtual int ReleaseFrame(TMVideoFrame &frame)                        = 0;

    // TMCamera interface
    virtual int GetModes(TMCameraModes_s &modes)                              = 0;
    virtual int SetMode(TMCameraModeCfg_s &mode)                              = 0;
    virtual int OpenChannel(int chnID, TMPropertyList *propList = NULL)       = 0;
    virtual int CloseChannel(int chnID)                                       = 0;
    virtual int SetChannelConfig(int chnID, TMPropertyList &propList)         = 0;
    virtual int GetChannelConfig(int chnID, TMPropertyList &propList)         = 0;
    virtual int StartChannel(int chnID)                                       = 0;
    virtual int StopChannel(int chnID)                                        = 0;
    virtual int RecvChannelFrame(int chnID, TMVideoFrame &frame, int timeout) = 0;
    virtual int ReleaseChannelFrame(int chnID, TMVideoFrame &frame)           = 0;

    // TODO: Add 3A_Ready event/function

protected:
    TMPropertyList mDefaultPropertyList[TM_CAMERA_CHANNEL_MAX_COUNT];
    TMPropertyList mCurrentPropertyList[TM_CAMERA_CHANNEL_MAX_COUNT];

    virtual void InitDefaultPropertyList()
    {
        TMPropertyList *pList;
        for (int loop = 0; loop < 2; loop++)
        {
            for (uint32_t i = 0; i < TM_CAMERA_CHANNEL_MAX_COUNT; i++)
            {
                pList = (loop == 0) ? &mDefaultPropertyList[i] :  &mCurrentPropertyList[i];
                pList->Add(TMProperty((int)PropID::CHANNEL_CAPTURE_TYPE, TMCAMERA_CAPTURE_NONE, "channel capture type"));
                pList->Add(TMProperty((int)PropID::CHANNEL_OUTPUT_WIDTH, -1, "channel output width"));
                pList->Add(TMProperty((int)PropID::CHANNEL_OUTPUT_HEIGHT, -1, "channel output height"));
                pList->Add(TMProperty((int)PropID::CHANNEL_OUTPUT_PIXEL_FORMAT, (int32_t)(TMImageInfo::PixelFormat::PIXEL_FORMAT_NV12), "channel output pixel format"));
            }
        }
    }
};

#endif  // TM_FORMAT_CAMERA_H
