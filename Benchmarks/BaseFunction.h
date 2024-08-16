#ifndef BASEFUNCTION_H
#define BASEFUNCTION_H

#include <algorithm>
#include <cmath>
#include <iostream>

#define PI (3.141592653589793238462643383279)
#define E  (2.718281828459045235360287471352)
#define L(i) ((int64_t)i)
#define D(i) ((double)i)

using std::cout;
using std::endl;


double* multiply(double*vector, double**matrix, int dim);
double elliptic(double*x, int dim);
double rastrigin(double*x, int dim);
double ackley(double*x, int dim);
double schwefel(double*x, int dim);
double rosenbrock(double*x, int dim);
double griewank(double*x, int dim);
double ellipsoid(double* x,int dim);
void transform_osz(double* z, int dim);
void transform_asy(double* z, double beta, int dim);
void Lambda(double* z, double alpha, int dim);
int sign(double x);
double hat(double x);
double c1(double x);
double c2(double x);

double* multiply(double*vector, double**matrix, int dim) {
	int    i, j;
	//double*result = (double*)malloc(sizeof(double) * dim);
	double*result = new double[dim];

	for (i = dim - 1; i >= 0; i--) {
		result[i] = 0;

		for (j = dim - 1; j >= 0; j--) {
			result[i] += vector[j] * matrix[i][j];
		}
	}

	return(result);
}

double elliptic(double*x, int dim) {
	double result = 0.0;
	int    i;

	transform_osz(x, dim);

	for (i = 0; i < dim; i++)
	{
		// printf("%f %f %f\n",result, x[i], pow(1.0e6,  i/((double)(dim - 1)) ));
		result += pow(1.0e6, i / ((double)(dim - 1))) * x[i] * x[i];
	}
	return(result);
}

double ellipsoid(double* x,int dim){
	double result = 0.0;
	int i;
	for(i=0;i<dim;i++)
	{
		result += x[i] * x[i] * (i+1);
	}
	return result;
}

double rastrigin(double*x, int dim) {
	double sum = 0;
	int    i;


	// T_{osz}
	transform_osz(x, dim);

	// T_{asy}^{0.2}
	transform_asy(x, 0.2, dim);
	// lambda
	Lambda(x, 10, dim);

	for (i = dim - 1; i >= 0; i--) {
		sum += x[i] * x[i] - 10.0 * cos(2 * PI * x[i]) + 10.0;
	}
	return(sum);
}

double ackley(double*x, int dim) {
	double sum1 = 0.0;
	double sum2 = 0.0;
	double sum;
	int    i;

	// T_{osz}
	transform_osz(x, dim);

	// T_{asy}^{0.2}
	transform_asy(x, 0.2, dim);

	// lambda
	Lambda(x, 10, dim);

	for (i = dim - 1; i >= 0; i--) {
		sum1 += (x[i] * x[i]);
		sum2 += cos(2.0 * PI * x[i]);
	}

	sum = -20.0 * exp(-0.2 * sqrt(sum1 / dim)) - exp(sum2 / dim) + 20.0 + E;

	//cout<<sum1<<" "<<sum2<<" "<<sum<<endl;
	return(sum);
}

double schwefel(double*x, int dim) {
	int    j;
	double s1 = 0;
	double s2 = 0;

	// T_{osz}
	transform_osz(x, dim);

	// T_{asy}^{0.2}
	transform_asy(x, 0.2, dim);

	for (j = 0; j < dim; j++) {
		s1 += x[j];
		s2 += (s1 * s1);
	}

	return(s2);
}

double rosenbrock(double*x, int dim) {
	int    j;
	double oz, t;
	double s = 0.0;
	j = dim - 1;

	for (--j; j >= 0; j--) {
		oz = x[j + 1];
		t = ((x[j] * x[j]) - oz);
		s += (100.0 * t * t);
		t = (x[j] - 1.0);
		s += (t * t);
	}
	return(s);
}

double griewank(double* x,int dim){
	double res = 0;
	for(int i=0;i<dim;i++){
		res += x[i]*x[i]/4000;
	}
	double t = 1;
	for(int i=0;i<dim;i++){
		t = t*cos(x[i]/sqrt(i+1));
	}
	res -= t;
	res += 1;
	return res;
}

void transform_osz(double* z, int dim)
{
	// apply osz transformation to z
	for (int i = 0; i < dim; ++i)
	{
		double temp = sign(z[i]) * exp(hat(z[i]) + 0.049 * (sin(c1(z[i]) * hat(z[i])) + sin(c2(z[i])* hat(z[i]))));
		// cout<<fabs(z[i])<<" ";
		z[i]=temp;
	}
}

void transform_asy(double* z, double beta, int dim)
{
	for (int i = 0; i < dim; ++i)
	{
		if (z[i] > 0)
		{
			z[i] = pow(z[i], 1 + beta * i / ((double)(dim - 1)) * sqrt(z[i]));
		}
	}
}

void Lambda(double* z, double alpha, int dim)
{
	for (int i = 0; i < dim; ++i)
	{
		z[i] = z[i] * pow(alpha, 0.5 * i / ((double)(dim - 1)));
	}
}

int sign(double x)
{
	if (x > 0) return 1;
	if (x < 0) return -1;
	return 0;
}

double hat(double x)
{
	if (x == 0)
	{
		return 0;
	}
	else
	{
		return log(fabs(x));
	}
}

double c1(double x)
{
	if (x > 0)
	{
		return 10;
	}
	else
	{
		return 5.5;
	}
}

double c2(double x)
{
	if (x > 0)
	{
		return 7.9;
	}
	else
	{
		return 3.1;
	}
}

#endif