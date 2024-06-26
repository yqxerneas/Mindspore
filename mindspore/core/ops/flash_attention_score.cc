/**
 * Copyright 2023 Huawei Technologies Co., Ltd
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

#include "ops/flash_attention_score.h"

#include <string>

#include "abstract/ops/primitive_infer_map.h"
#include "ops/nn_ops.h"
#include "utils/check_convert_utils.h"
#include "ops/primitive_c.h"
#include "mindapi/src/helper.h"

namespace mindspore {
namespace ops {
namespace {
constexpr size_t kSoftmaxLastDim = 8;
constexpr size_t kInputQueryBSHRank = 3;
constexpr size_t kInputQueryBNSDRank = 4;
constexpr char kInputLayoutBSH[] = "BSH";
constexpr char kInputLayoutBNSD[] = "BNSD";

// None indicates that the optional input is not passed
bool IsOptionalInputNotPass(const AbstractBasePtr &input) {
  MS_EXCEPTION_IF_NULL(input);
  return input->BuildType()->type_id() == kMetaTypeNone;
}

void CheckInputShape(const AbstractBasePtr &input, const ShapeVector &expect_shape, const std::string &op_name,
                     const std::string &input_name, bool optional = false) {
  MS_EXCEPTION_IF_NULL(input);
  if (IsOptionalInputNotPass(input) && optional) {
    return;
  }
  auto input_shape = CheckAndConvertUtils::ConvertShapePtrToShapeMap(input->BuildShape())[kShape];
  if (input_shape != expect_shape) {
    MS_LOG(EXCEPTION) << op_name << ": The shape of input `" << input_name << "' must be " << expect_shape
                      << ", but got shape is " << input_shape;
  }
}

void CheckInputShape(const AbstractBasePtr &input, const std::vector<ShapeVector> &expect_shape_list,
                     const std::string &op_name, const std::string &input_name, bool optional = false) {
  MS_EXCEPTION_IF_NULL(input);
  if (IsOptionalInputNotPass(input) && optional) {
    return;
  }
  auto input_shape = CheckAndConvertUtils::ConvertShapePtrToShapeMap(input->BuildShape())[kShape];
  if (std::all_of(expect_shape_list.begin(), expect_shape_list.end(),
                  [&input_shape](const ShapeVector &expect_shape) { return input_shape != expect_shape; })) {
    MS_LOG(EXCEPTION) << op_name << ": The shape of input `" << input_name << "' must be one of " << expect_shape_list
                      << ", but got shape is " << input_shape;
  }
}

abstract::TupleShapePtr FlashAttentionScoreInferShape(const PrimitivePtr &primitive,
                                                      const std::vector<AbstractBasePtr> &input_args) {
  MS_EXCEPTION_IF_NULL(primitive);
  auto op_name = primitive->name();
  CheckAndConvertUtils::CheckInputArgs(input_args, kEqual, kFlashAttentionScoreInputsNum, op_name);
  auto input_layout = GetValue<std::string>(primitive->GetAttr("input_layout"));
  const std::vector valid_layout = {kInputLayoutBSH, kInputLayoutBNSD};
  if (std::find(valid_layout.begin(), valid_layout.end(), input_layout) == valid_layout.end()) {
    MS_LOG(EXCEPTION) << op_name << ": The value of attribute 'input_layout' must be one of" << valid_layout
                      << ", but got " << input_layout;
  }
  int64_t batch_size;
  int64_t q_seq_len;
  auto q_head_num = GetValue<int64_t>(primitive->GetAttr("head_num"));
  int64_t kv_seq_len;
  int64_t kv_head_num;
  auto query_shape = CheckAndConvertUtils::ConvertShapePtrToShapeMap(
    input_args[kFlashAttentionScoreInputQueryIndex]->BuildShape())[kShape];
  auto key_shape = CheckAndConvertUtils::ConvertShapePtrToShapeMap(
    input_args[kFlashAttentionScoreInputKeyIndex]->BuildShape())[kShape];
  if (input_layout == kInputLayoutBSH) {
    if (query_shape.size() != kInputQueryBSHRank || key_shape.size() != query_shape.size()) {
      MS_LOG(EXCEPTION) << op_name << ": The rank of 'query' and 'key' must be " << kInputQueryBSHRank << ", but got "
                        << query_shape.size() << " and " << key_shape.size();
    }
    batch_size = query_shape[0];
    q_seq_len = query_shape[1];
    auto q_hidden_size = query_shape[2];
    if (q_hidden_size % q_head_num != 0) {
      MS_LOG(EXCEPTION) << op_name << ": 'hidden_size` must be divisible by `head_num`, but got " << q_hidden_size
                        << " and " << q_head_num;
    }
    int64_t head_size = q_hidden_size / q_head_num;
    kv_seq_len = key_shape[kIndex1];
    kv_head_num = key_shape[kIndex2] / head_size;
  } else {
    if (query_shape.size() != kInputQueryBNSDRank) {
      MS_LOG(EXCEPTION) << op_name << ": The rank of 'query' must be " << kInputQueryBNSDRank << ", but got "
                        << query_shape.size();
    }
    batch_size = query_shape[kIndex0];
    if (q_head_num != query_shape[kIndex1]) {
      MS_LOG(EXCEPTION) << op_name << ": query_shape[1] must be equal to attribute 'head_num', but got "
                        << query_shape[1] << " and " << q_head_num;
    }
    q_seq_len = query_shape[kIndex2];
    kv_seq_len = key_shape[kIndex2];
    kv_head_num = key_shape[kIndex1];
  }
  if (q_head_num % kv_head_num != 0) {
    MS_LOG(EXCEPTION) << op_name << ": The head num of 'key' must be a factor of the head num of 'query', but got "
                      << kv_head_num << " and " << q_head_num;
  }
  CheckInputShape(input_args[kFlashAttentionScoreInputValueIndex], key_shape, op_name, "value");
  CheckInputShape(input_args[kFlashAttentionScoreInputRealShiftIndex],
                  {{batch_size, q_head_num, q_seq_len, kv_seq_len}, {batch_size, q_head_num, 1, kv_seq_len}}, op_name,
                  "real_shift", true);
  CheckInputShape(input_args[kFlashAttentionScoreInputDropMaskIndex],
                  {batch_size, q_head_num, q_seq_len, kv_seq_len / 8}, op_name, "drop_mask", true);
  CheckInputShape(
    input_args[kFlashAttentionScoreInputAttnMaskIndex],
    {{batch_size, q_head_num, q_seq_len, kv_seq_len}, {batch_size, 1, q_seq_len, kv_seq_len}, {q_seq_len, kv_seq_len}},
    op_name, "attn_mask", true);
  CheckInputShape(input_args[kFlashAttentionScoreInputPrefixIndex], ShapeVector{batch_size}, op_name, "prefix", true);

  // Infer placeholder index
  std::vector<int64_t> placeholder_index;
  for (size_t i = 0; i < input_args.size(); ++i) {
    if (IsOptionalInputNotPass(input_args[i])) {
      placeholder_index.push_back(SizeToLong(i));
    }
  }
  primitive->AddAttr(kAttrPlaceHolderIndex, MakeValue<std::vector<int64_t>>(placeholder_index));

  abstract::BaseShapePtrList output_shape_ptr_list(kFlashAttentionScoreOutputsNum);
  output_shape_ptr_list[kFlashAttentionScoreOutputSoftmaxMaxIndex] =
    std::make_shared<abstract::Shape>(ShapeVector{batch_size, q_head_num, q_seq_len, kSoftmaxLastDim});
  output_shape_ptr_list[kFlashAttentionScoreOutputSoftmaxSumIndex] =
    std::make_shared<abstract::Shape>(ShapeVector{batch_size, q_head_num, q_seq_len, kSoftmaxLastDim});
  output_shape_ptr_list[kFlashAttentionScoreOutputSoftmaxOutIndex] = std::make_shared<abstract::Shape>(ShapeVector{1});
  output_shape_ptr_list[kFlashAttentionScoreOutputAttentionOutIndex] = std::make_shared<abstract::Shape>(query_shape);
  return std::make_shared<abstract::TupleShape>(output_shape_ptr_list);
}

TuplePtr FlashAttentionScoreInferType(const PrimitivePtr &prim, const std::vector<AbstractBasePtr> &input_args) {
  const std::set valid_types = {kFloat16, kFloat32, kBFloat16};
  auto op_name = prim->name();
  std::map<std::string, TypePtr> types;
  // "x", "kernel_query", "kernel_key", "kernel_value", "gamma", " beta", "bias_query", "bias_key", "bias_value"
  (void)types.emplace("query", input_args[kFlashAttentionScoreInputQueryIndex]->BuildType());
  (void)types.emplace("key", input_args[kFlashAttentionScoreInputKeyIndex]->BuildType());
  (void)types.emplace("value", input_args[kFlashAttentionScoreInputValueIndex]->BuildType());
  if (!IsOptionalInputNotPass(input_args[kFlashAttentionScoreInputRealShiftIndex])) {
    (void)types.emplace("real_shift", input_args[kFlashAttentionScoreInputRealShiftIndex]->BuildType());
  }
  auto type = CheckAndConvertUtils::CheckTensorTypeSame(types, valid_types, op_name);
  if (!IsOptionalInputNotPass(input_args[kFlashAttentionScoreInputAttnMaskIndex])) {
    auto attn_mask_type = input_args[kFlashAttentionScoreInputAttnMaskIndex]->BuildType();
    CheckAndConvertUtils::CheckTensorTypeValid("attn_mask", attn_mask_type, {kUInt8}, op_name);
  }
  if (!IsOptionalInputNotPass(input_args[kFlashAttentionScoreInputPaddingMaskIndex])) {
    MS_LOG(EXCEPTION) << op_name << ": 'padding_mask' must be None currently.";
  }
  if (!IsOptionalInputNotPass(input_args[kFlashAttentionScoreInputPrefixIndex])) {
    auto prefix_type = input_args[kFlashAttentionScoreInputPrefixIndex]->BuildType();
    CheckAndConvertUtils::CheckTensorTypeValid("prefix", prefix_type, {kInt64}, op_name);
  }

  auto keep_prob_value_ptr = prim->GetAttr("keep_prob");
  MS_EXCEPTION_IF_NULL(keep_prob_value_ptr);
  auto keep_prob = GetValue<float>(keep_prob_value_ptr);
  if (keep_prob > 1 || keep_prob < 0) {
    MS_LOG(EXCEPTION) << op_name << ": attribute `keep_prob` must be a floating point number in [0, 1], but got "
                      << keep_prob;
  }
  if (common::IsFloatEqual(keep_prob, 1.0)) {
    if (!IsOptionalInputNotPass(input_args[kFlashAttentionScoreInputDropMaskIndex])) {
      MS_LOG(EXCEPTION) << op_name << ": 'drop_mask' must be None when keep_prob is 1.0.";
    }
  } else {
    auto drop_mask_type = input_args[kFlashAttentionScoreInputDropMaskIndex]->BuildType();
    CheckAndConvertUtils::CheckTensorTypeValid("drop_mask", drop_mask_type, {kUInt8}, op_name);
  }

  TypePtrList output_type_ptr_list(kFlashAttentionScoreOutputsNum);
  output_type_ptr_list[kFlashAttentionScoreOutputSoftmaxMaxIndex] = kFloat32;
  output_type_ptr_list[kFlashAttentionScoreOutputSoftmaxSumIndex] = kFloat32;
  output_type_ptr_list[kFlashAttentionScoreOutputSoftmaxOutIndex] = type;
  output_type_ptr_list[kFlashAttentionScoreOutputAttentionOutIndex] = type;
  return std::make_shared<Tuple>(output_type_ptr_list);
}
}  // namespace

AbstractBasePtr FlashAttentionScoreInfer(const abstract::AnalysisEnginePtr &, const PrimitivePtr &primitive,
                                         const std::vector<AbstractBasePtr> &input_args) {
  MS_EXCEPTION_IF_NULL(primitive);
  CheckAndConvertUtils::CheckInputArgs(input_args, kEqual, kFlashAttentionScoreInputsNum, primitive->name());
  auto infer_type = FlashAttentionScoreInferType(primitive, input_args);
  auto infer_shape = FlashAttentionScoreInferShape(primitive, input_args);
  return abstract::MakeAbstract(infer_shape, infer_type);
}

MIND_API_OPERATOR_IMPL(FlashAttentionScore, BaseOperator);

// AG means auto generated
class MIND_API AGFlashAttentionScoreInfer : public abstract::OpInferBase {
 public:
  BaseShapePtr InferShape(const PrimitivePtr &primitive,
                          const std::vector<AbstractBasePtr> &input_args) const override {
    return FlashAttentionScoreInferShape(primitive, input_args);
  }

  TypePtr InferType(const PrimitivePtr &primitive, const std::vector<AbstractBasePtr> &input_args) const override {
    return FlashAttentionScoreInferType(primitive, input_args);
  }
  AbstractBasePtr InferShapeAndType(const abstract::AnalysisEnginePtr &engine, const PrimitivePtr &primitive,
                                    const std::vector<AbstractBasePtr> &input_args) const override {
    return FlashAttentionScoreInfer(engine, primitive, input_args);
  }
};

REGISTER_PRIMITIVE_OP_INFER_IMPL(FlashAttentionScore, prim::kPrimFlashAttentionScore, AGFlashAttentionScoreInfer,
                                 false);
}  // namespace ops
}  // namespace mindspore
