#include <stdio.h>
#include <iostream>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
using namespace std;

const char operands[] = "<>^v.,;:0#[{()}]_8+-/*%=&|!~?\'\"@x$abcdefghijABCDEFGHIJ";
const char variables[] = "@x$abcdefghij";
const char expressions[] = "ABCDEFGHIJ";
const int opLen = 53;
const int varLen = 13;
const int expLen = 10;
const int chromNum = 10;
const int chromLen = 10;
const int maxNum = 1000000;
const int faceSwap = 30;

int returns[10][2];
bool truths[10][2];
int numbers[10][2];
int equals[10];
int RWvalues[10][2];
bool operation_B[10];
bool read_B[10];
bool write_B[10];
bool loop_B[10];

int startEn = 1200;
const int copyEn = 500;
const int motionEn = 50;
const int jumpEn = 300;
const int stepEn = 1;
const int maxEn = 3000;
const int minEn = 1;
const int sexEn = copyEn / chromNum;
const int maxAge = 200;

struct Creature {
	char genome[10][10];	//genome[x][y] x: chromNum y: chromLen
	int values[13];
	int generation;
	int mutations;
	int energy;
	char face;
	bool live;
	bool fertile;
	int direction;
	int age;
};

class World {
	Creature ***grid;
	int nX;
	int nY;
	int sunX;
	int sunY;
	int brightness;
	int focusX;
	int focusY;
	public:
		World(int x, int y, bool load){
			ifstream file("tupo.sav");
			string buffer;
			int number;
			char ch;
			grid = new Creature**[x];
			nX = x;
			nY = y;
			sunX = 0;
			sunY = 0;
			focusX = 0;
			focusY = 0;
			brightness = 1500;
			if(load){
				getline(file, buffer);
				printf("loading from %s ...", buffer.c_str()); cout << flush;
				file >> number;
				file >> number;
			}
			for(int i=0; i<nX; i++){
				grid[i] = new Creature*[y];
				for(int j=0; j<nY; j++){
					grid[i][j] = new Creature;
					init_creature(*grid[i][j]);
					if(load){
						file >> number; grid[i][j]->energy = number;
						file >> number; grid[i][j]->live = number;
						file >> number; grid[i][j]->fertile = number;
						file >> number; grid[i][j]->generation = number;
						file >> number; grid[i][j]->mutations = number;
						file >> ch; grid[i][j]->face = ch;
						for(int k=0; k<chromNum; k++){
							for(int l=0; l<chromLen; l++){
								file >> ch; grid[i][j]->genome[k][l] = ch;
							}
						}
						for(int k=0; k<varLen; k++){
							file >> number; grid[i][j]->values[k] = number;
						}
					}
					grid[i][j]->genome[0][0] = '^';
					grid[i][j]->genome[0][1] = '0';
					grid[i][j]->genome[0][2] = '#';
				}
			}
		}
		void print_world(int x1, int x2, int y1, int y2){
			for(int i=x1; i<x2; i++){
				for(int j=y1; j<y2; j++)
					if(grid[i][j]->live) cout << grid[i][j]->face;
					else cout << ' ';
				cout << '\n';
			}
		}
		void print_creature(int x, int y){
			Creature &creature = *grid[x][y];
			cout << "   " << creature.face << '\t' << x << ',' << y << "\tgen: " << creature.generation << "\ten: " << creature.energy << "\tdir: " << creature.direction << '\n';
			for(int i=0; i<chromNum; i++){
				cout << expressions[i] << ": ";
				for(int j=0; j<chromLen; j++){
					cout << creature.genome[i][j];
				}
				cout << '\n';
			}
			for(int i=0; i<varLen; i++) cout << variables[i] << ':' << setw(10) << creature.values[i] << "  " ;
			cout << '\n';
		}
		void string_creature(string &str, int x, int y){
			Creature &creature = *grid[x][y];
			std::ostringstream stream;
			//stream << "#\t" << x << ',' << y << '\n';
			stream << creature.energy << '\t' << creature.live  << '\t' << creature.fertile  << '\t' << creature.generation  << '\t' << creature.mutations  << '\t' << creature.face << '\n';
			for(int i=0; i<chromNum; i++){
				for(int j=0; j<chromLen; j++){
					stream << creature.genome[i][j];
				}
				stream << '\n';
			}
			for(int i=0; i<varLen; i++) stream << creature.values[i] << '\t';
			stream << '\n';
			str = stream.str();
		}
		void save(){
			ofstream file;
			time_t rawtime;
			time (&rawtime);
			string str;
			file.open("tupo.sav");
			if(! file.is_open()) return;
			file << ctime(&rawtime);
			file << sunX << '\t' << sunY << '\n';
			for(int i=0; i<nX; i++){
				for(int j=0; j<nY; j++){
					string_creature(str, i, j);
					file << str;
				}
			}
		}
		double stats(){
			int energy = 0, alive = 0;
			double aliveR;
			for(int i=0; i<nX; i++)
				for(int j=0; j<nY; j++){
					energy += grid[i][j]->energy;
					alive += grid[i][j]->live;
				}
			aliveR = double(alive) / double(nX*nY);
			cout << "live cells: " << int(aliveR*100) << "\%   energy live: " << energy/alive << "   energy dead: " << energy/(nX*nY - alive);
			cout << "   sun: " << sunX << ',' << sunY << "   energy " << focusX << ',' << focusY << ": " << sun(focusX,focusY) << flush;
			return aliveR;
		}
		void distribute_energy(int energy, int points){
			for(int i=0; i<points; i++){
				rand_creature().energy += energy;
			}
		}
		void step(bool print){
			int x = rand() % nX;
			int y = rand() % nY;
			Creature &creature = *grid[x][y];
			if(creature.live < minEn){
				if(print) step(print);
				return;
			}
			if(++creature.age > maxAge){
				creature.live = false;
				return;
			}
			for(int i=0; i<chromNum; i++){
				truths[i][0] = true;
				truths[i][1] = true;
				numbers[i][0] = 1;
				numbers[i][1] = 1;
				equals[i] = 1;
				RWvalues[i][0] = 0;
				RWvalues[i][1] = 0;
				operation_B[i] = false;
				read_B[i] = false;
				write_B[i] = false;
				loop_B[i] = false;
			}
			if(print){ print_creature(x,y); cout << x << ',' << y << '\n'; cout << "A: "; }

			creature.fertile = false;
			returns[0][0] = -1;
			int &en = creature.energy;
			int &dir = creature.direction;
			en += sun(x,y);
			if(en > maxEn) en = maxEn;
			int origX = x, origY = y, i = 0, j = 0, nextI, nextJ, k, l, brackets, transEn, oldDist = 0, dist;
			bool expReturn = false, truthBuf, moved = false;
			Creature* p_buffer;
			char ch;
			creature.values[1] = rand() % (maxNum*2) - maxNum;
			while(i >= 0 && en >= 0){
				ch = creature.genome[i][j];
				if(print) cout << ch;
				switch(ch){
					case '>': j++;
						dir = (dir+1) % 4;
						break;
					case '<': j++;
						dir = abs( (dir-1) % 4 );
						break;
					case '^': j++;
						switch(dir){
							case 0: x--; break;
							case 1: y++; break;
							case 2: x++; break;
							case 3: y--; break;
						}
						wrap(x,y);
						dist = distance(origX, origY, x, y);
						if(dist > oldDist){
							en -= motionEn * (dist - oldDist);
							oldDist = dist;
						}
						break;
					case 'v': j++;
						switch(dir){
							case 0: x++; break;
							case 1: y--; break;
							case 2: x--; break;
							case 3: y++; break;
						}
						wrap(x,y);
						dist = distance(origX, origY, x, y);
						if(dist > oldDist){
							en -= motionEn * (dist - oldDist);
							oldDist = dist;
						}
						break;
					case '0': j++; wrap(x,y);
						en -= distance(origX, origY, x, y) * jumpEn;
						if( en >= minEn && ! grid[x][y]->live){
							if(print){ cout << " ---jump:" << x << ',' << y << "--- "; }
							p_buffer = grid[x][y];
							grid[x][y] = grid[origX][origY];
							grid[origX][origY] = p_buffer;
							origX = x;
							origY = y;
							moved = true;	
						}
						break;	
					case '#': j++; wrap(x,y);
						en -= copyEn;
						if(en >= minEn && moved){
							copy_creature(creature, x, y);
							if(print){ cout << " ---copy:" << x << ',' << y << "--- "; }
						}
						moved = false;
						break;
					case ':': j++; wrap(x,y);
						if(numbers[i][1] > 0){
							transEn = min(numbers[i][1], en);
							grid[x][y]->energy += transEn;
							en -= transEn;
						} break;
					case ';': j++; wrap(x,y);
						if(numbers[i][1] < 0){
							en += numbers[i][1];
						}
						else if(numbers[i][1] > grid[x][y]->energy){
							en -= numbers[i][1] - grid[x][y]->energy;
						}
						else{
							grid[x][y]->energy -= numbers[i][1];
							en += numbers[i][1] / 2;
						} break;
					case '_': j = chromLen; expReturn = true; break;
					case '8': j++; wrap(x,y);
						en -= sexEn;
						if(grid[x][y]->live && grid[x][y]->fertile){ 
							fuck(creature, *grid[x][y], numbers[i][1]);
							creature.fertile = true;
						}
						break;
					case '=': j++;
						equals[i] = numbers[i][1]; operation_B[i] = true; break;
					case '+': j++;
						equals[i] = numbers[i][0] + numbers[i][1]; operation_B[i] = true; break;
					case '-': j++;
						equals[i] = numbers[i][0] - numbers[i][1]; operation_B[i] = true; break;
					case '*': j++;
						equals[i] = numbers[i][0] * numbers[i][1]; operation_B[i] = true; break;
					case '/': j++;
						if(numbers[i][0] == 0 || numbers[i][1] == 0) break;
						equals[i] = numbers[i][0] / numbers[i][1]; operation_B[i] = true; break;
					case '%': j++;
						if(numbers[i][0] == 0 || numbers[i][1] == 0) break;
						equals[i] = numbers[i][0] % numbers[i][1]; operation_B[i] = true; break;
					case '\'': j++;
						truths[i][0] = truths[i][1]; truths[i][1] = numbers[i][0] < numbers[i][1]; break;
					case '\"': j++;
						truths[i][0] = truths[i][1]; truths[i][1] = numbers[i][0] > numbers[i][1]; break;
					case '~': j++;
						truths[i][0] = truths[i][1]; truths[i][1] = numbers[i][0] == numbers[i][1]; break;
					case '?': j++;
						truths[i][0] = truths[i][1]; truths[i][1] = numbers[i][0] != numbers[i][1]; break;
					case '&': j++;
						truthBuf = truths[i][1]; truths[i][1] = truths[i][0] && truths[i][1]; truths[i][0] = truthBuf; break;
					case '|': j++;
						truthBuf = truths[i][1]; truths[i][1] = truths[i][0] || truths[i][1]; truths[i][0] = truthBuf; break;
					case '!': j++;
						truthBuf = truths[i][1]; truths[i][1] = ! truths[i][1]; truths[i][0] = truthBuf; break;
					case '[': if(truths[i][1]) j++;
						else{
							j = chromLen;
						}
						break;
					case '{': if(! truths[i][1]) j++;
						else{
							j = chromLen; 
						}
						break;
					case ']': if(truths[i][1]) j++;
						else{
							j = 0;
						}
						break;
					case '}': if(! truths[i][1]) j++;
						else{
							j = 0; 
						}
						break;
					case '(': j = chromLen;
						break;
					case ')': j = 0;
						break;
					case '.': j++;
						write_B[i] = true; break;
					case ',': j++;
						read_B[i] = true; break;
					default:
						for(k=0; k<varLen; k++){ if(ch == variables[k]){
							if(ch == '@'){ creature.values[0] = en; }
							if(ch == 'x'){ creature.values[1] = rand() % (maxNum*2) - maxNum; }
							if(ch == '$'){ wrap(x,y); creature.values[2] = int(grid[x][y]->face); }
							RWvalues[i][0] = RWvalues[i][1];
							RWvalues[i][1] = k;
							if(operation_B[i]){
								creature.values[k] = equals[i];
								operation_B[i] = false;
							}
							else{
								numbers[i][0] = numbers[i][1];
								numbers[i][1] = creature.values[k];
							}
							if(write_B[i]){
								wrap(x,y);
								grid[x][y]->values[RWvalues[i][1]] = creature.values[RWvalues[i][0]];
								write_B[i] = false;
							}
							if(read_B[i]){
								wrap(x,y);
								creature.values[RWvalues[i][1]] = grid[x][y]->values[RWvalues[i][0]];
								read_B[i] = false;
							}
							j++;
							goto stop;
						}}
						for(k=0; k<expLen; k++){ if(ch == expressions[k]){
							returns[k][0] = i;
							returns[k][1] = j + 1;
							numbers[k][0] = numbers[i][0];
							numbers[k][1] = numbers[i][1];
							truths[k][0] = truths[i][0];
							truths[k][1] = truths[i][1];
							i=k;
							j=0;
							if(print) cout << "  ->  " << expressions[i] << ": ";
							goto stop;
						}}
						stop: break;
				}
				if(j >= chromLen){
					nextI = returns[i][0];
					nextJ = returns[i][1];
					if(expReturn){
						truths[nextI][1] = truths[i][1];
						numbers[nextI][1] = numbers[i][1];
						expReturn = false;
					}
					i = nextI;
					j = nextJ;
					if(print && i>=0) cout << "  <-  " << expressions[i] << ": ";
				}
				if(numbers[i][1] > maxNum || numbers[i][1] < -maxNum){ numbers[i][1] = 0; }
				en -= stepEn;
			}
			if(en < minEn){
				creature.live = false;
				creature.energy = 0;
			}
			else creature.live = true;
		}
		void smite(int smiteLen){
			int x = rand() % nX;
			int y = rand() % nY;
			for(int i=x; (i-x) < smiteLen && i<nX; i++){
				for(int j=y; (j-y) < smiteLen && j<nY; j++){
					grid[i][j]->live = false;
				}
			}
		}
		void move_sun(){
				int x = sunX;
				x += rand() % 2 - 1;
				int y = sunY; 
				y += rand() % 2 - 1;
				wrap(x,y);
				sunX = x;
				sunY = y; 
		}
		void seed(){
				for(int i=0; i<nX; i++){
					for(int j=0; j<nY; j++){
						if(! grid[i][j]->live){
							init_creature(*grid[i][j]);
		}}}}
		void changeFocus(int x, int y){
			focusX = x;
			focusY = y;
		}
		void point_mut(){
			Creature &creature = rand_creature();
			if(creature.energy < minEn){ return; }
			int i = rand() % chromNum;
			int j = rand() % chromLen;
			creature.genome[i][j] = get_gene();
			creature.generation = 0;
			if(creature.mutations % faceSwap == 0) creature.face = char(rand() % 93 + 33);
			else creature.mutations++;
		}
		void inDel_mut(){
			Creature &creature = rand_creature();
			if(creature.energy < minEn){ return; }
			int i = rand() % chromNum;
			int j1 = rand() % chromLen;
			int j2 = rand() % (chromLen + 1) - 1 ;
			char buff;
			if(abs(j1 - j2) <= 1){
				inDel_mut();
				return;
			}

			buff = creature.genome[i][j1];
			if(j1 < j2 ){
				for(int k=j1; k < j2; k++){
					creature.genome[i][k] = creature.genome[i][k+1];
				}
				creature.genome[i][j2-1] = buff;
			}
			else{
				for(int k=j1; k > j2; k--){
					creature.genome[i][k] = creature.genome[i][k-1];
				}
				creature.genome[i][j2+1] = buff;
			}
			creature.generation = 0;
			if(creature.mutations % faceSwap == 0) creature.face = char(rand() % 93 + 33);
			creature.mutations++;
		}
		void val_mut(){
			Creature &creature = rand_creature();
			if(creature.energy < minEn){ return; }
			int i = rand() % varLen;
			creature.values[i] = rand() % (maxNum*2) - maxNum;
		}
		void dupli_mut(){
			Creature &creature = rand_creature();
			if(creature.energy < minEn){ return; }
			int x,y,z1,z2,buff;
			do{
				x = rand() % chromNum;
				y = rand() % chromNum;
			} while(x == y);
			do{
				z1 = rand() % chromLen;
				z2 = rand() % chromLen;
			} while(z1 == z2);
			if(z1 > z2){ 
				buff = z1;
				z1 = z2;
				z2 = buff;
			}
			for(int i=z1; i <= z2; i++){
				creature.genome[x][i] = creature.genome[y][i];
			}
			creature.generation = 0;
			if(creature.mutations % faceSwap == 0) creature.face = char(rand() % 93 + 33);
			else creature.mutations++;
		}
		void homolog_mut(){
			Creature &creature1 = rand_creature();
			if(! creature1.live){ return; }
			Creature &creature2 = rand_creature();
			int x = rand() % chromNum;
			for(int i=0; i<chromLen; i++){
				creature1.genome[x][i] = creature2.genome[x][i];
			}
			creature1.generation = 0;
			if(creature1.mutations % faceSwap == 0) creature1.face = char(rand() % 93 + 33);
			else creature1.mutations++;
		}
		void immunity_mut(){
			Creature &creature = rand_creature();
			if(! creature.live){ return; }
			creature.fertile = ! creature.fertile;
		}
		void except_input(){
			int ch;
			ch = getchar();
		}
	private:
		void init_creature(Creature & creature){
			for(int i=0; i<chromNum; i++)
				for(int j=0; j<chromLen; j++)
					creature.genome[i][j] = get_gene();
			for(int i=0; i<varLen; i++) creature.values[i] = 0;
			creature.generation = 0;
			creature.mutations = 0;
			creature.energy = startEn;
			creature.face = char(rand() % 93 + 33);
			creature.live = true;
			creature.fertile = false;
			creature.direction = rand() % 4;
			creature.age = 0;
		}
		Creature & rand_creature(){
			return *grid[rand() % nX][rand() % nY];
		}
		char get_gene(){
			return operands[rand() % (opLen-1)];
		}
		void wrap(int &x, int &y){
			if(x >= 0 && x < nX && y >= 0 && y < nY) return;
			int bX = x % nX;
			int bY = y % nY;
			if(bX < 0) bX += nX;
			if(bY < 0) bY += nY;
			x = bX;
			y = bY;
		}
		int distance(int origX, int origY, int x, int y){
			int distNormX = abs(origX - x);
			int distX = min(distNormX, nX-distNormX);
			int distNormY = abs(origY - y);
			int distY = min(distNormY, nY-distNormY);
			return max( abs(distX), abs(distY));
		}
		void copy_creature(Creature & creature1, int x, int y){
			Creature &creature2 = *grid[x][y];
			for(int i=0; i<chromNum; i++)
				for(int j=0; j<chromLen; j++)
					creature2.genome[i][j] = creature1.genome[i][j];
			for(int i=0; i<varLen; i++) creature2.values[i] = creature1.values[i];
			creature2.generation = creature1.generation + 1;
			creature2.mutations = creature2.mutations;
			creature2.face = creature1.face;
			creature2.live = true;
			creature2.fertile = false;
			creature2.age = 0;
			creature2.direction = creature1.direction;
		}
		void fuck(Creature & creature1, Creature & creature2, int chromosome){
			if(chromosome < 0 || chromosome >= chromNum){
				chromosome = rand() % chromNum;
			}
			for(int i=0; i<chromLen; i++){
				creature2.genome[chromosome][i] = creature1.genome[chromosome][i];
			}
		}
		int sun(int x, int y){
			int energy = brightness - distance(sunX, sunY, x, y) * ((brightness*2)/nX);
			if(energy <= minEn){ return minEn; }
			else{ return energy; }
		}
};

int main(int argc, char* argv[]){
	srand(time(NULL));
	int frame = 1;
	int load = false;
	double aliveR = 1;
	if(argc > 1){
		load = ! strcmp(argv[1], "-l");
	}
	World world(500, 500, load);
	
	while(true){
		frame++;
		world.step(false);
		if(frame % 50 == 0){
			world.point_mut();
			world.inDel_mut();
		}
		if(frame % 100 == 0){
			world.val_mut();
			world.dupli_mut();
			world.homolog_mut();
			world.immunity_mut();
		}
		if(frame % 1000000 == 0){
			cout << string(10, '\n');
			world.move_sun();
			world.step(true);
			cout << string(2, '\n');
			world.print_world(0,40,0,190);
			aliveR = world.stats();
			if(aliveR < 0.0001){
				world.seed();
			}
//	world.except_input();
		}
		if(frame % 1000000000 == 0){
			cout << "saving...\n";
			world.save();
		}
	}
}

