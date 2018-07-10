
#include "numpy.h"
#include <math.h>
#include <stdlib.h>
struct actions_grads {
  array actions;
  std::map<std::string, array> grads;
};
struct adam_value {
  array a;
  setting b;
};
#include "actor_net.h"
namespace actor_net {
/*
A Three-layer fully-connected neural network for actor network. The net has an
input dimension of (N, D), with D being the cardinality of the state space.
There are two hidden layers, with dimension of H1 and H2, respectively.  The
output provides an action vetcor of dimenson of A. The network uses a ReLU
nonlinearity for the first and second layer and uses tanh (scaled by a factor of
ACTION_BOUND) for the final layer. In summary, the network has the following
architecture:

input - fully connected layer - ReLU - fully connected layer - RelU- fully co-
nected layer - tanh*ACTION_BOUND
*/
ActorNet::ActorNet(int input_size, int hidden_size1, int hidden_size2, int output_size, float std_) {
  auto &self = *this;
  /*
   Initialize the model. Weights are initialized to small random values and
   biases are initialized to zero. Weights and biases are stored in the
   variable self.params, which is a dictionary with the following keys:

   W1: First layer weights; has shape (D, H1)
   b1: First layer biases; has shape (H1,)
   W2: Second layer weights; has shape (H1, H2)
   b2: Second layer biases; has shape (H2,)
   W3: Third layer weights, has shape (H2, A)
   b3: Third layer biases; has shape  (A,)

   We also have the weights for a traget network (same architecture but
   different weights)
   W1_tgt: First layer weights; has shape (D, H1)
   b1_tgt: First layer biases; has shape (H1,)
   W2_tgt: Second layer weights; has shape (H1, H2)
   b2_tgt: Second layer biases; has shape (H2,)
   W3_tgt: Third layer weights, has shape (H2, A)
   b3_tgt: Third layer biases; has shape  (A,)

   Inputs:
   - input_size: The dimension D of the input data.
   - hidden_size: The number of neurons H in the hidden layer.
   - output_size: The continuous variables that constitutes an action vector
     of A dimension.
   */
  printf("An actor network is created.\n");
  self.params = {};
  self.params["W1"] = self._uniform_init(input_size, hidden_size1);
  self.params["b1"] = np.zeros(hidden_size1);
  self.params["W2"] = self._uniform_init(hidden_size1, hidden_size2);
  self.params["b2"] = np.zeros(hidden_size2);
  self.params["W3"] = np.random.uniform(-3e-3, 3e-3, hidden_size2, output_size);
  //#self.params["b3"] = np.random.uniform(-3e-3, 3e-3, output_size)
  self.params["b3"] = np.zeros(output_size);
  /*# Initialization based on "Continuous control with deep reinformcement
  # learning"
#    self.params["W1_tgt"] = self.params["W1"]
#    self.params["b1_tgt"] = self.params["b1"]
#    self.params["W2_tgt"] = self.params["W2"]
#    self.params["b2_tgt"] = self.params["b2"]
#    self.params["W3_tgt"] = self.params["W3"]
#    self.params["b3_tgt"] = self.params["b3"]
  */
  self.params["W1_tgt"] = self._uniform_init(input_size, hidden_size1);
  self.params["b1_tgt"] = np.zeros(hidden_size1);
  self.params["W2_tgt"] = self._uniform_init(hidden_size1, hidden_size2);
  self.params["b2_tgt"] = np.zeros(hidden_size2);
  self.params["W3_tgt"] = np.random.uniform(-3e-3, 3e-3, hidden_size2, output_size);
  self.params["b3_tgt"] = np.zeros(output_size);
#define None setting()
  self.optm_cfg = {};
  self.optm_cfg["W1"] = None;
  self.optm_cfg["b1"] = None;
  self.optm_cfg["W2"] = None;
  self.optm_cfg["b2"] = None;
  self.optm_cfg["W3"] = None;
  self.optm_cfg["b3"] = None;
#undef None
}
actions_grads ActorNet::evaluate_gradient(array &X, const array &action_grads, const float &action_bound, bool target) {
  auto &self = *this;
  int axis = 0;
  /*
  Compute the action and gradients for the network based on the input X

  Inputs:
  - X: Input data of shape (N, D). Each X[i] is a training sample.
  - target: use default weights if False; otherwise use target weights.
  - action_grads: the gradient output from the critic-network.
  - action_bound: the scaling factor for the action, which is environment
                  dependent.

  Returns:
   A tuple of:
  - actions: a continuous vector
  - grads: Dictionary mapping parameter names to gradients of those parameters;
    has the same keys as self.params.
  */
  //# Unpack variables from the params dictionary
  array W1, b1, W2, b2, W3, b3;
  if (!target) {
    W1 = self.params["W1"], b1 = self.params["b1"];
    W2 = self.params["W2"], b2 = self.params["b2"];
    W3 = self.params["W3"], b3 = self.params["b3"];
  } else {
    W1 = self.params["W1_tgt"], b1 = self.params["b1_tgt"];
    W2 = self.params["W2_tgt"], b2 = self.params["b2_tgt"];
    W3 = self.params["W3_tgt"], b3 = self.params["b3_tgt"];
  }
  auto batch_size = X.sx;
  //# Compute the forward pass
  auto z1 = np.dot(X, W1) + b1;
  auto H1 = np.maximum(0, z1); //#Activation first layer
  auto z2 = np.dot(H1, W2) + b2;
  auto H2 = np.maximum(0, z2); //#Activation second layer
  auto scores = np.dot(H2, W3) + b3;
  auto actions = np.tanh(scores) * action_bound;
  //# Backward pass: compute gradients
  map grads = {};
  //# The derivatve at the output. Note that the derivative of tanh(x)
  //#is 1-tanh(x)**2.
  auto grad_output = action_bound * (1.0f - np.pow(np.tanh(scores), 2)) * (-action_grads);
  //# Back-propagate to second hidden layer
  auto out1 = np.dot(grad_output, np.T(W3));
  //# derivative of the max() gate
  eq(out1, z2 <= 0, 0);
  //# Backpropagate to the first hidden layer
  auto out2 = np.dot(out1, np.T(W2));
  //# derivtive of the max() gate again
  eq(out2, z1 <= 0, 0);
  //# Calculate gradient using back propagation
  grads["W3"] = np.dot(np.T(H2), grad_output) / batch_size;
  grads["W2"] = np.dot(np.T(H1), out1) / batch_size;
  grads["W1"] = np.dot(np.T(X), out2) / batch_size;
  grads["b3"] = np.sum(grad_output, axis = 0) / batch_size;
  grads["b2"] = np.sum(out1, axis = 0) / batch_size;
  grads["b1"] = np.sum(out2, axis = 0) / batch_size;
  return {actions, grads};
}
void ActorNet::train(array &X, const array &action_grads, const float &action_bound) {
  auto &self = *this;
  /*
  Train this neural network using adam optimizer.
  Inputs:
  - X: A numpy array of shape (N, D) giving training data.
  */
  //# Compute out and gradients using the current minibatch
  auto grads = self.evaluate_gradient(X, action_grads, action_bound).grads;
  //# Update the weights using adam optimizer
  setting config;
  self.params["W3"] = self._adam(self.params["W3"], grads["W3"], config = self.optm_cfg["W3"]).a;
  self.params["W2"] = self._adam(self.params["W2"], grads["W2"], config = self.optm_cfg["W2"]).a;
  self.params["W1"] = self._adam(self.params["W1"], grads["W1"], config = self.optm_cfg["W1"]).a;
  self.params["b3"] = self._adam(self.params["b3"], grads["b3"], config = self.optm_cfg["b3"]).a;
  self.params["b2"] = self._adam(self.params["b2"], grads["b2"], config = self.optm_cfg["b2"]).a;
  self.params["b1"] = self._adam(self.params["b1"], grads["b1"], config = self.optm_cfg["b1"]).a;
  //# Update the configuration parameters to be used in the next iteration
  self.optm_cfg["W3"] = self._adam(self.params["W3"], grads["W3"], config = self.optm_cfg["W3"]).b;
  self.optm_cfg["W2"] = self._adam(self.params["W2"], grads["W2"], config = self.optm_cfg["W2"]).b;
  self.optm_cfg["W1"] = self._adam(self.params["W1"], grads["W1"], config = self.optm_cfg["W1"]).b;
  self.optm_cfg["b3"] = self._adam(self.params["b3"], grads["b3"], config = self.optm_cfg["b3"]).b;
  self.optm_cfg["b2"] = self._adam(self.params["b2"], grads["b2"], config = self.optm_cfg["b2"]).b;
  self.optm_cfg["b1"] = self._adam(self.params["b1"], grads["b1"], config = self.optm_cfg["b1"]).b;
  // printf("%s: %d  \n", __FUNCTION__, __LINE__);
}
void ActorNet::train_target(float tau) {
  auto &self = *this;
  /*
    Update the weights of the target network.
   -tau: coefficent for tracking the learned network.
   */
  self.params["W3_tgt"] = tau * self.params["W3"] + (1 - tau) * self.params["W3_tgt"];
  self.params["W2_tgt"] = tau * self.params["W2"] + (1 - tau) * self.params["W2_tgt"];
  self.params["W1_tgt"] = tau * self.params["W1"] + (1 - tau) * self.params["W1_tgt"];
  self.params["b3_tgt"] = tau * self.params["b3"] + (1 - tau) * self.params["b3_tgt"];
  self.params["b2_tgt"] = tau * self.params["b2"] + (1 - tau) * self.params["b2_tgt"];
  self.params["b1_tgt"] = tau * self.params["b1"] + (1 - tau) * self.params["b1_tgt"];
}
array ActorNet::predict(const array &X, const float &action_bound, bool target) {
  auto &self = *this;
  /*
  Use the trained weights of this network to predict the action vector for a
  given state.

  Inputs:
  - X: A numpy array of shape (N, D)
  - target: if False, use normal weights, otherwise use learned weight.
  - action_bound: the scaling factor for the action, which is environment
                  dependent.

  Returns:
  - y_pred: A numpy array of shape (N,)

  */
  array W1, b1, W2, b2, W3, b3;
  if (!target) {
    W1 = self.params["W1"], b1 = self.params["b1"];
    W2 = self.params["W2"], b2 = self.params["b2"];
    W3 = self.params["W3"], b3 = self.params["b3"];
  } else {
    W1 = self.params["W1_tgt"], b1 = self.params["b1_tgt"];
    W2 = self.params["W2_tgt"], b2 = self.params["b2_tgt"];
    W3 = self.params["W3_tgt"], b3 = self.params["b3_tgt"];
  }
  auto H1 = np.maximum(0, np.dot(X, W1) + b1);
  auto H2 = np.maximum(0, np.dot(H1, W2) + b2);
  auto scores = np.dot(H2, W3) + b3;
  //#print "scores=:", scores
  auto y_pred = np.tanh(scores) * action_bound;
  return y_pred;
}
adam_value ActorNet::_adam(array &x, array &dx, setting &config) {
  auto &self = *this;
  /*
  Uses the Adam update rule, which incorporates moving averages of both the
  gradient and its square and a bias correction term.

  config format:
  - learning_rate: Scalar learning rate.
  - beta1: Decay rate for moving average of first moment of gradient.
  - beta2: Decay rate for moving average of second moment of gradient.
  - epsilon: Small scalar used for smoothing to avoid dividing by zero.
  - m: Moving average of gradient.
  - v: Moving average of squared gradient.
  - t: Iteration number (time step)
  */
  if (!config.def) {
    config.setdefault("learning_rate", 1e-4);
    config.setdefault("beta1", 0.9);
    config.setdefault("beta2", 0.999);
    config.setdefault("epsilon", 1e-8);
    config.setdefault("m", np.zeros_like(x));
    config.setdefault("v", np.zeros_like(x));
    config.setdefault("t", 0);
    config.def = true;
  }
  //#Adam update formula,                                                 #
  config["t"] += 1;
  config.m = config["beta1"] * config.m + (1 - config["beta1"]) * dx;
  config.v = config["beta2"] * config.v + (1 - config["beta2"]) * (dx * dx);
  auto mb = config.m / (1 - pow(config["beta1"], config["t"]));
  auto vb = config.v / (1 - pow(config["beta2"], config["t"]));
  auto next_x = x - config["learning_rate"] * mb / (np.sqrt(vb) + config["epsilon"]);
  return {next_x, config};
}
array ActorNet::_uniform_init(int input_size, int output_size) {
  auto u = np.sqrt(6. / (input_size + output_size));
  return np.random.uniform(-u, u, input_size, output_size);
}
#if 0
void ActorNet::debug() {
  for (auto &i : params) {
    print(i.first.c_str(), i.second);
  }
}
#endif
} // namespace actor_net
std::shared_ptr<actor_net::ActorNet> local;
void *makeNet() {
  local.reset(new actor_net::ActorNet(3, 60, 60, 3, 5.0));
  return local.get();
}
array data;
std::vector<float> &getValue(void *net, std::vector<float> &input) {
  actor_net::ActorNet *n = (actor_net::ActorNet *)net;
  array x(input.size() / 3, 3);
  x.data = input;
  data = n->predict(x, 8.0, false);
  for (auto &i : data.data)
    i = fmaxf(-3.0, fminf(3.0, i));
  return data.data;
}
int trainNet(void *net, const std::vector<float> &input, const std::vector<float> target) {
  actor_net::ActorNet *n = (actor_net::ActorNet *)net;
  array v(input.size() / 3, 3);
  v.data = input;
  array ww(target.size() / 3, 3);
  ww.data = target;
  for (int i = 0; i < 5; ++i) {
    auto td = n->predict(v, 8.0, false);
    n->train(v, ww - td, 8.0);
  }
  auto td = ww - n->predict(v, 8.0, false);
  float mx = MAXFLOAT;
  int ret = 0;
  for (int i = 0; i < td.sx; ++i) {
    float d = 0;
    for (int j = 0; j < td.sy; ++j) {
      auto &x = td.data[i * td.sy + j];
      d += x * x;
    }
    d = sqrtf(d);
    if (d < mx) {
      mx = d;
      ret = i;
    }
  }
  return ret;
}