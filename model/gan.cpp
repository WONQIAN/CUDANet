#include "gan.h"

namespace model
{
	GAN::GAN(float* _data, int _data_dim, int _train_size, int _batch)
	{
		batch = _batch;
		train_size = _train_size;
		data_dim = _data_dim;
		label_dim = _data_dim; // �˴�Ϊͼ��ά�� h*w

		// ���ݷ���ռ�
		h_data = new float[data_dim * train_size];
		h_data = _data;
		callCuda(cudaMalloc(&data, sizeof(float) * data_dim * batch));
		callCuda(cudaMalloc(&rec_data, sizeof(float) * data_dim * batch));
		// ��ǩ����ռ�
		callCuda(cudaMalloc(&rec_label, sizeof(float) * data_dim * batch));
		//callCuda(cudaMalloc(&dis_label, sizeof(float) * batch));
		callCuda(cudaMalloc(&ONE, sizeof(float) * batch));
		callCuda(cudaMalloc(&ZERO, sizeof(float) * batch));
		net_utils::setGpuValue(ONE, batch, 1);
		net_utils::setGpuValue(ZERO, batch, 0);
		// ���ݵ�������ռ�
		callCuda(cudaMalloc(&dis_diff_1, sizeof(float) * 2 * batch));
		callCuda(cudaMalloc(&dis_diff_0, sizeof(float) * 2 * batch));
		callCuda(cudaMalloc(&gen_diff_rec, sizeof(float) * data_dim * batch));
		callCuda(cudaMalloc(&gen_diff_1, sizeof(float) * data_dim * batch));

		generator = new Reconstruction(batch);
		generator->data_dim = data_dim; // label_dim = data_dim
		callCuda(cudaMalloc(&generator->data, sizeof(float) * data_dim * batch));
		callCuda(cudaMalloc(&generator->label, sizeof(float) * data_dim * batch));
		/*generator->data = data;
		generator->label = data;*/

		//callCuda(cudaMalloc(&hybrid_data, sizeof(float) * data_dim  * batch));
		//callCuda(cudaMalloc(&hybrid_label, sizeof(float) * batch));
		//callCuda(cudaMemcpy(hybrid_label, ONE, sizeof(float) * batch / 2, cudaMemcpyDeviceToDevice));
		//callCuda(cudaMemcpy(hybrid_label + (batch / 2), ZERO, sizeof(float) * batch / 2, cudaMemcpyDeviceToDevice));

		discrimintor = new Network(batch);
		discrimintor->data_dim = data_dim; // label_dim = data_dim
		//discrimintor->data = data;
		callCuda(cudaMalloc(&discrimintor->data, sizeof(float) * data_dim * batch));
		discrimintor->label = ONE;

		dis_loss_1 = 0.0f;
		dis_loss_0 = 0.0f;
		gen_loss_rec = 0.0f;
		gen_loss_1 = 0.0f;
	}

