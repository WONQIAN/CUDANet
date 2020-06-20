#include "PFEAN.h"

namespace PFEAN
{
	PFEANet::PFEANet(float* _h_data, int _h, int _w, int _c, int _batch, int _data_size) : GAN()
	{
		data_dim = _h * _w *_c;
		batch = _batch;
		size = _data_size;
		h_data = _h_data;

		callCuda(cudaMalloc(&d_data, sizeof(float) * data_dim * batch));
		// ��������ع����ԣ���ǩ����ԭʼ��������
		callCuda(cudaMalloc(&d_label, sizeof(float) * data_dim * batch));
	}

	PFEANet::~PFEANet()
	{

	}
}