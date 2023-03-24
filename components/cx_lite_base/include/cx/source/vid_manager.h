/**
 * @file vid_manager.h
 * @copyright Copyright (C) 2022 Alibaba Group Holding Limited
 */

#ifndef CX_SROUCE_VID_MANAGER_H
#define CX_SROUCE_VID_MANAGER_H

#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <cx/common/frame.h>
#include <cx/common/type.h>
// #include <cx/source/source_interface.h>
#include <cx/source/vid_channel.h>
#include <cx/source/entity_wrapper.h>
#include <tmedia_core/entity/codec/codec_inc.h>

namespace cx {
namespace source {

class VidSrcManager final {
public:
    int AddBoardConfig(std::shared_ptr<board::BoardConfig> &config);

    // return channel id
    int AddChannelConfig(const VidChannelConfig &config);
    shared_ptr<VidChannel> CreateChannel(int chanId);

    void ChannelInfoDump(void);

    // factory method
    static VidSrcManager *GetInstance(void) {
        return pObj;
    }
    /* channels linked to the same sensor */
    struct ChannelGroup {
        board::SensorConfig::Tag sensorTag;
        std::vector<ChannelPtr> chans;
    };
    vector<ChannelGroup> GetChannelGroup(void) {
        return mChanGroup;
    }
private:

    VidSrcManager();
    ~VidSrcManager();

    void VidChannelProcessRescale(vector<channelConfig> &rescaleList, vector<channelConfig> &cropList, shared_ptr<VpssWrapper> &baseVpss);
    void VidChanneProcesslCrop(std::vector<pair<double, vector<channelConfig>>> &cropList, cx::CropType type);
    int VidChannelCheckMerge(void);

    std::unique_ptr<std::vector<std::pair<VidChannelConfig, ChannelPtr>>> mChannels;
    std::shared_ptr<board::BoardConfig>         mBoardConfig;
    pthread_mutex_t mMutex;
    vector<ChannelGroup> mChanGroup;
    static VidSrcManager *pObj;
};

}
} 



#endif /* CX_SROUCE_VID_MANAGER_H */

