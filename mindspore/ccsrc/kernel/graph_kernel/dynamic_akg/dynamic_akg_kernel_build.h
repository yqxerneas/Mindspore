/**
 * Copyright 2021-2023 Huawei Technologies Co., Ltd
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

#ifndef MINDSPORE_CCSRC_BACKEND_KERNEL_COMPILER_DYNAMIC_AKG_KERNEL_BUILD_H_
#define MINDSPORE_CCSRC_BACKEND_KERNEL_COMPILER_DYNAMIC_AKG_KERNEL_BUILD_H_

#include <vector>
#include "ir/anf.h"
#include "kernel/graph_kernel/graph_kernel_builder.h"

namespace mindspore {
namespace kernel {
using graphkernel::GraphKernelJsonGenerator;

class BACKEND_EXPORT DynamicAkgKernelBuilder : public GraphKernelBuilder {
 public:
  DynamicAkgKernelBuilder() = default;
  virtual ~DynamicAkgKernelBuilder() = default;

  bool SingleOpParallelBuild(const std::vector<AnfNodePtr> &anf_nodes) override;
  bool ParallelBuild(const std::vector<JsonNodePair> &build_args) override;
};
}  // namespace kernel
}  // namespace mindspore

#endif  // MINDSPORE_CCSRC_BACKEND_KERNEL_COMPILER_DYNAMIC_AKG_KERNEL_BUILD_H_
