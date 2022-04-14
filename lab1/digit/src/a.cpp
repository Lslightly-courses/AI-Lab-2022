#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <bitset>
#include <queue>
#include <stack>
#include <map>
#include <list>
#include <ctime>

#define DIM 5
#define LEFT 3
#define RIGHT 2
#define UP 1
#define DOWN 0
#ifdef debug
#define DEBUGOUT(x) std::cout << x << endl;
#define DEBUGUSE(x) x
#else
#define DEBUGOUT(x) ;
#define DEBUGUSE(x) ;
#endif


using namespace std;

typedef struct {
	int i, j;
}Location;
class WalkAbility;
class State;
class Node;
class EvalCmp;
list<Node*> node_list;
list<State*> state_list;

map<char, string> map_direct_to_str = {
	{LEFT, "L"},
	{RIGHT, "R"},
	{UP, "U"},
	{DOWN, "D"},
};

class WalkAbility {
private:
	bitset<4> ability[DIM][DIM];
public:
	WalkAbility(vector<vector<int>> &input) {
		for (int i = 0; i < DIM; i++) {
			for (int j = 0; j < DIM; j++) {
				ability[i][j].set();
			}
		}
		for (int i = 0; i < DIM; i++) {
			ability[i][0].reset(LEFT);
			ability[i][DIM-1].reset(RIGHT);
		}
		for (int j = 0; j < DIM; j++) {
			ability[0][j].reset(UP);
			ability[DIM-1][j].reset(DOWN);
		}
		ability[0][2].set(UP);
		ability[DIM-1][2].set(DOWN);
		ability[2][0].set(LEFT);
		ability[2][DIM-1].set(RIGHT);
		for (int i = 0; i < DIM; i++) {
			for (int j = 0; j < DIM; j++) {
				if (input[i][j] < 0) {
					//	black hole
					ability[i][j].reset();
					ability[(i-1+DIM)%DIM][j].reset(DOWN);
					ability[(i+1+DIM)%DIM][j].reset(UP);
					ability[i][(j-1+DIM)%DIM].reset(RIGHT);
					ability[i][(j+1+DIM)%DIM].reset(LEFT);
				}
			}
		}
	}
	bool left(Location loc) {
		return ability[loc.i][loc.j][LEFT];
	}
	bool right(Location loc) {
		return ability[loc.i][loc.j][RIGHT];
	}
	bool up(Location loc) {
		return ability[loc.i][loc.j][UP];
	}
	bool down(Location loc) {
		return ability[loc.i][loc.j][DOWN];
	}
	void print() {
		for (int i = 0; i < DIM; i++) {
			for (int j = 0; j < DIM; j++) {
				for (int k = 0; k < 4; k++) {
					if (ability[i][j][k])
						cout << map_direct_to_str[k];
				}
				cout << " ";
			}
			cout << endl;
		}
	}
};

WalkAbility* walk_ability;

