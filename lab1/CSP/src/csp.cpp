#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <list>
#include <string>

#ifdef DEBUG
#define logf(...) printf(__VA_ARGS__);putchar('\n');fflush(stdout);
#define loguse(x) x;putchar('\n');
#else
#define logf(...) ;
#define loguse(x) ;
#endif

#define ONE_WEEK_DAYS 7

using namespace std;

typedef vector<bool> Value;
class CSP;
typedef struct InferenceResult {
	list<Value> delete_domain;
	bool success;
}InferenceResult;
class State {
public:
	Value day_values[ONE_WEEK_DAYS];
	State() {}
	State(CSP &csp);
	void print(CSP &csp);
	void debugPrint(CSP &csp);
};
class CSP {
private:
	int num_employees;
	list<Value> domain[ONE_WEEK_DAYS];
	bool (*checkValue)(Value &v);
public:
	int get_num_emp() {return num_employees;}
	void genAllValue(int order, int num_eple, Value v1, list<Value> &value_list);
	void initDomain(int num_eple, bool (*check)(Value &v));
	list<Value> orderDomainValues(int i, State &assignment);
	bool checkConsistent(State &old_s, Value v, int day);
	InferenceResult inference(State &s, int day);
	int selectUnassignedVar(int i);
	bool checkComplete(State &s);
	list<Value> setDomain(list<Value> &value_list, int day);
};

class Result {
public:
	State solution;
	bool success;
	Result() {
		solution = State();
		success = false;
	}
	Result(State &s, bool f): solution(s), success(f) {};
	Result(const Result &r) {
		solution = r.solution;
		success = r.success;
	}
};
void valueAssignPrint(Value &v, CSP &csp, int day);
void valuePrint(Value &v) {
	for (int i = 0; i < v.size(); i++) {
		if (v[i])
			printf("1 ");
		else
			printf("0 ");
	}
}
Result backtrack(State &s, int day, CSP &csp);


void CSP::genAllValue(int order, int num_eple, Value v1, list<Value> &value_list) {
	auto v2 = v1;
	if (order == num_eple-1) {
		v1.push_back(false);
		value_list.push_back(v1);
		v2.push_back(true);
		value_list.push_back(v2);
		return ;
	}
	v1.push_back(false);
	v2.push_back(true);
	genAllValue(order+1, num_eple, v1, value_list);
	genAllValue(order+1, num_eple, v2, value_list);
}
void CSP::initDomain(int num_eple, bool (*check)(Value &v)) {
	num_employees = num_eple;
	checkValue = check;
	list<Value> value_list;
	Value v;
	genAllValue(0, num_eple, v, value_list);
	list<Value> filterd_value_list;
	// loguse(
	// 	for (int i = 0; i < num_eple; i++) {
	// 		printf("%d ", i+1);
	// 	}
	// )
	for (auto v: value_list) {
		if (check(v)) {
			// loguse(valuePrint(v))
			filterd_value_list.push_back(v);
		}
	}
	for (auto i = 0; i < ONE_WEEK_DAYS; i++) {
		domain[i] = filterd_value_list;
	}
}
list<Value> CSP::orderDomainValues(int i, State &assignment) {
	//	TODO optimize
	return domain[i];
}
//	s为之前的状态,v为当前的赋值
bool CSP::checkConsistent(State &s, Value v, int day) {
	vector<int> count_relax(num_employees);
	for (int i = 0; i < num_employees; i++) {
		count_relax[i] = 0;
	}
	bool flag = true;
	for (int i = 0; i <= day; i++) {
		for (int emp = 0; emp < num_employees; emp++) {
			if (s.day_values[i][emp]) {
				count_relax[emp] = 0;
			} else {
				count_relax[emp]++;
				if (count_relax[emp] >= 3) {
					flag = false;
					break;
				}
			}
		}
		if (!flag) {
			break;
		}
	}
	return flag;
}
InferenceResult CSP::inference(State &s, int day) {
	//	TODO optimize
	return {{}, true};
}
int CSP::selectUnassignedVar(int i) {
	return i+1;
}
bool CSP::checkComplete(State &s) {
	vector<int> relax_count(num_employees);
	for (int i = 0; i < num_employees; i++) {
		relax_count[i] = 0;
	}
	for (int day = 0; day < ONE_WEEK_DAYS; day++) {
		for (int employee = 0; employee < num_employees; employee++) {
			if (!s.day_values[day][employee]) {
				relax_count[employee]++;
			}
		}
	}
	for (int employee = 0; employee < num_employees; employee++) {
		if (relax_count[employee] < 2) {
			return false;
		}
	}
	return true;
}
list<Value> CSP::setDomain(list<Value> &value_list, int day) {
	auto old_domain = domain[day];
	domain[day] = value_list;
	return old_domain;
}
State::State(CSP &csp) {
	int num_emp = csp.get_num_emp();
	for (int day = 0; day < ONE_WEEK_DAYS; day++) {
		day_values[day] = Value(num_emp);
		for (int emp = 0; emp < num_emp; emp++) {
			day_values[day][emp] = false;
		}
	}
}
void State::print(CSP &csp) {
	int num_emp = csp.get_num_emp();
	string result = "";
	for (int i = 0; i < ONE_WEEK_DAYS; i++) {
		for (int emp = 0; emp < num_emp; emp++) {
			if (day_values[i][emp]) {
				result += to_string(emp+1) + " ";
			}
		}
		result += "\n";
	}
	cout << result;
}
void State::debugPrint(CSP &csp) {
	int num_emp = csp.get_num_emp();
	printf("      ");
	for (int emp = 0; emp < num_emp; emp++) {
		printf("%d ", emp+1);
	}
	putchar('\n');
	for (int i = 0; i < ONE_WEEK_DAYS; i++) {
		printf("day%d: ", i+1);
		for (int emp = 0; emp < num_emp; emp++) {
			if (day_values[i][emp]) {
				printf("%d ", 1);
			} else {
				printf("%d ", 0);
			}
		}
		putchar('\n');
	}
}

