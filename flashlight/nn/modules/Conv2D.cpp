/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <stdexcept>

#include "flashlight/nn/modules/Conv2D.h"

#include "flashlight/autograd/Functions.h"
#include "flashlight/nn/Init.h"
#include "flashlight/nn/Utils.h"

namespace fl {

using detail::IntOrPadMode;

Conv2D::Conv2D(
    int nin,
    int nout,
    int wx,
    int wy,
    int sx,
    int sy,
    IntOrPadMode px,
    IntOrPadMode py,
    bool bias,
    int groups)
    : nIn_(nin),
      nOut_(nout),
      xFilter_(wx),
      yFilter_(wy),
      xStride_(sx),
      yStride_(sy),
      xPad_(px.padVal),
      yPad_(py.padVal),
      bias_(bias),
      groups_(groups) {
  initialize();
}

Conv2D::Conv2D(
    const Variable& w,
    int sx,
    int sy,
    IntOrPadMode px,
    IntOrPadMode py,
    int groups)
    : UnaryModule({w}),
      nIn_(w.dims(2)),
      nOut_(w.dims(3)),
      xFilter_(w.dims(0)),
      yFilter_(w.dims(1)),
      xStride_(sx),
      yStride_(sy),
      xPad_(px.padVal),
      yPad_(py.padVal),
      bias_(false),
      groups_(groups) {}

Conv2D::Conv2D(
    const Variable& w,
    const Variable& b,
    int sx,
    int sy,
    IntOrPadMode px,
    IntOrPadMode py,
    int groups)
    : UnaryModule({w, b}),
      nIn_(w.dims(2)),
      nOut_(w.dims(3)),
      xFilter_(w.dims(0)),
      yFilter_(w.dims(1)),
      xStride_(sx),
      yStride_(sy),
      xPad_(px.padVal),
      yPad_(py.padVal),
      bias_(true),
      groups_(groups) {
  if (b.dims(2) != w.dims(3)) {
    throw std::invalid_argument(
        "output channel dimension mismatch between Conv2D weight and bias");
  }
  if (b.elements() != b.dims(2)) {
    throw std::invalid_argument(
        "only 3rd dimension of Conv2D bias may be non-singleton");
  }
}

Variable Conv2D::forward(const Variable& input) {
  auto px = detail::derivePadding(input.dims(0), xFilter_, xStride_, xPad_);
  auto py = detail::derivePadding(input.dims(1), yFilter_, yStride_, yPad_);
  if (!(px >= 0 && py >= 0)) {
    throw std::invalid_argument("invalid padding for Conv2D");
  }
  if (bias_) {
    return conv2d(
        input, params_[0], params_[1], xStride_, yStride_, px, py, groups_);
  } else {
    return conv2d(input, params_[0], xStride_, yStride_, px, py, groups_);
  }
}

void Conv2D::initialize() {
  auto wt = kaimingUniform(af::dim4(xFilter_, yFilter_, nIn_ / groups_, nOut_));
  if (bias_) {
    int fan_in = detail::computeFans(wt.dims()).first;
    double bound = std::sqrt(1.0 / fan_in);
    auto bs = uniform(af::dim4(1, 1, nOut_, 1), -bound, bound);
    params_ = {wt, bs};
  } else {
    params_ = {wt};
  }
}

std::string Conv2D::prettyString() const {
  std::ostringstream ss;
  ss << "Conv2D";
  ss << " (" << nIn_ << "->" << nOut_ << ", " << xFilter_ << "x" << yFilter_
     << ", " << xStride_ << "," << yStride_ << ", ";
  if (xPad_ == static_cast<int>(PaddingMode::SAME)) {
    ss << "SAME";
  } else {
    ss << xPad_;
  }
  ss << ",";
  if (yPad_ == static_cast<int>(PaddingMode::SAME)) {
    ss << "SAME";
  } else {
    ss << yPad_;
  }
  ss << ")";
  if (bias_) {
    ss << " (with bias)";
  } else {
    ss << " (without bias)";
  }
  return ss.str();
}

} // namespace fl
