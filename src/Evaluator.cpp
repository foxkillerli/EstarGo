#include "../include/Evaluator.h"

namespace goeval{

Evaluator::Evaluator() {
}

Evaluator::~Evaluator() {
}

void Evaluator::PolicyAndValueEval(const GoBoard *board, const PolicyAndValueCallbackType callback) {
    policy_value_evaluator_.Eval(board, callback);
}

Latency& Evaluator::get_latency() {
  return policy_value_evaluator_.get_latency();
}

PolicyAndValueEvaluatorClient& Evaluator::get_eval() {
    return policy_value_evaluator_;
}

};