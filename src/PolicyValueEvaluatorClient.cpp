#include "PolicyValueEvaluatorClient.h"
#include <string.h>
#include <unistd.h>
#include <iostream>
#include "../include/utils/Config.h"
#include "../include/utils/Logger.h"

PolicyAndValueEvaluatorClient::PolicyAndValueEvaluatorClient() {
  request_batch_size_ = (int)Config::batch_size();
  int queue_size = (int)Config::queue_size();
  full_ = new Semaphore(queue_size);
  empty_ = new Semaphore(0);

  std::string proxy_name = Config::proxy_name();
  proxy_number_ = Config::proxy_number();
  std::string conf_file = Config::ice_conf();

  if (!Config::use_server_evaluator()) {
    return;
  }

  // Create a communicator
  ch_ = Ice::initialize(conf_file.c_str());

  for (int i = 0; i < proxy_number_; ++i) {
    try {
      char num[10];
      sprintf(num ,"%d",i);
      std::string name = proxy_name + num;
      Ice::ObjectPrx base_prx = ch_->propertyToProxy(name + "Prx");
      if(!base_prx) fprintf(stderr, "%s\n", "[Error] Could not create proxy. ");
      prxs_.push_back(InterfaceEvalPrx::checkedCast(base_prx));
      Latency* each_request_latency_ = new Latency();
      request_latencys_.push_back(each_request_latency_);
      Latency* each_dummy_request_latency_ = new Latency();
      dummy_request_latencys_.push_back(each_dummy_request_latency_);
      if(!prxs_[i])  fprintf(stderr, "%s\n", "[Error] Invalid proxy. ");
    } catch(const std::exception& ex) {
      std::cerr << "proxy " << i << " start failed" << std::endl;
      std::cerr << ex.what() << std::endl;
      exit(1);
    }
  }

  // Create a callback for asyn requests
  PolicyAndValueCallbackPtr cb = new PolicyAndValueCallback;
  cb_safe_ = newCallback_InterfaceEval_evaluate(cb, &PolicyAndValueCallback::evaluate_cb, &PolicyAndValueCallback::failure_cb);

  Start();
}

PolicyAndValueEvaluatorClient::~PolicyAndValueEvaluatorClient() {
  Stop();
  delete full_;
  delete empty_;
  for(size_t i=0; i<request_latencys_.size(); ++i) {
      delete request_latencys_[i];
  }
  for(size_t i=0; i<dummy_request_latencys_.size(); ++i) {
      delete dummy_request_latencys_[i];
  }
}

