
double get_array_mean(int* arr, int size){
    double res = 0;
    for(int i=0;i<size;i++){
        res += arr[i];
    }
    return res/size;
}

double* get_mtx_mean(double** arr, int row, int col){
    double* mean_vector = new double[col]{0};
    for(int i=0;i<row;i++){
        for(int j=0;j<col;j++){
            mean_vector[j]+=arr[i][j];
        }
    }
    //计算均值
    for(int i=0;i<col;i++)
        mean_vector[i] /= row;
    
    return mean_vector;
}

double get_mtx_std(double** arr, int row, int col){

    double* mean_vector = get_mtx_mean(arr,row,col);
    //计算标准差
    double std=0;
    for(int i=0;i<row;i++){
        double dis = 0;
        for(int j=0;j<col;j++){
            dis += (arr[i][j]-mean_vector[j])*(arr[i][j]-mean_vector[j]);
        }
        std +=sqrt(dis);
    }
    std /= row;
    delete[] mean_vector;
    return std;
}