class State {
private:
	vector<vector<int>> star_dist;
	Location cur_loc;
	char direction;
	void gotoDst(Location dst) {
		dst.i = (dst.i+DIM)%DIM;
		dst.j = (dst.j+DIM)%DIM;
		star_dist[cur_loc.i][cur_loc.j] = star_dist[dst.i][dst.j];
		star_dist[dst.i][dst.j] = 0;
		cur_loc = dst;
	}
public:
	static State *create_state(vector<vector<int>> dist, Location my_loc) {
		State *s = new State;
		s->star_dist = dist;
		s->cur_loc = my_loc;
		return s;
	}
	State *walk(char direction) {
		State *s = create_state(star_dist, cur_loc);
		switch (direction)
		{
		case LEFT:
			s->gotoDst({cur_loc.i, cur_loc.j-1});
			s->direction = LEFT;
			break;
		case RIGHT:
			s->gotoDst({cur_loc.i, cur_loc.j+1});
			s->direction = RIGHT;
			break;
		case UP:
			s->gotoDst({cur_loc.i-1, cur_loc.j});
			s->direction = UP;
			break;
		case DOWN:
			s->gotoDst({cur_loc.i+1, cur_loc.j});
			s->direction = DOWN;
			break;
		default:
			break;
		}
		return s;
	}
	vector<State*> succ() {
		vector<State*> result;
		if (walk_ability->left(cur_loc)) {
			auto tmp = walk(LEFT);
			result.push_back(tmp);
		}
		if (walk_ability->right(cur_loc)) {
			auto tmp = walk(RIGHT);
			result.push_back(tmp);
		}
		if (walk_ability->up(cur_loc)) {
			auto tmp = walk(UP);
			result.push_back(tmp);
		}
		if (walk_ability->down(cur_loc)) {
			auto tmp = walk(DOWN);
			result.push_back(tmp);
		}
		return result;
	}
	vector<vector<int>>& get_dist() {
		return star_dist;
	}
	char get_direction() {
		return direction;
	}
	friend bool equal(State *r, State *s) {
		for (int i = 0; i < DIM; i++) {
			for (int j = 0; j < DIM; j++) {
				if (r->star_dist[i][j] != s->star_dist[i][j])
					return false;
			}
		}
		return true;
	}
	void print() {
		cout << map_direct_to_str[direction] << endl;
		for (int i = 0; i < DIM; i++) {
			for (int j = 0; j < DIM; j++) {
				cout << star_dist[i][j] << " ";
			}
			cout << endl;
		}
	}
};

int (*Hfunc)(const vector<vector<int>>&, const vector<vector<int>>&);
State *target_state;

class Node {
private:
	Node *parent;
	State *cur_state;
	int g;
	int eval;
	string traceback;
public:
	static Node* create_node(State *s, int g, string path) {
		Node *node = new Node;
		node->parent = nullptr;
		node->cur_state = s;
		node->g = g;
		node->eval = g+Hfunc(s->get_dist(), target_state->get_dist());
		node->traceback = path;
		return node;
	}
	void set_parent(Node *p) {parent = p;}
	Node* get_parent() {return parent;}
	bool goal_test() {
		return equal(cur_state, target_state);
	}
	int get_eval() const {
		return eval;
	}
	int get_g() const {
		return g;
	}
	State *get_state() const {
		return cur_state;
	}
	vector<Node*> succ() {
		auto succ_state = cur_state->succ();
		vector<Node*> result;
		for (auto s: succ_state) {
			auto node = create_node(s, g+1, traceback+map_direct_to_str[s->get_direction()]);
			node->set_parent(this);
			result.push_back(node);
		}
		return result;
	}
	string traverse() {
		return traceback;
	}
	void print() {
		cout << "g: " << g << " eval: " << eval << endl;
		cur_state->print();
		cout << endl;
	}
	~Node() {
		free(cur_state);
	}
};

class NodeLessCmp {
public:
	bool operator()(Node *a, Node *b) {
		if (equal(a->get_state(), b->get_state())) {
			return false;
		} else {
			if (a->get_eval() < b->get_eval()) {
				return true;
			} else if (a->get_eval() > b->get_eval()) {
				return false;
			} else {
				return a < b;
			}
		}
	}
};

