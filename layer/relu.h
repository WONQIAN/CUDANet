#ifndef RELU_CUH_
#define RELU_CUH_

#include "neuron.h"

namespace layer {

class ReLU: public Neuron {
public:
	ReLU(Layer* _prev, int _output_size, float dropout_rate, float alpha,
			float sigma = 0.01f, float momentum = 0.9f, float weight_decay = 0);
	virtual ~ReLU();
	void forward_activation();
	void backward_activation();

private:
	cudnnActivationDescriptor_t activation_descriptor;
};

} /* namespace layer */
#endif /* RELU_CUH_ */
