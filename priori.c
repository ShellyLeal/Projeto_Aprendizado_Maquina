// Codigo para classificador a priori
// Shelly Leal
// CTC-17
// ----------------------------------
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


int movieAvgRatings[4000];
double sumRatings[4000];
int numRatings[4000];

void AvgClassifier(int treinamento, int nMovies) {
	
	for(int i=1; i<=nMovies; i++) {
		sumRatings[i] = 0.0;
		numRatings[i] = 0;
	}
	for(int it = 0; it < treinamento; it++) {
		int movieid = blocks[it].data[0];
		numRatings[movieid]++;
		sumRatings[movieid] += (double)blocks[it].value;
	}

	for(int i=1; i<=nMovies; i++) {
		if (numRatings[i] == 0) movieAvgRatings[i] = 3;
		else movieAvgRatings[i] = int(sumRatings[i]/numRatings[i] + 0.4999999);
		if (movieAvgRatings[i] < 1.0 || movieAvgRatings[i] > 5.0) printf("%.3f %d %.3f\n", sumRatings[i], numRatings[i], movieAvgRatings[i]);
	}
}

double desvio;
double acerto;

void TrainingTest(nblock begin, nblock end) {
	int datasetSize = int(end - begin);
	desvio = 0.0;
	acerto = 0.0;
	for(nblock it = begin; it != end; it++) {
		int averageRating = movieAvgRatings[it->data[0]];
		int trueRating = it->value;
		desvio += (averageRating - trueRating)*(averageRating - trueRating);
		acerto += (averageRating == trueRating ? 1.0 : 0.0);
	}
	desvio = sqrt(desvio / datasetSize);
	acerto /= datasetSize;
}

void ValidationTest(nblock begin, nblock end) {
	int datasetSize = int(end - begin);
	desvio = 0.0;
	acerto = 0.0;
	for(nblock it = begin; it != end; it++) {
		int averageRating = movieAvgRatings[it->data[0]];
		int trueRating = it->value;
		desvio += (averageRating - trueRating)*(averageRating - trueRating);
		acerto += (averageRating == trueRating ? 1.0 : 0.0);
	}
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
		
		//Build priori classification
		printf("Fazendo a classificacao a priori... \n");
		AvgClassifier(treinamento, movies.size());
		
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
		
		//Build priori classification
		printf("Fazendo a classificacao a priori... \n");
		AvgClassifier(treinamento, movies.size());
		
		
		printf("Conjunto de treino com %d blocos:\n", treinamento);
		TrainingTest(blocks.begin(), blocks.begin() + treinamento);
		printf("Erro medio: %.3f Taxa de acerto: %.2f%%\n\n", desvio, 100.0*acerto);
	
		printf("Conjunto de validacao com %d blocos:\n", validacao);
		ValidationTest(blocks.begin() + treinamento, blocks.end());
		printf("Erro medio: %.3f Taxa de acerto: %.2f%%\n", desvio, 100.0*acerto);

	
	return 0;


}