	void GAN::Train(std::string para_path, int iteration, float step_decrease, bool debug)
	{
		// train the network multiple times
		reconstruction_data = new float[data_dim * train_size];

		int batchNum = train_size / batch;
		std::vector<int> vDataIndex;
		for (int i = 0; i < train_size; ++i) vDataIndex.push_back(i); // offset += batch

		//using namespace optimizer;
		int GLOBAL_ADAMP_ITERA = 1;
		optimizer::Adam adam(-1e-3, 0.0005f, 0.9, 0.999, 1e-8);
		optimizer::SGD sgd(-1, 0.0005f);

		ReadParas("C:\\Users\\ms952\\Desktop\\lcd_gan_paras\\");

		for (int k = 1; k <= iteration; k++)
		{
			dis_loss_1 = 0.0f;
			dis_loss_0 = 0.0f;
			gen_loss_rec = 0.0f;
			gen_loss_1 = 0.0f;

			std::cout << "Iteration " << k << std::endl;

			//if (debug)
			//{
			//	std::cout << "discrimintor: " << std::endl;
			//	for (int i = discrimintor->layers.size() - 1; i > 0; i--) {
			//		if (discrimintor->layers[i]->param_size != 0)
			//			net_utils::printGpuMax(discrimintor->layers[i]->param, discrimintor->layers[i]->param_size);
			//	}
			//	std::cout << std::endl;

			//	std::cout << "generator: " << std::endl;
			//	for (int i = generator->layers.size() - 1; i > 0; i--)
			//	{
			//		if (generator->layers[i]->param_size != 0)
			//			net_utils::printGpuMax(generator->layers[i]->param, generator->layers[i]->param_size);
			//	}
			//	std::cout << std::endl;
			//}

			int offset = 0;
			std::random_shuffle(vDataIndex.begin(), vDataIndex.end());

			for (int b = 0; b < batchNum; ++b)
			{
				// 0��ѡ��һ�� batch �� data �� label
				for (int id = 0; id < batch; ++id)
				{
					callCuda(cudaMemcpy(generator->data + data_dim * id, h_data + (vDataIndex.at(offset + id)) * data_dim,
						sizeof(float) * data_dim, cudaMemcpyHostToDevice));
					callCuda(cudaMemcpy(generator->label + data_dim * id, h_data + (vDataIndex.at(offset + id)) * data_dim,
						sizeof(float) * data_dim, cudaMemcpyHostToDevice));

				}

				// 1������ generator ǰ�򴫲�һ�Σ� ������һ���ع�����
				for (int n = 1; n < generator->layers.size(); n++)
					generator->layers[n]->forward(false);
				rec_data = generator->layers[generator->layers.size() - 2]->data;

				// 2��ѵ���б��� �� �б�����Ϊ����ģʽ��һ�Ƕ������б� �������ؼ��б����ޣ��� �����붼��ԭͼ��Сһ�µ����ݣ�ֻ���б����������ʧ��һ��
				TrainDis(sgd, adam, GLOBAL_ADAMP_ITERA);

				// 3��ѵ�������� �� ��ʱ���ع�������п��ţ��������������Ǵ�Сһ�µ�ͼ��
				TrainGen(sgd, adam, GLOBAL_ADAMP_ITERA);

				offset += batch;
			}
			GLOBAL_ADAMP_ITERA++;

			// �����ع��ĵ�һ��ͼ��
			callCuda(cudaMemcpy(reconstruction_data, generator->layers[generator->layers.size() - 2]->data, sizeof(float) * data_dim * batch, cudaMemcpyDeviceToHost));
			std::string str = "C:\\Users\\ms952\\Desktop\\lcd\\" + std::to_string(k) + "_rec.bmp";
			net_utils::saveImage(str, reconstruction_data, 256, 256, 1, 0);
			// �����һ��ѵ��ԭͼ
			callCuda(cudaMemcpy(reconstruction_data, ((EuclideanLoss*)generator->layers[generator->layers.size() - 1])->label,
				sizeof(float) * data_dim * batch, cudaMemcpyDeviceToHost));
			net_utils::saveImage("C:\\Users\\ms952\\Desktop\\lcd\\" + std::to_string(k) + "_label.bmp", reconstruction_data, 256, 256, 1, 0);

			float dis_loss = (dis_loss_0 + dis_loss_1) / (batch * batchNum);
			float gen_loss = (gen_loss_rec + gen_loss_1) / (batch * batchNum);
			//std::cout << "The loss of discrimintor is " << dis_loss << " ";
			//std::cout << "The loss of generator is " << gen_loss << std::endl;
			std::cout << "dis_loss: " << dis_loss << ";  dis_loss_0: " << dis_loss_0 / (batch * batchNum) 
				<< ";  dis_loss_1: " << dis_loss_1 / (batch * batchNum) << std::endl;
			std::cout << "gen_loss: " << gen_loss << ";  gen_loss_rec: " << gen_loss_rec / (batch * batchNum) 
				<< ";  gen_loss_1: " << gen_loss_1 / (batch * batchNum) << std::endl;

			SaveParas("C:\\Users\\ms952\\Desktop\\lcd_gan_paras\\");
		}
		
	}

