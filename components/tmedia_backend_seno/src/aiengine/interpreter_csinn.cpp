/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */
#ifdef __linux__
#include <tmedia_config.h>
#endif
#ifdef CONFIG_TMEDIA_AIENGINE_INTERPRETER_CSINN
#include <dlfcn.h>
#include <iostream>
#include <fstream>
#include <tmedia_core/entity/aiengine/aiengine_inc.h>
#include <tmedia_backend_seno/aiengine/interpreter_csinn.h>

static inline DataType_e DataType_CSI2AIE(int32_t csi_dtype)
{
    switch (csi_dtype)
    {
    case CSINN_DTYPE_INT8:
        return DATA_8S;
    case CSINN_DTYPE_UINT8:
        return DATA_8U;
    case CSINN_DTYPE_INT16:
        return DATA_16S;
    case CSINN_DTYPE_UINT16:
        return DATA_16U;
    case CSINN_DTYPE_INT32:
        return DATA_32S;
    case CSINN_DTYPE_UINT32:
        return DATA_32U;
    case CSINN_DTYPE_FLOAT16:
        return DATA_16F;
    case CSINN_DTYPE_FLOAT32:
        return DATA_32F;
    case CSINN_DTYPE_FLOAT64:
        return DATA_64F;
    default:
        break;
    }
    return DATA_8U;
}

template <typename T>
T ProductVector(std::vector<T> &vec)
{
    T res = 1;
    for (size_t i = 0; i < vec.size(); i++)
    {
        res *= vec[i];
    }
    return res;
}

InterpreterCSINN::InterpreterCSINN()
{
    mDev = -1;
    mSess = NULL;
    mOutput = NULL;
    lib_handle = NULL;
}

InterpreterCSINN::~InterpreterCSINN()
{
}

int InterpreterCSINN::Open(int dev)
{
    mDev = dev;
    return TMResult::TM_OK;
}

int InterpreterCSINN::Close()
{
    return TMResult::TM_OK;
}

int InterpreterCSINN::LoadNet(const InterpreterNet *net)
{
    if (net == NULL)
    {
        return TMResult::TM_EINVAL;
    }

    std::string suffix_bm = net->weights.substr(net->weights.length() - 3); // .bm

    lib_handle = dlopen(net->model.c_str(), RTLD_NOW);
    if (!lib_handle)
    {
        printf("dlopen failed:%s lib:%s\n", dlerror(), net->model.c_str());
        return TMResult::TM_ENOENT;
    }

    // create network
    std::ifstream file(net->weights.data(), std::ios::binary);
    file.seekg(0, std::ios::end);
    mData.resize(file.tellg());
    file.seekg(0);
    file.read(reinterpret_cast<char *>(mData.data()), mData.size());
    file.close();
    char *params = reinterpret_cast<char *>(mData.data());
    if (params == NULL)
    {
        printf("fail to read file:%s\n", net->weights.c_str());
        return TMResult::TM_ENOENT;
    }

    if (suffix_bm == ".bm")
    {
        struct shl_bm_sections *section = (struct shl_bm_sections *)(params + 4128);
        if (section->graph_offset)
        {
            // run model for inferencing with binary graph
            mSess = csinn_import_binary_model(params);
            if (!mSess)
                printf("csinn_import_binary_model failed.\n");
        }
        else
        {
            // generate binary graph by .bm
            void *(*csinn_)(char *params_base);
            csinn_ = (decltype(csinn_))dlsym(lib_handle, "csinn_");
            mSess = (struct csinn_session *)csinn_(params + section->params_offset * 4096);
            if (!mSess)
                printf("csinn_ failed.\n");
        }
    }
    else
    {
        printf("invalid weight file:%s\n", net->weights.c_str());
        return TMResult::TM_ENOENT;
    }

    return TMResult::TM_OK;
}

int InterpreterCSINN::UnLoadNet()
{
    if (lib_handle)
    {
        dlclose(lib_handle);
    }

    csinn_session_deinit(mSess);
    csinn_free_session(mSess);

    return TMResult::TM_OK;
}

int InterpreterCSINN::SetNetConfig(const NetConfig_t *cfg)
{
    return TMResult::TM_EBUSY;
}

int InterpreterCSINN::GetNetConfig(NetConfig_t *cfg)
{
    return TMResult::TM_EBUSY;
}

int InterpreterCSINN::GetPerfProfile(std::vector<std::string> &names, std::vector<uint32_t> &timings_us)
{
    return TMResult::TM_EBUSY;
}

