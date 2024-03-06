/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#include <tmedia_core/entity/format/format_inc.h>

using namespace std;

TMFormatDemuxer *TMFormatDemuxerFactory::CreateEntity(TMMediaInfo::FormatID formatID, string class_name)
{
    map<TMMediaInfo::FormatID, pair<string, class_new_t>>::iterator it;
    if (class_name.empty()) {
        it = mDemuxerClasses().find(formatID);
        if (it != mDemuxerClasses().end()) {
            TMFormatDemuxer *demuxer = (TMFormatDemuxer*)it->second.second();
            demuxer->mName = "Demuxer_" + TMMediaInfo::Name(formatID);
            return demuxer;
        } else {
            cout << "Format Demuxer '" << TMMediaInfo::Name(formatID) << "' not supported" << endl;
            return NULL;
        }
    } else {
        for (it = mDemuxerClasses().begin(); it != mDemuxerClasses().end(); it++) {
            if ((it->first == formatID) && (it->second.first == class_name))
            {
                TMFormatDemuxer *demuxer = (TMFormatDemuxer*)it->second.second();
                demuxer->mName = "Demuxer_" + TMMediaInfo::Name(formatID);
                return demuxer;
            }
        }
    }
    cout << "Format Demuxer '" << class_name << "' not supported" << endl;
    return NULL;
}

TMFormatMuxer *TMFormatMuxerFactory::CreateEntity(TMMediaInfo::FormatID formatID, string class_name)
{
    map<TMMediaInfo::FormatID, pair<string, class_new_t>>::iterator it;
    if (class_name.empty()) {
        it = mMuxerClasses().find(formatID);
        if (it != mMuxerClasses().end()) {
            return (TMFormatMuxer*)it->second.second();
        } else {
            cout << "Format Muxer '" << TMMediaInfo::Name(formatID) << "' not supported" << endl;
            return NULL;
        }
    } else {
        for (it = mMuxerClasses().begin(); it != mMuxerClasses().end(); it++) {
            if ((it->first == formatID) && (it->second.first == class_name))
                return (TMFormatMuxer*)it->second.second();
        }
    }
    cout << "Format Muxer '" << class_name << "' not supported" << endl;
    return NULL;
}

TMVideoInput *TMFormatVideoInputFactory::CreateEntity(TMMediaInfo::DeviceID deviceID, string class_name)
{
    map<TMMediaInfo::DeviceID, pair<string, class_new_t>>::iterator it;
    if (class_name.empty()) {
        it = mVideoInputClasses().find(deviceID);
        if (it != mVideoInputClasses().end()) {
            return (TMVideoInput*)it->second.second();
        } else {
            cout << "Format VideoInput '" << TMMediaInfo::Name(deviceID) << "' not supported" << endl;
            return NULL;
        }
    } else {
        for (it = mVideoInputClasses().begin(); it != mVideoInputClasses().end(); it++) {
            if ((it->first == deviceID) && (it->second.first == class_name))
                return (TMVideoInput*)it->second.second();
        }
    }
    cout << "Format VideoInput '" << class_name << "' not supported" << endl;
    return NULL;
}

TMVideoOutput *TMFormatVideoOutputFactory::CreateEntity(TMMediaInfo::DeviceID deviceID, string class_name)
{
    map<TMMediaInfo::DeviceID, pair<string, class_new_t>>::iterator it;
    if (class_name.empty()) {
        it = mVideoOutputClasses().find(deviceID);
        if (it != mVideoOutputClasses().end()) {
            return (TMVideoOutput*)it->second.second();
        } else {
            cout << "Format VideoOutput '" << TMMediaInfo::Name(deviceID) << "' not supported" << endl;
            return NULL;
        }
    } else {
        for (it = mVideoOutputClasses().begin(); it != mVideoOutputClasses().end(); it++) {
            if ((it->first == deviceID) && (it->second.first == class_name))
                return (TMVideoOutput*)it->second.second();
        }
    }
    cout << "Format VideoOutput '" << class_name << "' not supported" << endl;
    return NULL;
}

TMAudioInput *TMFormatAudioInputFactory::CreateEntity(TMMediaInfo::DeviceID deviceID, string class_name)
{
    map<TMMediaInfo::DeviceID, pair<string, class_new_t>>::iterator it;
    if (class_name.empty()) {
        it = mAudioInputClasses().find(deviceID);
        if (it != mAudioInputClasses().end()) {
            return (TMAudioInput*)it->second.second();
        } else {
            cout << "Format AudioInput '" << TMMediaInfo::Name(deviceID) << "' not supported" << endl;
            return NULL;
        }
    } else {
        for (it = mAudioInputClasses().begin(); it != mAudioInputClasses().end(); it++) {
            if ((it->first == deviceID) && (it->second.first == class_name))
                return (TMAudioInput*)it->second.second();
        }
    }
    cout << "Format AudioInput '" << class_name << "' not supported" << endl;
    return NULL;
}

TMAudioOutput *TMFormatAudioOutputFactory::CreateEntity(TMMediaInfo::DeviceID deviceID, string class_name)
{
    map<TMMediaInfo::DeviceID, pair<string, class_new_t>>::iterator it;
    if (class_name.empty()) {
        it = mAudioOutputClasses().find(deviceID);
        if (it != mAudioOutputClasses().end()) {
            return (TMAudioOutput*)it->second.second();
        } else {
            cout << "Format AudioOutput '" << TMMediaInfo::Name(deviceID) << "' not supported" << endl;
            return NULL;
        }
    } else {
        for (it = mAudioOutputClasses().begin(); it != mAudioOutputClasses().end(); it++) {
            if ((it->first == deviceID) && (it->second.first == class_name))
                return (TMAudioOutput*)it->second.second();
        }
    }
    cout << "Format AudioOutput '" << class_name << "' not supported" << endl;
    return NULL;
}
