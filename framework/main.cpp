
#include <iostream>
#include <mpi.h>
#include <sys/time.h>
#include "../framework/framework.h"
#include "../util/utils.h"
using namespace std;

void agent_function(Framework*);
double evaluate_criterion();

long getCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
int main(int argc, char* argv[]){
    int myrank, nprocs, name;
    char proc_name[MPI_MAX_PROCESSOR_NAME];
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Get_processor_name(proc_name, &name);

    string func_id = argv[argc-1];

    Framework* handler = new Framework(myrank,func_id);

    srand(getCurrentTime()+myrank);
    agent_function(handler);

    MPI_Barrier(MPI_COMM_WORLD);

    int dimension = handler->get_problem_dim();
    double *gather_final_solution = nullptr;
    int *gather_commu_cost = nullptr; 
    if (myrank == 0)
        gather_final_solution = new double[dimension * nprocs]{0};
        gather_commu_cost = new int[nprocs]{0};
    MPI_Gather(handler->final_solution,dimension,MPI_DOUBLE,gather_final_solution,dimension,MPI_DOUBLE,0,MPI_COMM_WORLD);
    int commu_cost = handler->get_commu_cost();
    MPI_Gather(&commu_cost,1,MPI_INT,gather_commu_cost,1,MPI_INT,0,MPI_COMM_WORLD);
        
    if (myrank == 0){
        double **total_solutions = new double*[nprocs];
        for(int i=0;i<nprocs;i++){
            total_solutions[i] = new double[dimension]{0};
            memcpy(total_solutions[i],gather_final_solution+i*dimension,dimension*sizeof(double));
        }
        double *average_solution = get_mtx_mean(total_solutions,nprocs,dimension);

        Benchmarks* global_func = new Benchmarks(func_id);
        double fitness = global_func->global_eva(average_solution);

        double disagreement = get_mtx_std(total_solutions,nprocs,dimension);

        double average_commu_cost = get_array_mean(gather_commu_cost,nprocs);

        printf("algorithm performance: [fitness:%.4e, disagreement:%.4e, communication cost:%.4e]\n",fitness, disagreement, average_commu_cost);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

    return 0;
}