void valueAssignPrint(Value &v, CSP &csp, int day) {
	int num_emp = csp.get_num_emp();
	cout << "      ";
	for (int i = 0; i < num_emp; i++) {
		cout << i+1 << " ";
	}
	cout << endl;
	cout << "day" << day+1 << ": ";
	for (int i = 0; i < num_emp; i++) {
		cout << v[i] << " ";
	}
	cout << endl;
}

Result backtrack(State &s, int day, CSP &csp) {
	if (day == ONE_WEEK_DAYS) {
		if (csp.checkComplete(s))
			return Result(s, true);
		else
			return Result(s, false);
	}
	auto var = csp.selectUnassignedVar(day);
	auto value_list = csp.orderDomainValues(day, s);
	// logf("-----------------------------")
	// loguse(s.debugPrint(csp))
	// logf("day%d begin assignment", day+1)
	for (auto value: value_list) {
		// loguse(valueAssignPrint(value, csp, day))
		Value old_v = s.day_values[day];
		s.day_values[day] = value;
		if (csp.checkConsistent(s, value, day)) {
			auto infer_result = csp.inference(s, day);
			if (infer_result.success) {
				auto result = backtrack(s, day+1, csp);
				if (result.success) {
					return result;
				}
			}
		}
		s.day_values[day] = old_v;
	}
	return Result(s, false);
}

bool checkValue1(Value &v) {
	int count = 0;
	for (auto emp: v) {
		if (emp) count++;
	}
	if (count < 4) { return false; }
	//	1	4
	if (v[0] & v[3]) { return false; }
	//	2	3
	if (v[1] & v[2]) { return false; }
	//	3	6
	if (v[2] & v[5]) { return false; }
	//	0 1 senior
	if (!(v[0] | v[1])) { return false; }
	return true;
}

bool checkValue2(Value &v) {
	int count = 0;
	for (auto emp: v) {
		if (emp) count++;
	}
	if (count < 5) { return false; }
	//	1	5
	if (v[0] & v[4]) { return false; }
	//	2	6
	if (v[1] & v[5]) { return false; }
	//	8	10
	if (v[7] & v[9]) { return false; }
	//	1 2 8 10 senior
	if (!(v[0] | v[1] | v[7] | v[9])) { return false; }
	return true;
}


int main(void) {
	CSP csp1;
	csp1.initDomain(7, checkValue1);
	State s(csp1);
	auto result = backtrack(s, 0, csp1);
	if (result.success) {
		cout << "csp1: \n";
		result.solution.print(csp1);
		loguse(result.solution.debugPrint(csp1);)
	}
	CSP csp2;
	csp2.initDomain(10, checkValue2);
	s = State(csp2);
	result = backtrack(s, 0, csp2);
	if (result.success) {
		cout << "csp2: \n";
		result.solution.print(csp2);
		loguse(result.solution.debugPrint(csp2);)
	}
	return 0;
}