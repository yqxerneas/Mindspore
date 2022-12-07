/**
 * Copyright 2022 Huawei Technologies Co., Ltd
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

#ifndef MINDSPORE_CORE_OPS_REDUCE_STD_H_
#define MINDSPORE_CORE_OPS_REDUCE_STD_H_

#include <memory>
#include <vector>

#include "ops/base_operator.h"
#include "mindapi/base/types.h"
#include "mindspore/core/ops/core_ops.h"

namespace mindspore {
namespace ops {
constexpr auto kNameReduceStd = "ReduceStd";
/// \brief Returns the standard-deviation and mean of each row of the input tensor in the dimension `axis`.
/// Refer to Python API @ref mindspore.ops.ReduceStd for more details.
class MIND_API ReduceStd : public BaseOperator {
 public:
  MIND_API_BASE_MEMBER(ReduceStd);
  /// \brief Constructor.
  ReduceStd() : BaseOperator(kNameReduceStd) { InitIOName({"input_x"}, {"output_std", "output_mean"}); }

  void Init(bool unbiased);
  void Init(const int64_t axis);
  void Init(const std::vector<int64_t> &axis);

  bool get_unbiased() const;
  void set_unbiased(bool unbiased);
  std::vector<int64_t> get_axis() const;
  void set_axis(const std::vector<int64_t> &axis);
};

MIND_API abstract::AbstractBasePtr ReduceStdInfer(const abstract::AnalysisEnginePtr &, const PrimitivePtr &primitive,
                                                  const std::vector<abstract::AbstractBasePtr> &input_args);
}  // namespace ops
}  // namespace mindspore

#endif  // MINDSPORE_CORE_OPS_REDUCE_STD_H_
