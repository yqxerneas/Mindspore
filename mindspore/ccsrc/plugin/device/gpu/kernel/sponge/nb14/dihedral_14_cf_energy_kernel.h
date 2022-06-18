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

#ifndef MINDSPORE_CCSRC_BACKEND_KERNEL_COMPILER_GPU_SPONG_NB14_DIHEDRAL_14_CF_ENERGY_KERNEL_H_
#define MINDSPORE_CCSRC_BACKEND_KERNEL_COMPILER_GPU_SPONG_NB14_DIHEDRAL_14_CF_ENERGY_KERNEL_H_

#include <cuda_runtime_api.h>
#include <vector>
#include <string>
#include <map>
#include "plugin/device/gpu/kernel/gpu_kernel.h"
#include "plugin/device/gpu/kernel/gpu_kernel_factory.h"
#include "plugin/device/gpu/kernel/cuda_impl/cuda_ops/cuda_common.h"
#include "plugin/device/gpu/kernel/cuda_impl/sponge/nb14/dihedral_14_cf_energy_impl.cuh"

namespace mindspore {
namespace kernel {
template <typename T, typename T1>
class Dihedral14CFEnergyGpuKernelMod : public DeprecatedNativeGpuKernelMod {
 public:
  Dihedral14CFEnergyGpuKernelMod() : ele_uint_crd(1) {}
  ~Dihedral14CFEnergyGpuKernelMod() override = default;

  bool Init(const CNodePtr &kernel_node) override {
    kernel_node_ = kernel_node;
    dihedral_14_numbers = static_cast<int>(GetAttr<int64_t>(kernel_node, "dihedral_14_numbers"));
    atom_numbers = static_cast<int>(GetAttr<int64_t>(kernel_node, "atom_numbers"));

    auto shape_uint_crd = common::AnfAlgo::GetPrevNodeOutputInferShape(kernel_node, 0);
    auto shape_LJtype = common::AnfAlgo::GetPrevNodeOutputInferShape(kernel_node, 1);
    auto shape_charge = common::AnfAlgo::GetPrevNodeOutputInferShape(kernel_node, 2);
    auto shape_boxlength_f = common::AnfAlgo::GetPrevNodeOutputInferShape(kernel_node, 3);
    auto shape_a_14 = common::AnfAlgo::GetPrevNodeOutputInferShape(kernel_node, 4);
    auto shape_b_14 = common::AnfAlgo::GetPrevNodeOutputInferShape(kernel_node, 5);
    auto shape_cf_scale_factor = common::AnfAlgo::GetPrevNodeOutputInferShape(kernel_node, 6);

    ele_uint_crd *= SizeOf(shape_uint_crd);
    ele_LJtype *= SizeOf(shape_LJtype);
    ele_charge *= SizeOf(shape_charge);
    ele_boxlength_f *= SizeOf(shape_boxlength_f);
    ele_a_14 *= SizeOf(shape_a_14);
    ele_b_14 *= SizeOf(shape_b_14);
    ele_cf_scale_factor *= SizeOf(shape_cf_scale_factor);

    InitSizeLists();
    return true;
  }

  bool Launch(const std::vector<AddressPtr> &inputs, const std::vector<AddressPtr> &workspace,
              const std::vector<AddressPtr> &outputs, void *stream_ptr) override {
    auto uint_crd_f = GetDeviceAddress<const T1>(inputs, 0);
    auto LJtype = GetDeviceAddress<const T1>(inputs, 1);
    auto charge = GetDeviceAddress<const T>(inputs, 2);
    auto boxlength_f = GetDeviceAddress<T>(inputs, 3);
    auto a_14 = GetDeviceAddress<const T1>(inputs, 4);
    auto b_14 = GetDeviceAddress<const T1>(inputs, 5);
    auto cf_scale_factor = GetDeviceAddress<T>(inputs, 6);
    auto ene = GetDeviceAddress<T>(outputs, 0);
    auto uint_crd_with_LJ = GetDeviceAddress<T>(workspace, 0);

    Dihedral14CFEnergy(dihedral_14_numbers, atom_numbers, uint_crd_f, LJtype, charge, uint_crd_with_LJ, boxlength_f,
                       a_14, b_14, cf_scale_factor, ene, reinterpret_cast<cudaStream_t>(stream_ptr));

    return true;
  }

 protected:
  void InitSizeLists() override {
    input_size_list_.push_back(ele_uint_crd * sizeof(T1));
    input_size_list_.push_back(ele_LJtype * sizeof(T1));
    input_size_list_.push_back(ele_charge * sizeof(T));
    input_size_list_.push_back(ele_boxlength_f * sizeof(T));
    input_size_list_.push_back(ele_a_14 * sizeof(T1));
    input_size_list_.push_back(ele_b_14 * sizeof(T1));
    input_size_list_.push_back(ele_cf_scale_factor * sizeof(T));
    workspace_size_list_.push_back(atom_numbers * sizeof(UINT_VECTOR_LJ_TYPE));
    output_size_list_.push_back(dihedral_14_numbers * sizeof(T));
  }

 private:
  size_t ele_uint_crd = 1;
  size_t ele_LJtype = 1;
  size_t ele_charge = 1;
  size_t ele_boxlength_f = 1;
  size_t ele_a_14 = 1;
  size_t ele_b_14 = 1;
  size_t ele_cf_scale_factor = 1;

  int dihedral_14_numbers;
  int atom_numbers;
  struct UINT_VECTOR_LJ_TYPE {
    unsigned int uint_x;
    unsigned int uint_y;
    unsigned int uint_z;
    int LJ_type;
    float charge;
  };
};
}  // namespace kernel
}  // namespace mindspore
#endif  // MINDSPORE_CCSRC_BACKEND_KERNEL_COMPILER_GPU_SPONG_NB14_DIHEDRAL_14_LJ_FORCE_WITH_DIRECT_CF_KERNEL_H_
