#ifndef SOFTMAX_anl_CUH_
#define SOFTMAX_anl_CUH_

#include "layer.h"

namespace layer {

	class SoftmaxAnL : public Layer {
	public:
		float* tmp_data; // // ���softmax����ĸ���ֵa
		float* label;	// real label
		float* predict_label;	// predict label
		int class_num;	// eg. 10 for digit classfication

	public:
		SoftmaxAnL(Layer* _prev, float* _label, int _class_num, int _batch);
		virtual ~SoftmaxAnL();

		void forward(bool train = true);
		void backward();
		void update();
	};

} /* namespace layer */
#endif /* SOFTMAX_anl_CUH_ */