/* Copyright (c) 2018 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#ifdef FUSION_CONVADD_OP

#include "operators/kernel/conv_add_kernel.h"

namespace paddle_mobile {
namespace operators {

template <>
bool ConvAddKernel<FPGA, float>::Init(FusionConvAddParam<FPGA> *param) {
  // bool relu_enabled = false;
  paddle_mobile::fpga::ActivationType activation_enable =
      paddle_mobile::fpga::NONE;
  int16_t leaky_relu_negative_slope = 0;
  auto input = const_cast<LoDTensor *>(param->Input());
  const Tensor *bias = param->Bias();
  auto bias_ptr = bias->data<float>();
  auto filter = const_cast<LoDTensor *>(param->Filter());
  auto out = param->Output();
  float Si = input->scale[0];
  float So = out->scale[0];
  float Sf = fpga::filter_find_max(filter);

  PADDLE_MOBILE_ENFORCE(out->dims()[1] == bias->dims()[0],
                        "Output channel should be equal to bias number");
  int channel = out->dims()[1];
  auto bs_ptr =
      (float *)fpga::fpga_malloc(2 * channel * sizeof(float));  // NOLINT
  for (int i = 0; i < channel; i++) {
    //    bs_ptr[i + channel] = 1;
    //    bs_ptr[i] = bias_ptr[i];
    bs_ptr[i + channel] = Si / So * Sf / 127.0;
    bs_ptr[i] = bias_ptr[i] * 127.0 / So;
  }

  fpga::format_conv_data(filter, out, &bs_ptr, param->Groups());
  fpga::SplitConvArgs conv_arg = {0};
  fpga::fill_split_arg(&conv_arg, input, out, filter, activation_enable,
                       leaky_relu_negative_slope, param->Groups(),
                       param->Strides()[0], param->Strides()[1],
                       param->Paddings()[0], param->Paddings()[1], bs_ptr);
  param->SetFpgaArgs(conv_arg);
  return true;
}

template <>
void ConvAddKernel<FPGA, float>::Compute(
    const FusionConvAddParam<FPGA> &param) {
  fpga::ComputeFpgaConv(param.FpgaArgs());
}

}  // namespace operators
}  // namespace paddle_mobile

#endif