int InterpreterCSINN::GetInputTensor(Tensor_t **input)
{
    *input = new Tensor_t();
    Tensor_t *input_p = *input;

    struct csinn_tensor *input_tensor;

    input_p->mtx_num = csinn_get_input_number(mSess);
    input_p->mtx = new Matrix_t[input_p->mtx_num];
    for (int i = 0; i < input_p->mtx_num; i++)
    {
        input_tensor = mSess->input[i];
        // print_tensor_info(input_tensor);
        struct csinn_quant_info *qinfo = input_tensor->qinfo;
        input_p->mtx[i].mquant.scale = qinfo->scale;
        input_p->mtx[i].mquant.zero_point = qinfo->zero_point;
        input_p->mtx[i].mquant.min = qinfo->min;
        input_p->mtx[i].mquant.max = qinfo->max;

        input_p->mtx[i].mspec.name = input_tensor->name;
        input_p->mtx[i].mspec.dims.resize(input_tensor->dim_count);
        for (int j = 0; j < input_tensor->dim_count; j++)
        {
            input_p->mtx[i].mspec.dims[j] = input_tensor->dim[j];
        }

        input_p->mtx[i].mdata.type = DataType_CSI2AIE(input_tensor->dtype);
        input_p->mtx[i].mdata.size = ProductVector(input_p->mtx[i].mspec.dims);
    }

    return TMResult::TM_OK;
}

int InterpreterCSINN::GetOutputTensor(Tensor_t **output)
{
    *output = new Tensor_t();
    Tensor_t *output_p = *output;

    struct csinn_tensor *output_tensor;

    output_p->mtx_num = csinn_get_output_number(mSess);
    output_p->mtx = new Matrix_t[output_p->mtx_num];
    for (int i = 0; i < output_p->mtx_num; i++)
    {
        output_tensor = mSess->output[i];
        // print_tensor_info(output_tensor);
        struct csinn_quant_info *qinfo = output_tensor->qinfo;
        output_p->mtx[i].mquant.scale = qinfo->scale;
        output_p->mtx[i].mquant.zero_point = qinfo->zero_point;
        output_p->mtx[i].mquant.min = qinfo->min;
        output_p->mtx[i].mquant.max = qinfo->max;

        output_p->mtx[i].mspec.name = output_tensor->name;
        output_p->mtx[i].mspec.dims.resize(output_tensor->dim_count);
        for (int j = 0; j < output_tensor->dim_count; j++)
        {
            output_p->mtx[i].mspec.dims[j] = output_tensor->dim[j];
        }

        output_p->mtx[i].mdata.type = DataType_CSI2AIE(output_tensor->dtype);
        output_p->mtx[i].mdata.size = ProductVector(output_p->mtx[i].mspec.dims);
    }

    return TMResult::TM_OK;
}

int InterpreterCSINN::SetInputTensor(const Tensor_t *input)
{
    if (input == NULL)
    {
        return TMResult::TM_EINVAL;
    }

    struct csinn_tensor input_tensor;
    for (int i = 0; i < input->mtx_num; i++)
    {
        input_tensor.data = *(input->mtx[i].mdata.data);
        csinn_update_input(i, &input_tensor, mSess);
    }

    return TMResult::TM_OK;
}

int InterpreterCSINN::SetOutputTensor(const Tensor_t *output)
{
    if (output == NULL)
    {
        return TMResult::TM_EINVAL;
    }

    mOutput = output;

    return TMResult::TM_OK;
}

int InterpreterCSINN::Run(int32_t timeout)
{
    int ret = csinn_session_run(mSess);
    if (ret == CSINN_TRUE)
    {
        struct csinn_tensor *output_tensor = csinn_alloc_tensor(NULL);
        for (int i = 0; i < mOutput->mtx_num; i++)
        {
            output_tensor->data = NULL;
            csinn_get_output(i, output_tensor, mSess);
            memcpy(mOutput->mtx[i].mdata.data[0], output_tensor->data, mOutput->mtx[i].mdata.size);
        }
        csinn_free_tensor(output_tensor);

        return TMResult::TM_OK;
    }
    else
    {
        printf("csinn_session_run failed. ret=%d\n", ret);
    }

    return TMResult::TM_EBUSY;
}

int InterpreterCSINN::ReleaseTensor(Tensor_t *tensor)
{
    if (tensor == NULL)
    {
        return TMResult::TM_EINVAL;
    }

    delete[] tensor->mtx;
    delete tensor;

    return TMResult::TM_OK;
}

REGISTER_INTERPRETER_CLASS(InterpreterCSINN);

#endif
