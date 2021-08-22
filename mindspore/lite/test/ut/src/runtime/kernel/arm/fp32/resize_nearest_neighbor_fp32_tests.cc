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
#include <vector>
#include "common/common_test.h"
#include "nnacl/resize_parameter.h"
#include "src/kernel_registry.h"

namespace mindspore {

class TestResizeNearestNeighborFp32 : public mindspore::CommonTest {
 public:
  TestResizeNearestNeighborFp32() = default;
  void Prepare(const std::vector<int> &input_shape, const std::vector<int> &output_shape, float *input_data,
               float *output_data, const bool align_corners, const int thread_num);

  void TearDown() override;

 public:
  float err_tol = 1e-5;
  lite::Tensor in_tensor_;
  lite::Tensor out_tensor_;
  std::vector<lite::Tensor *> inputs_{&in_tensor_};
  std::vector<lite::Tensor *> outputs_{&out_tensor_};
  ResizeParameter param_ = {{}};
  kernel::KernelKey desc = {kernel::KERNEL_ARCH::kCPU, kNumberTypeFloat32, schema::PrimitiveType_Resize};
  lite::InnerContext ctx_ = lite::InnerContext();
  kernel::KernelCreator creator_ = nullptr;
  kernel::InnerKernel *kernel_ = nullptr;
};

void TestResizeNearestNeighborFp32::TearDown() {
  in_tensor_.set_data(nullptr);
  out_tensor_.set_data(nullptr);
}

void TestResizeNearestNeighborFp32::Prepare(const std::vector<int> &input_shape, const std::vector<int> &output_shape,
                                            float *input_data, float *output_data, const bool align_corners,
                                            const int thread_num) {
  in_tensor_.set_data_type(kNumberTypeFloat32);
  in_tensor_.set_shape(input_shape);
  out_tensor_.set_data_type(kNumberTypeFloat32);
  out_tensor_.set_shape(output_shape);
  in_tensor_.set_data(input_data);
  out_tensor_.set_data(output_data);

  ResizeParameter param_ = {
    {}, static_cast<int>(schema::ResizeMethod_NEAREST), output_shape[1], output_shape[2], align_corners};
  desc = {kernel::KERNEL_ARCH::kCPU, kNumberTypeFloat32, schema::PrimitiveType_Resize};
  ctx_ = lite::InnerContext();
  ctx_.thread_num_ = thread_num;
  ASSERT_EQ(lite::RET_OK, ctx_.Init());
  creator_ = lite::KernelRegistry::GetInstance()->GetCreator(desc);
  ASSERT_NE(creator_, nullptr);
  kernel_ = creator_(inputs_, outputs_, reinterpret_cast<OpParameter *>(&param_), &ctx_, desc);
  ASSERT_NE(kernel_, nullptr);
  auto ret = kernel_->Init();
  EXPECT_EQ(0, ret);
}
// 1*1 -> 1*1
TEST_F(TestResizeNearestNeighborFp32, ResizeNearestNeighborTest1) {
  float input_data[] = {1.0};
  float output_data[1] = {0};
  std::vector<int> input_shape = {1, 1, 1, 1};
  std::vector<int> output_shape = {1, 1, 1, 1};
  std::vector<float> expect = {1.0};
  size_t output_size = 1;
  bool align_corners = false;

  Prepare(input_shape, output_shape, input_data, output_data, align_corners, 1);
  auto ret = kernel_->Run();
  EXPECT_EQ(0, ret);

  ASSERT_EQ(0, CompareOutputData(output_data, expect.data(), output_size, err_tol));
}

// 2*2 -> 1*1
TEST_F(TestResizeNearestNeighborFp32, ResizeNearestNeighborTest2) {
  float input_data[] = {0.0, 1.0, 2.0, 3.0};
  float output_data[1] = {0};
  std::vector<int> input_shape = {1, 2, 2, 1};
  std::vector<int> output_shape = {1, 1, 1, 1};
  std::vector<float> expect = {0.0};
  size_t output_size = 1;
  bool align_corners = false;

  Prepare(input_shape, output_shape, input_data, output_data, align_corners, 1);
  auto ret = kernel_->Run();
  EXPECT_EQ(0, ret);

  ASSERT_EQ(0, CompareOutputData(output_data, expect.data(), output_size, err_tol));
}

// 2*2 -> 1*2
TEST_F(TestResizeNearestNeighborFp32, ResizeNearestNeighborTest3) {
  float input_data[] = {0.0, 1.0, 2.0, 3.0};
  float output_data[2] = {0};
  std::vector<int> input_shape = {1, 2, 2, 1};
  std::vector<int> output_shape = {1, 1, 2, 1};
  std::vector<float> expect = {0.0, 1.0};
  size_t output_size = 2;
  bool align_corners = false;

  Prepare(input_shape, output_shape, input_data, output_data, align_corners, 1);
  auto ret = kernel_->Run();
  EXPECT_EQ(0, ret);

  ASSERT_EQ(0, CompareOutputData(output_data, expect.data(), output_size, err_tol));
}

// 2*2 -> 2*1
TEST_F(TestResizeNearestNeighborFp32, ResizeNearestNeighborTest4) {
  float input_data[] = {0.0, 1.0, 2.0, 3.0};
  float output_data[2] = {0};
  std::vector<int> input_shape = {1, 2, 2, 1};
  std::vector<int> output_shape = {1, 2, 1, 1};
  std::vector<float> expect = {0.0, 2.0};
  size_t output_size = 2;
  bool align_corners = false;

  Prepare(input_shape, output_shape, input_data, output_data, align_corners, 1);
  auto ret = kernel_->Run();
  EXPECT_EQ(0, ret);

  ASSERT_EQ(0, CompareOutputData(output_data, expect.data(), output_size, err_tol));
}

// 2*2 -> 2*2
TEST_F(TestResizeNearestNeighborFp32, ResizeNearestNeighborTest5) {
  float input_data[] = {0.0, 1.0, 2.0, 3.0};
  float output_data[4] = {0};
  std::vector<int> input_shape = {1, 2, 2, 1};
  std::vector<int> output_shape = {1, 2, 2, 1};
  std::vector<float> expect = {0.0, 1.0, 2.0, 3.0};
  size_t output_size = 4;
  bool align_corners = false;

  Prepare(input_shape, output_shape, input_data, output_data, align_corners, 1);
  auto ret = kernel_->Run();
  EXPECT_EQ(0, ret);

  ASSERT_EQ(0, CompareOutputData(output_data, expect.data(), output_size, err_tol));
}

// 2*2 -> 1*4
TEST_F(TestResizeNearestNeighborFp32, ResizeNearestNeighborTest6) {
  float input_data[] = {0.0, 1.0, 2.0, 3.0};
  float output_data[4] = {0};
  std::vector<int> input_shape = {1, 2, 2, 1};
  std::vector<int> output_shape = {1, 1, 4, 1};
  std::vector<float> expect = {0.0, 0.0, 1.0, 1.0};
  size_t output_size = 4;
  bool align_corners = false;

  Prepare(input_shape, output_shape, input_data, output_data, align_corners, 1);
  auto ret = kernel_->Run();
  EXPECT_EQ(0, ret);

  ASSERT_EQ(0, CompareOutputData(output_data, expect.data(), output_size, err_tol));
}

// 2*2 -> 4*1
TEST_F(TestResizeNearestNeighborFp32, ResizeNearestNeighborTest7) {
  float input_data[] = {0.0, 1.0, 2.0, 3.0};
  float output_data[4] = {0};
  std::vector<int> input_shape = {1, 2, 2, 1};
  std::vector<int> output_shape = {1, 4, 1, 1};
  std::vector<float> expect = {0.0, 0.0, 2.0, 2.0};
  size_t output_size = 4;
  bool align_corners = false;

  Prepare(input_shape, output_shape, input_data, output_data, align_corners, 1);
  auto ret = kernel_->Run();
  EXPECT_EQ(0, ret);

  ASSERT_EQ(0, CompareOutputData(output_data, expect.data(), output_size, err_tol));
}

// 2*2 -> 2*4
TEST_F(TestResizeNearestNeighborFp32, ResizeNearestNeighborTest8) {
  float input_data[] = {0.0, 1.0, 2.0, 3.0};
  float output_data[8] = {0};
  std::vector<int> input_shape = {1, 2, 2, 1};
  std::vector<int> output_shape = {1, 2, 4, 1};
  std::vector<float> expect = {0.0, 0.0, 1.0, 1.0, 2.0, 2.0, 3.0, 3.0};
  size_t output_size = 8;
  bool align_corners = false;

  Prepare(input_shape, output_shape, input_data, output_data, align_corners, 1);
  auto ret = kernel_->Run();
  EXPECT_EQ(0, ret);

  ASSERT_EQ(0, CompareOutputData(output_data, expect.data(), output_size, err_tol));
}

// 2*2 -> 4*2
TEST_F(TestResizeNearestNeighborFp32, ResizeNearestNeighborTest9) {
  float input_data[] = {0.0, 1.0, 2.0, 3.0};
  float output_data[8] = {0};
  std::vector<int> input_shape = {1, 2, 2, 1};
  std::vector<int> output_shape = {1, 4, 2, 1};
  std::vector<float> expect = {0.0, 1.0, 0.0, 1.0, 2.0, 3.0, 2.0, 3.0};
  size_t output_size = 8;
  bool align_corners = false;

  Prepare(input_shape, output_shape, input_data, output_data, align_corners, 1);
  auto ret = kernel_->Run();
  EXPECT_EQ(0, ret);

  ASSERT_EQ(0, CompareOutputData(output_data, expect.data(), output_size, err_tol));
}

// 2*2 -> 3*3
TEST_F(TestResizeNearestNeighborFp32, ResizeNearestNeighborTest10) {
  float input_data[] = {0.0, 1.0, 2.0, 3.0};
  float output_data[9] = {0};
  std::vector<int> input_shape = {1, 2, 2, 1};
  std::vector<int> output_shape = {1, 3, 3, 1};
  std::vector<float> expect = {0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 2.0, 2.0, 3.0};
  size_t output_size = 9;
  bool align_corners = false;

  Prepare(input_shape, output_shape, input_data, output_data, align_corners, 1);
  auto ret = kernel_->Run();
  EXPECT_EQ(0, ret);

  ASSERT_EQ(0, CompareOutputData(output_data, expect.data(), output_size, err_tol));
}

// 2*2 -> 4*4
TEST_F(TestResizeNearestNeighborFp32, ResizeNearestNeighborTest11) {
  float input_data[] = {0.0, 1.0, 2.0, 3.0};
  float output_data[16] = {0};
  std::vector<int> input_shape = {1, 2, 2, 1};
  std::vector<int> output_shape = {1, 4, 4, 1};
  std::vector<float> expect = {0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 2.0, 2.0, 3.0, 3.0, 2.0, 2.0, 3.0, 3.0};
  size_t output_size = 16;
  bool align_corners = false;

  Prepare(input_shape, output_shape, input_data, output_data, align_corners, 1);
  auto ret = kernel_->Run();
  EXPECT_EQ(0, ret);

  ASSERT_EQ(0, CompareOutputData(output_data, expect.data(), output_size, err_tol));
}

// 2*2*2*5 -> 2*4*4*5
TEST_F(TestResizeNearestNeighborFp32, ResizeNearestNeighborTest12) {
  float input_data[] = {0.0,  1.0,  2.0,  3.0,  4.0,  5.0,  6.0,  7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0,
                        14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0,
                        28.0, 29.0, 30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0};
  float output_data[160] = {0};
  std::vector<int> input_shape = {2, 2, 2, 5};
  std::vector<int> output_shape = {2, 4, 4, 5};
  std::vector<float> expect = {
    0.0,  1.0,  2.0,  3.0,  4.0,  0.0,  1.0,  2.0,  3.0,  4.0,  5.0,  6.0,  7.0,  8.0,  9.0,  5.0,  6.0,  7.0,
    8.0,  9.0,  0.0,  1.0,  2.0,  3.0,  4.0,  0.0,  1.0,  2.0,  3.0,  4.0,  5.0,  6.0,  7.0,  8.0,  9.0,  5.0,
    6.0,  7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0, 14.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0,
    19.0, 15.0, 16.0, 17.0, 18.0, 19.0, 10.0, 11.0, 12.0, 13.0, 14.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0,
    17.0, 18.0, 19.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0, 20.0, 21.0, 22.0, 23.0, 24.0,
    25.0, 26.0, 27.0, 28.0, 29.0, 25.0, 26.0, 27.0, 28.0, 29.0, 20.0, 21.0, 22.0, 23.0, 24.0, 20.0, 21.0, 22.0,
    23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0, 33.0, 34.0, 30.0,
    31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0, 35.0, 36.0, 37.0, 38.0, 39.0, 30.0, 31.0, 32.0, 33.0,
    34.0, 30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0, 35.0, 36.0, 37.0, 38.0, 39.0};
  size_t output_size = 160;
  bool align_corners = false;

  Prepare(input_shape, output_shape, input_data, output_data, align_corners, 1);
  auto ret = kernel_->Run();
  EXPECT_EQ(0, ret);

  ASSERT_EQ(0, CompareOutputData(output_data, expect.data(), output_size, err_tol));
}

// 2*2*2*5 -> 2*4*4*5 thread_num 2
TEST_F(TestResizeNearestNeighborFp32, ResizeNearestNeighborTest13) {
  float input_data[] = {0.0,  1.0,  2.0,  3.0,  4.0,  5.0,  6.0,  7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0,
                        14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0,
                        28.0, 29.0, 30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0};
  float output_data[160] = {0};
  std::vector<int> input_shape = {2, 2, 2, 5};
  std::vector<int> output_shape = {2, 4, 4, 5};
  std::vector<float> expect = {
    0.0,  1.0,  2.0,  3.0,  4.0,  0.0,  1.0,  2.0,  3.0,  4.0,  5.0,  6.0,  7.0,  8.0,  9.0,  5.0,  6.0,  7.0,
    8.0,  9.0,  0.0,  1.0,  2.0,  3.0,  4.0,  0.0,  1.0,  2.0,  3.0,  4.0,  5.0,  6.0,  7.0,  8.0,  9.0,  5.0,
    6.0,  7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0, 14.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0,
    19.0, 15.0, 16.0, 17.0, 18.0, 19.0, 10.0, 11.0, 12.0, 13.0, 14.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0,
    17.0, 18.0, 19.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0, 20.0, 21.0, 22.0, 23.0, 24.0,
    25.0, 26.0, 27.0, 28.0, 29.0, 25.0, 26.0, 27.0, 28.0, 29.0, 20.0, 21.0, 22.0, 23.0, 24.0, 20.0, 21.0, 22.0,
    23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0, 33.0, 34.0, 30.0,
    31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0, 35.0, 36.0, 37.0, 38.0, 39.0, 30.0, 31.0, 32.0, 33.0,
    34.0, 30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0, 35.0, 36.0, 37.0, 38.0, 39.0};
  size_t output_size = 160;
  bool align_corners = false;

  Prepare(input_shape, output_shape, input_data, output_data, align_corners, 2);
  auto ret = kernel_->Run();
  EXPECT_EQ(0, ret);

  ASSERT_EQ(0, CompareOutputData(output_data, expect.data(), output_size, err_tol));
}

// 2*2*2*5 -> 2*4*4*5 thread_num 4
TEST_F(TestResizeNearestNeighborFp32, ResizeNearestNeighborTest14) {
  float input_data[] = {0.0,  1.0,  2.0,  3.0,  4.0,  5.0,  6.0,  7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0,
                        14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0,
                        28.0, 29.0, 30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0};
  float output_data[160] = {0};
  std::vector<int> input_shape = {2, 2, 2, 5};
  std::vector<int> output_shape = {2, 4, 4, 5};
  std::vector<float> expect = {
    0.0,  1.0,  2.0,  3.0,  4.0,  0.0,  1.0,  2.0,  3.0,  4.0,  5.0,  6.0,  7.0,  8.0,  9.0,  5.0,  6.0,  7.0,
    8.0,  9.0,  0.0,  1.0,  2.0,  3.0,  4.0,  0.0,  1.0,  2.0,  3.0,  4.0,  5.0,  6.0,  7.0,  8.0,  9.0,  5.0,
    6.0,  7.0,  8.0,  9.0,  10.0, 11.0, 12.0, 13.0, 14.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0,
    19.0, 15.0, 16.0, 17.0, 18.0, 19.0, 10.0, 11.0, 12.0, 13.0, 14.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0,
    17.0, 18.0, 19.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0, 20.0, 21.0, 22.0, 23.0, 24.0,
    25.0, 26.0, 27.0, 28.0, 29.0, 25.0, 26.0, 27.0, 28.0, 29.0, 20.0, 21.0, 22.0, 23.0, 24.0, 20.0, 21.0, 22.0,
    23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0, 33.0, 34.0, 30.0,
    31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0, 35.0, 36.0, 37.0, 38.0, 39.0, 30.0, 31.0, 32.0, 33.0,
    34.0, 30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0, 35.0, 36.0, 37.0, 38.0, 39.0};
  size_t output_size = 160;
  bool align_corners = false;

  Prepare(input_shape, output_shape, input_data, output_data, align_corners, 4);
  auto ret = kernel_->Run();
  EXPECT_EQ(0, ret);

  ASSERT_EQ(0, CompareOutputData(output_data, expect.data(), output_size, err_tol));
}

// 4*4 -> 6*6 align_corners True
TEST_F(TestResizeNearestNeighborFp32, ResizeNearestNeighborTest15) {
  float input_data[] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0};
  float output_data[36] = {0};
  std::vector<int> input_shape = {1, 4, 4, 1};
  std::vector<int> output_shape = {1, 6, 6, 1};
  std::vector<float> expect = {0.0, 1.0, 1.0, 2.0,  2.0,  3.0,  4.0,  5.0,  5.0,  6.0,  6.0,  7.0,
                               4.0, 5.0, 5.0, 6.0,  6.0,  7.0,  8.0,  9.0,  9.0,  10.0, 10.0, 11.0,
                               8.0, 9.0, 9.0, 10.0, 10.0, 11.0, 12.0, 13.0, 13.0, 14.0, 14.0, 15.0};
  size_t output_size = 36;
  bool align_corners = true;

