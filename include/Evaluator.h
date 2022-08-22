#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "GoBoard.h"
#include "Eval_Def.h"
#include "PolicyValueEvaluatorClient.h"
#include "Latency.h"
#include <string.h>

namespace goeval {

class Evaluator {
 public:
  Evaluator();
  ~Evaluator();

  void PolicyAndValueEval(const GoBoard *board, const PolicyAndValueCallbackType callback);
  PolicyAndValueEvaluatorClient& get_eval();
  Latency& get_latency();

 private:
  PolicyAndValueEvaluatorClient policy_value_evaluator_;

};

}; // namespace goeval

#endif