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
#ifndef AICPU_KERNELS_NORMALIZED_REDUCEPROD_H_
#define AICPU_KERNELS_NORMALIZED_REDUCEPROD_H_

#include "cpu_ops_kernel.h"

namespace aicpu {
class ReduceProdCpuKernel : public CpuKernel {
 public:
  ReduceProdCpuKernel() = default;
  ~ReduceProdCpuKernel() override = default;

 protected:
  uint32_t Compute(CpuKernelContext &ctx) override;

 private:
  template <typename T>
  static T ComputeMul(T num_1, T num_2);

  template <typename T1, typename T2>
  static uint32_t ReduceProdCompute(CpuKernelContext &ctx);

  template <typename T1, typename T2>
  static uint32_t ReduceProdCompute_Complex(CpuKernelContext &ctx);
};
}  // namespace aicpu
#endif