int A_h1(const vector<vector<int>> &start, const vector<vector<int>>
&target) {
	int count = 0;
	for (int i = 0; i < DIM; i++) {
		for (int j = 0; j < DIM; j++) {
			if (start[i][j] != target[i][j] && target[i][j] != 0)
				count++;
		}
	}
	return count;
}
int A_h2(const vector<vector<int> > &start, const vector<vector<int>>
&target) {
	Location two_locs[DIM*DIM][2];
	for (int i = 0; i < DIM; i++) {
		for (int j = 0; j < DIM; j++) {
			two_locs[start[i][j]][0] = {i, j};
			two_locs[target[i][j]][1] = {i, j};
		}
	}
	int max_dist = 0;
	for (int i = 0; i < DIM*DIM; i++) {
		auto i_dist_abs = abs(two_locs[i][0].i - two_locs[i][1].i);
		auto j_dist_abs = abs(two_locs[i][0].j - two_locs[i][1].j);
		auto min_i_dist = min(i_dist_abs, DIM-i_dist_abs);
		auto min_j_dist = min(j_dist_abs, DIM-j_dist_abs);
		if (max_dist < min_i_dist + min_j_dist) {
			max_dist = min_i_dist + min_j_dist;
		}
	}
	auto ah1 = A_h1(start, target);
	if (max_dist < ah1) {
		max_dist = ah1;
	}
	return max_dist;
}
int IDA_h1(const vector<vector<int> > &start, const vector<vector<int>>
&target) {
	int count = 0;
	for (int i = 0; i < DIM; i++) {
		for (int j = 0; j < DIM; j++) {
			if (start[i][j] != target[i][j] && target[i][j] != 0)
				count++;
		}
	}
	return count;
}
int IDA_h2(const vector<vector<int> > &start, const vector<vector<int>>
&target) {
	Location two_locs[DIM*DIM][2];
	for (int i = 0; i < DIM; i++) {
		for (int j = 0; j < DIM; j++) {
			two_locs[start[i][j]][0] = {i, j};
			two_locs[target[i][j]][1] = {i, j};
		}
	}
	int max_dist = 0;
	for (int i = 0; i < DIM*DIM; i++) {
		auto i_dist_abs = abs(two_locs[i][0].i - two_locs[i][1].i);
		auto j_dist_abs = abs(two_locs[i][0].j - two_locs[i][1].j);
		auto min_i_dist = min(i_dist_abs, DIM-i_dist_abs);
		auto min_j_dist = min(j_dist_abs, DIM-j_dist_abs);
		if (max_dist < min_i_dist + min_j_dist) {
			max_dist = min_i_dist + min_j_dist;
		}
	}
	auto ah1 = A_h1(start, target);
	if (max_dist < ah1) {
		max_dist = ah1;
	}
	return max_dist;
}

typedef struct {} Empty;

Node *AStar(State *start_s, State *target_s, int (*Hfunc)(const vector<vector<int>> &start, const vector<vector<int>> &target)) {
	map<Node*, Empty, NodeLessCmp> open_list;
	target_state = target_s;
	Node* start_node = Node::create_node(start_s, 0, "");
	start_node->set_parent(nullptr);
	open_list[start_node] = {};
	Node* result = nullptr;
	while (!open_list.empty()) {
		auto n = open_list.begin()->first;
		open_list.erase(open_list.begin());
		if (n->goal_test()) {
			result = n;
			break;
		}
		auto succs = n->succ();
		for (auto succ: succs) {
			auto iter = open_list.find(succ);
			if (iter == open_list.end()) {
				open_list[succ] = {};
			} else if (iter->first->get_eval() > succ->get_eval()) {
				open_list.erase(iter);
				open_list[succ] = {};
			} else {
				delete succ;
				continue;
			}
		}
		delete n;
	}
	for (auto pair: open_list) {
		if (pair.first != result)
			delete pair.first;
	}
	return result;
}


Node *IDAStar(State *start_s, State *target_s, int (*Hfunc)(const vector<vector<int>> &start, const vector<vector<int>> &target)) {
	target_state = target_s;
	int d_limit = 32;
	map<Node*, Empty, NodeLessCmp> open_list;
	auto start_node = Node::create_node(start_s, 0, "");
	open_list[start_node] = {};
	Node* result = nullptr;
	while (1) {
		int next_d_limit = INT32_MAX;
		while (!open_list.empty()) {
			auto s = open_list.begin()->first;
			open_list.erase(open_list.begin());
			if (s->get_g() > d_limit) {
				next_d_limit = min(next_d_limit, s->get_g());
				continue;
			}
			if (s->goal_test()) {
				result = s;
				break;
			}
			auto succs = s->succ();
			for (auto succ: succs) {
				auto iter = open_list.find(succ);
				if (iter == open_list.end()) {
					open_list[succ] = {};
				} else if (succ->get_eval() < iter->first->get_eval()) {
					open_list.erase(iter);
					open_list[succ] = {};
				} else {
					delete succ;
					continue;
				}
			}
			delete s;
		}
		if (result) {
			break;
		}
		d_limit = next_d_limit;
	}
	for (auto pair: open_list) {
		if (pair.first != result)
			delete pair.first;
	}
	return result;
}

