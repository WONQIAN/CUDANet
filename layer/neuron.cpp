#include "neuron.h"

using namespace global;

namespace layer {

Neuron::Neuron(Layer* _prev, int _output_size, float dropout_rate, float alpha,
		float sigma, float momentum, float weight_decay):
		Layer(alpha, momentum, weight_decay) {
	prev = _prev;
	prev->next = this;

	int _n, _c, _h, _w, _tmp;
	cudnnDataType_t _t;
	callCudnn(cudnnGetTensor4dDescriptor(
		prev->t_data,
		&_t, 
		&_n,
		&_c, 
		&_h,
		&_w, 
		&_tmp,
		&_tmp,
		&_tmp, 
		&_tmp));
	batch = _n;
	input_size = _c * _h * _w;
	output_size = _output_size;
	callCudnn(cudnnCreateTensorDescriptor(&t_data));
	callCudnn(cudnnSetTensor4dDescriptor(
		t_data, 
		CUDNN_TENSOR_NCHW, 
		CUDNN_DATA_FLOAT,
		batch, 
		output_size,
		1, 
		1));
	data_size = batch * output_size;
	callCuda(cudaMalloc(&data, sizeof(float) * data_size));
	callCuda(cudaMalloc(&tmp_data, sizeof(float) * data_size));
	callCuda(cudaMalloc(&diff, sizeof(float) * prev->data_size));
	callCuda(cudaMalloc(&tmp_diff, sizeof(float) * data_size));

	param_size = input_size * output_size;
	param_bias_size = output_size;
	callCuda(cudaMalloc(&param, sizeof(float) * param_size));
	callCuda(cudaMalloc(&param_bias, sizeof(float) * param_bias_size));
	callCuda(cudaMalloc(&gradient, sizeof(float) * param_size));
	callCuda(cudaMalloc(&gradient_bias, sizeof(float) * param_bias_size));

	net_utils::setGpuNormalValue(param, param_size, 0, sigma);   // input_size x output_size   ??????
	net_utils::setGpuNormalValue(param_bias, param_bias_size, 0, sigma); // output_size x 1 ??????
	net_utils::setGpuValue(gradient, param_size, 0);
	net_utils::setGpuValue(gradient_bias, param_bias_size, 0);

	callCuda(cudaMalloc(&one, sizeof(float) * batch));
	net_utils::setGpuValue(one, batch, 1);

	this->dropout_rate = dropout_rate;
}

Neuron::~Neuron() {
	callCudnn(cudnnDestroyTensorDescriptor(t_data));
	callCuda(cudaFree(data));
	callCuda(cudaFree(tmp_data));
	callCuda(cudaFree(diff));
	callCuda(cudaFree(tmp_diff));
	callCuda(cudaFree(param));
	callCuda(cudaFree(param_bias));
	callCuda(cudaFree(gradient));
	callCuda(cudaFree(gradient_bias));
	callCuda(cudaFree(one));
}

void Neuron::forward(bool train) {
	float a = 1;
	float b = 0;
	dropout(train);
	callCuda(cublasSgemm(
		cublasHandle,
		CUBLAS_OP_T, 
		CUBLAS_OP_N,
		output_size, // m
		batch,  // n
		input_size,  // k
		&a, 
		param, input_size,  // input_size x output_size
		prev->data, input_size,  // input_size x batch
		&b,
		tmp_data, output_size));  // m x n
	callCuda(cublasSgemm(
		cublasHandle, 
		CUBLAS_OP_N, 
		CUBLAS_OP_N, 
		output_size,  // m 
		batch,  // n
		1,  // k
		&a, 
		param_bias, output_size,  // output_size x 1
		one, 1,	 // 1 x batch
		&a,	
		tmp_data, output_size));
	forward_activation();
}

void Neuron::backward() {
	//float a = alpha; // learning rate
	float a = 1; // learning rate
	//float b = momentum;
	float b = 0;

	backward_activation();
	callCuda(cublasSgemm(
		cublasHandle, 
		CUBLAS_OP_N, 
		CUBLAS_OP_T, 
		input_size,
		output_size, 
		batch, 
		&a, 
		prev->data, input_size, 
		tmp_diff, output_size,
		&b, 
		gradient, input_size));
	callCuda(cublasSgemv(
		cublasHandle, 
		CUBLAS_OP_N, 
		output_size, 
		batch,
		&a, 
		tmp_diff, output_size, 
		one, 1, 
		&b, 
		gradient_bias, 1));
	a = 1;
	b = 0;
	callCuda(cublasSgemm(
		cublasHandle, 
		CUBLAS_OP_N,
		CUBLAS_OP_N, 
		input_size,
		batch,
		output_size,
		&a, 
		param, input_size,
		tmp_diff, output_size,
		&b, 
		diff, input_size));
}

void Neuron::update() {
	float a = 1 - weight_decay;
	callCuda(cublasSaxpy(
		cublasHandle, param_size, 
		&a, 
		gradient, 1, 
		param, 1));
	callCuda(cublasSaxpy(
		cublasHandle, param_bias_size,	
		&a,
		gradient_bias, 1, 
		param_bias, 1));
}

void Neuron::dropout(bool train) {
	if (train)
		net_utils::dropGpuValue(prev->data, prev->data_size, dropout_rate);
	else
		net_utils::scaleGpuValue(prev->data, prev->data_size, 1 - dropout_rate);
}

}
