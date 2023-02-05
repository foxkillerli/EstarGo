//
// Created by Kaihua Li on 2022/7/4.
//

#include "../../include/Utils.h"
#include<sys/time.h>
#include<cstring>
#include<vector>
#include<ctime>
#include<cstdlib>


long gettime(){
    struct timeval ts;
    gettimeofday(&ts,NULL);
    return ts.tv_sec*1000+ts.tv_usec/1000;
}

long getutime(){
    struct timeval ts;
    gettimeofday(&ts,NULL);
    return ts.tv_sec*1000000+ts.tv_usec;
}

void print_pos(const char *label,int action){
    char s_pos[10];
    sprintf(s_pos, "%c%d", 'A' + action%19 + (action%19 >= 'I' - 'A'), action/19 + 1);
    fprintf(stderr, "%s:%s\n",label,s_pos);
}


/**
 * Dirichlet Noise for root node, for selfplay; faked
 *
 * f(x1, x2, ..., xk; a) = gamma(ak) * {x1^(a-1) * x2^(a-1) * ... * xk^(a-1) / gamma(a)^k
 * gamma function: tgamma(x)
 */
void faked_dirichlet_noise(std::vector<float> &noise_prob){
    int k = noise_prob.size();
    float sum = 0.0;
    for (int i = 0; i < k; i++) {
        srand(time(NULL) + i);
        noise_prob[i] = 1.0 / k + float(rand() % 500) / 10000.0;
        sum += noise_prob[i];
    }

    for (int i = 0; i < k; i++) {
        noise_prob[i] /= sum;
    }
}
