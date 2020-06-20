#pragma once
#ifndef BRANCH_CUH_
#define BRANCH_CUH_

#include "layer.h"
#include "dec.h"
//#include ".././utils/set_value.h"

using namespace global;

namespace layer {

	class Cluster : public Layer {
	private:
		int cenNum;
		int dataDim;
		int feaNum; // ��������
		int feaDim; // ����ά��

		float weight;
		/*float* cluster_diff;*/
		cudnnTensorDescriptor_t fea_discriptor;
		float* fea_data; // NHWC��ʽ
		float* fea_diff; // NHWC��ʽ
		float cluster_loss = 0;

		DEC* dec;

	public:
		Cluster(Layer* _prev, int K, float cluster_weight, float alpha, float* init_param = NULL,
			float sigma = 0.01f, float momentum = 0.9f, float weight_decay = 0);
		virtual ~Cluster();
		void forward(bool train = true);
		void backward();
		void update();
	};

} /* namespace layer */
#endif /* BRANCH_CUH_ */