typedef struct {
	string solution;
	double duration;
}Result;

class Driver {
private:
	void clear() {
		free(walk_ability);
	}
	State *get_state_from_txt(string src) {
		ifstream fin;
		fin.open(src);
		string line;
		vector<vector<int>> dist;
		Location my_loc;
		for (int i = 0; i < DIM; i++) {
			vector<int> row;
			for (int j = 0; j < DIM; j++) {
				fin >> line;
				int num = stoi(line);
				row.push_back(num);
				if (num == 0) {
					my_loc.i = i;
					my_loc.j = j;
				}
			}
			dist.push_back(row);
		}
		fin.close();
		return State::create_state(dist, my_loc);
	}
	Node* algo(string algo, State *start_s, State *target_s) {
		walk_ability = new WalkAbility(start_s->get_dist());
		walk_ability->print();
		if (algo == "A_h1") {
			Hfunc = A_h1;
			return AStar(start_s, target_s, A_h1);
		} else if (algo == "A_h2") {
			Hfunc = A_h2;
			return AStar(start_s, target_s, A_h2);
		} else if (algo == "IDA_h1") {
			Hfunc = IDA_h1;
			return IDAStar(start_s, target_s, IDA_h1);
		} else if (algo == "IDA_h2") {
			Hfunc = IDA_h2;
			return IDAStar(start_s, target_s, IDA_h2);
		} else {
			cerr << "invalid algorithm " << algo;
			exit(1);
		}
	}
	void output(string algo_str, Result result) {
		ofstream fout;
		string output_txt = "../output/output_" + algo_str + ".txt";
		fout.open(output_txt, ios::app);
		fout << result.solution << "," << result.duration << endl;
		fout.close();
	}
public:
	void run(string algo_str, string input_txt, string target_txt) {
		clock_t start, end;
		auto start_s = get_state_from_txt(input_txt);
		auto target_s = get_state_from_txt(target_txt);
		start = clock();
		auto node_ptr = algo(algo_str, start_s, target_s);
		auto solution = node_ptr->traverse();
		delete node_ptr;
		end = clock();
		double duration = double(end-start)/CLOCKS_PER_SEC;
		output(algo_str, {solution, duration});
		clear();
	}
};

void getAll();

int main(int argc, char **argv) {
	// if (argc != 4) {
	// 	cerr << "3 argument need, get " << argc-1;
	// }
	// string algo = argv[1];
	// string input_txt = argv[2];
	// string target_txt = argv[3];
	// Driver driver;
	// driver.run(algo, input_txt, target_txt);
	getAll();
	return 0;
}

void getAll() {
	string input_prefix = "../../data/input", target_prefix = "../../data/target", suffix = ".txt";
	string input_txt = "", target_txt = "";
	Driver driver;
	for (int i = 0; i < 10; i++) {
		input_txt = input_prefix+"0"+to_string(i)+suffix;
		target_txt = target_prefix + "0" + to_string(i) + suffix;
		cout << input_txt << " " << target_txt << endl;
		driver.run("IDA_h2", input_txt, target_txt);
	}
	for (int i = 10; i < 12; i++) {
		input_txt = input_prefix+to_string(i)+suffix;
		target_txt = target_prefix + to_string(i) + suffix;
		cout << input_txt << " " << target_txt << endl;
		driver.run("IDA_h2", input_txt, target_txt);
	}
}