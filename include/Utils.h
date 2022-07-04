//
// Created by Kaihua Li on 2022/7/4.
//

#ifndef ESTAR_GO_UTILS_H
#define ESTAR_GO_UTILS_H


#include <ctime>
#include <cstdio>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <cctype>



long gettime();
long getutime();
void print_pos(const char *label,int action);
void faked_dirichlet_noise(std::vector<float> &noise_prob);

inline int fast_rand(int &seed){
    seed = (214013*seed+2531011);
    return (seed>>16)&0x7FFF;
}

inline float fast_randf(int &seed){
    seed*=16807;
    return (float)(seed & 0x7FFFFFFF) * 4.6566129e-010f;
}

inline std::string to_upper_string(std::string& src) {
    std::string dst;
    std::transform(src.begin(), src.end(), std::back_inserter(dst), ::toupper);
    return dst;
}

#include <mutex>
#include <condition_variable>

class Semaphore {
public:
    Semaphore (int count_ = 0)
            : count(count_) {}

    inline void notify(int add = 1)
    {
        std::unique_lock<std::mutex> lock(mtx);
        count+=add;
        cv.notify_one();
    }

    inline int wait(int max = 1)
    {
        std::unique_lock<std::mutex> lock(mtx);
        while(count == 0){
            cv.wait(lock);
        }
        int ret=0;
        if(max>count){
            ret=count;
            count=0;
        }else{
            count-=max;
            ret=max;
        }
        return ret;
    }

    inline int get()
    {
        std::unique_lock<std::mutex> lock(mtx);
        int ret = count;
        return ret;
    }

private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;
};


#endif //ESTAR_GO_UTILS_H
