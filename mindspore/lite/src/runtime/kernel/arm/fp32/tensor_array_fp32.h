/**
 * Copyright 2021 Huawei Technologies Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP32_TENSORARRAY_H_
#define MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP32_TENSORARRAY_H_

#include <vector>
#include <memory>
#include "nnacl/tensor_array_parameter.h"
#include "src/inner_kernel.h"
#include "src/tensorlist.h"

namespace mindspore::kernel {
class TensorArrayCPUKernel : public InnerKernel {
 public:
  TensorArrayCPUKernel(OpParameter *parameter, const std::vector<lite::Tensor *> &inputs,
                       const std::vector<lite::Tensor *> &outputs, const lite::InnerContext *ctx)
      : InnerKernel(parameter, inputs, outputs, ctx) {
    ta_param_ = reinterpret_cast<TensorArrayParameter *>(parameter);
  }

  ~TensorArrayCPUKernel() = default;

  int Init() override;
  int ReSize() override { return 0; }
  int Run() override;

 private:
  TensorArrayParameter *ta_param_{nullptr};
  std::unique_ptr<lite::TensorList> tensor_list_;
};

class TensorArrayBaseCPUKernel : public InnerKernel {
 public:
  TensorArrayBaseCPUKernel(OpParameter *parameter, const std::vector<lite::Tensor *> &inputs,
                           const std::vector<lite::Tensor *> &outputs, const lite::InnerContext *ctx)
      : InnerKernel(parameter, inputs, outputs, ctx) {}
  ~TensorArrayBaseCPUKernel() = default;

  int Init() override;
  int ReSize() override { return 0; }
  inline int Run() override;

 protected:
  lite::Tensor *handle_{nullptr};
  int index_{0};
};

class TensorArrayReadCPUKernel : public TensorArrayBaseCPUKernel {
 public:
  TensorArrayReadCPUKernel(OpParameter *parameter, const std::vector<lite::Tensor *> &inputs,
                           const std::vector<lite::Tensor *> &outputs, const lite::InnerContext *ctx)
      : TensorArrayBaseCPUKernel(parameter, inputs, outputs, ctx) {}
  ~TensorArrayReadCPUKernel() = default;

  int Init() override;
  int ReSize() override { return 0; }
  int Run() override;
};

class TensorArrayWriteCPUKernel : public TensorArrayBaseCPUKernel {
 public:
  TensorArrayWriteCPUKernel(OpParameter *parameter, const std::vector<lite::Tensor *> &inputs,
                            const std::vector<lite::Tensor *> &outputs, const lite::InnerContext *ctx)
      : TensorArrayBaseCPUKernel(parameter, inputs, outputs, ctx) {}
  ~TensorArrayWriteCPUKernel() = default;

  int Init() override;
  int ReSize() override { return 0; }
  int Run() override;
};

}  // namespace mindspore::kernel

#endif  // MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP32_TENSORARRAY_H_
