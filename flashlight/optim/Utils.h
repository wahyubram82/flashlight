/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <vector>

#include "flashlight/autograd/Variable.h"

namespace fl {

double clipGradNorm(const std::vector<Variable>& parameters, double max_norm);

}
