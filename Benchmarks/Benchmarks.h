#ifndef BENCHMARKS_H
#define BENCHMARKS_H

#include <string>
#include <vector>
#include "../util/json.hpp"
using json = nlohmann::json;
using std::string;
using std::vector;

class Benchmarks {
private:
	int node_num;
    json func_config;
	string funcID;
	template<typename T>T** read_data(string);
	
	int **group;
	double ***R, **xopt;
	double **R_global, **A, **W=nullptr;
	bool if_rotate;
	bool if_shift;
	double global_eva_type2(double* x);
	double weight;
	bool if_heterogeneous;

public:
	int max_eva_times;
	int eva_count;
	bool reach_max_eva_times;
	Benchmarks(string ID,int max_eva_times = 100000);
	~Benchmarks();
	double global_eva(double* x);
	double local_eva(double* x, int groupIndex);
	double getMinX();
	double getMaxX();
	int getNodeNum();
	int getDimension();
	bool reachMaxEva();
	double** getNetworkGraph();
	vector<int> getNeighbors(int);
};

#endif