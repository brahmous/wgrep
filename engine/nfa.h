#include <iostream>
#include <map>
#include <vector>
#include <memory>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <string>

constexpr const wchar_t UNION = '|';
constexpr const wchar_t CONCATENATION = 'ε';
constexpr const wchar_t KLEENE = '*';

class State {
public:
	std::map<char, std::vector<std::shared_ptr<State>>> transitions; 
	std::vector<std::shared_ptr<State>> epsilon_transitions; 
	bool accepting;

	State(bool accepting = false) : accepting{ accepting }
	{
		std::cout << "default constructor of State executed!\n";
	}

	State(const State& other) = default;
	State& operator=(const State& other) = default;

	State(State&& other) noexcept
		: transitions{ std::move(other.transitions) },
		epsilon_transitions{ std::move(other.epsilon_transitions) },
		accepting{ other.accepting }
	{}

	State& operator=(State&& other) noexcept
	{
		transitions = std::move(other.transitions);
		epsilon_transitions = std::move(other.epsilon_transitions);
		accepting = other.accepting;
	}

	int id = 0;

	~State() = default;
};


class Fragment {
public:
	Fragment()
		: start_state{ std::make_shared<State>(false) }
	{
		std::shared_ptr<State> accepting_state = std::make_shared<State>(true);
		start_state->epsilon_transitions.push_back(accepting_state);
		end_state = accepting_state;
	}

	Fragment(char character)
		: start_state{ std::make_shared<State>(false) }
	{
		std::shared_ptr<State> accepting_state = std::make_shared<State>(true);
		start_state->transitions.insert(std::make_pair(character, std::vector<std::shared_ptr<State>>({ accepting_state })));
		end_state = accepting_state;
	}

	std::shared_ptr<State> start_state;
	std::weak_ptr<State> end_state;
};

template <wchar_t U>
class NFA
{
public:
	// Union
	template <typename T, typename N, wchar_t C = U, std::enable_if_t<C == UNION, bool> = true>
	NFA(const T& s1, const N& s2)
		: start_state{ std::make_shared<State>(false) }
	{
		std::shared_ptr<State> accepting_state = std::make_shared<State>(true);
		std::cout << "UNION CONSTRUCTOR" << std::endl;
		start_state->epsilon_transitions.push_back(s1.start_state);
		start_state->epsilon_transitions.push_back(s2.start_state);

		if (std::shared_ptr<State> s1_accepting_state = s1.end_state.lock())
		{
			s1_accepting_state->accepting = false;
			s1_accepting_state->epsilon_transitions.push_back(accepting_state);
		}
		else {
			throw std::runtime_error("Error upper");
		}

		if (std::shared_ptr<State> s2_accepting_state = s2.end_state.lock())
		{
			s2_accepting_state->accepting = false;
			s2_accepting_state->epsilon_transitions.push_back(accepting_state);
		}
		else {
			throw std::runtime_error("Error lower");
		}
		end_state = accepting_state;
	}
	// KLEENE
	template <typename T, wchar_t C = U, std::enable_if_t<C == KLEENE, bool> = true>
	NFA(const T& s)
		: start_state{ std::make_shared<State>(false) }
	{
		std::cout << "KLEENE CONSTRUCTOR" << std::endl;

		std::shared_ptr<State> accepting_state = std::make_shared<State>(true);
		start_state->epsilon_transitions.push_back(accepting_state);
		start_state->epsilon_transitions.push_back(s.start_state);
		if (std::shared_ptr<State> s_accepting_state = s.end_state.lock()) {
			s_accepting_state->epsilon_transitions.push_back(accepting_state);
			s_accepting_state->accepting = false;
		}
		else {
			throw std::runtime_error("error accpeting state");
		}
		accepting_state->epsilon_transitions.push_back(s.start_state);

		end_state = accepting_state;
	}

	// CONCATENATION
	template <typename T, typename N, wchar_t C = U, std::enable_if_t<(C == CONCATENATION), bool> = true>
	NFA(const T& s1, const N& s2)
		: start_state{ s1.start_state } {
		std::cout << "Concatenation constructor called\n" << std::endl;
		if (std::shared_ptr<State> s1_accepting_state = s1.end_state.lock())
		{
			s1_accepting_state->epsilon_transitions.push_back(s2.start_state);
			s1_accepting_state->accepting = false;
		}
		else {
			throw std::runtime_error("Reference error first");
		}

		if (std::shared_ptr<State> s2_accepting_state = s2.end_state.lock()) {
			s2_accepting_state->accepting = true;
			end_state = s2_accepting_state;
		}
		else {
			throw std::runtime_error("Reference error second");
		}
	}

	std::shared_ptr<State> start_state;
	std::weak_ptr<State> end_state;
};


