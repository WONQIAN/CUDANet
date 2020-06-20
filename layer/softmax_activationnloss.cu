#include "softmax_activationnloss.h"

using namespace global;

namespace layer {

	SoftmaxAnL::SoftmaxAnL(Layer* _prev, float* _label, int _class_num, int _batch) : Layer() 
	{
		prev = _prev;
		prev->next = this;

		label = _label; // gpu data
		class_num = _class_num;
		batch = _batch;
		data_size = batch; // �����С
		param_size = 0;
		param_bias_size = 0;

		callCudnn(cudnnCreateTensorDescriptor(&t_data));
		callCudnn(cudnnSetTensor4dDescriptor(
			t_data,
			CUDNN_TENSOR_NCHW,
			CUDNN_DATA_FLOAT,
			batch,
			class_num,
			1,
			1));

		callCuda(cudaMalloc(&tmp_data, sizeof(float) * prev->data_size)); // ���softmax����ĸ���ֵa
		callCuda(cudaMalloc(&data, sizeof(float) * 1)); // Loss
		callCuda(cudaMalloc(&diff, sizeof(float) * prev->data_size)); // diff 
		callCuda(cudaMalloc(&predict_label, sizeof(float) * data_size)); // �˴����ڴ�� predict
	}

	SoftmaxAnL::~SoftmaxAnL() 
	{
		callCudnn(cudnnDestroyTensorDescriptor(t_data));

		callCuda(cudaFree(tmp_data));
		callCuda(cudaFree(data));
		callCuda(cudaFree(diff));
		callCuda(cudaFree(predict_label));
	}


	__global__ void corssEntropyLoss(float *softmax_output_a, float *label, int class_num, int batch, float *predict_label, float *loss)
	{
		int idx = blockIdx.x * blockDim.x + threadIdx.x;
		if (idx < batch)
		{
			int label_value = 0;
			float max = -1;
			for (int i = 0; i < class_num; i++) {
				if (softmax_output_a[idx * class_num + i] > max) {
					max = softmax_output_a[idx * class_num + i];
					label_value = i;
				}
			}
			predict_label[idx] = (float)label_value;

			atomicAdd(loss, -log(softmax_output_a[idx * class_num + (int)label[idx]]));
		}
	}

	// ���㽻������ʧ��softmax�������ݣ�δ��һ�����ĵ�����diff = f(zl)-1�������Ҫ�Ƚ�softmax���������ֵ��a������diff������labelλ�ü�1
	__global__ void softmaxDiff(const float *label, int class_num, int batch, float *diff)
	{
		int idx = blockIdx.x * blockDim.x + threadIdx.x;
		if (idx < batch)
		{
			const int label_value = static_cast<int>(label[idx]);
			diff[idx * class_num + label_value] -= 1.0f; // ��zi��
		}
	}

	void SoftmaxAnL::forward(bool train)
	{
		float a = 1;
		float b = 0;
		callCudnn(cudnnSoftmaxForward(
			cudnnHandle,
			CUDNN_SOFTMAX_FAST,
			CUDNN_SOFTMAX_MODE_CHANNEL,
			&a,
			t_data,
			prev->data,
			&b,
			t_data,
			tmp_data));

		net_utils::setGpuValue(data, 1, 0); // loss = 0
		corssEntropyLoss <<< (batch + 127) / 128, 128 >>> (tmp_data, label, class_num, batch, predict_label, data);
	}

	void SoftmaxAnL::backward()
	{
		callCuda(cudaMemcpy(diff, tmp_data, sizeof(float) * prev->data_size, cudaMemcpyDeviceToDevice));
		softmaxDiff <<< (batch + 127) / 128, 128 >>> (label, class_num, batch, diff);
	}

	void SoftmaxAnL::update()
	{
		//#                        .::::.
		//#                      .::::::::.
		//#                     :::::::::::
		//#                  ..:::::::::::'
		//#               '::::::::::::'
		//#                 .::::::::::
		//#            '::::::::::::::..
		//#                 ..::::::::::::.
		//#               ``::::::::::::::::
		//#                ::::``:::::::::'        .:::.
		//#               ::::'   ':::::'       .::::::::.
		//#             .::::'      ::::     .:::::::'::::.
		//#            .:::'       :::::  .:::::::::' ':::::.
		//#           .::'        :::::.:::::::::'      ':::::.
		//#          .::'         ::::::::::::::'         ``::::.
		//#      ...:::           ::::::::::::'              ``::.
		//#     ```` ':.          ':::::::::'                  ::::..
		//#                        '.:::::'                    ':'````..
		//#                     ��Ů���� ����BUG
	}
}
