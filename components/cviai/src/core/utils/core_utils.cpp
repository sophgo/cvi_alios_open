#include "core_utils.hpp"
#include "cviai_log.hpp"
// #include "neon_utils.hpp"

#include <cvi_sys.h>
#include <math.h>
#include <string.h>
#include <algorithm>

namespace cviai {

void clip_boxes(int width, int height, cvai_bbox_t &box) {
  if (box.x1 < 0) {
    box.x1 = 0;
  }
  if (box.y1 < 0) {
    box.y1 = 0;
  }
  if (box.x2 > width - 1) {
    box.x2 = width - 1;
  }
  if (box.y2 > height - 1) {
    box.y2 = height - 1;
  }
}
void DequantizeScale(const int8_t *q_data, float *data, float dequant_scale, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    data[i] = float(q_data[i]) * dequant_scale;
  }
}

}  // namespace cviai
