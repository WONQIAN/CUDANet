#pragma once

#ifndef _COMMON_FUNCS_GPU_
#define _COMMON_FUNCS_GPU_

#include <ctime>
#include <cmath>
#include <iostream>

#include "cuda_runtime.h"
#include "cublas_v2.h"
#include "curand.h"

#include "utils.h"
#include "global.h"
#include "print.h"

#define ThreadsPerBlock_16 16
#define ThreadsPerBlock_32 32

namespace func_gpu {

	/*
	* ���ܣ����������ĵ��� y[i] = aplha / x[i]
	* ���룺x	�������׵�ַ
	* ���룺n	��������
	* ���룺aplha	��ϵ��

	* �����y	����R���׵�ַ
	*/
	void inverseGpu(const float* __restrict__ x, int n, float alpha, float* __restrict__ y);

	/*
	* ���ܣ����������� y[i] = aplha*x[i]^p + b
	* ���룺x	����X���׵�ַ
	* ���룺n	��������
	* ���룺p	������
	* ���룺aplha	��ϵ��
	* ���룺b	��ϵ��

	* �����y	ָ��������������ָ��
	*/
	void powGpu(const float* __restrict__  x, int n, int p, float alpha, float b, float* __restrict__ y);

	/*
	* ���ܣ���������log y[i] = aplha*log(x[i]) + b
	* ���룺x	����X���׵�ַ
	* ���룺n	��������
	* ���룺p	������
	* ���룺aplha	��ϵ��
	* ���룺b	��ϵ��

	* �����y	ָ��������������ָ��
	*/
	void logGpu(const float* __restrict__  x, int n, float alpha, float b, float* __restrict__  y);

	/*
	* ���ܣ����������뵥���� ponitwise ���� y[i] = x[i]*alpha + b; 
	* ���룺x	�������׵�ַ
	* ���룺n	��������
	* ���룺aplha	����
	* ���룺b	������

	* �����y	����R���׵�ַ
	*/
	void processElementGpu(const float* __restrict__  x, int n, float alpha, float b, float* __restrict__  y);

	/*
	* ���ܣ������������� z[i] = a*x[i] + b*y[i] + c
	* ���룺n	��������
	* ���룺x	����x���׵�ַ
	* ���룺a	x����
	* ���룺y	����y���׵�ַ
	* ���룺b	y����
	* ���룺c	��ϵ��

	* �����z	�������z���׵�ַ
	*/
	void vectorAddGpu(int n, const float* __restrict__ x, float a, const float* __restrict__ y, float b, float c, float* __restrict__ z);

	/*
	* ���ܣ���������������Ԫ����� z[i] = alpha*x[i]/y[i] + b;
	* ���룺x	����x���׵�ַ
	* ���룺y	����y���׵�ַ
	* ���룺n	��������
	* ���룺aplha	��ϵ��
	* ���룺b	��ϵ��

	* �����z	����z���׵�ַ
	*/
	void vectordivideElemGpu(const float* __restrict__ x, const float* __restrict__ y, int n, float alpha, float b, float* __restrict__ z);

	/*
	* ���ܣ���������������Ԫ����� z[i] = alpha*x[i]*y[i] + b;
	* ���룺x	����x���׵�ַ
	* ���룺y	����y���׵�ַ
	* ���룺n	��������
	* ���룺aplha	��ϵ��
	* ���룺b	��ϵ��

	* �����z	����z���׵�ַ
	*/
	void vectorMultiplyElemGpu(const float* __restrict__ x, const float* __restrict__ y, int n, float alpha, float b, float* __restrict__ z);

	/*
	* ���ܣ������������ж�ӦԪ����� z[i][j] = x[i][j] / y[i]
	* ���룺x	����x���׵�ַ		ά�� n = rows*cols
	* ���룺n	����x����
	* ���룺y	����y���׵�ַ		ά�� 1*cols
	* ���룺rows		����x����
	* ���룺clos		����x����

	* �����z	����z���׵�ַ		ά�� n = rows*cols
	*/
	void  divideElembyRowGpu(const float* __restrict__ x, int n, const float* __restrict__ y, int rows, int clos, float* __restrict__ z);

