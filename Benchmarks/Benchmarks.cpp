#include <sstream>
#include <fstream>
#include <iostream>
#include<cstring>
#include<cstdio>
#include<cmath>
#include<algorithm>
#include "../Benchmarks/Benchmarks.h"
#include "../Benchmarks/BaseFunction.h"
using namespace std;
using json = nlohmann::json;

Benchmarks::Benchmarks(string ID,int max_eva_times){    
    string config_path = "../Benchmarks/default_config.json";
    string data_path = "../Benchmarks/data/";
    ifstream file_config(config_path);
    json config;
    file_config >> config;
    file_config.close();
    this->func_config = config["benchmarks"][ID];
    this->funcID = ID;
    this->node_num = func_config["node_num"];

    this->max_eva_times = max_eva_times;
    this->eva_count = 0;
    this->reach_max_eva_times = false;

    this->if_rotate = func_config["if_rotate"];
    this->if_shift = func_config["if_shift"];
    this->if_heterogeneous = func_config["heterogeneous"];
    weight = func_config["weight"];
    int dim = func_config["dimension"];

    A = read_data<double>(data_path+"A_"+funcID);
    W = read_data<double>(data_path+"W_"+funcID);
    if(if_rotate == true)
        R_global = read_data<double>(data_path+"R_"+funcID);    
    if(if_shift == true)
        xopt = read_data<double>(data_path+"xopt_"+funcID);
    else{
        int len = getDimension();
        xopt = new double*[1];
        xopt[0] = new double[len]{0};
    }    
}

Benchmarks::~Benchmarks() {
    for (int i = 0; i < node_num; i++) {
        delete[] A[i];
        if(if_rotate == true)
            delete[] R_global[i];
    }
    delete[] A;
    if(if_rotate == true)
        delete[] R_global;
    if(if_shift == true){
        delete[] xopt[0];
        delete[] xopt;
    }

}

bool Benchmarks::reachMaxEva(){
    if(eva_count>=max_eva_times){
        if(!reach_max_eva_times){
            // cout<<"The time of evaluation has reached the maximum bound. Later evaluation results would not be recorded.\n"; 
            reach_max_eva_times = true;
        }
        return true;
    }
    return false;
}

double Benchmarks::local_eva(double* x, int groupIndex) {
    
    if (groupIndex < 0 || groupIndex >= node_num) {
		cout << "groupIndex error\n";
		return 0;
	}
    int len = getDimension();   
    
    double* shftx = new double[len];
    for (int j = 0; j < len; j++) {
        shftx[j] = x[j] - xopt[0][j];
    }

    double *input_x ;    
    if(if_rotate == true){
        input_x = multiply(shftx, R_global, len);
    }else{
        input_x = new double[len];
        memcpy(input_x,shftx,len*sizeof(double));
    }        

    string funcType;
    if(if_heterogeneous){
        funcType = func_config["base_function"][groupIndex%2];
    }else{
        funcType=func_config["base_function"];
    }

    double res = 0;
    if(funcType=="elliptic"){
        res = elliptic(input_x, len);
    }
    else if(funcType=="rastrigin"){
        res = rastrigin(input_x, len);
    }
    else if(funcType=="schwefel"){
        res = schwefel(input_x, len);
    }
    else if(funcType=="ackley"){
        res = ackley(input_x, len);
    }
    else if(funcType=="rosenbrock"){
        res = rosenbrock(input_x, len);
    }
    else if(funcType=="griewank"){
        res = griewank(input_x, len);
    }
    else if(funcType=="ellipsoid"){
        res = ellipsoid(input_x, len);
    }
    
    for(int j=0;j<len;j++){
        res += A[groupIndex][j] * input_x[j] * weight;
    }    
	delete[] input_x;
    delete[] shftx;
    
    eva_count += 1;
        
    return res;
}

double Benchmarks::global_eva_type2(double* x) {
    
    // int len = func_config["subproblems"][groupIndex]["dimension"];
    int len = getDimension();
    // double ub = func_config["upper_bound"];
    // double lb = func_config["lower_bound"];

    double* shftx = new double[len];
    for (int j = 0; j < len; j++) {
        shftx[j] = x[j] - xopt[0][j];
    }

    double *input_x;       

    if(if_rotate == true){
        input_x = multiply(shftx, R_global, len);
    }else{
        input_x  = new double[len];
        memcpy(input_x,shftx,len*sizeof(double));
    }        

    string funcType = func_config["base_function"];
    double res = 0;
    if(funcType=="elliptic"){
        res = elliptic(input_x, len);
    }
    else if(funcType=="rastrigin"){
        res = rastrigin(input_x, len);
    }
    else if(funcType=="schwefel"){
        res = schwefel(input_x, len);
    }
    else if(funcType=="ackley"){
        res = ackley(input_x, len);
    }
    else if(funcType=="rosenbrock"){
        res = rosenbrock(input_x, len);
    }
    else if(funcType=="griewank"){
        res = griewank(input_x, len);
    }
    
	delete[] input_x;
    delete[] shftx;
    
    eva_count += node_num;
        
    return res;
}

double Benchmarks::global_eva(double* x) {
	double res = 0;
    if(if_heterogeneous){
        for(int i=0;i<node_num;i++){
            double r = local_eva(x,i);
            res += r;
        }
        res/=node_num;
    }else{
        res += global_eva_type2(x);
    }    
	return res;
}

double Benchmarks::getMinX() {
	return func_config["lower_bound"];
}
double Benchmarks::getMaxX() {
	return func_config["upper_bound"];
}
int Benchmarks::getNodeNum() {
	return func_config["node_num"];
}
int Benchmarks::getDimension(){
    return func_config["dimension"];
}
double** Benchmarks::getNetworkGraph(){
    if(W==nullptr){
        string data_path = "../Benchmarks/data/";
        W=read_data<double>(data_path+"W_"+to_string(this->node_num)+"n");
    }
    return W;
}


vector<int> Benchmarks::getNeighbors(int groupIndex){
    vector<int> groups;
    for(int i=0;i<node_num;i++){
        if(i == groupIndex)
            continue;
        if(W[groupIndex][i]>0)
            groups.push_back(i);
    }    
    return groups;
}

template<typename T>
T** Benchmarks::read_data(string fileName) {
	// cout << fileName<<endl;
    T** res = nullptr;
    ifstream file(fileName);
	if (file.is_open()) {
		// cout << " is opened;\n";
        vector<vector<T>> data;
		string line;
        while(getline(file,line)){
    // cout<<"1"<<endl;
            stringstream ss(line);
            vector<T> rowData;
            T x;
            while(ss>>x){
                rowData.push_back(x);
            }
            data.push_back(rowData);
        }
		file.close();
    // cout<<"1"<<endl;

        res = new T*[data.size()];
        int count = 0;
        for(auto rowData:data){
    // cout<<"1"<<endl;
            // cout<<rowData.size()<<endl;
            T* row = new T[rowData.size()];
            memcpy(row,&rowData[0],rowData.size()*sizeof(T));
            res[count]=row;
            count++;
        }
	}
	else {
		// cout << " can not be opened;\n";
	}
	return res;
}
