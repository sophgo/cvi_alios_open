/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#include <iostream>

#ifdef __linux__
#include <tmedia_config.h>
#endif
#include <tmedia_core/entity/aiengine/aiengine_inc.h>

using namespace std;

ImageProc *ImageProcFactory::CreateEntity()
{
    return mImageProcClass() ? (ImageProc*)mImageProcClass()() : NULL;
}

OSD *OSDFactory::CreateEntity()
{
    return mOSDClass() ? (OSD*)mOSDClass()() : NULL;
}

Interpreter *InterpreterFactory::CreateEntity()
{
    return mInterpreterClass() ? (Interpreter*)mInterpreterClass()() : NULL;
}

FCE *FCEFactory::CreateEntity()
{
    return mFCEClass() ? (FCE*)mFCEClass()() : NULL;
}

