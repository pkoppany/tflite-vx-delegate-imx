/* Copyright 2021 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "utils.h"
#include "tensorflow/lite/minimal_logging.h"

using namespace tflite;

namespace vx {
namespace delegate {
namespace utils {

// transpose channel_dim while doing transpose operation.
int32_t TransposeChannelDim(const std::vector<uint32_t>& perm,
                                   int32_t channel_dim) {
  if (channel_dim < 0) {
    TFLITE_LOG_PROD(TFLITE_LOG_ERROR, "invalid channel_dim");
    return -1;
  }
  for (uint32_t i = 0; i < perm.size(); i++) {
    if (channel_dim == perm.at(i)) {
      return i;
    }
  }
  TFLITE_LOG_PROD(TFLITE_LOG_ERROR, "Can't find channle_dim");
  return -1;
}

// Convert the perm in TfLite to the perm in vx-delegate when transpose.
std::vector<uint32_t> GetOvxTransposePerm(const std::vector<uint32_t>& perm) {
  std::vector<uint32_t> perm_out(perm.rbegin(), perm.rend());
  std::vector<uint32_t> perm_in, ovx_perm;
  for (int i = perm.size() - 1; i >= 0; i--) {
    perm_in.push_back(i);
  }
  for (auto o : perm_out) {
    for (int i = 0; i < perm_in.size(); i++) {
      if (o == perm_in[i]) {
        ovx_perm.push_back(i);
        break;
      }
    }
  }

  return ovx_perm;
}


void GenerateWeightsDataForBilinear(float* data,
                                    const std::vector<uint32_t>& weight_shape,
                                    uint32_t scale_w,
                                    uint32_t scale_h) {
  int32_t width = weight_shape[0];
  int32_t height = weight_shape[1];
  int32_t channel_in = weight_shape[2];
  int32_t channel_out = weight_shape[3];
  for (int o = 0; o < channel_out; o++) {
    for (int h = 0; h < height; h++) {
      float center_w = width % 2 == 1 ? scale_w - 1.0 : scale_w - 0.5;
      float center_h = height % 2 == 1 ? scale_h - 1.0 : scale_h - 0.5;

      for (int w = 0; w < width; w++) {
        data[o * (channel_in + 1) * width * height + h * width + w] =
            (1 - std::abs(w - center_w) / scale_w) *
            (1 - std::abs(h - center_h) / scale_h);
      }
    }
  }

  return;
}

void GenerateWeightDataForNearest(float* data,
                                  const std::vector<uint32_t>& weight_shape) {
  uint32_t width = weight_shape[0];
  uint32_t height = weight_shape[1];
  uint32_t channel_in = weight_shape[2];
  uint32_t channel_out = weight_shape[3];

  for (int o = 0; o < channel_out; o++) {
    for (int h = 0; h < height; h++) {
      for (int w = 0; w < width; w++) {
        data[o * (channel_in + 1) * width * height + h * width + w] = 1;
      }
    }
  }

  return;
}



}  // namespace utils
}  // namespace delegate
}  // namespace vx
