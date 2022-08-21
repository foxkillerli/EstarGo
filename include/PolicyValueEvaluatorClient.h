#ifndef POLICY_VALUE_EVALUATOR_H
#define POLICY_VALUE_EVALUATOR_H

#include "Eval_Def.h"
#include <memory>
#include <atomic>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>
#include "GoBoard.h"
#include "EvalQueue.h"
#include "Proto.h"
#include "Latency.h"
#include "Utils.h"
#include <iostream>
#include <queue>

using namespace GoKu;

typedef std::function<void(std::vector<PolicyAndValueCallbackType>&)> RequestFailedCallbackType;

class PolicyAndValueCookie : public Ice::LocalObject {
 public:
  PolicyAndValueCookie(long send_time,Latency& latency, std::vector<PolicyAndValueCallbackType> callback_list,std::atomic_int& pending, Semaphore* remain_can_send) : 
    send_time_(send_time), request_latency_(latency), callback_list_(std::move(callback_list)), pending_(pending),remain_can_send_(remain_can_send) {}
  long get_send_time() { return send_time_; }
  Latency& get_latency() { return request_latency_; }
  std::vector<PolicyAndValueCallbackType>& get_callback_list() { return callback_list_; }
  std::atomic_int& get_pending() { return pending_; }
  Semaphore* get_remain_can_send() { return remain_can_send_; }
  RequestFailedCallbackType& get_failed_callback() { return failed_callback_; };

  void set_failed_callback(RequestFailedCallbackType failed_callback) {
    failed_callback_ = failed_callback;
  }


 private:
  long send_time_;
  Latency& request_latency_;
  std::vector<PolicyAndValueCallbackType> callback_list_;
  std::atomic_int& pending_;
  Semaphore* remain_can_send_;
  RequestFailedCallbackType failed_callback_;
};

typedef IceUtil::Handle<PolicyAndValueCookie> PolicyAndValueCookiePtr;

class PolicyAndValueCallback : public IceUtil::Shared {
 public:
  // Callback for successful response.
  // Cookie is also optional.
  // Use references for params.
  void evaluate_cb(const ReplyBatch& rep_batch, const PolicyAndValueCookiePtr& cookie) {
    long send_time = cookie->get_send_time();
    //cookie->get_latency().Record(gettime() - send_time);
    //cookie->get_remain_can_send()->notify();
    std::atomic_int& pending = cookie->get_pending();
    std::vector<PolicyAndValueCallbackType>& callback_list = cookie->get_callback_list();
    //for (int i = 0;i < (int)rep_batch.size(); ++i) {
    for (int i = 0;i < (int)callback_list.size(); ++i) {
      float value = rep_batch[i].v;
      std::vector<PolicyItem> policy_list;
      int policy_size = (int)rep_batch[i].p.size();
      for (int j = 0; j < policy_size; ++j) {
        int pos = rep_batch[i].p[j].pos;
        float prob = rep_batch[i].p[j].prob;
        policy_list.push_back(PolicyItem(pos, prob));
      }
      callback_list[i](true, value, policy_list);
      pending--;
    }
    cookie->get_latency().Record(gettime() - send_time);
    cookie->get_remain_can_send()->notify();
  }

  // Callback for failed response.
  // Must use Ice exception here. 
  void failure_cb(const Ice::Exception& ex, const PolicyAndValueCookiePtr& cookie) {
    std::cerr << ex.what() << std::endl;
    std::vector<PolicyAndValueCallbackType>& callback_list = cookie->get_callback_list();
    std::atomic_int& pending = cookie->get_pending();
    //cookie->get_remain_can_send()->notify();
    RequestFailedCallbackType& failed_callback = cookie->get_failed_callback();
    for (uint i = 0;i < callback_list.size(); ++i) {
      pending--;
    }
    failed_callback(callback_list);
    cookie->get_remain_can_send()->notify();
  }
};

typedef IceUtil::Handle<PolicyAndValueCallback> PolicyAndValueCallbackPtr;


class PolicyAndValueEvaluatorClient {
 public:
  PolicyAndValueEvaluatorClient();
  ~PolicyAndValueEvaluatorClient();
  void Eval(const GoBoard *board, PolicyAndValueCallbackType cb);
  void EvalByHistory(std::string& history, PolicyAndValueCallbackType cb);
  Latency& get_latency();
  std::vector<Latency*>& get_latencys();
  std::vector<Latency*>& get_dummy_latencys();
  int get_pending();
  int get_resend_times();
  void clear_resend_times();

 private:
  struct RequestItem {
    std::string history;
    PolicyAndValueCallbackType callback;
    RequestItem(){}
    RequestItem(std::string history,PolicyAndValueCallbackType cb): history(std::move(history)), callback(std::move(cb)){}
  };
  std::queue<RequestItem* > send_queue_;
  std::mutex eval_mutex_;
  Semaphore *full_, *empty_;
  std::atomic_int pending_{0};
  Latency request_latency_;
  std::vector<Latency*> request_latencys_;
  std::vector<Latency*> dummy_request_latencys_;
  int request_batch_size_;
  int proxy_number_;

  std::atomic_int eval_terminate{0};
  std::vector<std::thread> thread_list_;

  std::atomic_int resend_times_{0};

  std::atomic_int fetch_num{0};
  std::atomic_int fetch_times{0};

  //ice client
  Ice::CommunicatorHolder ch_;
  std::vector<InterfaceEvalPrx> prxs_;
  // InterfaceEvalPrx prx_;
  Callback_InterfaceEval_evaluatePtr cb_safe_;

  void ProcessEval();
  void Start();
  void Stop();
  void SyncRequest(RequestBatch& request_cache,std::vector<PolicyAndValueCallbackType>& callback_cache, int& proxy_count);
  void AsynRequest(RequestBatch& request_cache,std::vector<PolicyAndValueCallbackType>& callback_cache, Semaphore* remain_can_send, int & proxy_count);
  void AsyncRequestDummyData(RequestBatch& request_cache,std::vector<PolicyAndValueCallbackType>& callback_cache, Semaphore* remain_can_send, int & proxy_count);
};

#endif
