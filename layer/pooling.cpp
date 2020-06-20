/*
 * pooling.cu
 *
 *  Created on: Sep 20, 2015
 *      Author: lyx
 */

#include "pooling.h"

using namespace global;

namespace layer {

Pooling::Pooling(Layer* _prev, int size, int stride) {
	prev = _prev;
	prev->next = this;

	callCudnn(cudnnCreatePoolingDescriptor(&descriptor));
	callCudnn(cudnnSetPooling2dDescriptor(descriptor, 
		CUDNN_POOLING_MAX,
		CUDNN_NOT_PROPAGATE_NAN,
		size, 
		size,	
		0, 
		0, 
		stride, 
		stride));

	int _n, _c, _h, _w, _tmp;
	cudnnDataType_t _t;
	callCudnn(cudnnGetTensor4dDescriptor(prev->t_data, &_t, &_n, &_c, &_h, &_w, &_tmp,
			&_tmp, &_tmp, &_tmp));
	batch = _n;
	callCudnn(cudnnCreateTensorDescriptor(&t_data));
	callCudnn(cudnnSetTensor4dDescriptor(t_data, CUDNN_TENSOR_NCHW, CUDNN_DATA_FLOAT,
			_n, _c, _h / stride, _w / stride));
	data_size = _n * _c * (_h / stride) * (_w / stride);
	callCuda(cudaMalloc(&data, sizeof(float) * data_size));
	callCuda(cudaMalloc(&diff, sizeof(float) * prev->data_size));
	net_utils::setGpuValue(diff, prev->data_size, 0);

	param_size = 0;
	param_bias_size = 0;
}

Pooling::~Pooling() {
	callCudnn(cudnnDestroyPoolingDescriptor(descriptor));
	callCudnn(cudnnDestroyTensorDescriptor(t_data));
	callCuda(cudaFree(data));
	callCuda(cudaFree(diff));
}

void Pooling::forward(bool train) {
	float a = 1;
	float b = 0;
	callCudnn(cudnnPoolingForward(
		cudnnHandle,
		descriptor, 
		&a, 
		prev->t_data,
		prev->data, 
		&b, 
		t_data, 
		data));
}

void Pooling::backward() {
	float a = 1;
	float b = 0;
	callCudnn(cudnnPoolingBackward(
		cudnnHandle, 
		descriptor,
		&a,
		t_data, 
		data,
		t_data,
		next->diff,
		prev->t_data, 
		prev->data,
		&b, 
		prev->t_data, 
		diff));
}

void Pooling::update() {
	// nothing
}

}