	void GAN::TrainDis(optimizer::SGD sgd, optimizer::Adam adam, int adam_iter)
	{
		/*************************************************************************************************/
		// ѵ���б���
		//((SoftmaxLoss*)(discrimintor->layers[discrimintor->layers.size() - 1]))->label = hybrid_label;

		//callCuda(cudaMemcpy((discrimintor->data), data, sizeof(float) * data_dim * batch / 2, cudaMemcpyDeviceToDevice));
		//callCuda(cudaMemcpy((discrimintor->data) + data_dim * batch / 2, rec_data, sizeof(float) * data_dim * batch / 2, cudaMemcpyDeviceToDevice));

		//((SoftmaxAnL*)(discrimintor->layers[discrimintor->layers.size() - 1]))->label = ONE;
		callCuda(cudaMemcpy(((SoftmaxAnL*)(discrimintor->layers[discrimintor->layers.size() - 1]))->label, ONE, sizeof(float) * batch, cudaMemcpyDeviceToDevice));
		callCuda(cudaMemcpy((discrimintor->data), data, sizeof(float) * data_dim * batch, cudaMemcpyDeviceToDevice));
		
		for (int i = 0; i < discrimintor->layers.size(); i++)
			discrimintor->layers[i]->forward(true);
		
		// ������ʧ dis_loss_1
		float temp_loss1 = 0.0f;
		callCuda(cudaMemcpy(&temp_loss1, discrimintor->layers[discrimintor->layers.size() - 1]->data,
			sizeof(float) * 1, cudaMemcpyDeviceToHost));
		dis_loss_1 += temp_loss1;

		for (int i = discrimintor->layers.size() - 1; i > 0; i--) {
			discrimintor->layers[i]->backward();
			//discrimintor->layers[i]->update();
			adam.optimize(discrimintor->layers[i], adam_iter);
		}
		
		//std::cout << "ORI input label(all 1):" << std::endl;
		//net_utils::printGpuMatrix(((SoftmaxAnL*)discrimintor->layers[discrimintor->layers.size() - 1])->label, 16, 1, 16); // checked
		//std::cout << "ORI softmax input z:" << std::endl;
		//net_utils::printGpuMatrix(discrimintor->layers[discrimintor->layers.size() - 1]->prev->data, 32, 16, 2);
		//std::cout << "ORI softmax output a:" << std::endl;
		//net_utils::printGpuMatrix(((SoftmaxAnL*)discrimintor->layers[discrimintor->layers.size() - 1])->tmp_data, 32, 16, 2);
		//std::cout << "ORI loss:" << std::endl;
		//net_utils::printGpuMatrix(discrimintor->layers[discrimintor->layers.size() - 1]->data, 1, 1);

		/*************************************************************************************************/
		//callCuda(cudaMemcpy((discrimintor->data), data + (data_dim * batch / 2), sizeof(float) * data_dim * batch / 2, cudaMemcpyDeviceToDevice));
		//callCuda(cudaMemcpy((discrimintor->data) + data_dim * batch / 2, rec_data + (data_dim * batch / 2), sizeof(float) * data_dim * batch / 2, cudaMemcpyDeviceToDevice));

		//((SoftmaxAnL*)(discrimintor->layers[discrimintor->layers.size() - 1]))->label = ZERO;
		callCuda(cudaMemcpy(((SoftmaxAnL*)(discrimintor->layers[discrimintor->layers.size() - 1]))->label, ZERO, sizeof(float) * batch, cudaMemcpyDeviceToDevice));
		callCuda(cudaMemcpy((discrimintor->data), rec_data, sizeof(float) * data_dim * batch, cudaMemcpyDeviceToDevice));

		for (int i = 0; i < discrimintor->layers.size(); i++)
			discrimintor->layers[i]->forward(true);

		// ������ʧ dis_loss_0
		float temp_loss0 = 0.0f;
		callCuda(cudaMemcpy(&temp_loss0, discrimintor->layers[discrimintor->layers.size() - 1]->data,
			sizeof(float) * 1, cudaMemcpyDeviceToHost));
		dis_loss_0 += temp_loss0;

		for (int i = discrimintor->layers.size() - 1; i > 0; i--) {
			discrimintor->layers[i]->backward();
			//discrimintor->layers[i]->update();
			adam.optimize(discrimintor->layers[i], adam_iter);
		}


		//std::cout << "REC input label(all 0):" << std::endl;
		//net_utils::printGpuMatrix(((SoftmaxAnL*)discrimintor->layers[discrimintor->layers.size() - 1])->label, 16, 1, 16); // checked
		//std::cout << "REC softmax input z:" << std::endl;
		//net_utils::printGpuMatrix(discrimintor->layers[discrimintor->layers.size() - 1]->prev->data, 32, 16, 2);
		//std::cout << "REC softmax output a:" << std::endl;
		//net_utils::printGpuMatrix(((SoftmaxAnL*)discrimintor->layers[discrimintor->layers.size() - 1])->tmp_data, 32, 16, 2);
		//std::cout << "REC loss:" << std::endl;
		//net_utils::printGpuMatrix(discrimintor->layers[discrimintor->layers.size() - 1]->data, 1, 1);

		//std::cout << "b dis_loss: " << (temp_loss1 + temp_loss0) / batch << ";  b dis_loss_1: " << temp_loss1 / batch
		//	<< ";  b dis_loss_0: " << temp_loss0 / batch << std::endl;
		//std::cout << std::endl;
	}