	/*
	* ���ܣ������������ж�ӦԪ����� z[i][j] = x[i][j] / y[j]
	* ���룺x	����x���׵�ַ		ά�� n = rows*cols
	* ���룺n	����x����
	* ���룺y	����y���׵�ַ		ά�� rows*1
	* ���룺rows		����x����
	* ���룺clos		����x����

	* �����z	����z���׵�ַ		ά�� n = rows*cols
	*/
	void  divideElembyColGpu(const float* __restrict__ x, int n, const float* __restrict__ y, int rows, int clos, float* __restrict__ z);


	/*
	* ���ܣ�������x���� ���ȳ�����y��Ȩ	z[i] = x[i] * y[i/step]
	* ���룺x	����A���׵�ַ		ά�� (rows*step) * cols
	* ���룺xN	����x����
	* ���룺y	����y���׵�ַ	ά�� rows * cols
	* ���룺yN	����y����

	* �����z	����z���׵�ַ		ά�� n = rows*cols
	*/
	void asyMultiplyElemGpu(const float* __restrict__ x, int xN, const float* __restrict__ y, int yN, int step, float* z);

	/*
	* ���ܣ���������feature������center�������в�residual
	* ���룺fDim		feature��center������ά��
	* ���룺feature	����feature���׵�ַ ά�� fDim * feaNum
	* ���룺feaNum	feature�����ĸ���
	* ���룺center	����center���׵�ַ ά�� fDim * CenNum
	* ���룺cenNum	center�����ĸ���

	* �����residual		����в�residual���׵�ַ ά��  fDim * feaNum * CenNum
	*/
	void encodeResidualGpu(int fDim, const float* __restrict__ feature, int feaNum, const float* __restrict__ center, int cenNum, float* __restrict__ residual);

	/*
	* ���ܣ���������feature������center�������distance
	* ���룺fDim		feature��center������ά��
	* ���룺feature	����feature���׵�ַ ά�� fDim * feaNum
	* ���룺feaNum	feature�����ĸ���
	* ���룺center	����center���׵�ַ ά�� fDim * CenNum
	* ���룺cenNum	center�����ĸ���

	* �����distance		�������distance���׵�ַ ά��  feaNum * CenNum
	*/
	void encodeDistanceGpu(int fDim, const float* __restrict__ feature, int feaNum, const float* __restrict__ center, int cenNum, float* __restrict__ distance);

	/*
	* ���ܣ���������x������y��KLɢ�Ⱦ��� z[i] = x[i] * log(x[i] / y[i]);
	* ���룺x	����x���׵�ַ
	* ���룺y	����y���׵�ַ
	* ���룺n	��������

	* �����z	����z���׵�ַ
	*/
	void divergKLGpu(const float* __restrict__ x, const float* __restrict__ y, int n, float* z);

	/*
	* ���ܣ�adam�Ż����ݶȸ��²��� gradient_update[i] = m1[i]/(1-��1^t) / sqrt(m2[i]/(1-��2^t) + epsilon) ;
	* ���룺m1	һ�׶������׵�ַ
	* ���룺m2	���׶������׵�ַ
	* ���룺n	��������
	* ���룺iter		��������
	* ���룺beta1	adam�Ż�ϵ����1 0.99
	* ���룺beta2	adam�Ż�ϵ����2 0.999
	* ���룺epsilon	��ֹ��ĸΪ0

	* �����gradient_update	�ݶȸ��µ��׵�ַ
	*/
	void adamUpdateGpu(const float* m1, const float* m2, const int n, const int iter, const float beta1, const float beta2, const float epsilon, float* gradient_update);
}

#endif