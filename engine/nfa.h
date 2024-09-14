#include <iostream>
#include <map>
#include <vector>
#include <memory>

constexpr const wchar_t UNION = '|';
constexpr const wchar_t CONCATENATION = '~';
constexpr const wchar_t EPSILON = 'ε';
constexpr const wchar_t KLEENE = '*';


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

	~State() = default;
};

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
	template <typename T, const wchar_t C = U, std::enable_if_t<C == UNION, bool> = true>
	NFA(const T& s1, const T& s2) {
		std::cout << "UNION CONSTRUCTOR" << std::endl;
	}
	// KLEENE
	template <typename T, const wchar_t C = U, std::enable_if_t<C == KLEENE, bool> = true>
	NFA(const T& s1, const T& s2) {
		std::cout << "KLEENE CONSTRUCTOR" << std::endl;

	}
	// EPSILON
	template <typename T, const wchar_t C = U, std::enable_if_t<C == EPSILON, bool> = true>
	NFA(const T& s1, const T& s2) {
		std::cout << "EPSILON CONSTRUCTOR" << std::endl;

	}
	// CONCATENATION
	template <typename T, const wchar_t C = U, std::enable_if_t<(C != UNION && C != KLEENE && C != EPSILON), bool> = true>
	NFA(const T& s1, const T& s2, const wchar_t character) {
		std::wcout << "CONCATENATION CONSTRUCTOR: "<< character << std::endl;

	}
	std::shared_ptr<State> start_state;
	std::weak_ptr<State> state_end;
};