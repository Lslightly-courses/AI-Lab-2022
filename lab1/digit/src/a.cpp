#include <iostream>
#include <chrono>
#include <fstream>
#include <vector>
#include <string>
#include <bitset>
#include <stack>
#include <set>
#include <map>
#include <list>
#include <ctime>

#define DIM 5
#define NODIRECTION 4
#define LEFT 3
#define RIGHT 2
#define UP 1
#define DOWN 0
#ifdef OUT
#define OUTPUT(x) x
#else
#define OUTPUT(x) ;
#endif
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
class WalkAbility;	//	行走能力
class State;		//	状态
class Node;			//	节点
list<State*> state_list;	//	状态集合

//	方向字符串
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
	/**
	 * @brief 构建行走能力表格
	 * 
	 * @param input 输入方格
	 */
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
	/**
	 * @brief Create a state object
	 * 
	 * @param dist 分布图
	 * @param my_loc 飞船当前状态，即0号位置
	 * @return State* 
	 */
	static State *create_state(vector<vector<int>> dist, Location my_loc) {
		State *s = new State;
		s->star_dist = dist;
		s->cur_loc = my_loc;
		s->direction = NODIRECTION;
		state_list.push_back(s);
		return s;
	}
	/**
	 * @brief 根据方向产生新状态
	 * 
	 * @param direction 方向
	 * @return State* 
	 */
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
	/**
	 * @brief 求后继状态集合
	 * 
	 * @return vector<State*> 
	 */
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

/**
 * @brief 检查是否为相反方向，若为相反方向，则返回true,否则返回false
 * 
 * @param a 方向a
 * @param b 方向b
 * @return true 相反方向
 * @return false 相同方向
 */
bool reverseDirection(char a, char b) {
	switch (a) {
		case LEFT:
			if (b == RIGHT) {
				return true;
			} else {
				return false;
			}
		case RIGHT:
			if (b == LEFT) {
				return true;
			} else {
				return false;
			}
		case UP:
			if (b == DOWN) {
				return true;
			} else {
				return false;
			}
		case DOWN:
			if (b == UP) {
				return true;
			} else {
				return false;
			}
		case NODIRECTION:	//	都不是相反方向
			return false;
		default:
			return false;
	}
}

class Node {
private:
	State *cur_state;
	int g;				//	已走路径
	int eval;			//	eval=g+h
	string traceback;	//	回溯路径，这里直接继承下来。
public:
	static Node* create_node(State *s, int g, string path) {
		Node *node = new Node();
		node->cur_state = s;
		node->g = g;
		node->eval = g+Hfunc(s->get_dist(), target_state->get_dist());
		node->traceback = path;
		return node;
	}
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
	/**
	 * @brief 创建node的后续节点，对于返回产生该节点的节点情况进行忽略处理。同时需要将路径记录在新生成的节点中。
	 * 
	 * @return vector<Node*> 
	 */
	vector<Node*> succ() {
		auto succ_state = cur_state->succ();
		vector<Node*> result;
		for (auto s: succ_state) {
			if (reverseDirection(s->get_direction(), cur_state->get_direction())) {
				continue;
			}
			auto node = create_node(s, g+1, traceback+map_direct_to_str[s->get_direction()]);
			result.push_back(node);
		}
		return result;
	}
	/**
	 * @brief 
	 * 
	 * @return string 路径
	 */
	string Trace() {
		return traceback;
	}
	void print() {
		cout << "g: " << g << " eval: " << eval << endl;
		cur_state->print();
		cout << endl;
	}
};

/**
 * @brief 节点比较大小，如果两个节点的状态相同，则视为相同，否则按照eval进行排序。
 * 
 * 如果eval还相同，则按照Node *指针大小进行排序
 * 
 */
