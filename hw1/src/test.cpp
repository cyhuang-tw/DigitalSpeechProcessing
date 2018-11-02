#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "hmm.h"

using namespace std;

double getProb(HMM*,string&);
double getAcc(vector<string>&,vector<string>&);
void printFile(fstream*,vector<string>&,vector<double>&);
void getData(fstream*,vector<string>&);


int main(int argc,char *argv[])
{
	if(argc != 4){
		cerr << "Incorrect number of arguments!" << endl;
		return 0;
	}

	HMM models[5];
	load_models(argv[1],models,5);

	fstream file;
	file.open(argv[2],ios::in);

	vector<string> testData;
	getData(&file,testData);
	file.close();

	vector<string> predict_ans;
	vector<double> predict_prob;

	for(int i = 0; i < testData.size(); ++i){
		double maxProb = 0;
		int num = 1;
		for(int j = 0; j < 5; ++j){
			double prob = getProb(&models[j],testData[i]);
			//cout << j << " : " << prob << endl;
			if(prob > maxProb){
				maxProb = prob;
				num = j+1;
			}
		}
		predict_ans.push_back("model_0" + to_string(num) + ".txt");
		predict_prob.push_back(maxProb);
	}

	file.open(argv[3],ios::out);
	printFile(&file,predict_ans,predict_prob);
	file.close();

	//cout << "Accuracy: " << getAcc(ans,predict_ans) << endl;

	return 0;
}

double getProb(HMM* model,string& data)
{
	double** delta = new double*[model -> state_num];
	for(int i = 0; i < model -> state_num; ++i){
		delta[i] = new double[data.length()];
		delta[i][0] = (model -> initial[i]) * (model -> observation[int(data[0])-65][i]);
	}
	for(int t = 1; t < data.length(); ++t){
		for(int j = 0; j < model -> state_num; ++j){
			double max = 0;
			for(int i = 0; i < model -> state_num; ++i){
				if(delta[i][t-1] * (model -> transition[i][j]) > max)
					max = delta[i][t-1] * (model -> transition[i][j]);
			}
			delta[j][t] = max * (model -> observation[int(data[t])-65][j]);
		}
	}
	double prob = 0;
	for(int i = 0; i < model -> state_num; ++i){
		if(prob < delta[i][data.size()-1])
			prob = delta[i][data.size()-1];
	}

	for(int i = 0; i < model -> state_num; ++i)
		delete[] delta[i];
	delete[] delta;

	return prob;
}

double getAcc(vector<string>& ans,vector<string>& predict)
{
	if(ans.size() != predict.size()){
		cerr << "wrong size!" << endl;
		return 0.0;
	}

	int count = 0;

	for(int i = 0; i < ans.size(); ++i){
		if(ans[i] == predict[i])
			++count;
	}

	return double(count)/ double(ans.size());
}

void printFile(fstream* file,vector<string>& models,vector<double>& prob)
{
	for(int i = 0; i < models.size(); ++i){
		*file << models[i] << " " << prob[i] << endl;
	}
	return;
}

void getData(fstream* file,vector<string>& data)
{
	string str = "";
	while(getline(*file,str))
		data.push_back(str);

	return;
}