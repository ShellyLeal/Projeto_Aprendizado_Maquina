// Codigo para classificador de arvore de decisao
// Shelly Leal
// CTC-17
// ----------------------------------------------
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <map>
#include <vector>

using namespace std;

struct user {
	int id;
	char gender;
	int age;
	int occupation;
	int zipcode;
};
vector<user> users;

struct movie {
	int id;
	string genre;
	string title;
};
vector<movie> movies;

struct rating {
	int userid;
	int movieid;
	int value;
	int timestamp;
};
vector<rating> ratings;

struct block {
	int data[6];
	int value;
};
vector<block> blocks;
typedef vector<block>::iterator nblock;

void ReadData(){
	//Users
	ifstream us("users.dat");
	users.clear();
	string usbuffer;
	while(us) {
		getline(us, usbuffer);
		int len = usbuffer.size();
		if (len < 8) break;
		user current;
		sscanf(usbuffer.c_str(), "%d::%c::%d::%d::%d", &current.id, &current.gender, &current.age, &current.occupation, &current.zipcode);
		users.push_back(current);
	}
	
	us.close();

	//Movies
	ifstream mov("movies.dat");
	movies.clear();
	string movbuffer;
	while(mov) {
		getline(mov, movbuffer);
		int len = movbuffer.size();
		if (len < 4) break;
		movbuffer[--len] = 0;
		movie current;
		sscanf(movbuffer.c_str(), "%d::", &current.id);
		int st = 0, ed;
		while(movbuffer[st] != ':' || movbuffer[st+1] != ':') st++;
		st += 2;
		ed = st;
		while(movbuffer[ed] != ':' || movbuffer[ed+1] != ':') ed++;
		movbuffer[ed] = 0;
		ed += 2;
		current.title = string(&movbuffer[st]);
		current.genre = string(&movbuffer[ed]);
		movies.push_back(current);
	}
	
	mov.close();
	
	//Ratings
	ifstream rat("ratings.dat");
	
	ratings.clear();
	string ratbuffer;
	while(rat) {
		getline(rat, ratbuffer);
		int len = ratbuffer.size();
		if (len < 6) break;
		rating current;
		sscanf(ratbuffer.c_str(), "%d::%d::%d::%d", &current.userid, &current.movieid, &current.value, &current.timestamp);
		ratings.push_back(current);
	}
	
	rat.close();
	printf("Feita a leitura dos arquivos\n\n");
}


inline movie GetMovie(int id) {
	int min = 0;
	int max = movies.size();
	while(max > min + 1) {
		int mid = (max + min) / 2;
		if (movies[mid].id <= id) min = mid;
		else max = mid;
	}
	return movies[min];
}

inline user GetUser(int id) {
	int min = 0;
	int max = users.size();
	while(max > min + 1) {
		int mid = (max + min) / 2;
		if (users[mid].id <= id) min = mid;
		else max = mid;
	}
	return users[min];
}

void MovieBlock() {
		blocks.clear();
		for(int i=0; i<(int)ratings.size(); i++) {		
		block current;
		const rating & rat = ratings[i];
		const user usr = GetUser(rat.userid);
		const movie mov = GetMovie(rat.movieid);
		current.data[0] = mov.id;
		current.data[1] = usr.age;
		current.data[2] = usr.gender;
		current.data[3] = usr.occupation;
		current.data[4] = usr.zipcode;
		current.data[5] = rat.timestamp;
		current.value = min(5, max(1, rat.value));
		blocks.push_back(current);
	}
}

struct node {
	int value;
	int atribute;
	int max;
	int size;
	map<int, node*> sons;
	node(int _value) : value(_value), atribute(-1), max(3), size(1) {}
};

inline int FindMax(nblock begin, nblock end) {
	int stars[6], ans = 3;
	memset(&stars, 0, sizeof stars);
	for(nblock it = begin; it != end; it++) {
		stars[it->value]++;
		if (stars[it->value] > stars[ans]) ans = it->value;
	}
	return ans;
}

bool comp(const block & a, const block & b) {
	
	for(int i=0; i<6; i++) {
		if (a.data[i] != b.data[i]) return a.data[i] < b.data[i];
	}
	return false;
}

node* DecisionTree(nblock begin, nblock end, int atribute, int pattern) {
	if (begin == end) 
		return new node(pattern);
	
	bool equal = true;
	for(nblock it = begin; (it+1) != end && equal; it++) {
		if (it->value != (it+1)->value) equal = false;
	}
	if (equal) 
		return new node(begin->value);
	
	if (atribute == 6) 
		return new node(FindMax(begin, end));
	
	node* tree = new node(-1);
	tree->max = FindMax(begin, end);
	tree->atribute = atribute;
	
	for(nblock it = begin, last = begin; it != end; it++) {
		if ((it+1) == end || (it+1)->data[atribute] != it->data[atribute]) {
			tree->sons[it->data[atribute]] = DecisionTree(last, it+1, atribute+1, tree->max);
			tree->size += tree->sons[it->data[atribute]]->size;
			last = it+1;
		}
	}
	
	return tree;
}


