#ifndef INTERNAL_OPTIMIZER_H
#define INTERNAL_OPTIMIZER_H

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <random>
#include <vector>
#include <Eigen/Dense>

using namespace Eigen;
using std::cout;
using std::endl;

class LLSO{
// LLSO: level-based swarm optimizer [Q. Yang, W. -N. Chen, J. D. Deng, Y. Li, T. Gu and J. Zhang, "A Level-Based Learning Swarm Optimizer for Large-Scale Optimization," in IEEE Transactions on Evolutionary Computation, vol. 22, no. 4, pp. 578-594, Aug. 2018, doi: 10.1109/TEVC.2017.2743016. ]
public:
    //优化器基础参数
    int NL, LS, NL_index=0;
    double epsilon = 0.5;
    double bestFit;
    double max_improvement = 0;
    int *rand_level_set; //the pool of the number of levels
    int rand_level_size;
    double *level_size_performance;
    LLSO(){
        rand_level_size = 6;
        rand_level_set = new int[6]{4, 6, 8, 10, 20, 50};

        level_size_performance = new double[rand_level_size];
        for (int i = 0; i < rand_level_size; i++)
            level_size_performance[i] = 1;
        
    }
    int select_level_size(double *a)
    {
        double total = 0;
        for (int i = 0; i < rand_level_size; i++)
        {
            total += exp(7 * a[i]);
        }
        double *pro = new double[rand_level_size + 1];
        pro[0] = 0;
        for (int i = 0; i < rand_level_size; i++)
        {
            pro[i + 1] = pro[i] + exp(7 * a[i]) / total;
        }
        double tmp = rand() * 1.0 / RAND_MAX;
        int selected = -1;
        for (int i = 0; i < rand_level_size; i++)
        {
            if (tmp <= pro[i + 1])
            {
                selected = i;
                break;
            }
        }
        if (selected == -1)
        {
            cout << "select level size error" << endl;
            for (int i = 0; i <= rand_level_size; i++)
            {
                cout << pro[i] << " ";
            }
            cout << endl;
            for (int i = 0; i < rand_level_size; i++)
            {
                cout << a[i] << " ";
            }
            cout << endl;
            for (int i = 0; i < rand_level_size; i++)
            {
                cout << level_size_performance[i] << " ";
            }
            cout << endl;
            selected = rand() % rand_level_size;
            cout << "total:" << total << " bestFit:" << bestFit << endl;
        }
        delete[] pro;
        return selected;
    }
    void update_performance(VectorXd pop_fit){
        if(bestFit>pop_fit.minCoeff()){
            if(max_improvement != 0){
                level_size_performance[NL_index] = (bestFit - pop_fit.minCoeff()) / max_improvement;
            }
            max_improvement = std::max(max_improvement,bestFit - pop_fit.minCoeff());
            bestFit = pop_fit.minCoeff();
        }else{
            level_size_performance[NL_index] = 0;
        }
    }
    VectorXi sort_vec(VectorXd vec){
        VectorXi ind=VectorXi::LinSpaced(vec.size(),0,vec.size()-1);
        auto rule=[vec](int i, int j)->bool{
            return vec(i)<vec(j);
        };
        std::sort(ind.data(),ind.data()+ind.size(),rule);
        return ind;
    }
    void step(MatrixXd* swarm_x, MatrixXd *swarm_v, VectorXd pop_fit){
        int swarmSize = swarm_x->rows();
        int dimension = swarm_x->cols();
        NL_index = select_level_size(level_size_performance);
        NL = rand_level_set[NL_index];
        LS = swarmSize / NL;
        VectorXi seq = sort_vec(pop_fit);
        // cout<<"NL="<<NL<<endl;
        for (int level_index = NL - 1; level_index >= 1; level_index--)
        {
            int NUM = LS;
            if(level_index == NL-1){
                NUM += swarmSize%NL;
            }
            for (int p_index = 0; p_index < NUM; p_index++)
            {
                int p_cur = (level_index)*LS + p_index;
                int p1, p2;

                if (level_index >= 2)
                {
                    int rl1 = rand() % (level_index);
                    int rl2 = rand() % (level_index);
                    while (rl1 == rl2)
                    {
                        rl2 = rand() % (level_index);
                    }
                    if (rl1 > rl2)
                    {
                        std::swap(rl1, rl2);
                    }
                    //对于level rl1, 元素的index在 [ LS*(rl1-1），LS*rl1-1]
                    p1 = rand() % LS + LS * rl1;
                    p2 = rand() % LS + LS * rl2;
                }
                else if (level_index == 1)
                {
                    p1 = rand() % LS;
                    p2 = rand() % LS;
                    while (p1 == p2)
                    {
                        p2 = rand() % LS;
                    }

                    if (p2<p1)
                    {
                        std::swap(p1, p2);
                    }
                }
                VectorXd r1 =  VectorXd::Random(dimension,1).array()/2 + 0.5;
                VectorXd r2 =  VectorXd::Random(dimension,1).array()/2 + 0.5;
                VectorXd r3 =  VectorXd::Random(dimension,1).array()/2 + 0.5;
                swarm_v->row(seq(p_cur)) = r1.transpose().cwiseProduct(swarm_v->row(seq(p_cur))) + r2.transpose().cwiseProduct(swarm_x->row(seq(p1)) - swarm_x->row(seq(p_cur))) + r3.transpose().cwiseProduct(swarm_x->row(seq(p2)) - swarm_x->row(seq(p_cur)))*epsilon;
                swarm_x->row(seq(p_cur)) += swarm_v->row(seq(p_cur));
            }
        }

    }
};


#endif