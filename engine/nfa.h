﻿#include <iostream>
#include <map>
#include <vector>
#include <memory>
#include <stack>
#include <unordered_map>

constexpr const wchar_t UNION = '|';
constexpr const wchar_t CONCATENATION = 'ε';
constexpr const wchar_t KLEENE = '*';

/*
	1 (t) [(c, 2)(e, 4)]
	2 (f) []
*/	

class State {
public:
	std::map<const wchar_t, std::vector<std::shared_ptr<State>>> transitions;
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


/*
	make DFS matcher/printer
*/

class Fragment{
public:
	// TODO: Should I take these as non const Rvalue references?
	Fragment(const State& s1, const State& s2)
		: start_state{ std::make_shared<State>(s1) }
	{
		std::shared_ptr<State> accepting_state = std::make_shared<State>(s2);

		start_state->accepting = false;
		accepting_state->accepting = true;

		start_state->epsilon_transitions.push_back(accepting_state);
		end_state = accepting_state;
	}

	// TODO: Should I take these as non const Rvalue references?
	Fragment(const State& s1, const State& s2, const wchar_t character)
		: start_state {std::make_shared<State>(s1)}
	{
		std::shared_ptr<State> accepting_state = std::make_shared<State> (s2);
		start_state->accepting = false;
		accepting_state->accepting = true;
		start_state->transitions.insert(std::make_pair(character, std::vector<std::shared_ptr<State>>({accepting_state})));
		end_state = accepting_state;
	}

	std::shared_ptr<State> start_state;
	std::weak_ptr<State> end_state;
};

template <const wchar_t U>
class NFA
{
public:
	// Union
	template <typename T, typename N, const wchar_t C = U, std::enable_if_t<C == UNION, bool> = true>
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
	template <typename T, const wchar_t C = U, std::enable_if_t<C == KLEENE, bool> = true>
	NFA(const T& s)
		: start_state{ std::make_shared<State>(false) }
	{
		std::cout << "KLEENE CONSTRUCTOR" << std::endl;

		std::shared_ptr<State> accepting_state = std::make_shared<State>(true);
		// Skip the machine 
		start_state->epsilon_transitions.push_back(accepting_state);
		// Or enter the machine
		start_state->epsilon_transitions.push_back(s.start_state);
		// Go out of the machine to the accepting state
		if (std::shared_ptr<State> s_accepting_state = s.end_state.lock()) {
			s_accepting_state->epsilon_transitions.push_back(accepting_state);
			s_accepting_state->accepting = false;
		}
		else {
			throw std::runtime_error("error accpeting state");
		}
		// Renter the machine from the accepting state
		accepting_state->epsilon_transitions.push_back(s.start_state);

		end_state = accepting_state;
	}

	// CONCATENATION
	template <typename T, typename N, const wchar_t C = U, std::enable_if_t<(C == CONCATENATION), bool> = true>
	NFA(const T& s1, const N& s2, const wchar_t character)
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