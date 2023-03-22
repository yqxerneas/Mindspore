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

#include "nnacl/nnacl_kernel.h"
#include "src/tensor.h"
#include "common/tensor_util.h"
#include "include/errorcode.h"

using mindspore::lite::RET_ERROR;
using mindspore::lite::RET_OK;
namespace mindspore::nnacl {
NnaclKernel::~NnaclKernel() {
  if (in_ != nullptr) {
    free(in_);
    in_ = nullptr;
  }
  if (out_ != nullptr) {
    free(out_);
    out_ = nullptr;
  }

  if (kernel_ != nullptr) {
    kernel_->release(kernel_);

    free(kernel_);
    kernel_ = nullptr;
  }
}

void NnaclKernel::UpdateTensorData() {
  for (size_t i = 0; i < in_size_; i++) {
    in_[i].data_ = in_tensors().at(i)->data();
    in_[i].data_type_ = in_tensors_[i]->data_type();
  }
  for (size_t i = 0; i < out_size_; i++) {
    out_[i].data_ = out_tensors().at(i)->data();
    out_[i].data_type_ = out_tensors_[i]->data_type();
  }
  return;
}

void NnaclKernel::UpdateTensorC() {
  for (size_t i = 0; i < in_size_; i++) {
    Tensor2TensorC(in_tensors_[i], &in_[i]);
  }
  for (size_t i = 0; i < out_size_; i++) {
    Tensor2TensorC(out_tensors_[i], &out_[i]);
  }
  return;
}

int NnaclKernel::Prepare() {
  if (kernel_ == nullptr) {
    return RET_ERROR;
  }

  int ret = kernel_->prepare(kernel_);
  if (ret != RET_OK) {
    return ret;
  }

  if (!InferShapeDone()) {
    return RET_OK;
  }
  return ReSize();
}

int NnaclKernel::ReSize() {
  if (kernel_ == nullptr) {
    return RET_ERROR;
  }

  UpdateTensorC();

  return kernel_->resize(kernel_);
}

int NnaclKernel::Run() {
  if (kernel_ == nullptr) {
    return RET_ERROR;
  }
  UpdateTensorData();
  return kernel_->compute(kernel_);
}

int NnaclKernel::InferShape() {
  if (kernel_ == nullptr) {
    return RET_ERROR;
  }
  return kernel_->infershape(kernel_);
}

int NnaclKernel::InitKernel(const kernel::KernelKey &key, const lite::InnerContext *ctx) {
  CHECK_NULL_RETURN(ctx);

  in_size_ = in_tensors_.size();
  if (in_size_ == 0 || in_size_ > MAX_MALLOC_SIZE) {
    return RET_ERROR;
  }
  in_ = reinterpret_cast<TensorC *>(malloc(in_size_ * sizeof(TensorC)));
  if (in_ == nullptr) {
    return RET_ERROR;
  }

  out_size_ = out_tensors_.size();
  if (out_size_ == 0 || out_size_ > MAX_MALLOC_SIZE) {
    return RET_ERROR;
  }
  out_ = reinterpret_cast<TensorC *>(malloc(out_size_ * sizeof(TensorC)));
  if (out_ == nullptr) {
    return RET_ERROR;
  }

  UpdateTensorC();

  kernel_ = CreateKernel(op_parameter_, in_, in_size_, out_, out_size_, key.data_type,
                         const_cast<ExecEnv *>(ctx->GetExecEnv()));
  if (kernel_ == nullptr) {
    return RET_ERROR;
  }
  return RET_OK;
}
}  // namespace mindspore::nnacl