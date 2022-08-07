#ifndef EVAL_DEF_H
#define EVAL_DEF_H

#include <Ice/Ice.h>
#include "goku_proto.h"

struct PolicyItem {
  int pos;
  float prob;
  PolicyItem(int pos = 0, float p = 0): pos(pos), prob(p) {}
};

typedef std::function<void(bool, float)> ValueCallbackType;
typedef std::function<void(bool, std::vector<PolicyItem>&)> PolicyCallbackType;
typedef std::function<void(bool, float, std::vector<PolicyItem>&)> PolicyAndValueCallbackType;

#endif