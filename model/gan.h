/*
class classification inherits from Network
for classification model
20200103
created by wanqian
*/

#pragma once
#ifndef _GAN_H_
#define _GAN_H_

#include "../utils/image.h"
#include "network.h"
#include "reconstruction.h"
#include "../optimizer/adam_optimizer.h"
#include "../optimizer/sgd_optimizer.h"

namespace model
{
	class GAN
	{
	public:
		GAN() {};
		GAN(float* _data, int _data_dim, int _train_size, int _batch);

		int batch;
		int train_size;		// ѵ��ͼƬ��
		int data_dim;		// ��������ά�� H*W*C
		int label_dim;		// ������ǩά�����˴�ͬ�������ݣ� H*W*C(�ع�)

		// cpu
		float* h_data;			// ����ԭʼͼ������
		float* reconstruction_data;
		float* dis_label_h1;
		float* dis_label_h0;
		// gpu
		float* data;			// һ��batch��ԭʼ����
		float* rec_data;		// �������ع�����
		float* rec_label;		// ԭʼ�ع����ݱ�ǩ
		//float* dis_label;		// �����ǩ
		float* ONE;				// 1��ǩ
		float* ZERO;			// 0��ǩ

		float* hybrid_data;		// �б���ѵ��ʱ�Ļ������
		float* hybrid_label;	// �б���ѵ��ʱ�Ļ�ϱ�ǩ

		Reconstruction* generator;
		Network* discrimintor;

	private:
		// cpu
		float dis_loss_1;
		float dis_loss_0;

		float gen_loss_rec;
		float gen_loss_1;
		// gpu
		float* dis_diff_1;			// ����Ϊ softmax ������ݵĳ��ȣ���Ϊ batch*class_num(2)
		float* dis_diff_0;

		float* gen_diff_rec;		// �������ع����ݵ���
		float* gen_diff_1;			// �б������������ݷ��򴫲����������ݴ��ĵ��������ߵĳ��Ⱦ�Ϊ�ع����ݵĳ���


	public:
		~GAN();

		void TrainDis(optimizer::SGD sgd, optimizer::Adam adam, int adam_iter);
		void TrainGen(optimizer::SGD sgd, optimizer::Adam adam, int adam_iter);

		void Train(std::string para_path, int iteration, float step_decrease, bool debug);
		void Test();

		void SaveParas(std::string _para_path);
		void ReadGenParas(std::string _para_path);
		void ReadDisParas(std::string _para_path);
		void ReadParas(std::string _para_path);

	};

}

#endif // !_GAN_H_
