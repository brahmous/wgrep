#include <iostream>
#include <cassert>
#include <unordered_map>
#include "../engine/nfa.h"
#include <queue>

class Num {
public:
	int value;
	Num(int number) : value{ number }
	{}
};
//
//class Obj {
//public:
//	std::weak_ptr<Num> number;
//
//	Obj(int n) : number{ Num(n) }
//	{}
//};
template <typename T>
void print(const T& s) {
	int id = 0;

	std::unordered_map<State*, bool> visited;
	std::unordered_map<State*, int> named;


	std::queue<std::pair<int, std::shared_ptr<State>>> stack;

	stack.push({ id, s.start_state });
	named.insert({ s.start_state.get(), id });
	id++;

	while (stack.empty() != true)
	{
		std::pair<int, std::shared_ptr<State>> c = stack.front();
		stack.pop();

		if (visited.find(c.second.get()) == visited.end())
		{
			using Ts = std::pair<const wchar_t, std::vector<std::shared_ptr<State>>>;

			std::cout << "(" << c.first << "|" << (c.second->accepting ? "accepting" : "not accepting") << ") ";
			if (c.second->transitions.empty())
			{
				std::cout << "There are no character transitions ";
			}
			else {
				for (Ts t : c.second->transitions)
				{
					std::wcout << t.first << " => ";
					if (t.second.size() == 0) {
						std::cout << "There are no transitions on this character\n";
					}
					else {
						for (std::shared_ptr<State> s : t.second) {
							std::unordered_map<State*, int>::iterator name_info = named.find(s.get());
							if (name_info == named.end()) {
								stack.push({ id, s });
								std::cout << id << " ";
								named.insert({ s.get(), id });
								id++;
							}
							else {
								stack.push({ name_info->second, s });
								std::cout << name_info->second << " ";
							}
						}
					}
				}
			}

			std::cout << "| Epsilon => ";
			if (c.second->epsilon_transitions.size() == 0) {
				std::cout << "No epsilon transitions.";
			}
			else {
				for (std::shared_ptr<State> s : c.second->epsilon_transitions)
				{
					std::unordered_map<State*, int>::iterator name_info = named.find(s.get());
					if (name_info == named.end())
					{
						stack.push({ id, s });
						std::cout << id << " ";
						named.insert({ s.get(), id });
						id++;
					}
					else {
						stack.push({ name_info->second, s });
						std::cout << name_info->second << " ";
					}
				}
			}
			visited.insert({ c.second.get(), true });
			std::cout << "\n";
		}
	}
}

int main(int argc, char* argv[])
{
	State s1;
	State s2;

	Fragment f1(s1, s2, 'a');

	State s3;
	State s4;

	Fragment f2(s3, s4, 'b');

	NFA<UNION> n1{f1, f2};

	//State s5;
	//State s6;

	//Fragment f3(s5, s6, 'c');

	//State s7;
	//State s8;

	//Fragment f4(s7, s8, 'd');

	//NFA<UNION> n2{ f3, f4 };

	//NFA<CONCATENATION> n3(n1, n2);

	std::cout << n1 << std::endl;
};