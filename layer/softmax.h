#ifndef SOFTMAX_CUH_
#define SOFTMAX_CUH_

#include "neuron.h"

namespace layer {

class Softmax: public Neuron {
public:
	Softmax(Layer* _prev, int _output_size, float dropout_rate, float alpha,
			float sigma = 0.01f, float momentum = 0.9f, float weight_decay = 0);
	virtual ~Softmax();
	void forward_activation();
	void backward_activation();
};

} /* namespace layer */
#endif /* SOFTMAX_CUH_ */
