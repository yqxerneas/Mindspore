# Copyright 2022 Huawei Technologies Co., Ltd
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
# ============================================================================
"""pynative shard"""

import mindspore as ms
from mindspore import log as logger
from mindspore._c_expression import Shard_


class Shard(Shard_):
    """Shard operation"""

    def __init__(self):
        """Initialize Shard."""
        Shard_.__init__(self, 'Shard')
        self.shard_fn = None
        self.fn = None
        self.in_strategy = None
        self.out_strategy = None
        self.parameter_plan = None
        self.device = None
        self.level = None

    def __call__(self, fn, in_strategy, out_strategy=None, parameter_plan=None, device="Ascend", level=0):
        if ms.context.get_context("mode") != ms.context.PYNATIVE_MODE or \
                ms.context.get_auto_parallel_context("parallel_mode") not in ["auto_parallel"]:
            raise AssertionError(f"'Shard' only supports auto parallel under PyNative mode")
        if ms.context.get_context("device_target") not in ["Ascend", "GPU"]:
            raise AssertionError(f"'Shard' now only supports 'Ascend' and 'GPU'")
        if ms.context.get_auto_parallel_context("search_mode") != "sharding_propagation":
            raise AssertionError(f"'search_mode' must be 'sharding_propagation' for 'Shard'")
        if not isinstance(in_strategy, tuple):
            raise TypeError(f"For 'Shard', the 'in_strategy' should be a tuple, but got {type(in_strategy).__name__}")
        if not isinstance(out_strategy, (type(None), tuple)):
            raise TypeError(f"For 'Shard', the 'out_strategy' should be None or tuple, "
                            f"but got {type(out_strategy).__name__}")
        if not isinstance(parameter_plan, (dict, type(None))):
            raise TypeError(f"For 'Shard', the 'parameter_plan' should be a dict or None, "
                            f"but got {type(parameter_plan).__name__}")
        if isinstance(parameter_plan, dict):
            for k in parameter_plan.keys():
                v = parameter_plan[k]
                if not isinstance(k, str) or not isinstance(v, tuple):
                    raise TypeError(f"For 'Shard', the type of each key and value in 'parameter_plan' must be str and "
                                    f"tuple, but got {type(k).__name__} and {type(parameter_plan[v]).__name__}")
        parameter_plan = self._parameter_plan_dict2tuple(parameter_plan)

        if not isinstance(device, str):
            raise TypeError(f"For 'Shard', the 'device' should be a string, "
                            f"but got {type(device).__name__}")
        if not isinstance(level, int):
            raise TypeError(f"For 'Shard', the 'level' should be an integer, "
                            f"but got {type(level).__name__}")

        if ms.get_algo_parameters("fully_use_devices") is True:
            logger.warning("After calling 'shard', the environment variable 'fully_use_devices' "
                           "will be overwritten as False.")
            ms.set_algo_parameters(fully_use_devices=False)

        if ms.context.get_auto_parallel_context("full_batch_is_set") is False:
            logger.warning("When calling the shard interface, "
                           "'dataset_strategy' or 'full_batch' is not manually set by the user, "
                           "and the 'dataset_strategy' will be set to 'full_batch'.")
            ms.context.set_auto_parallel_context(dataset_strategy="full_batch")

        if self._is_attrs_has_been_set(fn, in_strategy, out_strategy, parameter_plan, device, level):
            return self.shard_fn
        shard_ = Shard()

        if isinstance(fn, ms.nn.Cell):
            for param in fn.trainable_params():
                param.is_in_shard = True

        def shard_fn(*args):
            args = (fn,) + args

            @ms.common.ms_function(hash_args=fn)
            def after_shard(*args):
                return shard_(fn, in_strategy, out_strategy, parameter_plan, device, level)(*args)

            return after_shard(*args)

        self.shard_fn = shard_fn
        self.fn = fn
        self.in_strategy = in_strategy
        self.out_strategy = out_strategy
        self.parameter_plan = parameter_plan
        self.device = device
        self.level = level
        return self.shard_fn

    @staticmethod
    def _parameter_plan_dict2tuple(parameter_plan):
        if not isinstance(parameter_plan, dict):
            return parameter_plan

        parameter_plan_tuple = ()
        for k in parameter_plan:
            parameter_plan_tuple += ((k, parameter_plan[k]),)
        return parameter_plan_tuple

    def _is_attrs_has_been_set(self, fn, in_strategy, out_strategy, parameter_plan, device, level):
        return self.shard_fn is not None and self.fn == fn and self.in_strategy == in_strategy and \
               self.out_strategy == out_strategy and self.parameter_plan == parameter_plan and \
               self.device == device and self.level == level


