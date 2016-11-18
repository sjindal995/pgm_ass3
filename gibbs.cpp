#include <bits/stdc++.h>

using namespace std;

int include_transition;
int include_skip;
int include_pair_skip;
string index2char = "etaoinshrd";
map<char,int> char2index;
double ocr_factor[10][1000];
double trans_factor[10][10];
vector<int> imgs1,imgs2;
vector<int> node_images;
int** edges;
int n_nodes;

int dict_size=10;
int burn_in_thresh=1000;
int max_iter = 100000;

void createEdges(){
	int imgs1_len = imgs1.size();
	int imgs2_len = imgs2.size();
	edges = new int*[imgs1_len+imgs2_len];
	node_images.clear();
	for(int i=0;i<imgs1_len;i++){
		node_images.push_back(imgs1.at(i));
	}
	for(int i=0;i<imgs2_len;i++){
		node_images.push_back(imgs2.at(i));
	}
	for(int i=0;i<imgs1_len+imgs2_len;i++){
		edges[i] = new int[imgs1_len+imgs2_len];
		for(int j=0;j<imgs1_len+imgs2_len;j++){
			edges[i][j]=0;
		}
	}
	if(include_transition){
		for(int i=0;i<imgs1_len-1;i++){
			edges[i][i+1]=1;
		}
		for(int i=0;i<imgs2_len-1;i++){
			edges[i+imgs1_len][i+1+imgs1_len]=1;
		}
	}
	if(include_skip){
		for(int i=0;i<imgs1_len-1;i++){
			for(int j=i+1;j<imgs1_len;j++){
				if(imgs1.at(i)==imgs1.at(j)) edges[i][j]=1;
			}
		}
		for(int i=0;i<imgs2_len-1;i++){
			for(int j=i+1;j<imgs2_len;j++){
				if(imgs2.at(i)==imgs2.at(j)) edges[i+imgs1_len][j+imgs1_len]=1;
			}
		}
	}
	if(include_pair_skip){
		for(int i=0;i<imgs1_len;i++){
			for(int j=0;j<imgs2_len;j++){
				if(imgs1.at(i)==imgs2.at(j)) edges[i][j+imgs1_len]=1;
			}
		}
	}
}

void loadOCRFactors(string& ocr_filename){
	ifstream ocr_file(ocr_filename);
	int image_id;
	char character;
	double factor;
	while(ocr_file >> image_id >> character >> factor){
		ocr_factor[char2index[character]][image_id] = factor;
	}
}


void loadTransFactor(string& trans_filename){
	ifstream trans_file(trans_filename);
	char char1, char2;
	double factor;
	while(trans_file >> char1 >> char2 >> factor){
		trans_factor[char2index[char1]][char2index[char2]] = factor;
	}
}

vector<string> string2vec(string line, char delimiter){
	vector<string> res;
	string cur = "";
	for(int i = 0; i < line.length(); i++){
		if(line[i] == delimiter){
			res.push_back(cur);
			cur = "";
		}
		else{
			cur += line[i];
		}
	}
	res.push_back(cur);
	return res;
}

double getProb(string conf, int query){
	double ans=1;
	ans *= ocr_factor[conf[query]-48][node_images.at(query)];
	int imgs1_len = imgs1.size();
	int imgs2_len = imgs2.size();
	if(include_transition){
		if(query < imgs1_len-1) ans *= trans_factor[conf[query]-48][conf[query+1]-48];
		if(query>0 && query < imgs1_len) ans *= trans_factor[conf[query-1]-48][conf[query]-48];
		if(query >= imgs1_len && query < n_nodes-1) ans *= trans_factor[conf[query]-48][conf[query+1]-48];
		if(query > imgs1_len && query < n_nodes) ans *= trans_factor[conf[query-1]-48][conf[query]-48];
	}
	if(include_skip){
		if(query < imgs1_len){
			for(int i=0;i<imgs1.size();i++){
				if((i != query) && (conf[i]==conf[query]) && (imgs1.at(i)==imgs1.at(query))){
					// cout << "came handy!!" << endl;
					ans *= 5.0;
				}
			}
		}
		else{
			for(int i=0;i<imgs2.size();i++){
				if((i != query-imgs1.size()) && (conf[i+imgs1.size()]==conf[query]) && (imgs2.at(i)==imgs2.at(query-imgs1.size()))){
					// cout << "came handy2!!" << endl;
					ans *= 5.0;
				}
			}
		}
	}
	if(include_pair_skip){
		if(query < imgs1.size()){
			for(int j=0;j<imgs2.size();j++){
				if((imgs1.at(query) == imgs2.at(j)) && (conf[j+imgs1.size()]==conf[query])) ans *= 5.0;
			}
		}
		else{
			for(int j=0;j<imgs1.size();j++){
				if((imgs2.at(query-imgs1.size())) == (imgs1.at(j) && conf[j] == conf[query])) ans *= 5.0;
			}	
		}
	}
	return ans;
}

