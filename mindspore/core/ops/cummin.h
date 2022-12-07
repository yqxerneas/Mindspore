/**
 * Copyright 2021-2022 Huawei Technologies Co., Ltd
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

#ifndef MINDSPORE_CORE_OPS_CUMMIN_H_
#define MINDSPORE_CORE_OPS_CUMMIN_H_
#include <map>
#include <vector>
#include <string>
#include <memory>

#include "ops/base_operator.h"
#include "mindapi/base/types.h"

namespace mindspore {
namespace ops {
constexpr auto kNameCummin = "Cummin";
class MIND_API Cummin : public BaseOperator {
 public:
  MIND_API_BASE_MEMBER(Cummin);
  Cummin() : BaseOperator(kNameCummin) { InitIOName({"x"}, {"y", "indices"}); }

  void Init(const int64_t &axis);

  void set_axis(const int64_t &axis);

  int64_t get_axis() const;
};

MIND_API abstract::AbstractBasePtr CumminInfer(const abstract::AnalysisEnginePtr &, const PrimitivePtr &primitive,
                                               const std::vector<abstract::AbstractBasePtr> &input_args);
using PrimCumminPtr = std::shared_ptr<Cummin>;
}  // namespace ops
}  // namespace mindspore
#endif  // MINDSPORE_CORE_OPS_CUMMIN_H_
