#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <list>
#include <string>
#include <bitset>

#ifdef DEBUG
#define logf(...) printf(__VA_ARGS__);putchar('\n');fflush(stdout);
#define loguse(x) x;putchar('\n');
#else
#define logf(...) ;
#define loguse(x) ;
#endif

#define ONE_WEEK_DAYS 7
#define UNASSIGNED 2
#define RELAX 0
#define WORK 1


using namespace std;

typedef vector<int> Value;

//	0bit表示是否可放假,1bit表示是否可上班
typedef bitset<2> DOMAIN;
class CSP;
typedef struct Location {
	int day;
	int emp;
	void print() {
		printf("Location day%d emp%d", day+1, emp+1);
	}
}Location;
typedef struct DeleteValues {
	Location loc;
	list<int> value_list;
}DeleteValues;
typedef struct InferenceResult {
	list<DeleteValues> delete_domains;
	bool failure;
	void print() {
		printf("delete values:\n");
		for (auto delelte_value: delete_domains) {
			delelte_value.loc.print();
			putchar('\n');
			for (auto v: delelte_value.value_list) {
				printf("%d ", v);
			}
			putchar('\n');
		}
	}
}InferenceResult;
class State {
public:
	Value assignment[ONE_WEEK_DAYS];
	State();
	void print();
	void debugPrint();
};
vector<DOMAIN> domain[ONE_WEEK_DAYS];
class CSP {
private:
	bool (*checkState)(State &s);
	list<DeleteValues> (*conflict)(State &);
public:
	static int num_employees;
	static list<int> seniors;
	void init(int num_eple, bool (*check)(State &s), list<DeleteValues> (*)(State &), list<int> seniors_list);
	list<int> orderDomainValues(Location loc);
	bool checkConsistent(State &s);
	InferenceResult inference(State &s);
	void recoverFromInfer(InferenceResult &infer);
	Location selectUnassignedVar(State &s);
	bool checkComplete(State &s);
};

int CSP::num_employees = 0;
list<int> CSP::seniors = list<int>();

void valueAssignPrint(Value &v, CSP &csp);
void valuePrint(Value &v) {
	for (int i = 0; i < v.size(); i++) {
		if (v[i])
			printf("1 ");
		else
			printf("0 ");
	}
}
bool backtrack(State &s, CSP &csp);