string getGibbsResult(){
	int t=0;
	bool burn_in_over = false;
	string sample="";
	for(int i=0;i<n_nodes;i++){
		sample.push_back(rand()%10 + 48);
	}
	while(!burn_in_over || n_nodes*t<=max_iter){
		t++;
		for(int i=0;i<n_nodes;i++){
			string tmp = sample;
			double max_prob=-1;
			string best_conf = "";
			for(int j=0;j<dict_size;j++){
				tmp[i]=j+48;
				double cur_prob = getProb(tmp, i);
				// cout << "tmp : " << tmp << ", query: " << i << ", cur_prob : "<< cur_prob << endl;;
				if(cur_prob > max_prob){
					max_prob = cur_prob;
					best_conf = tmp;
				}
			}
			if(burn_in_over){
				sample[i]=best_conf[i];
			}
		}
		if(!burn_in_over){
			if(n_nodes*t >= burn_in_thresh){
				burn_in_over=true;
				t=1;
			}
		}
	}
	return sample;
}

double getCharAcc(string output_filepath, string words_filepath){
	string line1,line2,line3,line4;
	ifstream out_file(output_filepath);
	ifstream word_file(words_filepath);
	int cor_chars=0;
	int total_chars=0;
	while(getline(out_file,line1) && getline(out_file,line2) && getline(word_file,line3) && getline(word_file,line4)){
		for(int i=0;i<line1.size();i++){
			if(line1[i]==line3[i]) cor_chars++;
			total_chars++;
		}
		for(int i=0;i<line2.size();i++){
			if(line2[i]==line4[i]) cor_chars++;
			total_chars++;
		}
		getline(out_file, line1);
		getline(word_file,line3);
	}
	return (cor_chars/float(total_chars));
}

double getWordAcc(string output_filepath, string words_filepath){
	string line1,line2,line3,line4;
	ifstream out_file(output_filepath);
	ifstream word_file(words_filepath);
	int cor_words=0;
	int total_words=0;
	while(getline(out_file,line1) && getline(out_file,line2) && getline(word_file,line3) && getline(word_file,line4)){
		if(line1==line3) cor_words++;
		total_words++;
		if(line2==line4) cor_words++;
		total_words++;
		getline(out_file, line1);
		getline(word_file,line3);
	}
	return (cor_words/float(total_words));	
}

int main(){
	char2index['e']	= 0;
	char2index['t']	= 1;
	char2index['a']	= 2;
	char2index['o']	= 3;
	char2index['i']	= 4;
	char2index['n']	= 5;
	char2index['s']	= 6;
	char2index['h']	= 7;
	char2index['r']	= 8;
	char2index['d']	= 9;

	string ocr_potentials_filepath;
	cout << "Enter ocr_potentials filepath : ";
	cin >> ocr_potentials_filepath;

	loadOCRFactors(ocr_potentials_filepath);

	cout << "Include transition : ";
	cin >> include_transition;

	if(include_transition){
		string trans_potentials_filepath;
		cout << "Enter trans_potentials filepath : ";
		cin >> trans_potentials_filepath;
		loadTransFactor(trans_potentials_filepath);
	}
	
	cout << "Include Skip : ";
	cin >> include_skip;
	
	cout << "Include Pair Skip : ";
	cin >> include_pair_skip;
	

	string images_filepath;
	cout << "Enter images_filepath : ";
	cin >> images_filepath;

	string output_filepath;
	cout << "Enter output filepath : ";
	cin >> output_filepath;
	
	string words_filepath;
	cout << "Enter words filepath : ";
	cin >> words_filepath;
	cout << endl;
	ifstream in_file(images_filepath);
	ofstream out_file(output_filepath);
	string line1="",line2="";
	int it=0;
	while(getline(in_file, line1) && getline(in_file, line2)){
		vector<string> vec1 = string2vec(line1,'\t');
		vector<string> vec2 = string2vec(line2,'\t');
		if(vec1.back()=="") vec1.pop_back();
		if(vec2.back()=="") vec2.pop_back();
		imgs1.clear();
		imgs2.clear();
		for(int i=0;i<vec1.size();i++){
			imgs1.push_back(atoi(vec1.at(i).c_str()));
		}
		for(int i=0;i<vec2.size();i++){
			imgs2.push_back(atoi(vec2.at(i).c_str()));
		}
		n_nodes = imgs1.size()+imgs2.size();
		createEdges();
		string ans = getGibbsResult();
		for(int i=0;i<imgs1.size();i++){
			out_file << index2char[ans[i]-48];
			// cout << ans[i];
		}
		out_file << endl;
		// cout << endl;
		for(int i=0;i<imgs2.size();i++){
			out_file << index2char[ans[i+imgs1.size()]-48];
			// cout << ans[i+imgs1.size()];
		}
		out_file << endl << endl;
		// cout << endl;
		it++;
		getline(in_file,line1);
		// if(it == 3){
		// 	break;
		// }
	}

	double char_acc = getCharAcc(output_filepath, words_filepath);
	double word_acc = getWordAcc(output_filepath, words_filepath);
	cout << "char_acc: " <<char_acc << endl;
	cout << "word_acc: " <<word_acc << endl;
}