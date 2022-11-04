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

#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <memory>

#include "frontend/parallel/pynative_shard/pynative_shard.h"
#include "frontend/parallel/graph_util/graph_info.h"
#include "frontend/parallel/step_parallel_utils.h"
#include "include/common/utils/parallel_context.h"
#include "frontend/parallel/step_parallel.h"
#include "utils/ms_context.h"
#include "include/common/utils/comm_manager.h"
#include "frontend/parallel/ops_info/ops_utils.h"

namespace mindspore {
namespace parallel {
static void GenerateDefaultStrategy(const ValueNodePtr &axes, const std::vector<AnfNodePtr> &nodes,
                                    std::vector<std::vector<int64_t>> *default_strategy) {
  auto strategies = axes->value()->cast<ValueTuplePtr>()->value();
  size_t i = 0;
  for (auto &strategy : strategies) {
    auto node = nodes[i];
    if (strategy->isa<None>()) {
      (void)default_strategy->emplace_back(Shape());
    } else {
      (void)default_strategy->emplace_back(GetValue<Shape>(strategy));
    }
    i += 1;
  }
}

// Generate strategies like ((), (), ..., ())
Shapes GenerateEmptyStrategies(const CNodePtr &cnode) {
  auto shape_list = ExtractShape(cnode);
  if (shape_list.empty()) {
    MS_LOG(EXCEPTION) << "Node: " << cnode->DebugString() << " failed to extract shape.";
  }
  return Shapes(shape_list[0].size(), Shape());
}

static bool CheckOneDimensionalIntTuple(const ValuePtr &value_ptr) {
  if (!value_ptr->isa<ValueTuple>()) {
    return false;
  }
  auto elements = value_ptr->cast<ValueTuplePtr>()->value();
  for (auto &element : elements) {
    if (!element->isa<Int64Imm>()) {
      return false;
    }
  }
  return true;
}

static bool CheckLayout(const ValueNodePtr &axes, bool *need_default_strategy, size_t *axes_size) {
  auto strategies = axes->value()->cast<ValueTuplePtr>()->value();
  for (auto &strategy : strategies) {
    *axes_size += 1;
    if (strategy->isa<None>()) {
      *need_default_strategy = true;
      continue;
    }
    if (!CheckOneDimensionalIntTuple(strategy)) {
      return false;
    }
  }
  return true;
}

static Shapes GenerateFullStrategy(const Shapes &current_strategy, const CNodePtr &cnode) {
  OperatorInfoPtr op_info = CreateOperatorInfo(cnode);
  MS_EXCEPTION_IF_NULL(op_info);
  return op_info->GenerateFullStrategy(current_strategy);
}

static void GetInputNodes(const FuncGraphPtr &func_graph, std::vector<AnfNodePtr> *input_nodes) {
  auto parameters = func_graph->parameters();
  for (auto &parameter : parameters) {
    if (parameter->cast<ParameterPtr>()->name() == "u" || parameter->cast<ParameterPtr>()->name() == "io") {
      continue;
    }
    input_nodes->push_back(parameter);
  }
}

static bool CheckDeviceNum(const std::vector<std::vector<int64_t>> &strategies, const int64_t &device_num) {
  for (size_t i = 0; i < strategies.size(); ++i) {
    auto strategy = strategies[i];
    int64_t required_num = 1;
    (void)std::for_each(strategy.begin(), strategy.end(),
                        [&required_num](const int64_t data) { required_num *= data; });
    if (required_num > device_num) {
      MS_LOG(ERROR) << "required device number: " << required_num
                    << " is larger than available device number: " << device_num << " at index: " << i;
      return false;
    }
    if (device_num % required_num != 0) {
      MS_LOG(ERROR) << "required device number: " << required_num
                    << " is not divisible by device number: " << device_num << " at index: " << i;
      return false;
    }
  }
  return true;
}

// Generate strategy for cnode by input_strategy.
// For the i-th input:
// 1. If it is specified in input_strategy, the strategy in input_strategy is used;
// 2. Otherwise, its strategy is assigned as ()
static Shapes GenerateDefaultStrategiesForCNode(const CNodePtr &cnode, const Shapes &input_strategy) {
  auto current_inputs = cnode->inputs();
  Shapes elements;
  for (size_t i = 1; i < current_inputs.size(); ++i) {
    auto current_input = current_inputs[i];
    if (current_input->isa<ValueNode>()) {
      auto current_value = current_input->cast<ValueNodePtr>()->value();
      if (!current_value->isa<mindspore::tensor::Tensor>()) {
        continue;
      }
    }
    if (IsPrimitiveCNode(current_input, prim::kPrimTupleGetItem)) {
      auto tuple_getitem_cnode = current_input->cast<CNodePtr>();
      auto tuple_index = tuple_getitem_cnode->input(2);
      auto value_node = tuple_index->cast<ValueNodePtr>();
      auto index = GetValue<int64_t>(value_node->value());
      elements.push_back(input_strategy[index]);
    } else {
      (void)elements.emplace_back(Shape());
    }
  }
  return elements;
}

static ValueTuplePtr ShapesToValueTuplePtr(const Shapes &shapes) {
  std::vector<ValuePtr> value_list;
  (void)std::transform(shapes.begin(), shapes.end(), std::back_inserter(value_list),
                       [](const Shape &shape) { return MakeValue(shape); });
  return std::make_shared<ValueTuple>(value_list);
}

static Shapes ValueTuplePtrToShapes(const ValueTuplePtr &value_tuple_ptr) {
  Shapes shapes;
  auto value_list = value_tuple_ptr->value();
  (void)std::transform(value_list.begin(), value_list.end(), std::back_inserter(shapes),
                       [](const ValuePtr &value_ptr) { return GetValue<Shape>(value_ptr); });
  return shapes;
}

AnfNodeIndexSet FindAnfNodeIndexSetToInsertStrategy(const FuncGraphPtr &func_graph, const AnfNodeIndexSet &node_users) {
  FuncGraphManagerPtr manager = func_graph->manager();
  AnfNodeIndexSet ret_set;
  std::queue<std::pair<AnfNodePtr, int>> bfs_list;
  (void)std::for_each(node_users.begin(), node_users.end(),
                      [&bfs_list](const std::pair<AnfNodePtr, int> &user) { bfs_list.push(user); });

  while (!bfs_list.empty()) {
    auto user = bfs_list.front();
    bfs_list.pop();
    CNodePtr cnode = user.first->cast<CNodePtr>();
    // If the cnode is not a splittable operator, apply strategy to the next cnode
    if (!IsSplittableOperator(GetPrimName(cnode)) || IsPrimitiveCNode(cnode, prim::kPrimVirtualDataset) ||
        IsPrimitiveCNode(cnode, prim::kPrimCast) || IsPrimitiveCNode(cnode, prim::kPrimReshape)) {
      auto tmp_users = manager->node_users()[cnode];
      (void)std::for_each(tmp_users.begin(), tmp_users.end(),
                          [&bfs_list](const std::pair<AnfNodePtr, int> &user) { bfs_list.push(user); });
      continue;
    }
    ret_set.insert(user);
  }
  return ret_set;
}

// New a primitive for cnode and set in_strategy to it.
void SetStrategyToCNode(const CNodePtr &cnode, const Shapes &strategies) {
  auto strategy = ShapesToValueTuplePtr(strategies);
  PrimitivePtr prim = GetCNodePrimitive(cnode);
  MS_EXCEPTION_IF_NULL(prim);
  PrimitivePtr new_prim;
  if (prim->isa<PrimitivePy>()) {
    PrimitivePyPtr prim_py = prim->cast<PrimitivePyPtr>();
    MS_EXCEPTION_IF_NULL(prim_py);
    new_prim = std::make_shared<PrimitivePy>(*prim_py);
  } else {
    new_prim = std::make_shared<Primitive>(*prim);
  }
  auto attrs_temp = prim->attrs();
  attrs_temp[parallel::IN_STRATEGY] = strategy;
  (void)new_prim->SetAttrs(attrs_temp);

  ValuePtr new_prim_value = MakeValue(new_prim);
  ValueNodePtr new_prim_value_node = NewValueNode(new_prim_value);
  AnfNodePtr new_prim_anf_node = new_prim_value_node->cast<AnfNodePtr>();
  MS_EXCEPTION_IF_NULL(new_prim_anf_node);
  cnode->set_input(0, new_prim_anf_node);
}

static std::set<CNodePtr> SetInputLayout(const FuncGraphPtr &func_graph, const AnfNodePtr &in_strategy,
                                         const int64_t &device_num) {
  auto in_strategy_tuple = in_strategy->cast<ValueNodePtr>();
  bool need_default_strategy = false;
  size_t in_strategy_size = 0;
  if (!IsValueNode<ValueTuple>(in_strategy_tuple) ||
      !CheckLayout(in_strategy_tuple, &need_default_strategy, &in_strategy_size)) {
    MS_LOG(EXCEPTION) << "in_strategy should be a two-dimension tuple";
  }
  std::vector<AnfNodePtr> input_nodes;
  GetInputNodes(func_graph, &input_nodes);
  if (input_nodes.size() != in_strategy_size) {
    MS_LOG(EXCEPTION) << "Input numbers: " << input_nodes.size()
                      << " is not equal to in_strategy numbers: " << in_strategy_size;
  }
  std::vector<std::vector<int64_t>> input_strategy;
  if (need_default_strategy) {
    GenerateDefaultStrategy(in_strategy_tuple, input_nodes, &input_strategy);
  } else {
    input_strategy = GetValue<std::vector<std::vector<int64_t>>>(in_strategy_tuple->value());
  }
  if (!CheckDeviceNum(input_strategy, device_num)) {
    MS_LOG(EXCEPTION) << "check device number failed";
  }
  std::set<CNodePtr> concerned_nodes;
  FuncGraphManagerPtr manager = func_graph->manager();
  auto parameters = func_graph->parameters();
  for (size_t i = 0; i < parameters.size(); ++i) {
    auto parameter = parameters[i];
    if (parameter->cast<ParameterPtr>()->name() == "u" || parameter->cast<ParameterPtr>()->name() == "io") {
      continue;
    }
    // Verify that the user has set the valid layout, if the layout is generated by 'GenareteDefaultStrategy', ignored
    // its check.
    auto output_shape = common::AnfAlgo::GetOutputInferShape(parameter, 0);
    if (!input_strategy[i].empty() && output_shape.size() != input_strategy[i].size()) {
      MS_LOG(EXCEPTION) << "Input dimension: " << output_shape.size()
                        << " is not equal to in_strategy dimension: " << input_strategy[i].size() << " at index " << i;
    }
    AnfNodeIndexSet param_sub_set = manager->node_users()[parameter];
    auto to_insert_nodes_set = FindAnfNodeIndexSetToInsertStrategy(func_graph, param_sub_set);
    for (auto &node : to_insert_nodes_set) {
      CNodePtr param_cnode = node.first->cast<CNodePtr>();
      auto param_attrs = GetCNodePrimitive(param_cnode)->attrs();
      if (StrategyFound(param_attrs)) {
        auto origin_strategies = ValueTuplePtrToShapes(param_attrs[parallel::IN_STRATEGY]->cast<ValueTuplePtr>());
        MS_LOG(WARNING) << "For " << param_cnode->fullname_with_scope() << ", its in_strategy has been set to "
                        << origin_strategies << ", the relevant settings in input_strategy will be ignored";
        continue;
      }
      (void)concerned_nodes.insert(param_cnode);
    }
  }

  for (auto &cnode : concerned_nodes) {
    Shapes ret_strategy = GenerateDefaultStrategiesForCNode(cnode, input_strategy);
    SetStrategyToCNode(cnode, ret_strategy);
  }
  return concerned_nodes;
}

static std::set<CNodePtr> SetParameterLayout(const FuncGraphPtr &root, const FuncGraphPtr &func_graph,
                                             const std::set<CNodePtr> &input_concerned_node) {
  FuncGraphManagerPtr manager = func_graph->manager();
  auto root_parameters = root->parameters();
  std::set<CNodePtr> concerned_cnode;
  for (auto param : root_parameters) {
    auto parameter = param->cast<ParameterPtr>();
    auto param_info = parameter->param_info();
    if (param_info == nullptr || param_info->param_strategy().empty()) {
      // Do not set param_strategy, skip it.
      continue;
    }
    auto param_strategy = parameter->param_info()->param_strategy();
    auto param_name = parameter->param_info()->name();
    AnfNodeIndexSet users = manager->node_users()[parameter];
    auto to_insert_nodes_set = FindAnfNodeIndexSetToInsertStrategy(func_graph, users);
    for (auto user : to_insert_nodes_set) {
      CNodePtr target_cnode = user.first->cast<CNodePtr>();
      Shapes current_strategies;
      if (input_concerned_node.find(target_cnode) == input_concerned_node.end()) {
        // If target_cnode is not involve inputs, insert an identity between Load and target_cnode,
        // and setting layout into identity.
        // e.g Load(param) -> identity{in_strategy} -> target_cnode
        auto pre_cnode = target_cnode->input(user.second)->cast<CNodePtr>();
        MS_EXCEPTION_IF_NULL(pre_cnode);
        if (IsPrimitiveCNode(pre_cnode, prim::kPrimCast)) {
          pre_cnode = pre_cnode->inputs().at(kIndex1)->cast<CNodePtr>();
        }
        if (!IsPrimitiveCNode(pre_cnode, prim::kPrimLoad)) {
          MS_LOG(EXCEPTION) << "The operator type of the " << user.second << "-th input in "
                            << target_cnode->fullname_with_scope() << " must be 'Load', but got "
                            << GetCNodePrimitive(pre_cnode)->ToString();
        }
        auto identity_cnode = func_graph->NewCNode({NewValueNode(prim::kPrimIdentity), pre_cnode});
        auto pre_cnode_abstract = pre_cnode->abstract();
        MS_EXCEPTION_IF_NULL(pre_cnode_abstract);
        identity_cnode->set_abstract(pre_cnode_abstract->Clone());
        manager->Replace(pre_cnode, identity_cnode);
        target_cnode = identity_cnode;
        current_strategies = {param_strategy};
      } else {
        // Setting layout into target_cnode directly.
        PrimitivePtr prim = GetCNodePrimitive(target_cnode);
        MS_EXCEPTION_IF_NULL(prim);
        auto attrs = prim->attrs();
        if (StrategyFound(attrs)) {
          current_strategies = ValueTuplePtrToShapes(attrs[parallel::IN_STRATEGY]->cast<ValueTuplePtr>());
        } else {
          current_strategies = GenerateEmptyStrategies(target_cnode);
        }
        current_strategies[user.second - 1] = param_strategy;
        (void)concerned_cnode.insert(target_cnode);
      }
      SetStrategyToCNode(target_cnode, current_strategies);
      MS_LOG(DEBUG) << "The layout of \"" << param_name << "\" has been set to the " << user.second << "th of "
                    << target_cnode->fullname_with_scope() << "'s in_strategy. Current strategies is "
                    << current_strategies;
    }
  }
  return concerned_cnode;
}

void CompleteConcernedCNodeStrategies(std::set<CNodePtr> concerned_cnode) {
  for (auto cnode : concerned_cnode) {
    PrimitivePtr prim = GetCNodePrimitive(cnode);
    MS_EXCEPTION_IF_NULL(prim);
    auto attrs = prim->attrs();
    Shapes current_strategies = ValueTuplePtrToShapes(attrs[parallel::IN_STRATEGY]->cast<ValueTuplePtr>());
    Shapes full_strategies = GenerateFullStrategy(current_strategies, cnode);
    attrs[parallel::IN_STRATEGY] = ShapesToValueTuplePtr(full_strategies);
    (void)prim->SetAttrs(attrs);
    MS_LOG(INFO) << cnode->fullname_with_scope() << ": Completion strategies success. " << current_strategies << " -> "
                 << full_strategies << "(origin_strategies -> completion_strategies)";
  }
}

static bool SetStrategyForShard(const FuncGraphPtr &root, const std::vector<AnfNodePtr> &all_nodes,
                                const int64_t &device_num) {
  constexpr size_t kShardFnIndex = 1;
  constexpr size_t kShardInStrategyIndex = 2;
  for (auto &node : all_nodes) {
    if (IsPrimitiveCNode(node, prim::kPrimShard)) {
      auto cnode = node->cast<CNodePtr>();
      auto vnode = cnode->input(kShardFnIndex)->cast<ValueNodePtr>();
      auto in_strategy = cnode->input(kShardInStrategyIndex);
      ScopeGuard scope_guard(vnode->scope());
      auto func_graph = GetValueNode<FuncGraphPtr>(vnode);
      MS_EXCEPTION_IF_NULL(func_graph);
      if (IsEmbedShardNode(func_graph)) {
        MS_LOG(EXCEPTION) << "Nested use of shard (e.g shard(shard(...), ...) is not supported currently."
                          << " | FuncGraph: " << func_graph->ToString();
      }
      if (HasNestedMetaFg(func_graph)) {
        return false;
      }
      std::set<CNodePtr> concerned_cnode;
      auto input_concerned_cnode = SetInputLayout(func_graph, in_strategy, device_num);
      auto parameter_concerned_cnode = SetParameterLayout(root, func_graph, input_concerned_cnode);
      (void)std::set_union(input_concerned_cnode.begin(), input_concerned_cnode.end(),
                           parameter_concerned_cnode.begin(), parameter_concerned_cnode.end(),
                           std::inserter(concerned_cnode, concerned_cnode.end()));
      CompleteConcernedCNodeStrategies(concerned_cnode);
      return true;
    }
  }
  return false;
}

bool PynativeShard(const FuncGraphPtr &root, const opt::OptimizerPtr &) {
  bool change = false;
  MS_EXCEPTION_IF_NULL(root);
  auto parallel_mode = ParallelContext::GetInstance()->parallel_mode();
  if (parallel_mode != kSemiAutoParallel && parallel_mode != kAutoParallel) {
    MS_LOG(INFO) << "Only auto_parallel and semi_auto_parallel support pynative shard";
    return change;
  }

  auto execution_mode = MsContext::GetInstance()->get_param<int>(MS_CTX_EXECUTION_MODE);
  if (execution_mode != kPynativeMode) {
    return change;
  }

  if (!ParallelContext::GetInstance()->device_num_is_set()) {
    MS_LOG(EXCEPTION) << "device_num must be set when use shard function";
  }

  if (ParallelInit() != SUCCESS) {
    MS_LOG(EXCEPTION) << "parallel init failed.";
  }

  AnfNodePtr ret = root->get_return();
  MS_EXCEPTION_IF_NULL(ret);
  std::vector<AnfNodePtr> all_nodes = DeepScopedGraphSearch(ret);
  auto device_num_shard = parallel::ParallelContext::GetInstance()->device_num();
  change = SetStrategyForShard(root, all_nodes, device_num_shard);
  MS_LOG(INFO) << "Leaving pynative shard";
  return change;
}
}  // namespace parallel
}  // namespace mindspore