void CSP::init(int num_eple, bool (*check)(State &s), list<DeleteValues> (*conflict_fn)(State &), list<int> seniors_list) {
	num_employees = num_eple;
	checkState = check;
	conflict = conflict_fn;
	seniors = seniors_list;
	for (int day = 0; day < ONE_WEEK_DAYS; day++) {
		domain[day] = vector<DOMAIN>(num_employees);
		for (int emp = 0; emp < num_employees; emp++) {
			domain[day][emp].set(WORK);
			domain[day][emp].set(RELAX);
		}
	}
}
list<int> CSP::orderDomainValues(Location loc) {
	list<int> result;
	if (domain[loc.day][loc.emp][RELAX]) {
		result.push_back(RELAX);
	}
	if (domain[loc.day][loc.emp][WORK]) {
		result.push_back(WORK);
	} 
	return result;
}
//	s为之前的状态,v为当前的赋值
bool CSP::checkConsistent(State &s) {
	return checkState(s);
}
InferenceResult CSP::inference(State &s) {
	//	前向检验
	list<DeleteValues> delete_values_list;
	auto conflict_values = conflict(s);
	for (auto conflict_value: conflict_values) {
		DeleteValues real_delete = {conflict_value.loc, {}};
		for (auto v: conflict_value.value_list) {
			if (domain[conflict_value.loc.day][conflict_value.loc.emp][v]) {
				domain[conflict_value.loc.day][conflict_value.loc.emp].reset(v);
				real_delete.value_list.push_back(v);
			} else {
				continue;
			}
		}
		delete_values_list.push_back(real_delete);
	}
	for (int day = 0; day < ONE_WEEK_DAYS; day++) {
		for (int emp = 0; emp < num_employees; emp++) {
			if (domain[day][emp].count() == 0) {
				return {delete_values_list, true};
			}
		}
	}
	return {delete_values_list, false};
}
void CSP::recoverFromInfer(InferenceResult &infer) {
	for (auto delete_values: infer.delete_domains) {
		for (auto v: delete_values.value_list) {
			domain[delete_values.loc.day][delete_values.loc.emp].set(v);
		}
	}
}
Location CSP::selectUnassignedVar(State &s) {
	Location min_loc;
	int min_domain = 3;
	for (int day = 0; day < ONE_WEEK_DAYS; day++) {
		for (auto senior: seniors) {
			if (s.assignment[day][senior] == UNASSIGNED) {
				return {day, senior};
			}
		}
		for (int emp = 0; emp < num_employees; emp++) {
			if (s.assignment[day][emp] == UNASSIGNED) {
				if (domain[day][emp].count() < min_domain) {
					min_loc = {day, emp};
					min_domain = domain[day][emp].count();
				}
			}
		}
	}
	return min_loc;
}
bool CSP::checkComplete(State &s) {
	for (int day = 0; day < ONE_WEEK_DAYS; day++) {
		for (int emp = 0; emp < num_employees; emp++) {
			if (s.assignment[day][emp] == UNASSIGNED) {
				return false;
			}
		}
	}
	return true;
}
State::State() {
	int num_emp = CSP::num_employees;
	for (int day = 0; day < ONE_WEEK_DAYS; day++) {
		assignment[day] = Value(num_emp);
		for (int emp = 0; emp < num_emp; emp++) {
			assignment[day][emp] = UNASSIGNED;
		}
	}
}
void State::print() {
	int num_emp = CSP::num_employees;
	string result = "";
	for (int i = 0; i < ONE_WEEK_DAYS; i++) {
		for (int emp = 0; emp < num_emp; emp++) {
			if (assignment[i][emp] == WORK) {
				result += to_string(emp+1) + " ";
			}
		}
		result += "\n";
	}
	cout << result;
}
void State::debugPrint() {
	int num_emp = CSP::num_employees;
	printf("      ");
	for (int emp = 0; emp < num_emp; emp++) {
		printf("%d ", emp+1);
	}
	putchar('\n');
	for (int day = 0; day < ONE_WEEK_DAYS; day++) {
		printf("day%d: ", day+1);
		for (int emp = 0; emp < num_emp; emp++) {
			printf("%d ", assignment[day][emp]);
		}
		putchar('\n');
	}
}

bool backtrack(State &s, CSP &csp) {
	if (csp.checkComplete(s))
		return true;
	auto var = csp.selectUnassignedVar(s);
	auto value_list = csp.orderDomainValues(var);
	logf("-----------------------------")
	loguse(var.print())
	for (auto value: value_list) {
		auto old_v = s.assignment[var.day][var.emp];
		s.assignment[var.day][var.emp] = value;
		logf("assign %d", value)
		loguse(s.debugPrint())
		if (csp.checkConsistent(s)) {
			auto infer = csp.inference(s);
			loguse(infer.print();)
			if (!infer.failure) {
				auto result = backtrack(s, csp);
				if (result) {
					return true;
				}
			}
			logf("recover");
			csp.recoverFromInfer(infer);
		}
		s.assignment[var.day][var.emp] = old_v;
	}
	return false;
}

bool atLeastOneSenior(int day, list<int> seniors, State &s) {
	bool all_assigned = true;
	for (auto senior: seniors) {
		if (s.assignment[day][senior] == UNASSIGNED) {
			all_assigned = false;
		} else if (s.assignment[day][senior] == WORK) {
			return true;
		}
	}
	return !all_assigned;
}

