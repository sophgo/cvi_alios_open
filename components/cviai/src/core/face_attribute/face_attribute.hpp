#pragma once
#include <cvi_comm_vb.h>
#include "core.hpp"
#include "core/face/cvai_face_types.h"

namespace cviai {

class FaceAttribute final : public Core {
 public:
  explicit FaceAttribute(bool with_attribute);
  virtual ~FaceAttribute();
  int inference(VIDEO_FRAME_INFO_S *stOutFrame, cvai_face_t *meta, int face_idx = -1);
#if 0
  void setHardwareGDC(bool use_wrap_hw);
#endif

 private:
  // int vpssPreprocess(VIDEO_FRAME_INFO_S *srcFrame, VIDEO_FRAME_INFO_S *dstFrame,
  //                    VPSSConfig &vpss_config) override;
  virtual int setupInputPreprocess(std::vector<InputPreprecessSetup> *data) override;
  virtual int onModelOpened() override;
  virtual int onModelClosed() override;
  void outputParser(cvai_face_t *meta, int meta_i);
  void Preprocessing(cvai_face_info_t *face_info,int img_width, int img_height);
  const bool m_with_attribute;
  VIDEO_FRAME_INFO_S m_wrap_frame;
  VB_BLK m_gdc_blk = (VB_BLK)-1;
#if 0
  bool m_use_wrap_hw;
#endif
};
}  // namespace cviai