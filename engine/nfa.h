#ifndef NFA_H 
#define NFA_H

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>

class NFATransitionKey {
public:
  using type = typename std::string;
  NFATransitionKey(const std::string& key): _key { key }
  {};
  NFATransitionKey(const char key) : _key{ key }
  {};

  bool operator==(const NFATransitionKey& other) const
  {
    /*
      this is what I don't know!!
    */
    return _key == other._key;
  }

  bool operator<(const NFATransitionKey& other) const
  {
    return _key < other._key;
  }

  std::string key()
  {
    return _key;
  }

  const std::string key() const
  {
    return _key;
  }

private:
  std::string _key;
};

class State {
public:
  using transition_key = NFATransitionKey;
  using transitions_t = std::map<transition_key, std::shared_ptr<State>>;
  using epsilon_transitions_t = std::vector<std::shared_ptr<State>>;

  State(bool accepting = false) : _accepting{ accepting } {};

  State(const State& other);
  State& operator=(const State& other);

  void set_accepting(bool accepting);

  void add_transition(const transition_key::type, std::shared_ptr<State> state);

  void add_epsilon_transition(std::shared_ptr<State> state);

  bool accepting();
  const bool accepting() const;

  template <typename C = transition_key::type>
  transitions_t::iterator get_transition(C);

  transitions_t& get_transitions();
  const transitions_t& get_transitions() const;

  epsilon_transitions_t& get_epsilon_transitions();
  const epsilon_transitions_t& get_epsilon_transitions() const;

  // MAYBE: get transitions, get transition given char, get epsilon transitions
private:
  transitions_t _transitions;
  epsilon_transitions_t _epsilon_transitions;
  bool _accepting;
};

class NFA {
public:
  NFA() = delete;
  NFA(const State::transition_key::type);

  NFA(const NFA& other);
  NFA& operator=(const NFA& other);

  NFA(NFA&& other) noexcept = default;
  NFA& operator=(NFA&& other) noexcept = default;

  ~NFA() = default;

  NFA& concat(const State::transition_key::type);
  NFA& concat(NFA& nfa);

  NFA& _or(const State::transition_key::type);
  NFA& _or(NFA nfa);

  NFA& repeat();

  NFA& repeat(const std::size_t n);
  NFA& atmost(const std::size_t count);
  NFA& atleast(const std::size_t count);
  NFA& between(const std::size_t n, std::size_t m);
  NFA& exclude(const State::transition_key::type);

  bool match(const std::string& input);

  friend std::ostream& operator<<(std::ostream& out, const NFA& nfa);

  // Check if it moves on return by value or should a reference be used.
  std::shared_ptr<State> entry() { return _start_state; }

  const std::shared_ptr<State> entry() const { return _start_state; }
  // Check if it moves on return by value or should a reference be used.
  std::shared_ptr<State> exit() {
    if (std::shared_ptr<State> state = _end_state.lock()) {
      return state;
    }
  }

  const std::shared_ptr<State> exit() const {
    if (std::shared_ptr<State> state = _end_state.lock()) {
      return state;
    }
  }

private:
  std::shared_ptr<State> _start_state;
  std::weak_ptr<State> _end_state;
};
#endif NFA_H