	void GAN::TrainGen(optimizer::SGD sgd, optimizer::Adam adam, int adam_iter)
	{
		/*************************************************************************************************/
		// ѵ��������
		// 1��data  ����  generator ǰ�򴫲�����Ӧ���Ϊ rec_data�� ������ʧ gen_loss_rec, ��ʧ���򴫲����� gen_diff_rec
		for (int n = 1; n < generator->layers.size(); n++)
			(generator->layers[n])->forward(true);
		rec_data = generator->layers[generator->layers.size() - 2]->data;
		
		gen_loss_rec += (*(generator->layers[generator->layers.size() - 1]->data)) / (float)data_dim; // δ����batch�������ܵ�ѵ�������г���������

		// ����gen_diff_rec
		generator->layers[generator->layers.size() - 1]->backward();
		gen_diff_rec = generator->layers[generator->layers.size() - 1]->diff;

		/*************************************************************************************************/
		// 2��rec_data ���� discriminator ǰ�򴫲�����Ӧ���Ϊ 1�� ������ʧ gen_loss_1, ��ʧ���򴫲����� gen_diff_1
		
		//((SoftmaxLoss*)(discrimintor->layers[discrimintor->layers.size() - 1]))->label = ONE;
		callCuda(cudaMemcpy(((SoftmaxAnL*)(discrimintor->layers[discrimintor->layers.size() - 1]))->label, ONE, sizeof(float) * batch, cudaMemcpyDeviceToDevice));
		callCuda(cudaMemcpy((discrimintor->data), rec_data, sizeof(float) * data_dim * batch, cudaMemcpyDeviceToDevice));

		for (int i = 0; i < discrimintor->layers.size(); i++)
			discrimintor->layers[i]->forward(false);

		// ������ʧ gen_loss_1
		float temp_loss = 0.0f;
		callCuda(cudaMemcpy(&temp_loss, discrimintor->layers[discrimintor->layers.size() - 1]->data,
			sizeof(float) * 1, cudaMemcpyDeviceToHost));
		gen_loss_1 += temp_loss;
		// discriminator ���򴫲��������²����������� gen_diff_1
		for (int i = discrimintor->layers.size() - 1; i > 0; i--)
			discrimintor->layers[i]->backward();
		gen_diff_1 = discrimintor->layers[1]->diff; // �ڶ���Ϊ��һ������㣬�� diff Ϊ�ع����ݵĵ���

		/*************************************************************************************************/
		// 3������ gen_diff_rec �� gen_diff_1���� generator ���з��򴫲��� �����²���
		float a = 1;
		callCuda(cublasSaxpy(
			global::cublasHandle,
			data_dim * batch,
			&a,
			gen_diff_1, 1,
			gen_diff_rec, 1)); // ->diff = gen_diff_rec = gen_diff_rec + gen_diff_1
		generator->layers[generator->layers.size() - 1]->diff = gen_diff_rec;

		// �ӵ����ڶ��㿪ʼ���򴫲�
		for (int n = generator->layers.size() - 2; n > 0; n--)
		{
			(generator->layers[n])->backward();
			adam.optimize(generator->layers[n], adam_iter);
			//sgd.optimize(layers[n]);
		}
	}

