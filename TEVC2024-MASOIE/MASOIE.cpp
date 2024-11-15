/* 
T. -Y. Chen, W. -N. Chen, F. -F. Wei, X. -M. Hu and J. Zhang, 
"Multi-Agent Swarm Optimization With Adaptive Internal and External Learning for Complex Consensus-Based Distributed Optimization," 
in IEEE Transactions on Evolutionary Computation, doi: 10.1109/TEVC.2024.3380436.
https://ieeexplore.ieee.org/document/10477458/keywords#keywords
*/

#include "../framework/framework.h"
#include "./internal_optimizer.h"
#include <iostream>
#include <mpi.h>
#include<thread>
#include <Eigen/Dense>
using namespace std;
using namespace Eigen;

// parameter setting
int swarmSize = 300;
int interval = 4;
int no_improve_tolerant=10;
double termination_threshold = 1E-5;

void agent_function(Framework* handler){

    // get optimization problem information
    int dim = handler->get_problem_dim();
    double lb = handler->get_lower_bound();
    double ub = handler->get_upper_bound();
    vector<int> nei_list = handler->get_neighbor_id();
    vector<double> nei_weight = handler->get_adjacent_weights();
    int nei_num = nei_list.size();
    vector<int> activate_nei_idx(nei_num,0);
    iota(activate_nei_idx.begin(), activate_nei_idx.end(), 0);

    // population initialization
    MatrixXd swarm_x = (MatrixXd::Random(swarmSize,dim).array()/2 + 0.5).array() * (ub-lb) + lb;
    MatrixXd external_swarm_v = MatrixXd::Zero(swarmSize,dim);
    VectorXd swarm_fit = VectorXd::Zero(swarmSize);
    for(int j = 0;j<swarmSize;j++){
        VectorXd inv = swarm_x.row(j);
        swarm_fit(j) = handler->local_evaluation(inv.data());
    }
    LLSO* internal_opt = new LLSO();
    internal_opt->bestFit = swarm_fit.minCoeff();

    // neighboring communication initialization
    vector<MatrixXd> nei_buffers;
    MPI_Request* terminate_recv_req = new MPI_Request[nei_num]; 
    MPI_Request* message_recv_req = new MPI_Request[nei_num];
    int if_nei_terminate[nei_num];
    for (int i =0 ;i <nei_num;i++) {
        // communication for optimization information, tag is 1
        MatrixXd buffer(swarmSize,dim);
        nei_buffers.push_back(buffer);
        MPI_Request recv_req_2;
        handler->Message_Irecv(nei_buffers[i].data(),swarmSize*dim,MPI_DOUBLE, nei_list[i],1,&recv_req_2);               
        message_recv_req[i] = recv_req_2;

        // communication for termination notification, tag is 5
        if_nei_terminate[i] =0;
        MPI_Request recv_req;
        handler->Message_Irecv(&if_nei_terminate[i], 1, MPI_INT, nei_list[i], 5, &recv_req);  
        terminate_recv_req[i] = recv_req;
    }

    double best_inv_fit = DBL_MAX;
    int no_improve_count=0;
    
    for(int iter=0;;iter++){

        // internal learning
        MatrixXd swarm_v = external_swarm_v.array();
        for(int i=0;i<interval;i++){
            internal_opt->step(&swarm_x,&swarm_v,swarm_fit);
            swarm_x = swarm_x.cwiseMin(ub);
            swarm_x = swarm_x.cwiseMax(lb);
            for(int j = 0;j<swarmSize;j++){
                VectorXd inv = swarm_x.row(j);
                swarm_fit(j) = handler->local_evaluation(inv.data());
            }
            internal_opt->update_performance(swarm_fit);            
        }

        // send message to neighbors
        double* trans_data = swarm_x.data();
        MPI_Request req[nei_num];
        for(int k:activate_nei_idx){
            handler->Message_Isend(trans_data,swarmSize*dim,MPI_DOUBLE,nei_list[k],1,&req[k]);
        }
        
        // wait for message from neighbors
        vector<int> new_activate_nei_idx;
        for(int k:activate_nei_idx){
            MPI_Request* nei_req = new MPI_Request[2];
            nei_req[0] = terminate_recv_req[k];
            nei_req[1] = message_recv_req[k];
            int index;
            MPI_Status status;
            MPI_Waitany(2, nei_req, &index, &status);
            if(status.MPI_TAG == 5){
                MPI_Cancel(&message_recv_req[k]);
            }else{
                new_activate_nei_idx.push_back(k);
                MPI_Request recv_req;
                handler->Message_Irecv(nei_buffers[k].data(),swarmSize*dim,MPI_DOUBLE, nei_list[k],1,&recv_req);               
                message_recv_req[k] = recv_req;
            }
        }
        activate_nei_idx = new_activate_nei_idx;

        // external learning
        MatrixXd randmtx = (MatrixXd::Random(swarmSize,1).array()/2 + 0.5).replicate(1,dim);
        external_swarm_v = external_swarm_v.cwiseProduct(randmtx);
        for(int k:activate_nei_idx){
            external_swarm_v += (nei_buffers[k] - swarm_x) * nei_weight[k];
        }
        swarm_x += external_swarm_v;
        swarm_x = swarm_x.cwiseMin(ub);
        swarm_x = swarm_x.cwiseMax(lb);     
        for(int j = 0;j<swarmSize;j++){
            VectorXd inv = swarm_x.row(j);
            swarm_fit(j) = handler->local_evaluation(inv.data());
        }

        // adaptive communication interval
        if(swarm_fit.minCoeff() < best_inv_fit){
            best_inv_fit = swarm_fit.minCoeff();
            no_improve_count = 0;
        }else{
            no_improve_count ++;
        }
        if(no_improve_count > no_improve_tolerant && interval > 2){
            interval = interval-1;
            no_improve_count = 0;
            best_inv_fit = swarm_fit.minCoeff();
        }
        
        // cout<<iter<<" "<<best_inv_fit<<endl;
        
        // termination decision
        if(external_swarm_v.array().abs().sum() < termination_threshold || handler->reachMaxEva()){
            // cout<<"terminate "<<handler->agent_id<<" "<<external_swarm_v.array().abs().sum()<<" "<<iter<<" "<<interval<<endl;
            int terminate_flag = 1;
            MPI_Request terminate_send_req[nei_num]; 
            for(int k = 0;k<nei_num;k++)    
                handler->Message_Isend(&terminate_flag,1,MPI_INT,nei_list[k],5,&terminate_send_req[k]);
            for(int k:activate_nei_idx){
                MPI_Cancel(&message_recv_req[k]);
            }
            break;
        }
    }

    // record optimization results
    VectorXd final_x = swarm_x.colwise().mean();
    handler->submit_final_solution(final_x.data());
    return;
}