  Prepare(input_shape, output_shape, input_data, output_data, align_corners, 2);
  auto ret = kernel_->Run();
  EXPECT_EQ(0, ret);

  ASSERT_EQ(0, CompareOutputData(output_data, expect.data(), output_size, err_tol));
}

// 2*7*5*8 -> 2*14*10*8 align_corners True
TEST_F(TestResizeNearestNeighborFp32, ResizeNearestNeighborTest16) {
  float input_data[] = {
    0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,   8.0,   9.0,   10.0,  11.0,  12.0,  13.0,  14.0,  15.0,
    16.0,  17.0,  18.0,  19.0,  20.0,  21.0,  22.0,  23.0,  24.0,  25.0,  26.0,  27.0,  28.0,  29.0,  30.0,  31.0,
    32.0,  33.0,  34.0,  35.0,  36.0,  37.0,  38.0,  39.0,  40.0,  41.0,  42.0,  43.0,  44.0,  45.0,  46.0,  47.0,
    48.0,  49.0,  50.0,  51.0,  52.0,  53.0,  54.0,  55.0,  56.0,  57.0,  58.0,  59.0,  60.0,  61.0,  62.0,  63.0,
    64.0,  65.0,  66.0,  67.0,  68.0,  69.0,  70.0,  71.0,  72.0,  73.0,  74.0,  75.0,  76.0,  77.0,  78.0,  79.0,
    80.0,  81.0,  82.0,  83.0,  84.0,  85.0,  86.0,  87.0,  88.0,  89.0,  90.0,  91.0,  92.0,  93.0,  94.0,  95.0,
    96.0,  97.0,  98.0,  99.0,  100.0, 101.0, 102.0, 103.0, 104.0, 105.0, 106.0, 107.0, 108.0, 109.0, 110.0, 111.0,
    112.0, 113.0, 114.0, 115.0, 116.0, 117.0, 118.0, 119.0, 120.0, 121.0, 122.0, 123.0, 124.0, 125.0, 126.0, 127.0,
    128.0, 129.0, 130.0, 131.0, 132.0, 133.0, 134.0, 135.0, 136.0, 137.0, 138.0, 139.0, 140.0, 141.0, 142.0, 143.0,
    144.0, 145.0, 146.0, 147.0, 148.0, 149.0, 150.0, 151.0, 152.0, 153.0, 154.0, 155.0, 156.0, 157.0, 158.0, 159.0,
    160.0, 161.0, 162.0, 163.0, 164.0, 165.0, 166.0, 167.0, 168.0, 169.0, 170.0, 171.0, 172.0, 173.0, 174.0, 175.0,
    176.0, 177.0, 178.0, 179.0, 180.0, 181.0, 182.0, 183.0, 184.0, 185.0, 186.0, 187.0, 188.0, 189.0, 190.0, 191.0,
    192.0, 193.0, 194.0, 195.0, 196.0, 197.0, 198.0, 199.0, 200.0, 201.0, 202.0, 203.0, 204.0, 205.0, 206.0, 207.0,
    208.0, 209.0, 210.0, 211.0, 212.0, 213.0, 214.0, 215.0, 216.0, 217.0, 218.0, 219.0, 220.0, 221.0, 222.0, 223.0,
    224.0, 225.0, 226.0, 227.0, 228.0, 229.0, 230.0, 231.0, 232.0, 233.0, 234.0, 235.0, 236.0, 237.0, 238.0, 239.0,
    240.0, 241.0, 242.0, 243.0, 244.0, 245.0, 246.0, 247.0, 248.0, 249.0, 250.0, 251.0, 252.0, 253.0, 254.0, 255.0,
    256.0, 257.0, 258.0, 259.0, 260.0, 261.0, 262.0, 263.0, 264.0, 265.0, 266.0, 267.0, 268.0, 269.0, 270.0, 271.0,
    272.0, 273.0, 274.0, 275.0, 276.0, 277.0, 278.0, 279.0, 280.0, 281.0, 282.0, 283.0, 284.0, 285.0, 286.0, 287.0,
    288.0, 289.0, 290.0, 291.0, 292.0, 293.0, 294.0, 295.0, 296.0, 297.0, 298.0, 299.0, 300.0, 301.0, 302.0, 303.0,
    304.0, 305.0, 306.0, 307.0, 308.0, 309.0, 310.0, 311.0, 312.0, 313.0, 314.0, 315.0, 316.0, 317.0, 318.0, 319.0,
    320.0, 321.0, 322.0, 323.0, 324.0, 325.0, 326.0, 327.0, 328.0, 329.0, 330.0, 331.0, 332.0, 333.0, 334.0, 335.0,
    336.0, 337.0, 338.0, 339.0, 340.0, 341.0, 342.0, 343.0, 344.0, 345.0, 346.0, 347.0, 348.0, 349.0, 350.0, 351.0,
    352.0, 353.0, 354.0, 355.0, 356.0, 357.0, 358.0, 359.0, 360.0, 361.0, 362.0, 363.0, 364.0, 365.0, 366.0, 367.0,
    368.0, 369.0, 370.0, 371.0, 372.0, 373.0, 374.0, 375.0, 376.0, 377.0, 378.0, 379.0, 380.0, 381.0, 382.0, 383.0,
    384.0, 385.0, 386.0, 387.0, 388.0, 389.0, 390.0, 391.0, 392.0, 393.0, 394.0, 395.0, 396.0, 397.0, 398.0, 399.0,
    400.0, 401.0, 402.0, 403.0, 404.0, 405.0, 406.0, 407.0, 408.0, 409.0, 410.0, 411.0, 412.0, 413.0, 414.0, 415.0,
    416.0, 417.0, 418.0, 419.0, 420.0, 421.0, 422.0, 423.0, 424.0, 425.0, 426.0, 427.0, 428.0, 429.0, 430.0, 431.0,
    432.0, 433.0, 434.0, 435.0, 436.0, 437.0, 438.0, 439.0, 440.0, 441.0, 442.0, 443.0, 444.0, 445.0, 446.0, 447.0,
    448.0, 449.0, 450.0, 451.0, 452.0, 453.0, 454.0, 455.0, 456.0, 457.0, 458.0, 459.0, 460.0, 461.0, 462.0, 463.0,
    464.0, 465.0, 466.0, 467.0, 468.0, 469.0, 470.0, 471.0, 472.0, 473.0, 474.0, 475.0, 476.0, 477.0, 478.0, 479.0,
    480.0, 481.0, 482.0, 483.0, 484.0, 485.0, 486.0, 487.0, 488.0, 489.0, 490.0, 491.0, 492.0, 493.0, 494.0, 495.0,
    496.0, 497.0, 498.0, 499.0, 500.0, 501.0, 502.0, 503.0, 504.0, 505.0, 506.0, 507.0, 508.0, 509.0, 510.0, 511.0,
    512.0, 513.0, 514.0, 515.0, 516.0, 517.0, 518.0, 519.0, 520.0, 521.0, 522.0, 523.0, 524.0, 525.0, 526.0, 527.0,
    528.0, 529.0, 530.0, 531.0, 532.0, 533.0, 534.0, 535.0, 536.0, 537.0, 538.0, 539.0, 540.0, 541.0, 542.0, 543.0,
    544.0, 545.0, 546.0, 547.0, 548.0, 549.0, 550.0, 551.0, 552.0, 553.0, 554.0, 555.0, 556.0, 557.0, 558.0, 559.0};
  float output_data[2240] = {0};
  std::vector<int> input_shape = {2, 7, 5, 8};
  std::vector<int> output_shape = {2, 14, 10, 8};
  std::vector<float> expect = {
    0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,   0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,
    8.0,   9.0,   10.0,  11.0,  12.0,  13.0,  14.0,  15.0,  8.0,   9.0,   10.0,  11.0,  12.0,  13.0,  14.0,  15.0,
    16.0,  17.0,  18.0,  19.0,  20.0,  21.0,  22.0,  23.0,  16.0,  17.0,  18.0,  19.0,  20.0,  21.0,  22.0,  23.0,
    24.0,  25.0,  26.0,  27.0,  28.0,  29.0,  30.0,  31.0,  24.0,  25.0,  26.0,  27.0,  28.0,  29.0,  30.0,  31.0,
    32.0,  33.0,  34.0,  35.0,  36.0,  37.0,  38.0,  39.0,  32.0,  33.0,  34.0,  35.0,  36.0,  37.0,  38.0,  39.0,
    0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,   0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,
    8.0,   9.0,   10.0,  11.0,  12.0,  13.0,  14.0,  15.0,  8.0,   9.0,   10.0,  11.0,  12.0,  13.0,  14.0,  15.0,
    16.0,  17.0,  18.0,  19.0,  20.0,  21.0,  22.0,  23.0,  16.0,  17.0,  18.0,  19.0,  20.0,  21.0,  22.0,  23.0,
    24.0,  25.0,  26.0,  27.0,  28.0,  29.0,  30.0,  31.0,  24.0,  25.0,  26.0,  27.0,  28.0,  29.0,  30.0,  31.0,
    32.0,  33.0,  34.0,  35.0,  36.0,  37.0,  38.0,  39.0,  32.0,  33.0,  34.0,  35.0,  36.0,  37.0,  38.0,  39.0,
    40.0,  41.0,  42.0,  43.0,  44.0,  45.0,  46.0,  47.0,  40.0,  41.0,  42.0,  43.0,  44.0,  45.0,  46.0,  47.0,
    48.0,  49.0,  50.0,  51.0,  52.0,  53.0,  54.0,  55.0,  48.0,  49.0,  50.0,  51.0,  52.0,  53.0,  54.0,  55.0,
    56.0,  57.0,  58.0,  59.0,  60.0,  61.0,  62.0,  63.0,  56.0,  57.0,  58.0,  59.0,  60.0,  61.0,  62.0,  63.0,
    64.0,  65.0,  66.0,  67.0,  68.0,  69.0,  70.0,  71.0,  64.0,  65.0,  66.0,  67.0,  68.0,  69.0,  70.0,  71.0,
    72.0,  73.0,  74.0,  75.0,  76.0,  77.0,  78.0,  79.0,  72.0,  73.0,  74.0,  75.0,  76.0,  77.0,  78.0,  79.0,
    40.0,  41.0,  42.0,  43.0,  44.0,  45.0,  46.0,  47.0,  40.0,  41.0,  42.0,  43.0,  44.0,  45.0,  46.0,  47.0,
    48.0,  49.0,  50.0,  51.0,  52.0,  53.0,  54.0,  55.0,  48.0,  49.0,  50.0,  51.0,  52.0,  53.0,  54.0,  55.0,
    56.0,  57.0,  58.0,  59.0,  60.0,  61.0,  62.0,  63.0,  56.0,  57.0,  58.0,  59.0,  60.0,  61.0,  62.0,  63.0,
    64.0,  65.0,  66.0,  67.0,  68.0,  69.0,  70.0,  71.0,  64.0,  65.0,  66.0,  67.0,  68.0,  69.0,  70.0,  71.0,
    72.0,  73.0,  74.0,  75.0,  76.0,  77.0,  78.0,  79.0,  72.0,  73.0,  74.0,  75.0,  76.0,  77.0,  78.0,  79.0,
    80.0,  81.0,  82.0,  83.0,  84.0,  85.0,  86.0,  87.0,  80.0,  81.0,  82.0,  83.0,  84.0,  85.0,  86.0,  87.0,
    88.0,  89.0,  90.0,  91.0,  92.0,  93.0,  94.0,  95.0,  88.0,  89.0,  90.0,  91.0,  92.0,  93.0,  94.0,  95.0,
    96.0,  97.0,  98.0,  99.0,  100.0, 101.0, 102.0, 103.0, 96.0,  97.0,  98.0,  99.0,  100.0, 101.0, 102.0, 103.0,
    104.0, 105.0, 106.0, 107.0, 108.0, 109.0, 110.0, 111.0, 104.0, 105.0, 106.0, 107.0, 108.0, 109.0, 110.0, 111.0,
    112.0, 113.0, 114.0, 115.0, 116.0, 117.0, 118.0, 119.0, 112.0, 113.0, 114.0, 115.0, 116.0, 117.0, 118.0, 119.0,
    80.0,  81.0,  82.0,  83.0,  84.0,  85.0,  86.0,  87.0,  80.0,  81.0,  82.0,  83.0,  84.0,  85.0,  86.0,  87.0,
    88.0,  89.0,  90.0,  91.0,  92.0,  93.0,  94.0,  95.0,  88.0,  89.0,  90.0,  91.0,  92.0,  93.0,  94.0,  95.0,
    96.0,  97.0,  98.0,  99.0,  100.0, 101.0, 102.0, 103.0, 96.0,  97.0,  98.0,  99.0,  100.0, 101.0, 102.0, 103.0,
    104.0, 105.0, 106.0, 107.0, 108.0, 109.0, 110.0, 111.0, 104.0, 105.0, 106.0, 107.0, 108.0, 109.0, 110.0, 111.0,
    112.0, 113.0, 114.0, 115.0, 116.0, 117.0, 118.0, 119.0, 112.0, 113.0, 114.0, 115.0, 116.0, 117.0, 118.0, 119.0,
    120.0, 121.0, 122.0, 123.0, 124.0, 125.0, 126.0, 127.0, 120.0, 121.0, 122.0, 123.0, 124.0, 125.0, 126.0, 127.0,
    128.0, 129.0, 130.0, 131.0, 132.0, 133.0, 134.0, 135.0, 128.0, 129.0, 130.0, 131.0, 132.0, 133.0, 134.0, 135.0,
    136.0, 137.0, 138.0, 139.0, 140.0, 141.0, 142.0, 143.0, 136.0, 137.0, 138.0, 139.0, 140.0, 141.0, 142.0, 143.0,
    144.0, 145.0, 146.0, 147.0, 148.0, 149.0, 150.0, 151.0, 144.0, 145.0, 146.0, 147.0, 148.0, 149.0, 150.0, 151.0,
    152.0, 153.0, 154.0, 155.0, 156.0, 157.0, 158.0, 159.0, 152.0, 153.0, 154.0, 155.0, 156.0, 157.0, 158.0, 159.0,
    120.0, 121.0, 122.0, 123.0, 124.0, 125.0, 126.0, 127.0, 120.0, 121.0, 122.0, 123.0, 124.0, 125.0, 126.0, 127.0,
    128.0, 129.0, 130.0, 131.0, 132.0, 133.0, 134.0, 135.0, 128.0, 129.0, 130.0, 131.0, 132.0, 133.0, 134.0, 135.0,
    136.0, 137.0, 138.0, 139.0, 140.0, 141.0, 142.0, 143.0, 136.0, 137.0, 138.0, 139.0, 140.0, 141.0, 142.0, 143.0,
    144.0, 145.0, 146.0, 147.0, 148.0, 149.0, 150.0, 151.0, 144.0, 145.0, 146.0, 147.0, 148.0, 149.0, 150.0, 151.0,
    152.0, 153.0, 154.0, 155.0, 156.0, 157.0, 158.0, 159.0, 152.0, 153.0, 154.0, 155.0, 156.0, 157.0, 158.0, 159.0,
    160.0, 161.0, 162.0, 163.0, 164.0, 165.0, 166.0, 167.0, 160.0, 161.0, 162.0, 163.0, 164.0, 165.0, 166.0, 167.0,
    168.0, 169.0, 170.0, 171.0, 172.0, 173.0, 174.0, 175.0, 168.0, 169.0, 170.0, 171.0, 172.0, 173.0, 174.0, 175.0,
    176.0, 177.0, 178.0, 179.0, 180.0, 181.0, 182.0, 183.0, 176.0, 177.0, 178.0, 179.0, 180.0, 181.0, 182.0, 183.0,
    184.0, 185.0, 186.0, 187.0, 188.0, 189.0, 190.0, 191.0, 184.0, 185.0, 186.0, 187.0, 188.0, 189.0, 190.0, 191.0,
    192.0, 193.0, 194.0, 195.0, 196.0, 197.0, 198.0, 199.0, 192.0, 193.0, 194.0, 195.0, 196.0, 197.0, 198.0, 199.0,
    160.0, 161.0, 162.0, 163.0, 164.0, 165.0, 166.0, 167.0, 160.0, 161.0, 162.0, 163.0, 164.0, 165.0, 166.0, 167.0,
    168.0, 169.0, 170.0, 171.0, 172.0, 173.0, 174.0, 175.0, 168.0, 169.0, 170.0, 171.0, 172.0, 173.0, 174.0, 175.0,
    176.0, 177.0, 178.0, 179.0, 180.0, 181.0, 182.0, 183.0, 176.0, 177.0, 178.0, 179.0, 180.0, 181.0, 182.0, 183.0,
    184.0, 185.0, 186.0, 187.0, 188.0, 189.0, 190.0, 191.0, 184.0, 185.0, 186.0, 187.0, 188.0, 189.0, 190.0, 191.0,
    192.0, 193.0, 194.0, 195.0, 196.0, 197.0, 198.0, 199.0, 192.0, 193.0, 194.0, 195.0, 196.0, 197.0, 198.0, 199.0,
    200.0, 201.0, 202.0, 203.0, 204.0, 205.0, 206.0, 207.0, 200.0, 201.0, 202.0, 203.0, 204.0, 205.0, 206.0, 207.0,
    208.0, 209.0, 210.0, 211.0, 212.0, 213.0, 214.0, 215.0, 208.0, 209.0, 210.0, 211.0, 212.0, 213.0, 214.0, 215.0,
    216.0, 217.0, 218.0, 219.0, 220.0, 221.0, 222.0, 223.0, 216.0, 217.0, 218.0, 219.0, 220.0, 221.0, 222.0, 223.0,
    224.0, 225.0, 226.0, 227.0, 228.0, 229.0, 230.0, 231.0, 224.0, 225.0, 226.0, 227.0, 228.0, 229.0, 230.0, 231.0,
    232.0, 233.0, 234.0, 235.0, 236.0, 237.0, 238.0, 239.0, 232.0, 233.0, 234.0, 235.0, 236.0, 237.0, 238.0, 239.0,
    200.0, 201.0, 202.0, 203.0, 204.0, 205.0, 206.0, 207.0, 200.0, 201.0, 202.0, 203.0, 204.0, 205.0, 206.0, 207.0,
    208.0, 209.0, 210.0, 211.0, 212.0, 213.0, 214.0, 215.0, 208.0, 209.0, 210.0, 211.0, 212.0, 213.0, 214.0, 215.0,
    216.0, 217.0, 218.0, 219.0, 220.0, 221.0, 222.0, 223.0, 216.0, 217.0, 218.0, 219.0, 220.0, 221.0, 222.0, 223.0,
    224.0, 225.0, 226.0, 227.0, 228.0, 229.0, 230.0, 231.0, 224.0, 225.0, 226.0, 227.0, 228.0, 229.0, 230.0, 231.0,
    232.0, 233.0, 234.0, 235.0, 236.0, 237.0, 238.0, 239.0, 232.0, 233.0, 234.0, 235.0, 236.0, 237.0, 238.0, 239.0,
    240.0, 241.0, 242.0, 243.0, 244.0, 245.0, 246.0, 247.0, 240.0, 241.0, 242.0, 243.0, 244.0, 245.0, 246.0, 247.0,
    248.0, 249.0, 250.0, 251.0, 252.0, 253.0, 254.0, 255.0, 248.0, 249.0, 250.0, 251.0, 252.0, 253.0, 254.0, 255.0,
    256.0, 257.0, 258.0, 259.0, 260.0, 261.0, 262.0, 263.0, 256.0, 257.0, 258.0, 259.0, 260.0, 261.0, 262.0, 263.0,
    264.0, 265.0, 266.0, 267.0, 268.0, 269.0, 270.0, 271.0, 264.0, 265.0, 266.0, 267.0, 268.0, 269.0, 270.0, 271.0,
    272.0, 273.0, 274.0, 275.0, 276.0, 277.0, 278.0, 279.0, 272.0, 273.0, 274.0, 275.0, 276.0, 277.0, 278.0, 279.0,
    240.0, 241.0, 242.0, 243.0, 244.0, 245.0, 246.0, 247.0, 240.0, 241.0, 242.0, 243.0, 244.0, 245.0, 246.0, 247.0,
    248.0, 249.0, 250.0, 251.0, 252.0, 253.0, 254.0, 255.0, 248.0, 249.0, 250.0, 251.0, 252.0, 253.0, 254.0, 255.0,
    256.0, 257.0, 258.0, 259.0, 260.0, 261.0, 262.0, 263.0, 256.0, 257.0, 258.0, 259.0, 260.0, 261.0, 262.0, 263.0,
    264.0, 265.0, 266.0, 267.0, 268.0, 269.0, 270.0, 271.0, 264.0, 265.0, 266.0, 267.0, 268.0, 269.0, 270.0, 271.0,
    272.0, 273.0, 274.0, 275.0, 276.0, 277.0, 278.0, 279.0, 272.0, 273.0, 274.0, 275.0, 276.0, 277.0, 278.0, 279.0,
    280.0, 281.0, 282.0, 283.0, 284.0, 285.0, 286.0, 287.0, 280.0, 281.0, 282.0, 283.0, 284.0, 285.0, 286.0, 287.0,
    288.0, 289.0, 290.0, 291.0, 292.0, 293.0, 294.0, 295.0, 288.0, 289.0, 290.0, 291.0, 292.0, 293.0, 294.0, 295.0,
    296.0, 297.0, 298.0, 299.0, 300.0, 301.0, 302.0, 303.0, 296.0, 297.0, 298.0, 299.0, 300.0, 301.0, 302.0, 303.0,
    304.0, 305.0, 306.0, 307.0, 308.0, 309.0, 310.0, 311.0, 304.0, 305.0, 306.0, 307.0, 308.0, 309.0, 310.0, 311.0,
    312.0, 313.0, 314.0, 315.0, 316.0, 317.0, 318.0, 319.0, 312.0, 313.0, 314.0, 315.0, 316.0, 317.0, 318.0, 319.0,
    280.0, 281.0, 282.0, 283.0, 284.0, 285.0, 286.0, 287.0, 280.0, 281.0, 282.0, 283.0, 284.0, 285.0, 286.0, 287.0,
    288.0, 289.0, 290.0, 291.0, 292.0, 293.0, 294.0, 295.0, 288.0, 289.0, 290.0, 291.0, 292.0, 293.0, 294.0, 295.0,
    296.0, 297.0, 298.0, 299.0, 300.0, 301.0, 302.0, 303.0, 296.0, 297.0, 298.0, 299.0, 300.0, 301.0, 302.0, 303.0,
    304.0, 305.0, 306.0, 307.0, 308.0, 309.0, 310.0, 311.0, 304.0, 305.0, 306.0, 307.0, 308.0, 309.0, 310.0, 311.0,
    312.0, 313.0, 314.0, 315.0, 316.0, 317.0, 318.0, 319.0, 312.0, 313.0, 314.0, 315.0, 316.0, 317.0, 318.0, 319.0,
    320.0, 321.0, 322.0, 323.0, 324.0, 325.0, 326.0, 327.0, 320.0, 321.0, 322.0, 323.0, 324.0, 325.0, 326.0, 327.0,
    328.0, 329.0, 330.0, 331.0, 332.0, 333.0, 334.0, 335.0, 328.0, 329.0, 330.0, 331.0, 332.0, 333.0, 334.0, 335.0,
    336.0, 337.0, 338.0, 339.0, 340.0, 341.0, 342.0, 343.0, 336.0, 337.0, 338.0, 339.0, 340.0, 341.0, 342.0, 343.0,
    344.0, 345.0, 346.0, 347.0, 348.0, 349.0, 350.0, 351.0, 344.0, 345.0, 346.0, 347.0, 348.0, 349.0, 350.0, 351.0,
    352.0, 353.0, 354.0, 355.0, 356.0, 357.0, 358.0, 359.0, 352.0, 353.0, 354.0, 355.0, 356.0, 357.0, 358.0, 359.0,
    320.0, 321.0, 322.0, 323.0, 324.0, 325.0, 326.0, 327.0, 320.0, 321.0, 322.0, 323.0, 324.0, 325.0, 326.0, 327.0,
    328.0, 329.0, 330.0, 331.0, 332.0, 333.0, 334.0, 335.0, 328.0, 329.0, 330.0, 331.0, 332.0, 333.0, 334.0, 335.0,
    336.0, 337.0, 338.0, 339.0, 340.0, 341.0, 342.0, 343.0, 336.0, 337.0, 338.0, 339.0, 340.0, 341.0, 342.0, 343.0,
    344.0, 345.0, 346.0, 347.0, 348.0, 349.0, 350.0, 351.0, 344.0, 345.0, 346.0, 347.0, 348.0, 349.0, 350.0, 351.0,
    352.0, 353.0, 354.0, 355.0, 356.0, 357.0, 358.0, 359.0, 352.0, 353.0, 354.0, 355.0, 356.0, 357.0, 358.0, 359.0,
    360.0, 361.0, 362.0, 363.0, 364.0, 365.0, 366.0, 367.0, 360.0, 361.0, 362.0, 363.0, 364.0, 365.0, 366.0, 367.0,
    368.0, 369.0, 370.0, 371.0, 372.0, 373.0, 374.0, 375.0, 368.0, 369.0, 370.0, 371.0, 372.0, 373.0, 374.0, 375.0,
    376.0, 377.0, 378.0, 379.0, 380.0, 381.0, 382.0, 383.0, 376.0, 377.0, 378.0, 379.0, 380.0, 381.0, 382.0, 383.0,
    384.0, 385.0, 386.0, 387.0, 388.0, 389.0, 390.0, 391.0, 384.0, 385.0, 386.0, 387.0, 388.0, 389.0, 390.0, 391.0,
    392.0, 393.0, 394.0, 395.0, 396.0, 397.0, 398.0, 399.0, 392.0, 393.0, 394.0, 395.0, 396.0, 397.0, 398.0, 399.0,
    360.0, 361.0, 362.0, 363.0, 364.0, 365.0, 366.0, 367.0, 360.0, 361.0, 362.0, 363.0, 364.0, 365.0, 366.0, 367.0,
    368.0, 369.0, 370.0, 371.0, 372.0, 373.0, 374.0, 375.0, 368.0, 369.0, 370.0, 371.0, 372.0, 373.0, 374.0, 375.0,
    376.0, 377.0, 378.0, 379.0, 380.0, 381.0, 382.0, 383.0, 376.0, 377.0, 378.0, 379.0, 380.0, 381.0, 382.0, 383.0,
    384.0, 385.0, 386.0, 387.0, 388.0, 389.0, 390.0, 391.0, 384.0, 385.0, 386.0, 387.0, 388.0, 389.0, 390.0, 391.0,
    392.0, 393.0, 394.0, 395.0, 396.0, 397.0, 398.0, 399.0, 392.0, 393.0, 394.0, 395.0, 396.0, 397.0, 398.0, 399.0,
    400.0, 401.0, 402.0, 403.0, 404.0, 405.0, 406.0, 407.0, 400.0, 401.0, 402.0, 403.0, 404.0, 405.0, 406.0, 407.0,
    408.0, 409.0, 410.0, 411.0, 412.0, 413.0, 414.0, 415.0, 408.0, 409.0, 410.0, 411.0, 412.0, 413.0, 414.0, 415.0,
    416.0, 417.0, 418.0, 419.0, 420.0, 421.0, 422.0, 423.0, 416.0, 417.0, 418.0, 419.0, 420.0, 421.0, 422.0, 423.0,
    424.0, 425.0, 426.0, 427.0, 428.0, 429.0, 430.0, 431.0, 424.0, 425.0, 426.0, 427.0, 428.0, 429.0, 430.0, 431.0,
    432.0, 433.0, 434.0, 435.0, 436.0, 437.0, 438.0, 439.0, 432.0, 433.0, 434.0, 435.0, 436.0, 437.0, 438.0, 439.0,
    400.0, 401.0, 402.0, 403.0, 404.0, 405.0, 406.0, 407.0, 400.0, 401.0, 402.0, 403.0, 404.0, 405.0, 406.0, 407.0,
    408.0, 409.0, 410.0, 411.0, 412.0, 413.0, 414.0, 415.0, 408.0, 409.0, 410.0, 411.0, 412.0, 413.0, 414.0, 415.0,
    416.0, 417.0, 418.0, 419.0, 420.0, 421.0, 422.0, 423.0, 416.0, 417.0, 418.0, 419.0, 420.0, 421.0, 422.0, 423.0,
    424.0, 425.0, 426.0, 427.0, 428.0, 429.0, 430.0, 431.0, 424.0, 425.0, 426.0, 427.0, 428.0, 429.0, 430.0, 431.0,
    432.0, 433.0, 434.0, 435.0, 436.0, 437.0, 438.0, 439.0, 432.0, 433.0, 434.0, 435.0, 436.0, 437.0, 438.0, 439.0,
    440.0, 441.0, 442.0, 443.0, 444.0, 445.0, 446.0, 447.0, 440.0, 441.0, 442.0, 443.0, 444.0, 445.0, 446.0, 447.0,
    448.0, 449.0, 450.0, 451.0, 452.0, 453.0, 454.0, 455.0, 448.0, 449.0, 450.0, 451.0, 452.0, 453.0, 454.0, 455.0,
    456.0, 457.0, 458.0, 459.0, 460.0, 461.0, 462.0, 463.0, 456.0, 457.0, 458.0, 459.0, 460.0, 461.0, 462.0, 463.0,
    464.0, 465.0, 466.0, 467.0, 468.0, 469.0, 470.0, 471.0, 464.0, 465.0, 466.0, 467.0, 468.0, 469.0, 470.0, 471.0,
    472.0, 473.0, 474.0, 475.0, 476.0, 477.0, 478.0, 479.0, 472.0, 473.0, 474.0, 475.0, 476.0, 477.0, 478.0, 479.0,
    440.0, 441.0, 442.0, 443.0, 444.0, 445.0, 446.0, 447.0, 440.0, 441.0, 442.0, 443.0, 444.0, 445.0, 446.0, 447.0,
    448.0, 449.0, 450.0, 451.0, 452.0, 453.0, 454.0, 455.0, 448.0, 449.0, 450.0, 451.0, 452.0, 453.0, 454.0, 455.0,
    456.0, 457.0, 458.0, 459.0, 460.0, 461.0, 462.0, 463.0, 456.0, 457.0, 458.0, 459.0, 460.0, 461.0, 462.0, 463.0,
    464.0, 465.0, 466.0, 467.0, 468.0, 469.0, 470.0, 471.0, 464.0, 465.0, 466.0, 467.0, 468.0, 469.0, 470.0, 471.0,
    472.0, 473.0, 474.0, 475.0, 476.0, 477.0, 478.0, 479.0, 472.0, 473.0, 474.0, 475.0, 476.0, 477.0, 478.0, 479.0,
    480.0, 481.0, 482.0, 483.0, 484.0, 485.0, 486.0, 487.0, 480.0, 481.0, 482.0, 483.0, 484.0, 485.0, 486.0, 487.0,
    488.0, 489.0, 490.0, 491.0, 492.0, 493.0, 494.0, 495.0, 488.0, 489.0, 490.0, 491.0, 492.0, 493.0, 494.0, 495.0,
    496.0, 497.0, 498.0, 499.0, 500.0, 501.0, 502.0, 503.0, 496.0, 497.0, 498.0, 499.0, 500.0, 501.0, 502.0, 503.0,
    504.0, 505.0, 506.0, 507.0, 508.0, 509.0, 510.0, 511.0, 504.0, 505.0, 506.0, 507.0, 508.0, 509.0, 510.0, 511.0,
    512.0, 513.0, 514.0, 515.0, 516.0, 517.0, 518.0, 519.0, 512.0, 513.0, 514.0, 515.0, 516.0, 517.0, 518.0, 519.0,
    480.0, 481.0, 482.0, 483.0, 484.0, 485.0, 486.0, 487.0, 480.0, 481.0, 482.0, 483.0, 484.0, 485.0, 486.0, 487.0,
    488.0, 489.0, 490.0, 491.0, 492.0, 493.0, 494.0, 495.0, 488.0, 489.0, 490.0, 491.0, 492.0, 493.0, 494.0, 495.0,
    496.0, 497.0, 498.0, 499.0, 500.0, 501.0, 502.0, 503.0, 496.0, 497.0, 498.0, 499.0, 500.0, 501.0, 502.0, 503.0,
    504.0, 505.0, 506.0, 507.0, 508.0, 509.0, 510.0, 511.0, 504.0, 505.0, 506.0, 507.0, 508.0, 509.0, 510.0, 511.0,
    512.0, 513.0, 514.0, 515.0, 516.0, 517.0, 518.0, 519.0, 512.0, 513.0, 514.0, 515.0, 516.0, 517.0, 518.0, 519.0,
    520.0, 521.0, 522.0, 523.0, 524.0, 525.0, 526.0, 527.0, 520.0, 521.0, 522.0, 523.0, 524.0, 525.0, 526.0, 527.0,
    528.0, 529.0, 530.0, 531.0, 532.0, 533.0, 534.0, 535.0, 528.0, 529.0, 530.0, 531.0, 532.0, 533.0, 534.0, 535.0,
    536.0, 537.0, 538.0, 539.0, 540.0, 541.0, 542.0, 543.0, 536.0, 537.0, 538.0, 539.0, 540.0, 541.0, 542.0, 543.0,
    544.0, 545.0, 546.0, 547.0, 548.0, 549.0, 550.0, 551.0, 544.0, 545.0, 546.0, 547.0, 548.0, 549.0, 550.0, 551.0,
    552.0, 553.0, 554.0, 555.0, 556.0, 557.0, 558.0, 559.0, 552.0, 553.0, 554.0, 555.0, 556.0, 557.0, 558.0, 559.0,
    520.0, 521.0, 522.0, 523.0, 524.0, 525.0, 526.0, 527.0, 520.0, 521.0, 522.0, 523.0, 524.0, 525.0, 526.0, 527.0,
    528.0, 529.0, 530.0, 531.0, 532.0, 533.0, 534.0, 535.0, 528.0, 529.0, 530.0, 531.0, 532.0, 533.0, 534.0, 535.0,
    536.0, 537.0, 538.0, 539.0, 540.0, 541.0, 542.0, 543.0, 536.0, 537.0, 538.0, 539.0, 540.0, 541.0, 542.0, 543.0,
    544.0, 545.0, 546.0, 547.0, 548.0, 549.0, 550.0, 551.0, 544.0, 545.0, 546.0, 547.0, 548.0, 549.0, 550.0, 551.0,
    552.0, 553.0, 554.0, 555.0, 556.0, 557.0, 558.0, 559.0, 552.0, 553.0, 554.0, 555.0, 556.0, 557.0, 558.0, 559.0};
  size_t output_size = 2240;
  bool align_corners = true;

  Prepare(input_shape, output_shape, input_data, output_data, align_corners, 2);
  auto ret = kernel_->Run();
  EXPECT_EQ(0, ret);

  ASSERT_EQ(0, CompareOutputData(output_data, expect.data(), output_size, err_tol));
}
}  // namespace mindspore