	void GAN::Test()
	{
		// ������Ҫ��������ǰ�򴫲�
		gen_loss_rec = 0.0f;
		int offset = 0;

		clock_t start1, ends1;
		clock_t start = clock();
		for (int b = 0; b < train_size / batch; b++) {
			//start1 = clock();
			callCuda(cudaMemcpy(data, h_data + offset * data_dim,
				sizeof(float) * data_dim * batch, cudaMemcpyHostToDevice));

			//start1 = clock();
			for (int n = 0; n < generator->layers.size(); n++)
			{
				generator->layers[n]->forward(false);

			}
			//ends1 = clock();
			//cout << "Running Foward Time : " << (double)(ends1 - start1) / CLOCKS_PER_SEC * 1000 << "ms" << endl;

			// �����ڶ�������ع��㣬���һ��ʱ��ʧ�㣬һ��Ϊ Euclidean loss
			//start1 = clock();

			callCuda(cudaMemcpy(reconstruction_data + offset * data_dim, generator->layers[generator->layers.size() - 2]->data,
				sizeof(float) * data_dim * batch, cudaMemcpyDeviceToHost));
			//ends1 = clock();
			//cout << "Running Reconstruction Data Copy Time : " << (double)(ends1 - start1) / CLOCKS_PER_SEC * 1000 << "ms" << endl;
			gen_loss_rec += (*(generator->layers[generator->layers.size() - 1]->data)) / (float)data_dim;
			offset += batch;
		}
		clock_t ends = clock();
		std::cout << "Running Time : " << (double)(ends - start) / CLOCKS_PER_SEC * 1000 << "ms" << std::endl;

		/*std::string str = "D:/GitHub/VGG_XNet_CUDNN_CUDA/VGG_XNet_CUDNN_CUDA/data/rec_lcd_test/" + std::to_string(0) + ".bmp";
		net_utils::saveImage(str, reconstruction_data, 256, 256, 1, 0);*/
		gen_loss_rec /= train_size;
		std::cout << "Reconstruction loss: " << gen_loss_rec << std::endl;
	}

	void GAN::SaveParas(std::string _para_path)
	{
		// ѵ��ʱ�������������б����Ĳ���
		generator->SaveParams(_para_path);
		discrimintor->SaveParams(_para_path);
	}

	void GAN::ReadGenParas(std::string _para_path)
	{
		// ��������������
		generator->ReadParams(_para_path);
	}

	void GAN::ReadDisParas(std::string _para_path)
	{
		// �����б�������
		discrimintor->ReadParams(_para_path);
	}

	void GAN::ReadParas(std::string _para_path)
	{
		// ���� ReadGenParas �� ReadDisParas ��ȡ���������б���������ѵ��ʱʹ��
		ReadGenParas(_para_path);
		ReadDisParas(_para_path);
	}

	GAN::~GAN()
	{
		delete[] h_data;
		delete[] reconstruction_data;

		callCuda(cudaFree(data));
		callCuda(cudaFree(rec_data));
		callCuda(cudaFree(rec_label));
		callCuda(cudaFree(ONE));
		callCuda(cudaFree(ZERO));
		callCuda(cudaFree(dis_diff_0));
		callCuda(cudaFree(dis_diff_1));
		callCuda(cudaFree(gen_diff_rec));
		callCuda(cudaFree(gen_diff_1));

		delete[] generator;
		delete[] discrimintor;
	}

}