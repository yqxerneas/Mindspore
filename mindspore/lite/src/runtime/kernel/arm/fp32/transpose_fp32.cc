/**
 * Copyright 2020 Huawei Technologies Co., Ltd
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

#include "src/runtime/kernel/arm/fp32/transpose_fp32.h"
#include "schema/model_generated.h"
#include "src/kernel_registry.h"
#include "src/runtime/runtime_api.h"
#include "nnacl/pack.h"

using mindspore::lite::KernelRegistrar;
using mindspore::lite::RET_ERROR;
using mindspore::lite::RET_OK;
using mindspore::lite::RET_OP_EXECUTE_FAILURE;
using mindspore::schema::PrimitiveType_Nchw2Nhwc;
using mindspore::schema::PrimitiveType_Nhwc2Nchw;
using mindspore::schema::PrimitiveType_Transpose;

namespace mindspore::kernel {

int TransposeCPUKernel::Init() {
  if (!InferShapeDone()) {
    return RET_OK;
  }
  return ReSize();
}

int TransposeCPUKernel::ReSize() {
  TransposeParameter *param = reinterpret_cast<TransposeParameter *>(op_parameter_);
  if (in_tensors_.at(kInputIndex)->shape().size() != static_cast<size_t>(param->num_axes_)) {
    return RET_OK;
  }
  auto &inTensor = in_tensors_.front();
  auto &outTensor = out_tensors_.front();
  auto in_shape = inTensor->shape();
  auto out_shape = outTensor->shape();
  param->strides_[param->num_axes_ - 1] = 1;
  param->out_strides_[param->num_axes_ - 1] = 1;
  param->data_size_ = inTensor->Size();
  for (int i = param->num_axes_ - 2; i >= 0; i--) {
    param->strides_[i] = in_shape.at(i + 1) * param->strides_[i + 1];
    param->out_strides_[i] = out_shape.at(i + 1) * param->out_strides_[i + 1];
  }

  if (this->out_shape_ != nullptr) {
    free(this->out_shape_);
    this->out_shape_ = nullptr;
  }

  out_shape_ = reinterpret_cast<int *>(malloc(out_shape.size() * sizeof(int)));
  if (out_shape_ == nullptr) {
    MS_LOG(ERROR) << "malloc out_shape_ failed.";
    return RET_ERROR;
  }
  memcpy(out_shape_, out_shape.data(), in_shape.size() * sizeof(int));
  return RET_OK;
}

TransposeCPUKernel::~TransposeCPUKernel() {
  if (this->out_shape_ != nullptr) {
    free(this->out_shape_);
  }
}

int TransposeCPUKernel::Run() {
  MS_ASSERT(in_tensors_.size() == 1 || in_tensors_.size() == 2);
  MS_ASSERT(out_tensors_.size() == 1);
  auto &in_tensor = in_tensors_.front();
  auto &out_tensor = out_tensors_.front();
  if (in_tensor == nullptr || out_tensor == nullptr) {
    MS_LOG(ERROR) << "null pointer dreferencing.";
    return RET_ERROR;
  }
  in_data_ = reinterpret_cast<float *>(in_tensor->MutableData());
  out_data_ = reinterpret_cast<float *>(out_tensor->MutableData());
  MS_ASSERT(in_data_);
  MS_ASSERT(out_data_);

  TransposeParameter *param = reinterpret_cast<TransposeParameter *>(this->op_parameter_);
  if (in_tensor->shape().size() != static_cast<size_t>(param->num_axes_)) {
    memcpy(out_data_, in_data_, in_tensor->ElementsNum() * sizeof(float));
    return RET_OK;
  }
  if (in_tensor->shape().size() == 4 && param->perm_[0] == 0 && param->perm_[1] == 2 && param->perm_[2] == 3 &&
      param->perm_[3] == 1) {
    if (in_tensor->data_type() == kNumberTypeFloat32) {
      PackNCHWToNHWCFp32(in_tensor->MutableData(), out_tensor->MutableData(), out_tensor->Batch(),
                         out_tensor->Height() * out_tensor->Width(), out_tensor->Channel());
    } else if (in_tensor->data_type() == kNumberTypeInt8) {
      PackNCHWToNHWCInt8(in_tensor->MutableData(), out_tensor->MutableData(), out_tensor->Batch(),
                         out_tensor->Height() * out_tensor->Width(), out_tensor->Channel());
    }
    return RET_OK;
  }
  if (in_tensor->shape().size() == 4 && param->perm_[0] == 0 && param->perm_[1] == 3 && param->perm_[2] == 1 &&
      param->perm_[3] == 2) {
    if (in_tensor->data_type() == kNumberTypeFloat32) {
      PackNHWCToNCHWFp32(in_tensor->MutableData(), out_tensor->MutableData(), out_tensor->Batch(),
                         out_tensor->Height() * out_tensor->Width(), out_tensor->Channel());
    } else if (in_tensor->data_type() == kNumberTypeInt8) {
      PackNHWCToNCHWInt8(in_tensor->MutableData(), out_tensor->MutableData(), out_tensor->Batch(),
                         out_tensor->Height() * out_tensor->Width(), out_tensor->Channel());
    }
    return RET_OK;
  }
  if (in_tensor->data_type() == kNumberTypeInt8) {
    MS_LOG(ERROR) << "not support now";
    return RET_ERROR;
  }

  int dims = out_tensor->shape().size();
  if (dims > MAX_TRANSPOSE_DIM_SIZE) {
    dim_size_ = reinterpret_cast<int *>(context_->allocator->Malloc(dims * sizeof(int)));
    if (dim_size_ == nullptr) {
      MS_LOG(ERROR) << "Malloc data failed";
      return RET_ERROR;
    }
    position_ = reinterpret_cast<int *>(context_->allocator->Malloc(dims * sizeof(int)));
    if (position_ == nullptr) {
      MS_LOG(ERROR) << "Malloc data failed";
      context_->allocator->Free(dim_size_);
      dim_size_ = nullptr;
      return RET_ERROR;
    }
  }

  MS_ASSERT(out_shape_);
  auto ret = DoTransposeFp32(in_data_, out_data_, out_shape_, param, dim_size_, position_);
  if (dims > MAX_TRANSPOSE_DIM_SIZE) {
    context_->allocator->Free(dim_size_);
    context_->allocator->Free(position_);
    dim_size_ = nullptr;
    position_ = nullptr;
  }
  if (ret != RET_OK) {
    MS_LOG(ERROR) << "Transpose run failed";
    return RET_ERROR;
  }

  return ret;
}

kernel::LiteKernel *CpuTransposeFp32KernelCreator(const std::vector<lite::Tensor *> &inputs,
                                                  const std::vector<lite::Tensor *> &outputs, OpParameter *opParameter,
                                                  const lite::InnerContext *ctx, const kernel::KernelKey &desc,
                                                  const mindspore::lite::PrimitiveC *primitive) {
  MS_ASSERT(desc.type == schema::PrimitiveType_Transpose || desc.type == schema::PrimitiveType_Nchw2Nhwc ||
            desc.type == schema::PrimitiveType_Nhwc2Nchw);
  if (opParameter == nullptr) {
    MS_LOG(ERROR) << "desc type is not Transpose";
    return nullptr;
  }
  auto *kernel = new (std::nothrow) TransposeCPUKernel(opParameter, inputs, outputs, ctx, primitive);
  if (kernel == nullptr) {
    MS_LOG(ERROR) << "New kernel fails.";
    free(opParameter);
    return nullptr;
  }

  auto ret = kernel->Init();
  if (ret != RET_OK) {
    MS_LOG(ERROR) << "Init kernel failed, name: " << opParameter->name_ << ", type: "
                  << schema::EnumNamePrimitiveType(static_cast<schema::PrimitiveType>(opParameter->type_));
    delete kernel;
    return nullptr;
  }
  return kernel;
}

REG_KERNEL(kCPU, kNumberTypeFloat32, PrimitiveType_Transpose, CpuTransposeFp32KernelCreator)
REG_KERNEL(kCPU, kNumberTypeInt8, PrimitiveType_Transpose, CpuTransposeFp32KernelCreator)
REG_KERNEL(kCPU, kNumberTypeFloat32, PrimitiveType_Nchw2Nhwc, CpuTransposeFp32KernelCreator)
REG_KERNEL(kCPU, kNumberTypeInt8, PrimitiveType_Nchw2Nhwc, CpuTransposeFp32KernelCreator)
REG_KERNEL(kCPU, kNumberTypeFloat32, PrimitiveType_Nhwc2Nchw, CpuTransposeFp32KernelCreator)
REG_KERNEL(kCPU, kNumberTypeInt8, PrimitiveType_Nhwc2Nchw, CpuTransposeFp32KernelCreator)
}  // namespace mindspore::kernel