bool checkStateGeneral(State &s, int num_emp, int at_least_count) {
	vector<bool> emp_all_assigned = vector<bool>(num_emp);
	for (int emp = 0; emp < num_emp; emp++) {
		emp_all_assigned[emp] = true;
	}
	for (int day = 0; day < ONE_WEEK_DAYS; day++) {
		bool one_day_all_assigned = true;
		int work_count = 0;
		if (!atLeastOneSenior(day, CSP::seniors, s)) {
			return false;
		}
		for (int emp = 0; emp < num_emp; emp++) {
			if (s.assignment[day][emp] == UNASSIGNED) {
				emp_all_assigned[emp] = false;
				one_day_all_assigned = false;
			} else if (s.assignment[day][emp] == WORK) {
				work_count++;
			}
		}
		// logf("day%d assigned?%d", day+1, one_day_all_assigned)
		if (!one_day_all_assigned) {
			continue;
		} else {
			if (work_count < at_least_count) {
				logf("day%d work %d < %d", day+1, work_count, at_least_count)
				return false;
			}
		}
	}
	for (int emp = 0; emp < num_emp; emp++) {
		int continue_relax_count = 0;
		int relax_count = 0;
		bool unassigned_flag = false;
		for (int day = 0; day < ONE_WEEK_DAYS; day++) {
			if (s.assignment[day][emp] == RELAX) {
				continue_relax_count++;
				if (continue_relax_count >= 3) {
					logf("emp%d continue relax >= 3", emp+1)
					return false;
				}
				relax_count++;
			} else if (s.assignment[day][emp] == WORK) {
				continue_relax_count = 0;
			} else {
				unassigned_flag = true;
				break;
			}
		}
		if (unassigned_flag) {
			continue;
		}
		if (relax_count < 2) {
			logf("emp%d relax < 2", emp+1)
			return false;
		}
	}
	return true;
}

bool checkState1(State &s) {
	// logf("------------check")
	// loguse(s.debugPrint())
	return checkStateGeneral(s, 7, 4);
}

bool checkState2(State &s) {
	return checkStateGeneral(s, 10, 5);
}

list<DeleteValues> conflict(State &s, int day, int emp, int emp1, int emp2) {
	list<DeleteValues> result_list;
	DeleteValues delete_values;
	if (emp == emp1 && s.assignment[day][emp] == WORK && s.assignment[day][emp2] == UNASSIGNED) {
		delete_values.loc = {day, emp2};
		delete_values.value_list.push_back(WORK);
		result_list.push_back(delete_values);
	}
	if (emp == emp2 && s.assignment[day][emp] == WORK && s.assignment[day][emp1] == UNASSIGNED) {
		delete_values.loc = {day, emp1};
		delete_values.value_list.push_back(WORK);
		result_list.push_back(delete_values);
	}
	return result_list;
}

list<DeleteValues> conflict1(State &s) {
	int num_emp = 7;
	list<DeleteValues> result_list;
	for (int day = 0; day < ONE_WEEK_DAYS; day++) {
		for (int emp = 0; emp < num_emp; emp++) {
			//	1 4
			result_list.splice(result_list.end(), conflict(s, day, emp, 0, 3));
			//	2 3
			result_list.splice(result_list.end(), conflict(s, day, emp, 1, 2));
			//	3 6
			result_list.splice(result_list.end(), conflict(s, day, emp, 2, 5));
		}
	}
	return result_list;
}

list<DeleteValues> conflict2(State &s) {
	int num_emp = 10;
	list<DeleteValues> result_list;
	for (int day = 0; day < ONE_WEEK_DAYS; day++) {
		for (int emp = 0; emp < num_emp; emp++) {
			//	1 5
			result_list.splice(result_list.end(), conflict(s, day, emp, 0, 4));
			//	2 6
			result_list.splice(result_list.end(), conflict(s, day, emp, 1, 5));
			//	8 10
			result_list.splice(result_list.end(), conflict(s, day, emp, 7, 9));
		}
	}
	return result_list;

}

int main(void) {
	CSP csp1;
	list<int> senior1 = {0, 1};
	csp1.init(7, checkState1, conflict1, senior1);
	State s;
	backtrack(s, csp1);
	freopen("../output/output1.txt", "w", stdout);
	s.print();
	loguse(s.debugPrint();)

	CSP csp2;
	//	1 2 8 10
	list<int> senior2 = {0, 1, 7, 9};
	csp2.init(10, checkState2, conflict2, senior2);
	State s2;
	backtrack(s2, csp2);
	freopen("../output/output2.txt", "w", stdout);
	s2.print();
	loguse(s2.debugPrint();)
	return 0;
}