template <wchar_t C>
std::ostream& operator << (std::ostream& out, const NFA<C>& nfa)
{
	std::size_t id = 1;
	std::map<std::size_t, std::shared_ptr<State>> id_state_map;
	std::map<State*, std::size_t> state_id_map;
	std::map<char, bool> alphabet;

	std::queue<std::shared_ptr<State>> queue;
	std::map<State*, bool> visited;
	std::size_t number_of_states = 0;


	queue.push({ nfa.start_state });
	visited.insert({ nfa.start_state.get(), true });

	while (!queue.empty())
	{
		std::shared_ptr<State> current = queue.front();
		state_id_map.insert({ current.get(), id });
		id_state_map.insert({ id, current });
		for (std::pair <char, std::vector<std::shared_ptr<State>>> ts : current->transitions)
		{
			alphabet.insert({ ts.first, true });
			for (std::shared_ptr<State> s : ts.second) {
				if (visited.find(s.get()) != visited.end()) continue;
				queue.push(s);
				visited.insert({ s.get(), true });
			}
		}
		for (std::shared_ptr<State> s : current->epsilon_transitions)
		{
			if (visited.find(s.get()) != visited.end()) continue;
			queue.push(s);
			visited.insert({ s.get(), true });
		}
		queue.pop();
		id++;
		number_of_states++;
	}

	std::vector<char> alphabet_s;
	alphabet_s.push_back(' ');

	for (std::pair<char, bool> letter : alphabet)
	{
		alphabet_s.push_back( letter.first );
	}
	std::sort(alphabet_s.begin(), alphabet_s.end());
	alphabet_s.push_back('*');

	std::vector<std::vector<std::string>> mtx(number_of_states + 1, std::vector<std::string>(alphabet_s.size()));

	for (std::size_t i = 0; i < alphabet_s.size(); i++)
	{
		mtx[0][i] = alphabet_s[i];
	}

	for (std::size_t i = 1; i < number_of_states + 1; i++)
	{
		std::map<std::size_t, std::shared_ptr<State>>::iterator state_pair = id_state_map.find(i);
		if (i == 1) {
			mtx[i][0] = ">" + std::to_string(i);
		}
		else if (state_pair->second->accepting) {
			mtx[i][0] = "'"+std::to_string(i);
		}
		else {
			mtx[i][0] = std::to_string(i);
		}
	}


	for (std::size_t i = 1; i < number_of_states + 1; i++) {
		std::map <std::size_t, std::shared_ptr<State>>::iterator state = id_state_map.find(i);
		if (state == id_state_map.end()) {
			throw std::runtime_error("no transition found");
		}
		else {
			for (std::size_t j = 1; j < alphabet_s.size()-1; j++) {
				std::map<char, std::vector<std::shared_ptr<State>>>::iterator transition = state->second->transitions.find(alphabet_s[j]);
				if (transition == state->second->transitions.end()) {
					mtx[i][j] = "-";
				}
				else {
					std::string entry = "";
					for (std::vector<std::shared_ptr<State>>::iterator s = transition->second.begin();
						s != transition->second.end();
						s++)
					{
						std::map<State*, std::size_t>::iterator state_id = state_id_map.find(s->get());
						if (state_id != state_id_map.end()) {
							if (s == transition->second.end() - 1) {
								entry += std::to_string(state_id->second);
							}
							else {
								entry += std::to_string(state_id->second) + ",";
							}
						}
						else {
							throw std::runtime_error("some error here");
						}
					}
					mtx[i][j] = entry;
				}
			}
			std::string epscl = "";
			for (std::shared_ptr<State> s : state->second->epsilon_transitions) {
				epscl += std::to_string(state_id_map.find(s.get())->second) + ",";
			}
			epscl += std::to_string(state_id_map.find(state->second.get())->second);
			mtx[i][alphabet_s.size() - 1] = epscl;
		}
	}

	std::vector<std::size_t> maxs(alphabet_s.size());

	for (std::size_t j = 0; j < alphabet_s.size(); j++) {
		std::size_t max = 1;
		for (std::size_t i = 0; i < number_of_states + 1; i++) {
			if (mtx[i][j].size() > max) {
				max = mtx[i][j].size();
			}
		}
		maxs[j] = max;
	}

	for (std::size_t i = 0; i < number_of_states + 1; i++) {
		for (std::size_t j = 0; j < alphabet_s.size(); j++) {
			std::string str = "";
			for (int s = 0; s < 2; s++) str += " ";
			str += mtx[i][j];
			for (int s = 0; s < maxs[j] - mtx[i][j].size(); s++) {
				str += " ";
			}
			for (int s = 0; s < 2; s++) str += " ";
			std::cout << str << "|";
		}
		std::cout << "\n";
	}

	return out;
};