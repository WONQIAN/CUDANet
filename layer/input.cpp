/*
 * input.cu
 *
 *  Created on: Sep 20, 2019
 *      Author: wanqian
 */

#include "input.h"

namespace layer {

Input::Input(int n, int c, int h, int w, float* _data) : Layer() {
	prev = NULL;

	batch = n;
	callCudnn(cudnnCreateTensorDescriptor(&t_data));
	callCudnn(cudnnSetTensor4dDescriptor(
		t_data,
		CUDNN_TENSOR_NCHW,	
		CUDNN_DATA_FLOAT,
		n, c, h, w));
	data_size = n * c * h * w;
	data = _data;

	callCuda(cudaMalloc(&diff, sizeof(float) * data_size)); // �ݶȻش���Ҫ�����ݿռ�

	param_size = 0;
	param_bias_size = 0;
}

Input::~Input() {
	callCudnn(cudnnDestroyTensorDescriptor(t_data));
	data = NULL;
}

void Input::forward(bool train) {
	// nothing
}

void Input::backward() {
	// nothing
}

void Input::update() {
	// nothing
}

}