def shard(fn, in_strategy, out_strategy=None, parameter_plan=None, device="Ascend", level=0):
    """
    Defining the input and output layouts of this cell and the parallel strategies of remaining ops will be
    generated by sharding propagation. In PyNative mode, use this method
    to specify a Cell for distributed execution in graph mode.
    in_strategy and out_strategy define the input and output layout respectively.
    in_strategy/out_strategy should be a tuple, each element of which corresponds to the desired layout of
    this input/output, and None represents data_parallel,
    which can refer to the description of `mindspore.ops.Primitive.shard`.
    The parallel strategies of remaining operators are derived from the strategy specified by the input and output.

    Note:
        You need to set the execution mode to PyNative mode,
        set the parallel mode in `set_auto_parallel_context` to "auto_parallel"
        and the search mode to "sharding_propagation".

    Args:
        fn (Union[Cell, Function]): Function to be executed in parallel.
                                    Its arguments and return value must be Tensor.
                                    If fn is a Cell with parameters, fn needs to be an instantiated object,
                                    otherwise its arguments cannot be accessed.
        in_strategy (tuple): Define the layout of inputs, each element of the tuple should be a tuple or None.
                             Tuple defines the layout of the corresponding input
                             and None represents a data parallel strategy.
        out_strategy (Union[None, tuple]): Define the layout of outputs similar with in_strategy.
                                           It is not in use right now. Default: None.
        parameter_plan (Union[dict, None]): Define the layout for the specified parameters. Each element in dict
                                            defines the layout of the parameter like "param_name: layout".
                                            The key is a parameter name of type 'str'.
                                            The value is a 1-D integer tuple, indicating the corresponding layout.
                                            If the parameter name is incorrect or the corresponding parameter
                                            has been set, the parameter setting will be ignored.
                                            Default: None.
        device (string): Select a certain device target. It is not in use right now.
                         Support ["CPU", "GPU", "Ascend"]. Default: "Ascend".
        level (int): Option for parallel strategy infer algorithm, namely the object function, maximize computation
                     over communication ratio, maximize speed performance, minimize memory usage etc. It is not in
                     use right now. Support ["0", "1", "2"]. Default: "0".

    Returns:
        Function, return the function that will be executed under auto parallel process.

    Raises:
        AssertionError:
            - If execute mode is not PYNATIVE_MODE.
            - If parallel mode is not "auto_parallel".
            - If search_mode it not "sharding_propagation".
            - If device_target it not "Ascend" or "GPU".

        TypeError:
            - If `in_strategy` is not a tuple.
            - If `out_strategy` is not a tuple or None.
            - If `parameter_plan` is not a dict or None.
            - If any key in `parameter_plan` is not a str.
            - If any value in `parameter_plan` is not a tuple.
            - If `device` is not a str.
            - If `level` is not a integer.

    Supported Platforms:
        ``Ascend`` ``GPU``

    Examples:
        >>> import numpy as np
        >>> from mindspore import Tensor, set_context, set_auto_parallel_context, shard, PYNATIVE_MODE
        >>> from mindspore.communication import init
        >>> set_context(mode=PYNATIVE_MODE)
        >>> init()
        >>> set_auto_parallel_context(parallel_mode="auto_parallel", search_mode="sharding_propagation",
        ...                           device_num=2)
        >>> def test_shard(x, y):
        ...     return x + y
        >>> x = Tensor(np.ones(shape=(32, 10)))
        >>> y = Tensor(np.ones(shape=(32, 10)))
        >>> output = shard(test_shard, in_strategy=((2, 1), (2, 1)))(x, y)
        >>> print(output.shape)
        (32, 10)
    """
    if not isinstance(fn, (ms.nn.Cell)):
        logger.warning("'fn' is not a mindspore.nn.Cell, and its definition cannot involve Parameter; "
                       "otherwise, the result may be incorrect.")
    return Shard()(fn, in_strategy, out_strategy, parameter_plan, device, level)