void Delete(node* treeNode) {
	if (treeNode == NULL) return;
	for(map<int, node*>::iterator it = treeNode->sons.begin(); it != treeNode->sons.end(); it++) {
		Delete(it->second);
	}
	delete treeNode;
}

int Search(const block & queryBlock, node* treeNode, int atribute) {
	if (treeNode->value != -1) {
		return treeNode->value;
	}
	if (atribute == 6 || !treeNode->sons.count(queryBlock.data[atribute])) {
		return treeNode->max;
	}
	return Search(queryBlock, treeNode->sons[queryBlock.data[atribute]], atribute+1);
}


double desvio;

double acerto;

node* decisionTree;

//Test for the training cases
void TrainingTest(nblock begin, nblock end) {
	

	int datasetSize = int(end - begin);
	desvio = 0.0;

	acerto = 0.0;

	

	for(nblock it = begin; it != end; it++) {
		

		int decisionTreeRating = Search(*it, decisionTree, 0);

		int trueRating = it->value;
		desvio += (decisionTreeRating - trueRating)*(decisionTreeRating - trueRating);

		acerto += (decisionTreeRating == trueRating ? 1.0 : 0.0);

	}
	
	//Desvio medio e taxa de acerto
	desvio = sqrt(desvio / datasetSize);

	acerto /= datasetSize;

}

//Test for the validation cases
void ValidationTest(nblock begin, nblock end) {
	

	int datasetSize = int(end - begin);
	desvio = 0.0;

	acerto = 0.0;

	

	for(nblock it = begin; it != end; it++) {
		

		int decisionTreeRating = Search(*it, decisionTree, 0);

		int trueRating = it->value;
		desvio += (decisionTreeRating - trueRating)*(decisionTreeRating - trueRating);

		acerto += (decisionTreeRating == trueRating ? 1.0 : 0.0);

	}
	
	//Standart deviation and sum of absolute values of diferences
	desvio = sqrt(desvio / datasetSize);

	acerto /= datasetSize;

}

int main() {
		ReadData();
		MovieBlock();
	
		//Randomly choose 50 blocks
		printf("Reiniciando a arvore com 50 blocos aleatorios\n");
		int validacao = 50;
		int treinamento = blocks.size() - validacao;
		random_shuffle(blocks.begin(), blocks.end());
		
		
		//Build decision tree
		printf("Construindo a arvore... \n\n");
		sort(blocks.begin(), blocks.begin() + treinamento, comp);
		decisionTree = DecisionTree(blocks.begin(), blocks.begin() + treinamento, 0, 3);
		
		
		printf("Conjunto de treino com %d blocos:\n", treinamento);
		TrainingTest(blocks.begin(), blocks.begin() + treinamento);
		printf("Erro medio: %.3f Taxa de acerto: %.2f%%\n\n", desvio, 100.0*acerto);
	
		printf("Conjunto de validacao com %d blocos:\n", validacao);
		ValidationTest(blocks.begin() + treinamento, blocks.end());
		printf("Erro medio: %.3f Taxa de acerto: %.2f%%\n", desvio, 100.0*acerto);

		//Randomly choose 100 blocks
		printf("\n\nReiniciando a arvore com 100 blocos aleatorios\n");
		validacao = 100;
		treinamento = blocks.size() - validacao;
		random_shuffle(blocks.begin(), blocks.end());
		
		
		//Build decision tree
		printf("Construindo a arvore... \n\n");
		sort(blocks.begin(), blocks.begin() + treinamento, comp);
		decisionTree = DecisionTree(blocks.begin(), blocks.begin() + treinamento, 0, 3);
		
		
		printf("Conjunto de treino com %d blocos:\n", treinamento);
		TrainingTest(blocks.begin(), blocks.begin() + treinamento);
		printf("Erro medio: %.3f Taxa de acerto: %.2f%%\n\n", desvio, 100.0*acerto);
	
		printf("Conjunto de validacao com %d blocos:\n", validacao);
		ValidationTest(blocks.begin() + treinamento, blocks.end());
		printf("Erro medio: %.3f Taxa de acerto: %.2f%%\n", desvio, 100.0*acerto);

		
		Delete(decisionTree);
	
	return 0;
}
