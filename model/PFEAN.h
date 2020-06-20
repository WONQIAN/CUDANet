/*
* PFEAN.h
*
* prior guided feature encodere adversarial network
*
*  Created on: 1 11, 2020
*      Author: wanqian
*/

#ifndef _PFEAN_H_
#define _PFEAN_H_

#include "gan.h"
#include "../utils/image_priors.h"

#define PRIOR_NUM 3

namespace PFEAN
{
	using namespace net_utils;
	using namespace model;

	class PriorExtractor : public Prior
	{
	public:
		PriorExtractor() : Prior() {};
		~PriorExtractor() {};
	};


	class PFEANet : public GAN 
	{
	public:

		PFEANet(float* _h_data, int _h, int _w, int _c, int _batch, int _data_size);

		~PFEANet();

	public:
		int size; // ѵ�����ݵ���������
		int batch; // batch��С
		int data_dim; // ��������ά�� HxWxC
		float* h_data; // host ����ͼ������
		float* d_data; // device �������ݣ� ����ԭͼ��������ȡ��Ϣ Bx256x256x4
		float* d_label; // device �������ݱ�ǩ
	};
}

#endif // !_PFEAN_H_