void PolicyAndValueEvaluatorClient::Start() {
  eval_terminate = 0;
  int send_thread = Config::server_send_thread();
  for (int i = 0;i < send_thread; ++i) {
    thread_list_.push_back(std::thread(&PolicyAndValueEvaluatorClient::ProcessEval,this));
  }
}
void PolicyAndValueEvaluatorClient::Stop() {
  eval_terminate = 1;
  while (send_queue_.size() <= 0) {
    GoBoard board(7.5, 19);
    board.apply_history("");
    Eval(&board, [](bool ok, float value_score, std::vector<PolicyItem>& policy_list) mutable {});
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  for (size_t i = 0; i < thread_list_.size(); i++) {
    thread_list_[i].join();
  }
  while (!send_queue_.empty()) send_queue_.pop();
}

void PolicyAndValueEvaluatorClient::ProcessEval() {
  long sleep_time = Config::sleep_interval();
  bool is_asyn = Config::use_asyn_request();
  int once_total_send = Config::asyn_once_send();
  Semaphore* remain_can_send = new Semaphore(once_total_send);
  int proxy_count = 0;
  //long st = gettime();
  while (!eval_terminate) {
    //st = gettime();
    std::vector<PolicyAndValueCallbackType> callback_cache;
    RequestBatch request_cache;
    if (is_asyn) {
      remain_can_send->wait();
      if(empty_->get() == 0) {
          request_cache.push_back(std::move(std::string("")));
          request_cache.push_back(std::move(std::string("")));
          request_cache.push_back(std::move(std::string("")));
          request_cache.push_back(std::move(std::string("")));
          AsyncRequestDummyData(request_cache, callback_cache, remain_can_send, proxy_count);
          proxy_count += 1;
          continue;
      }
    }
    //LOG_INFO("async wait time:" << gettime()-st);
    //int retry_times = 0;
    //while(empty_->get() < request_batch_size_ && retry_times < 50) {
    //    std::this_thread::sleep_for(std::chrono::microseconds(20));
    //    retry_times++;
    //}
    int fetch = empty_->wait(request_batch_size_);
    {
        std::lock_guard<std::mutex> lock(eval_mutex_);
        for (int i = 0;i < fetch; ++i) {
          RequestItem* item = send_queue_.front();
          request_cache.push_back(std::move(item->history));
          callback_cache.push_back(std::move(item->callback));
          send_queue_.pop();
          delete item;
        }
    }
    full_->notify(fetch);
    //fetch_num += fetch;
    //fetch_times += 1;
    //LOG_INFO("fetch percent= " << (fetch_num * 100.0 / (4 * fetch_times)));
    //LOG_INFO("fetch wait time:" << gettime()-st);
    if (is_asyn) {
      AsynRequest(request_cache, callback_cache, remain_can_send, proxy_count);
    }
    else {
      SyncRequest(request_cache, callback_cache, proxy_count);
    }
    proxy_count += 1;
    if (sleep_time > 0) {
      usleep(sleep_time);
    }
  }
  while (remain_can_send->get() < once_total_send) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  delete remain_can_send;
}

void PolicyAndValueEvaluatorClient::SyncRequest(RequestBatch& request_cache,std::vector<PolicyAndValueCallbackType>& callback_cache, int & proxy_count) {
  ReplyBatch rep_batch;
  long send_time = gettime();
  int target = proxy_count % proxy_number_;
  prxs_[target]->evaluate(request_cache, rep_batch);
  request_latency_.Record(gettime() - send_time);
  for (int i = 0;i < (int)rep_batch.size(); ++i) {
    float value = rep_batch[i].v;
    std::vector<PolicyItem> policy_list;
    int policy_size = (int)rep_batch[i].p.size();
    for (int j = 0; j < policy_size; ++j) {
      int pos = rep_batch[i].p[j].pos;
      float prob = rep_batch[i].p[j].prob;
      policy_list.push_back(PolicyItem(pos, prob));
    }
    callback_cache[i](true, value, policy_list);
    pending_--;
  }
}

void PolicyAndValueEvaluatorClient::AsynRequest(RequestBatch& request_cache,std::vector<PolicyAndValueCallbackType>& callback_cache, Semaphore* remain_can_send, int & proxy_count) {
  try {
    int target = proxy_count % proxy_number_;
    PolicyAndValueCookiePtr cookie = new PolicyAndValueCookie(gettime(), *request_latencys_[target], callback_cache, pending_, remain_can_send);
    cookie->set_failed_callback([this,request_cache](std::vector<PolicyAndValueCallbackType>& callbacks) mutable {
      resend_times_++;
      for (uint i = 0;i < request_cache.size(); ++i) {
        EvalByHistory(request_cache[i], callbacks[i]);
      }
    });
    prxs_[target]->begin_evaluate(request_cache, cb_safe_, cookie);
  } catch(const std::exception& ex) {
    std::cerr << ex.what() << std::endl;
  }
}

void PolicyAndValueEvaluatorClient::AsyncRequestDummyData(RequestBatch& request_cache,std::vector<PolicyAndValueCallbackType>& callback_cache, Semaphore* remain_can_send, int & proxy_count) {
  try {
    int target = proxy_count % proxy_number_;
    PolicyAndValueCookiePtr cookie = new PolicyAndValueCookie(gettime(), *dummy_request_latencys_[target], callback_cache, pending_, remain_can_send);
    cookie->set_failed_callback([this,request_cache](std::vector<PolicyAndValueCallbackType>& callbacks) mutable {});
    prxs_[target]->begin_evaluate(request_cache, cb_safe_, cookie);
  } catch(const std::exception& ex) {
    std::cerr << ex.what() << std::endl;
  }
}

void PolicyAndValueEvaluatorClient::Eval(const GoBoard *board, PolicyAndValueCallbackType cb) {
  full_->wait();
  {
      std::lock_guard<std::mutex> lock(eval_mutex_);
      pending_++;
      send_queue_.push(new RequestItem(board->get_history(), cb));
  }
  empty_->notify();
}
void PolicyAndValueEvaluatorClient::EvalByHistory(std::string& history, PolicyAndValueCallbackType cb) {
  full_->wait();
  {
      std::lock_guard<std::mutex> lock(eval_mutex_);
      pending_++;
      send_queue_.push(new RequestItem(history, cb));
  }
  empty_->notify();
}

Latency& PolicyAndValueEvaluatorClient::get_latency() {
  return request_latency_;
}

std::vector<Latency*>& PolicyAndValueEvaluatorClient::get_latencys() {
  return request_latencys_;
}

std::vector<Latency*>& PolicyAndValueEvaluatorClient::get_dummy_latencys() {
  return dummy_request_latencys_;
}

int PolicyAndValueEvaluatorClient::get_pending(){
  return pending_;
}

int PolicyAndValueEvaluatorClient::get_resend_times() {
  return resend_times_;
}
void PolicyAndValueEvaluatorClient::clear_resend_times() {
  resend_times_ = 0;
}
