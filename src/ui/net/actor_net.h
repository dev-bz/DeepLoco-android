#pragma once
namespace actor_net {
class ActorNet {
  std::map<std::string, setting> optm_cfg;
  map params;

public:
  ActorNet(int input_size, int hidden_size1, int hidden_size2, int output_size, float std_ = 5e-1);
  actions_grads evaluate_gradient(array &X,const array &action_grads, const float &action_bound, bool use_target = false);
  array evaluate_action_gradient(array &X_S, array &X_A, bool use_target = false);
  void train(array &X,const array &action_grads, const float &action_bound);
  void train_target(float tau);
  array predict(const array &X, const float &action_bound, bool target);
  adam_value _adam(array &x, array &dx, setting &config);
  array _uniform_init(int input_size, int output_size);
  void debug();
};
} // namespace actor_net