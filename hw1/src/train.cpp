#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include "hmm.h"

using namespace std;

void trainHMM(HMM*&,vector<vector<int> >&);
void getAlpha(HMM*&,vector<int>&,double**&);
void getBeta(HMM*&,vector<int>&,double**&);
void getGamma(HMM*&,double**&,double**&,vector<int>&,double**&);
void getEpsilon(HMM*&,double**&,double**&,vector<int>&,double***&);
void getData(fstream*,vector<string>&);
void str2int(string&,vector<int>&);
void delPtr(double***&,double***&,double***&,double****&,HMM*&,vector<vector<int> >&);

int main(int argc,char *argv[])
{
  
	if(argc != 5){
		cerr << "Incorrect number of arguments!" << endl;
		return 0;
	}
  
  clock_t start = clock();

	HMM* model = new HMM;
	loadHMM(model,argv[2]);

	fstream data;
	data.open(argv[3],ios::in);

	vector<string> tmpData;
	getData(&data,tmpData); //store training data into vector of string
	data.close();
  vector<vector<int> > trainData;

  for(int i = 0; i < tmpData.size(); ++i){
    vector<int> data;
    str2int(tmpData[i],data);
    trainData.push_back(data);
  }

  for(int i = 0; i < atoi(argv[1]); ++i){
    cerr << "====================" << "Epoch " << i+1 << "====================" << endl;
    trainHMM(model,trainData);
    dumpHMM(stdout,model);
    cout << endl;
  }
  clock_t end = clock();

  FILE* output = open_or_die(argv[4],"w");

  dumpHMM(output,model);

  cout << "Elapsed time: " << double(end - start)/CLOCKS_PER_SEC << endl;
	return 0;
}

void trainHMM(HMM*& model,vector<vector<int> >& data)
{
  double*** alpha = new double**[data.size()];
  double*** beta = new double**[data.size()];
  double*** gamma = new double**[data.size()];
  double**** epsilon = new double***[data.size()];

  for(int i = 0; i < data.size(); ++i){
    alpha[i] = new double*[model -> state_num];
    beta[i] = new double*[model -> state_num];
    gamma[i] = new double*[model -> state_num];
    for(int j = 0; j < model -> state_num; ++j){
      alpha[i][j] = new double[data[i].size()];
      beta[i][j] = new double[data[i].size()];
      gamma[i][j] = new double[data[i].size()];
    }
  }

  for(int i = 0; i < data.size(); ++i){
    epsilon[i] = new double**[data[i].size()-1];
    for(int j = 0; j < data[i].size()-1; ++j){
      epsilon[i][j] = new double*[model -> state_num];
      for(int k = 0; k < model -> state_num; ++k){
        epsilon[i][j][k] = new double[model -> state_num];
      }
    }
  }

/*
  double alpha[data.size()][MAX_STATE][MAX_SEQ];
  double beta[data.size()][MAX_STATE][MAX_SEQ];
  double gamma[data.size()][MAX_STATE][MAX_SEQ];
  double epsilon[data.size()][MAX_SEQ][MAX_STATE][MAX_STATE];
*/
  for(int i = 0; i < data.size(); ++i){
    getAlpha(model,data[i],alpha[i]);
    getBeta(model,data[i],beta[i]);
    getGamma(model,alpha[i],beta[i],data[i],gamma[i]);
    getEpsilon(model,alpha[i],beta[i],data[i],epsilon[i]);
  }

  /* update parameters */

  /* pi_i */
  for(int i = 0; i < model -> state_num; ++i){
    double sum = 0;
    for(int n = 0; n < data.size(); ++n){
      sum += gamma[n][i][0];
    }
    model -> initial[i] = sum / data.size();
  }

  /* a_ij */
  for(int i = 0; i < model -> state_num; ++i){
    double sum = 0;
    for(int n = 0; n < data.size(); ++n){
      for(int t = 0; t < data[n].size(); ++t){
        sum += gamma[n][i][t];
      }
    }
    for(int j = 0; j < model -> state_num; ++j){
      double tmp = 0;
      for(int n = 0; n < data.size(); ++n){
        for(int t = 0; t < data[n].size() - 1; ++t){
          tmp += epsilon[n][t][i][j];
        }
      }
      model -> transition[i][j] = tmp / sum;
    }
  }

  /* b_i(o_j) */
  for(int i = 0; i < model -> state_num; ++i){
    double sum = 0;
    for(int n = 0; n < data.size(); ++n){
      for(int t = 0; t < data[n].size(); ++t){
        sum += gamma[n][i][t];
      }
    }
    double gSum[model -> observ_num] = {};
    for(int n = 0; n < data.size(); ++n){
      for(int t = 0; t < data[n].size(); ++t){
        gSum[data[n][t]] += gamma[n][i][t];
      }
    }
    for(int x = 0; x < model -> observ_num; ++x){
      model -> observation[x][i] = gSum[x] / sum;
    }
  }
  delPtr(alpha,beta,gamma,epsilon,model,data);

}