class NodeLessCmp {
public:
	bool operator()(Node *a, Node *b) const {
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

/**
 * @brief 错位星球数，不包括飞船本身。显然admissible
 * 
 * @param start 
 * @param target 
 * @return int 
 */
int h1(const vector<vector<int>> &start, const vector<vector<int>> &target) {
	int count = 0;
	for (int i = 0; i < DIM; i++) {
		for (int j = 0; j < DIM; j++) {
			if (start[i][j] != target[i][j] && target[i][j] != 0)
				count++;
		}
	}
	return count;
}

/**
 * @brief 每个星球的绝对曼哈顿距离之和
 * 
 * 对于每个点，假设当前位置为{i, j}, 目标位置为{x, y}
 * 则绝对曼哈顿距离定义为min{|i-x|, 5-|i-x|}+min{|j-y|, 5-|j-y|}
 * 
 * 简单来说，就是在边界上将隧道全部打通，飞船可以在任何边界上进行跳跃。
 * 
 * 因为每个点必须通过飞船才能移动，而该点到达目标点的最短距离就是上面的绝对曼哈顿距离
 * 另外，由于飞船每次只能移动一个点，而不会对其他点有影响
 * 
 * 因此，所有星球的绝对曼哈顿距离之和一定小于真实移动步数。即admissible
 * 
 * @param start 开始状态
 * @param target 目标状态
 * @return int 
 */
int h2(const vector<vector<int>> &start, const vector<vector<int>> &target) {
	Location two_locs[DIM*DIM][2];
	for (int i = 0; i < DIM*DIM; i++) {
		two_locs[i][0] = {0, 0};
		two_locs[i][1] = {0, 0};
	}
	for (int i = 0; i < DIM; i++) {
		for (int j = 0; j < DIM; j++) {
			if (start[i][j] < 0) continue;
			if (target[i][j] < 0) continue;
			two_locs[start[i][j]][0] = {i, j};
			two_locs[target[i][j]][1] = {i, j};
		}
	}
	int max_dist = 0;
	for (int i = 1; i < DIM*DIM; i++) {
		auto i_dist_abs = abs(two_locs[i][0].i - two_locs[i][1].i);
		auto j_dist_abs = abs(two_locs[i][0].j - two_locs[i][1].j);
		auto min_i_dist = min(i_dist_abs, DIM-i_dist_abs);
		auto min_j_dist = min(j_dist_abs, DIM-j_dist_abs);
		max_dist += min_i_dist+min_j_dist;
	}
	auto h1_result = h1(start, target);
	if (max_dist < h1_result) {
		max_dist = h1_result;
	}
	return max_dist;
}

/**
 * @brief 获取飞船当前位置
 * 
 * @param dist 当前分布
 * @return Location 
 */
Location getMyLocation(const vector<vector<int>> &dist) {
	Location my_loc;
	for (int i = 0; i < DIM; i++) {
		for (int j = 0; j < DIM; j++) {
			if (dist[i][j] == 0) {
				my_loc.i = i;
				my_loc.j = j;
				return my_loc;
			}
		}
	}
	return {0, 0};
}

Node *AStar(State *start_s, State *target_s, int (*Hfunc)(const vector<vector<int>> &start, const vector<vector<int>> &target));
Node *IDAStar(State *start_s, State *target_s, int (*Hfunc)(const vector<vector<int>> &start, const vector<vector<int>> &target));

string solution_str;

//	调用A*,使用启发式h1
void A_h1(const vector<vector<int>> &start, const vector<vector<int>> &target) {
	auto start_s = State::create_state(start, getMyLocation(start));
	
	//	初始化行动能力表
	walk_ability = new WalkAbility(start_s->get_dist());
	auto target_s = State::create_state(target, getMyLocation(target));
	Hfunc = h1;
	auto n = AStar(start_s, target_s, h1);
	auto solution = n->Trace();
	cout << solution << endl;
	solution_str = solution;
	free(walk_ability);
}
//	调用A*,使用启发式h2
void A_h2(const vector<vector<int> > &start, const vector<vector<int>> &target) {
	auto start_s = State::create_state(start, getMyLocation(start));
	walk_ability = new WalkAbility(start_s->get_dist());
	auto target_s = State::create_state(target, getMyLocation(target));
	Hfunc = h2;
	auto n = AStar(start_s, target_s, h2);
	auto solution = n->Trace();
	cout << solution << endl;
	solution_str = solution;
	free(walk_ability);
}
//	调用IDA*,使用启发式h1
void IDA_h1(const vector<vector<int> > &start, const vector<vector<int>> &target) {
	auto start_s = State::create_state(start, getMyLocation(start));
	walk_ability = new WalkAbility(start_s->get_dist());
	auto target_s = State::create_state(target, getMyLocation(target));
	Hfunc = h1;
	auto n = IDAStar(start_s, target_s, h1);
	auto solution = n->Trace();
	cout << solution << endl;
	solution_str = solution;
	free(walk_ability);
}
//	调用IDA*,使用启发式h2
void IDA_h2(const vector<vector<int> > &start, const vector<vector<int>> &target) {
	auto start_s = State::create_state(start, getMyLocation(start));
	walk_ability = new WalkAbility(start_s->get_dist());
	auto target_s = State::create_state(target, getMyLocation(target));
	Hfunc = h2;
	auto n = IDAStar(start_s, target_s, h2);
	auto solution = n->Trace();
	cout << solution << endl;
	solution_str = solution;
	free(walk_ability);
}

typedef struct {} Empty;

/**
 * @brief tree search + admissible版本
 * 包含open_list和closed_list
 * 
 * @param start_s 
 * @param target_s 
 * @param Hfunc 
 * @return Node* 
 */
Node *AStar(State *start_s, State *target_s, int (*Hfunc)(const vector<vector<int>> &start, const vector<vector<int>> &target)) {
	//	通过map实现的优先队列，即可以进行排序，又方便查找
	map<Node*, Empty, NodeLessCmp> open_list;
	set<Node*, NodeLessCmp> closed_list;
	target_state = target_s;
	Node* start_node = Node::create_node(start_s, 0, "");
	open_list[start_node] = {};
	Node* result = nullptr;
	while (!open_list.empty()) {
		auto n = open_list.begin()->first;	//	eval最小的节点
		open_list.erase(open_list.begin());	//	pop
		if (n->goal_test()) {
			result = n;
			break;
		}
		auto succs = n->succ();
		for (auto succ: succs) {
			auto closed_iter = closed_list.find(succ);
			auto open_iter = open_list.find(succ);
			if (closed_iter == closed_list.end() && open_iter == open_list.end()) {	//	在open_list和closed_list中都没有找到，则直接加入open_list
				DEBUGOUT("not in closed list nor open list")
				open_list[succ] = {};
			} else {	//	在某一个list中找到，要么是open_list,要么是closed_list
				if (open_iter != open_list.end()) {
					if (open_iter->first->get_eval() > succ->get_eval()) {	//	如果发现eval < open_list中的结果，则进行更新
						DEBUGOUT("in open list and <")
						//	删除再插入可以进行更新
						open_list.erase(open_iter);
						open_list[succ] = {};
					} else {	//	直接回收，不考虑
						DEBUGOUT("in open list > delete")
						delete succ;
						DEBUGOUT("delete succeed")
						continue;
					}
				} else {	//	closed_list
					if ((*closed_iter)->get_eval() > succ->get_eval()) {	//	eval < closed_list中的结果，则需要从closed_list中删除，并添加到open_list中
						DEBUGOUT("in closed list and <")
						closed_list.erase(closed_iter);
						open_list[succ] = {};
					} else {	//	在closed_list中存在且不小于，则删除
						DEBUGOUT("in closed list but > delete")
						delete succ;
						DEBUGOUT("delete succeed")
						continue;
					}
				}
			}
		}
	}
	//	free
	for (auto pair: open_list) {
		if (pair.first != result)
			delete pair.first;
	}
	for (auto node: closed_list) {
		delete node;
	}
	for (auto s: state_list) {
		delete s;
	}
	state_list.clear();
	return result;
}

/**
 * @brief 根据eval来限制深度优先遍历的IDA*
 * 
 * @param start_s 
 * @param target_s 
 * @param Hfunc 
 * @return Node* 
 */
Node *IDAStar(State *start_s, State *target_s, int (*Hfunc)(const vector<vector<int>> &start, const vector<vector<int>> &target)) {
	target_state = target_s;
	
	auto start_node = Node::create_node(start_s, 0, "");
	int d_limit = start_node->get_eval();	//	最大结果为32,因此设置32，可以防止最长例子的进一步搜索。
	stack<Node*> list;	//	使用栈实现深度优先
	Node* result = nullptr;
	while (1) {
		int next_d_limit = INT32_MAX;
		list.push(start_node);
		while (!list.empty()) {
			auto n = list.top();	//	pop
			list.pop();

			if (n->get_eval() > d_limit) {	//	如果大于d_limit,则忽视该节点
				next_d_limit = min(next_d_limit, n->get_eval());
				continue;
			}
			if (n->goal_test()) {
				result = n;
				break;
			}
			auto succs = n->succ();	//	apply actions to s
			for (auto succ: succs) {
				list.push(succ);
			}
		}
		if (result) {
			break;
		}
		d_limit = next_d_limit;
	}
	while (!list.empty()) {
		auto n = list.top();
		delete n;
		list.pop();
	}
	for (auto s: state_list) {
		delete s;
	}
	state_list.clear();
	return result;
}

class Driver {
private:
	map<string, string> statistic_str;
	map<string, string> output_str;
	/**
	 * @brief Get the vec from input.txt and target.txt
	 * 
	 * @param src 
	 * @return vector<vector<int>> 
	 */
	vector<vector<int>> get_vec_from_txt(string src) {
		ifstream fin;
		fin.open(src);
		string line;
		vector<vector<int>> dist;
		for (int i = 0; i < DIM; i++) {
			vector<int> row;
			for (int j = 0; j < DIM; j++) {
				fin >> line;
				int num = stoi(line);
				row.push_back(num);
			}
			dist.push_back(row);
		}
		fin.close();
		return dist;
	}
	/**
	 * @brief 根据algo执行对应算法
	 * 
	 * @param algo 
	 * @param start 
	 * @param target 
	 */
	void algo(string algo, vector<vector<int>> &start, vector<vector<int>> &target) {
		if (algo == "A_h1") {
			A_h1(start, target);
		} else if (algo == "A_h2") {
			A_h2(start, target);
		} else if (algo == "IDA_h1") {
			IDA_h1(start, target);
		} else if (algo == "IDA_h2") {
			IDA_h2(start, target);
		} else {
			cerr << "invalid algorithm " << algo;
			exit(1);
		}
	}
	/**
	 * @brief 统计执行时间和行动序列等
	 * 
	 * @param algo 
	 * @param input_txt 
	 * @param target_txt 
	 * @param i 
	 */
	void statistic(string algo, string input_txt, string target_txt, int i) {
		cout << algo << ":\t" << input_txt << "\t";
		auto start = chrono::system_clock::now();
		run(algo, input_txt, target_txt);
		auto end = chrono::system_clock::now();
		auto duration = chrono::duration_cast<chrono::microseconds>(end-start);
		auto time = double(duration.count())*chrono::microseconds::period::num / chrono::microseconds::period::den;
		statistic_str[algo] += to_string(i) + "," + to_string(time) + "," + solution_str + "," + to_string(solution_str.length()) + "\n";
		output_str[algo] += solution_str+","+to_string(time)+"\n";
	}
	void output(string algo) {
		auto output_txt = "../output/output_"+algo+".txt";
		auto statistic_txt = "../output/statistic_"+algo+".txt";
		auto f = freopen(output_txt.c_str(), "w", stdout);
		cout << output_str[algo];
		fclose(f);
		freopen(statistic_txt.c_str(), "w", stdout);
		cout << statistic_str[algo];
	}
public:
	void runall() {
		string input_prefix = "../data/input", target_prefix = "../data/target";
		string suffix = ".txt";
		vector<string> num_suffixs, targets;
		for (int i = 0; i <= 9; i++) {
			auto num_suffix = "0"+to_string(i)+suffix;
			num_suffixs.push_back(num_suffix);
		}
		for (int i = 10; i <= 11; i++) {
			auto num_suffix = to_string(i)+suffix;
			num_suffixs.push_back(num_suffix);
		}
		statistic_str["A_h1"] = "";
		statistic_str["A_h2"] = "";
		statistic_str["IDA_h1"] = "";
		statistic_str["IDA_h2"] = "";
		output_str["A_h1"] = "";
		output_str["A_h2"] = "";
		output_str["IDA_h1"] = "";
		output_str["IDA_h2"] = "";
		int i = 0;
		for (auto num_suffix: num_suffixs) {
			auto input_txt = input_prefix+num_suffix, target_txt = target_prefix+num_suffix;
			statistic("A_h1", input_txt, target_txt, i);
			statistic("A_h2", input_txt, target_txt, i);
			statistic("IDA_h1", input_txt, target_txt, i);
			statistic("IDA_h2", input_txt, target_txt, i);
			i++;
		}
		output("A_h1");
		output("A_h2");
		output("IDA_h1");
		output("IDA_h2");
	}
	void run(string algo_str, string input_txt, string target_txt) {
		auto start = get_vec_from_txt(input_txt);
		auto target = get_vec_from_txt(target_txt);
		algo(algo_str, start, target);
	}
};

int main(int argc, char **argv) {
	if (argc != 4) {
		cerr << "3 argument need, get " << to_string(argc-1);
	}
	string algo = argv[1];
	string input_txt = argv[2];
	string target_txt = argv[3];
	Driver driver;
	driver.run(algo, input_txt, target_txt);
	OUTPUT(driver.runall();)	// 使用条件编译-DOUT可执行对所有input.txt执行
	return 0;
}
