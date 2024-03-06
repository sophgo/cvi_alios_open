#include <stdio.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "ScopTime.hpp"
#include "aos/cli.h"
#include "cvi_tpu_interface.h"
#include "cviruntime.h"
#include "fatfs_vfs.h"
#include "vfs.h"

using namespace std;

static int BGRPacked2BGRPlanar(uint8_t *packed, uint8_t *planar, int height, int width) {
    uint8_t *p_img = (uint8_t *)packed;
    uint8_t *p_r   = planar + height * width * 2;
    uint8_t *p_g   = planar + height * width;
    uint8_t *p_b   = planar;

    for (int i = 0; i < height * width; i++) {
        *p_b++ = *p_img++;
        *p_g++ = *p_img++;
        *p_r++ = *p_img++;
    }
    return 0;
}

static void load_file(const char *file_name, void **ptr, size_t *size) {
    int file_fd = aos_open(file_name, O_RDONLY);
    if (file_fd < 0) {
        printf("open label file failed %s\n", file_name);
        exit(1);
    }
    aos_statfs_t label_stat;
    int ret = aos_stat(file_name, &label_stat);
    if (ret < 0) {
        printf("get file stat failed\n");
        exit(1);
    }
    if (*size <= 0) {
        *ptr  = malloc(label_stat.st_size);
        *size = label_stat.st_size;
    }
    size_t r_size = aos_read(file_fd, *ptr, *size);
    if (r_size != *size) {
        printf("load file error\n");
        assert(0);
    }
    aos_close(file_fd);
    return;
}

/**
 * @brief 
 * test mobilenet_v2_fused_preprocess.cvimodel
 * @param 
 * need 3 files
 * cvimodel           SD_FATFS_MOUNTPOINT "/mobilenet_v2_fused_preprocess.cvimodel"
 * image.bgr          SD_FATFS_MOUNTPOINT "/cat_256x256.bgr"     pixel format:bgr888_package binary file
 * synset_words.txt   SD_FATFS_MOUNTPOINT "/synset_words.txt"
 * 
 * @param  
 * @return int 
 */

int test_classifier(int argc, char **argv) {
    cvi_tpu_init();
    // load model file
    const char *model_file = SD_FATFS_MOUNTPOINT "/mobilenet_v2_fused_preprocess.cvimodel";
    CVI_MODEL_HANDLE model = nullptr;
    int ret                = CVI_NN_RegisterModel(model_file, &model);
    if (CVI_RC_SUCCESS != ret) {
        printf("CVI_NN_RegisterModel failed, err %d\n", ret);
        exit(1);
    }
    printf("CVI_NN_RegisterModel succeeded\n");

    // get input output tensors
    CVI_TENSOR *input_tensors;
    CVI_TENSOR *output_tensors;
    int32_t input_num;
    int32_t output_num;
    CVI_NN_GetInputOutputTensors(model, &input_tensors, &input_num, &output_tensors,
                                 &output_num);
    CVI_TENSOR *input = CVI_NN_GetTensorByName(CVI_NN_DEFAULT_TENSOR, input_tensors, input_num);
    assert(input);
    printf("input, name:%s\n", input->name);
    CVI_TENSOR *output = CVI_NN_GetTensorByName(CVI_NN_DEFAULT_TENSOR, output_tensors, output_num);
    assert(output);

    CVI_SHAPE shape = CVI_NN_TensorShape(input);

    // nchw
    int32_t height = shape.dim[2];
    int32_t width  = shape.dim[3];

    // imread
    const char *img_name = SD_FATFS_MOUNTPOINT "/cat_256x256.bgr";
    uint8_t *ptr         = NULL;
    size_t img_size      = 0;
    load_file(img_name, (void **)&ptr, &img_size);
    assert(img_size == (size_t)(3 * width * height));
    printf("read img file success\n");

    uint8_t *ptr_planar = (uint8_t *)malloc(img_size);
    BGRPacked2BGRPlanar(ptr, ptr_planar, height, width);
    memcpy(CVI_NN_TensorPtr(input), ptr_planar, CVI_NN_TensorSize(input));
    free(ptr);
    free(ptr_planar);

    // run inference
    CVI_NN_Forward(model, input_tensors, input_num, output_tensors, output_num);
    printf("CVI_NN_Forward succeeded\n");

    std::vector<std::string> labels;
    const char *label_name = SD_FATFS_MOUNTPOINT "/synset_words.txt";
    std::ifstream file(label_name);
    if (!file) {
        printf("Didn't find synset_words file\n");
        exit(1);
    } else {
        std::string line;
        while (std::getline(file, line)) {
            labels.push_back(std::string(line));
        }
    }

    int32_t top_num = 5;
    float *prob     = (float *)CVI_NN_TensorPtr(output);
    int32_t count   = CVI_NN_TensorCount(output);
    // find top-k prob and cls
    std::vector<size_t> idx(count);
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(), [&prob](size_t idx_0, size_t idx_1) { return prob[idx_0] > prob[idx_1]; });
    // show results.
    printf("------\n");
    for (int32_t i = 0; i < top_num; i++) {
        int top_k_idx = idx[i];
        printf("  %f, idx %d", prob[top_k_idx], top_k_idx);
        if (!labels.empty())
            printf(", %s", labels[top_k_idx].c_str());
        printf("\n");
    }
    printf("------\n");
    CVI_NN_CleanupModel(model);
    printf("CVI_NN_CleanupModel succeeded\n");
    return 0;
}

ALIOS_CLI_CMD_REGISTER(test_classifier, test_classifier, test tpu sdk);