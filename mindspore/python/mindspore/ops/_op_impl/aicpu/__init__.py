# Copyright 2020-2022 Huawei Technologies Co., Ltd
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""aicpu ops"""
from .extract_glimpse import _extract_glimpse_aicpu
from .nextafter import _nextafter_aicpu
from .fill_diagonal import _fill_diagonal_aicpu
from .fractional_max_pool import _fractional_max_pool_aicpu
from .fractional_max_pool_grad import _fractional_max_pool_grad_aicpu
from .fractional_avg_pool import _fractional_avg_pool_aicpu
from .fractional_avg_pool_grad import _fractional_avg_pool_grad_aicpu
from .unravel_index import _unravel_index_aicpu
from .hsv_to_rgb import _hsv_to_rgb_aicpu
from .rgb_to_hsv import _rgb_to_hsv_aicpu
from .unique import _unique_aicpu
from .lu_solve import _lu_solve_aicpu
from .matrix_solve import _matrix_solve_aicpu
from .cholesky_inverse import _cholesky_inverse_aicpu
from .betainc import _betainc_aicpu
from .blackman_window import _blackman_window_aicpu
from .no_repeat_ngram import _no_repeat_ngram_aicpu
from .init_data_set_queue import _init_data_set_queue_aicpu
from .embedding_lookup import _embedding_lookup_aicpu
from .log import _log_aicpu
from .padding import _padding_aicpu
from .gather import _gather_aicpu
from .gather_grad import _gather_grad_aicpu
from .gather_d_grad_v2 import _gather_d_grad_v2_aicpu
from .gather_d import _gather_d_aicpu
from .hamming_window import _hamming_window_aicpu
from .scatter import _scatter_aicpu
from .exp import _exp_aicpu
from .expm1 import _expm1_aicpu
from .identity import _identity_aicpu
from .identity_n import _identity_n_aicpu
from .index_fill import _index_fill_aicpu
from .edit_distance import _edit_distance_aicpu
from .unique_with_pad import _unique_with_pad_aicpu
from .bartlett_window import _bartlett_window_aicpu
from .add_n import _add_n_aicpu
from .sub_and_filter import _sub_and_filter_aicpu
from .pad_and_shift import _pad_and_shift_aicpu
from .data_format_vec_permute import _data_format_vec_permute_aicpu
from .dropout_genmask import _dropout_genmask_aicpu
from .dropout_genmask_v3 import _dropout_genmask_v3_aicpu
from .dropout2d import _dropout2d_aicpu
from .dropout3d import _dropout3d_aicpu
from .dynamic_stitch import _dynamic_stitch_aicpu
from .get_next import _get_next_aicpu
from .print_tensor import _print_aicpu
from .topk import _top_k_aicpu
from .logical_xor import _logical_xor_aicpu
from .log1p import _log1p_aicpu
from .asin import _asin_aicpu
from .asin_grad import _asin_grad_aicpu
from .histogram import _histogram_aicpu
from .is_finite import _is_finite_aicpu
from .is_inf import _is_inf_aicpu
from .is_nan import _is_nan_aicpu
from .igamma import _igamma_aicpu
from .igammac import _igammac_aicpu
from .igammagrada import _igammagrada_aicpu
from .reshape import _reshape_aicpu
from .fill_v2 import _fill_v2_aicpu
from .flatten import _flatten_aicpu
from .sin import _sin_aicpu
from .cos import _cos_aicpu
from .sinh import _sinh_aicpu
from .cosh import _cosh_aicpu
from .sign import _sign_aicpu
from .squeeze import _squeeze_aicpu
from .acos import _acos_aicpu
from .acos_grad import _acos_grad_aicpu
from .real_div import _real_div_aicpu
from .expand import _expand_aicpu
from .expand_dims import _expand_dims_aicpu
from .range_v2 import _range_v2_aicpu
from .randperm import _randperm_aicpu
from .random_poisson import _random_poisson_aicpu
from .random_choice_with_mask import _random_choice_with_mask_aicpu
from .rsqrt import _rsqrt_aicpu
from .rsqrt_grad import _rsqrt_grad_aicpu
from .search_sorted import _search_sorted_aicpu
from .stack import _stack_aicpu
from .unstack import _unstack_aicpu
from .addcmul import _addcmul_aicpu
from .sample_distorted_bounding_box_v2 import _sample_distorted_bounding_box_v2_aicpu
from .sparse_apply_centered_rms_prop import _sparse_apply_centered_rms_prop_aicpu
from .uniform_candidate_sampler import _uniform_candidate_sampler_aicpu
from .log_uniform_candidate_sampler import _log_uniform_candidate_sampler_aicpu
from .compute_accidental_hits import _compute_accidental_hits_aicpu
from .ctcloss import _ctcloss_aicpu
from .multi_margin_loss import _multi_margin_loss_aicpu
from .multi_margin_loss_grad import _multi_margin_loss_grad_aicpu
from .reverse_sequence import _reverse_sequence_aicpu
from .matrix_inverse import _matrix_inverse_aicpu
from .matrix_determinant import _matrix_determinant_aicpu
from .log_matrix_determinant import _log_matrix_determinant_aicpu
from .sparse_tensor_dense_add import _sparse_tensor_dense_add_aicpu
from .lstsq import _lstsq_aicpu
from .crop_and_resize import _crop_and_resize_aicpu
from .crop_and_resize_grad_boxes import _crop_and_resize_grad_boxes_aicpu
from .acosh import _acosh_aicpu
from .acosh_grad import _acosh_grad_aicpu
from .rnnt_loss import _rnnt_loss_aicpu
from .random_categorical import _random_categorical_aicpu
from .tanh import _tanh_aicpu
from .tanh_grad import _tanh_grad_aicpu
from .mvlgamma import _mvlgamma_aicpu
from .mvlgamma_grad import _mvlgamma_grad_aicpu
from .cast import _cast_aicpu
from .coalesce import _coalesce_aicpu
from .list_diff import _list_diff_aicpu
from .mirror_pad import _mirror_pad_aicpu
from .select import _select_aicpu
from .add_v2 import _add_v2_aicpu
from .masked_select import _masked_select_aicpu
from .masked_select_grad import _masked_select_grad_aicpu
from .mirror_pad_grad import _mirror_pad_grad_aicpu
from .mul import _mul_aicpu
from .non_max_suppression_with_overlaps import _non_max_suppression_with_overlaps_aicpu
from .standard_normal import _standard_normal_aicpu
from .gamma import _gamma_aicpu
from .random_gamma import _random_gamma_aicpu
from .sub import _sub_aicpu
from .not_equal import _not_equal_aicpu
from .poisson import _poisson_aicpu
from .update_cache import _update_cache_aicpu
from .cache_swap_table import _cache_swap_table_aicpu
from .uniform_int import _uniform_int_aicpu
from .uniform_real import _uniform_real_aicpu
from .standard_laplace import _standard_laplace_aicpu
from .strided_slice import _strided_slice_aicpu
from .neg import _neg_aicpu
from .div_no_nan import _div_no_nan_aicpu
from .strided_slice_grad import _strided_slice_grad_aicpu
from .end_of_sequence import _end_of_sequence_aicpu
from .fused_sparse_adam import _fused_sparse_adam_aicpu
from .fused_sparse_lazy_adam import _fused_sparse_lazy_adam_aicpu
from .fused_sparse_ftrl import _fused_sparse_ftrl_aicpu
from .fused_sparse_proximal_adagrad import _fused_sparse_proximal_adagrad_aicpu
from .meshgrid import _meshgrid_aicpu
from .hypot import _hypot_aicpu
from .div import _div_aicpu
from .xdivy import _xdivy_aicpu
from .xlogy import _xlogy_aicpu
from .real import _real_aicpu
from .imag import _imag_aicpu
from .complex import _complex_aicpu
from .angle import _angle_aicpu
from .trans_data import _trans_data_aicpu
from .stack_push_pop import _stack_init_aicpu
from .stack_push_pop import _stack_push_aicpu
from .stack_push_pop import _stack_pop_aicpu
from .triplet_margin_loss import _triplet_margin_loss_aicpu
from .asinh import _asinh_aicpu
from .asinh_grad import _asinh_grad_aicpu
from .stack_push_pop import _stack_destroy_aicpu
from .matrix_diag_v3 import _matrix_diag_v3_aicpu
from .matrix_diag_part_v3 import _matrix_diag_part_v3_aicpu
from .matrix_set_diag_v3 import _matrix_set_diag_v3_aicpu
from .ragged_range import _raggedrange_aicpu
from .tan import _tan_aicpu
from .argmax_with_value import _argmax_with_value_aicpu
from .ctc_greedy_decoder import _ctc_greedy_decoder_aicpu
from .reduce_prod import _reduce_prod_aicpu
from .reduce_mean import _reduce_mean_aicpu
from .resize_bilinear import _resize_bilinear_aicpu
from .resize_bilinear_grad import _resize_bilinear_grad_aicpu
from .resize_nearest_neighbor_v2 import _resize_nearest_neighbor_v2_aicpu
from .resize_nearest_neighbor_v2_grad import _resize_nearest_neighbor_v2_grad_aicpu
from .scatter_elements import _scatter_elements_aicpu
from .multilabel_margin_loss_grad import _multilabel_margin_loss_grad_aicpu
from .non_max_suppression import _non_max_suppression_aicpu
from .square import _square_aicpu
from .squared_difference import _squared_difference_aicpu
from .lower_bound import _lower_bound_aicpu
from .non_zero import _non_zero_aicpu
from .upper_bound import _upper_bound_aicpu
from .zeros_like import _zeros_like_aicpu
from .ones_like import _ones_like_aicpu
from .concat import _concat_aicpu
from .relu_v3 import _relu_v3_aicpu
from .relu_grad_v3 import _relu_grad_v3_aicpu
from .grid_sampler_3d import _grid_sampler_3d_aicpu
from .atanh import _atanh_aicpu
from .grid_sampler_3d_grad import _grid_sampler_3d_grad_aicpu
from .environ_create import _environ_create_aicpu
from .environ_set import _environ_set_aicpu
from .environ_get import _environ_get_aicpu
from .environ_destroy_all import _environ_destroy_all_aicpu
from .cross import _cross_aicpu
from .check_numerics import _check_numerics_aicpu
from .affine_grid import _affine_grid_aicpu
from .cummax import _cummax_aicpu
from .lcm import _lcm_aicpu
from .round import _round_aicpu
from .gcd import _gcd_aicpu
from .truncated_normal import _truncated_normal_aicpu
from .stft import _stft_aicpu
from .resize_bicubic import _resize_bicubic_aicpu
from .resize_bicubic_grad import _resize_bicubic_grad_aicpu
from .floor_div import _floor_div_aicpu
from .non_deterministic_ints import _non_deterministic_ints_aicpu
from .one_hot import _one_hot_aicpu
from .mul_no_nan import _mul_no_nan_aicpu
from .adjust_contrastv2 import _adjust_contrastv2_aicpu
from .priority_replay_buffer import _prb_create_op_cpu
from .priority_replay_buffer import _prb_push_op_cpu
from .conjugate_transpose import _conjugate_transpose_aicpu
from .priority_replay_buffer import _prb_sample_op_cpu
from .priority_replay_buffer import _prb_update_op_cpu
from .equal import _equal_aicpu
from .complex_abs import _complex_abs_aicpu
from .priority_replay_buffer import _prb_destroy_op_cpu
from .right_shift import _right_shift_aicpu
from .parameterized_truncated_normal import _parameterized_truncated_normal_aicpu
from .tril import _tril_aicpu
from .bucketize import _bucketize_aicpu
from .eye import _eye_aicpu
from .logspace import _logspace_aicpu
from .triu import _triu_aicpu
from .dense_to_dense_set_operation import _dense_to_dense_set_operation_aicpu
from .fractional_max_pool3d_with_fixed_ksize import _fractional_max_pool3d_with_fixed_ksize_aicpu
from .fractional_max_pool3d_grad_with_fixed_ksize import _fractional_max_pool3d_grad_with_fixed_ksize_aicpu
from .nth_element import _nth_element_aicpu
from .transpose import _transpose_aicpu
from .trace import _trace_aicpu
from .tracegrad import _tracegrad_aicpu
from .zeta import _zeta_aicpu
from .adjust_hue import _adjust_hue_aicpu
from .avgpool_v1 import _avgpool_v1_aicpu
from .avgpool_grad_v1 import _avgpool_grad_v1_aicpu
from .maxpool_v1 import _maxpool_v1_aicpu
from .maxpool_grad_v1 import _maxpool_grad_v1_aicpu
from .max_unpool2d import _max_unpool2d_aicpu
from .max_unpool2d_grad import _max_unpool2d_grad_aicpu
from .max_unpool3d import _max_unpool3d_aicpu
from .max_unpool3d_grad import _max_unpool3d_grad_aicpu
from .dense_to_csr_sparse_matrix import _dense_to_csr_sparse_matrix_aicpu
from .adjust_saturation import _adjust_saturation_aicpu
from .grid_sampler_2d import _grid_sampler_2d_aicpu
from .grid_sampler_2d_grad import _grid_sampler_2d_grad_aicpu
from .segment_max import _segment_max_aicpu
from .segment_mean import _segment_mean_aicpu
from .segment_min import _segment_min_aicpu
from .segment_prod import _segment_prod_aicpu
from .segment_sum import _segment_sum_aicpu
from .scatter_nd_max import _scatter_nd_max_aicpu
from .conj import _conj_aicpu
from .ctc_loss_v2 import _ctc_loss_v2_aicpu
from .ctc_loss_v2_grad import _ctc_loss_v2_grad_aicpu
from .scatter_nd_min import _scatter_nd_min_aicpu
from .cholesky import _cholesky_aicpu
from .sspaddmm import _sspaddmm_aicpu
from .cholesky_solve import _cholesky_solve_aicpu
from .resize_area import _resize_area_aicpu
from .sparse_apply_adagrad_da import _sparse_apply_adagrad_da_aicpu
from .addcdiv import _addcdiv_aicpu
from . left_shift import _left_shift_aicpu
from .unique_consecutive import _unique_consecutive_aicpu
from .sparse_tensor_dense_mat_mul import _sparse_tensor_dense_mat_mul_aicpu
from .sparse_matrix_nnz import _sparse_matrix_nnz_aicpu
from .multinomial import _multinomial_aicpu
from .pow import _pow_aicpu
from .depth_to_space import _depth_to_space_aicpu
from .space_to_depth import _space_to_depth_aicpu
from .csr_sparse_matrix_to_dense import _csr_sparse_matrix_to_dense_aicpu
from .sparse_matrix_transpose import _sparse_matrix_transpose_aicpu
from .sparse_tensor_to_csr_sparse_matrix import _sparse_tensor_to_csr_sparse_matrix_aicpu
from .csr_sparse_matrix_to_sparse_tensor import _csr_sparse_matrix_to_sparse_tensor_aicpu
from .split import _split_aicpu
from .sparse_apply_proximal_gradient_descent import _sparse_apply_proximal_gradient_descent_aicpu
from .sparse_apply_momentum import _sparse_apply_momentum_aicpu
from .linear_sum_assignment import _linear_sum_assignment_aicpu
