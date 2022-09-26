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

#include <string>
#include <algorithm>
#include <memory>
#include <set>
#include <vector>
#include <map>

#include "ops/binary_cross_entropy.h"
#include "ops/op_utils.h"
#include "utils/check_convert_utils.h"
#include "abstract/ops/primitive_infer_map.h"
#include "mindapi/src/helper.h"

namespace mindspore {
namespace ops {
class BinaryCrossEntropyInfer : public abstract::OpInferBase {
 public:
  BaseShapePtr InferShape(const PrimitivePtr &primitive,
                          const std::vector<AbstractBasePtr> &input_args) const override {
    MS_EXCEPTION_IF_NULL(primitive);
    const int64_t kInputNum = 3;
    auto prim_name = primitive->name();
    CheckAndConvertUtils::CheckInputArgs(input_args, kGreaterEqual, kInputNum, prim_name);
    auto x_shape = CheckAndConvertUtils::ConvertShapePtrToShapeMap(input_args[kInputIndex0]->BuildShape())[kShape];
    auto y_shape = CheckAndConvertUtils::ConvertShapePtrToShapeMap(input_args[kInputIndex1]->BuildShape())[kShape];
    auto weight_shape = CheckAndConvertUtils::ConvertShapePtrToShapeMap(input_args[kInputIndex2]->BuildShape())[kShape];
    auto x_shape_BaseShapePtr = input_args[kInputIndex0]->BuildShape();
    auto y_shape_BaseShapePtr = input_args[kInputIndex1]->BuildShape();
    auto weight_shape_BaseShapePtr = input_args[kInputIndex2]->BuildShape();
    auto x_shape_ptr = x_shape_BaseShapePtr->cast<abstract::ShapePtr>();
    auto y_shape_ptr = y_shape_BaseShapePtr->cast<abstract::ShapePtr>();
    auto weight_shape_ptr = weight_shape_BaseShapePtr->cast<abstract::ShapePtr>();
    if (!x_shape_ptr->IsDynamic() && !y_shape_ptr->IsDynamic())
      CheckAndConvertUtils::Check("logits shape", x_shape, kEqual, y_shape, prim_name, ValueError);
    if (weight_shape.size() > 0) {
      if (!y_shape_ptr->IsDynamic() && !weight_shape_ptr->IsDynamic())
        CheckAndConvertUtils::Check("labels shape", y_shape, kEqual, weight_shape, prim_name, ValueError);
    }

    auto out_shape = x_shape;
    bool reduction_is_none;
    auto reduction_ptr = primitive->GetAttr(kReduction);
    if (reduction_ptr->isa<StringImm>()) {
      auto reduction = GetValue<std::string>(reduction_ptr);
      reduction_is_none = reduction == kNone;
    } else {
      auto reduction = Reduction(GetValue<int64_t>(reduction_ptr));
      reduction_is_none = reduction == Reduction::NONE;
    }

    if (!reduction_is_none) {
      out_shape.resize(0);
      return std::make_shared<abstract::Shape>(out_shape);
    }
    return x_shape_ptr;
  }

  TypePtr InferType(const PrimitivePtr &prim, const std::vector<AbstractBasePtr> &input_args) const override {
    MS_EXCEPTION_IF_NULL(prim);
    const int64_t kInputNum = 3;
    auto prim_name = prim->name();
    CheckAndConvertUtils::CheckInputArgs(input_args, kGreaterEqual, kInputNum, prim_name);
    std::set<TypePtr> valid_types = {kFloat16, kFloat32};
    std::map<std::string, TypePtr> types1, types2;
    (void)types1.emplace("logits", input_args[kInputIndex0]->BuildType());
    (void)types1.emplace("labels", input_args[kInputIndex1]->BuildType());
    (void)CheckAndConvertUtils::CheckTensorTypeSame(types1, valid_types, prim_name);
    auto weight_shape = CheckAndConvertUtils::ConvertShapePtrToShapeMap(input_args[kInputIndex2]->BuildShape())[kShape];
    if (weight_shape.size() > 0) {
      (void)types2.emplace("logits", input_args[kInputIndex0]->BuildType());
      (void)types2.emplace("weight", input_args[kInputIndex2]->BuildType());
      (void)CheckAndConvertUtils::CheckTensorTypeSame(types2, valid_types, prim_name);
    }
    return input_args[kInputIndex0]->BuildType();
  }
};

MIND_API_OPERATOR_IMPL(BinaryCrossEntropy, BaseOperator);
void BinaryCrossEntropy::set_reduction(const Reduction &reduction) {
  std::string reduce;
  if (reduction == Reduction::REDUCTION_SUM) {
    reduce = "sum";
  } else if (reduction == Reduction::MEAN) {
    reduce = "mean";
  } else {
    reduce = "none";
  }
  (void)this->AddAttr(kReduction, api::MakeValue(reduce));
}

Reduction BinaryCrossEntropy::get_reduction() const {
  auto value_ptr = MakeValue(GetValue<std::string>(GetAttr(kReduction)));
  int64_t reduction = 0;
  CheckAndConvertUtils::GetReductionEnumValue(value_ptr, &reduction);
  return Reduction(reduction);
}
void BinaryCrossEntropy::Init(const Reduction &reduction) { this->set_reduction(reduction); }

REGISTER_PRIMITIVE_OP_INFER_IMPL(BinaryCrossEntropy, prim::kPrimBinaryCrossEntropy, BinaryCrossEntropyInfer, false);
}  // namespace ops
}  // namespace mindspore