void getAlpha(HMM*& model,vector<int>& data,double**& alpha)
{
  for(int i = 0; i < model -> state_num; ++i){
    alpha[i][0] = (model -> initial[i]) * (model -> observation[data[0]][i]);
  }

  for(int t = 0; t < data.size() - 1; ++t){
    for(int j = 0; j < model -> state_num; ++j){
      double sum = 0;
      for(int i = 0; i < model -> state_num; ++i)
        sum += alpha[i][t] * (model -> transition[i][j]);
      alpha[j][t+1] = sum * (model -> observation[data[t+1]][j]);
    }
  }

  return;
}

void getBeta(HMM*& model,vector<int>& data,double**& beta)
{
  for(int i = 0; i < model -> state_num; ++i){
    beta[i][data.size()-1] = 1;
  }

  for(int t = data.size() - 2; t >= 0; --t){
    for(int i = 0; i < model -> state_num; ++i){
      double sum = 0;
      for(int j = 0; j < model -> state_num; ++j)
        sum += (model -> transition[i][j]) * (model -> observation[data[t+1]][j]) * beta[j][t+1];
      beta[i][t] = sum;
    }
  }
  return;
}

void getGamma(HMM*& model,double**& alpha,double**& beta,vector<int>& data,double**& gamma)
{
  for(int t = 0; t < data.size(); ++t){
    double sum = 0;
    for(int i = 0; i < model -> state_num; ++i){
      sum += alpha[i][t] * beta[i][t];
    }
    for(int i = 0; i < model -> state_num; ++i){
      gamma[i][t] = (alpha[i][t] * beta[i][t]) / sum;
    }
  }

  return;
}

void getEpsilon(HMM*& model,double**& alpha,double**& beta,vector<int>& data,double***& epsilon)
{
  for(int t = 0; t < data.size() - 1; ++t){
    double sum = 0;
    for(int i = 0; i < model -> state_num; ++i){
      for(int j = 0; j < model -> state_num; ++j){
        sum += alpha[i][t] * (model -> transition[i][j]) * (model -> observation[data[t+1]][j]) * beta[j][t+1];
      }
    }
    for(int i = 0; i < model -> state_num; ++i){
      for(int j = 0; j < model -> state_num; ++j){
        epsilon[t][i][j] = (alpha[i][t] * (model -> transition[i][j]) * (model -> observation[data[t+1]][j]) * beta[j][t+1]) / sum;
      }
    }
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

void str2int(string& str,vector<int>& ans)
{
  for(int i = 0; i < str.length(); ++i){
    ans.push_back(int(str[i] - 'A'));
  }
  return;
}

void delPtr(double***& alpha,double***& beta,double***& gamma,double****& epsilon,HMM*& model,vector<vector<int> >& data)
{
  for(int i = 0; i < data.size(); ++i){
    for(int j = 0; j < model -> state_num; ++j){
      delete[] alpha[i][j];
      delete[] beta[i][j];
      delete[] gamma[i][j];
    }
    delete[] alpha[i];
    delete[] beta[i];
    delete[] gamma[i];
  }

  for(int i = 0; i < data.size(); ++i){
    for(int j = 0; j < data[i].size()-1; ++j){
      for(int k = 0; k < model -> state_num; ++k){
        delete[] epsilon[i][j][k];
      }
      delete[] epsilon[i][j];
    }
    delete[] epsilon[i];
  }

  delete[] alpha;
  delete[] beta;
  delete[] gamma;
  delete[] epsilon;

  return;
}
