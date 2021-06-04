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

#include "coder/opcoders/nnacl/fp32/nhwc2nchw_fp32_coder.h"
#include <string>
#include "coder/opcoders/serializers/nnacl_serializer/nnacl_fp32_serializer.h"
#include "coder/opcoders/file_collector.h"

using mindspore::schema::PrimitiveType_Nhwc2Nchw;
namespace mindspore::lite::micro::nnacl {
int Nhwc2NchwFP32Coder::Prepare(CoderContext *const context) { return RET_OK; }

int Nhwc2NchwFP32Coder::DoCode(CoderContext *const context) {
  // generate code .h .c
  Collect(context,
          {
            "nnacl/pack.h",
          },
          {
            "pack.c",
          });

  NNaclFp32Serializer code;
  if (input_tensor_->shape().size() == DIMENSION_4D) {
    if (input_tensor_->data_type() == kNumberTypeFloat32) {
      code.CodeFunction("PackNHWCToNCHWFp32", input_tensor_, output_tensor_, output_tensor_->Batch(),
                        output_tensor_->Height() * output_tensor_->Width(), output_tensor_->Channel());
    } else if (input_tensor_->data_type() == kNumberTypeInt8) {
      code.CodeFunction("PackNHWCToNCHWInt8", input_tensor_, output_tensor_, output_tensor_->Batch(),
                        output_tensor_->Height() * output_tensor_->Width(), output_tensor_->Channel());
    } else {
      MS_LOG(ERROR) << "unsupported format transform";
    }
  } else {
    code.CodeFunction("memcpy", output_tensor_, input_tensor_, input_tensor_->ElementsNum() * sizeof(float));
  }

  context->AppendCode(code.str());
  return RET_OK;
}

REG_OPERATOR_CODER(kAllTargets, kNumberTypeFloat32, PrimitiveType_Nhwc2Nchw, CPUOpCoderCreator<Nhwc2NchwFP32Coder>)
}  // namespace mindspore::lite::micro::nnacl
