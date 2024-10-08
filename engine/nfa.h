#ifndef NFA_H 
#define NFA_H

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>


class State {
 public:
  using trns_t = std::map<char, std::vector<std::shared_ptr<State>>>;
  using etrns_t = std::vector<std::shared_ptr<State>>;

  State(bool accepting = false) : _accepting{accepting} {};

  State(const State& other);
  State& operator=(const State& other);

  void set_accepting(bool accepting);

  void add_transition(const char c, std::shared_ptr<State> state);

  void add_eps_transition(std::shared_ptr<State> state);

  bool accepting();
  const bool accepting() const;

  trns_t::iterator get_transition(char character);

  trns_t& get_transitions();
  const trns_t& get_transitions() const;

  etrns_t& get_eps_transitions();
  const etrns_t& get_eps_transitions() const;

  // MAYBE: get transitions, get transition given char, get epsilon transitions
 private:
  trns_t _trns;
  etrns_t _etrs;
  bool _accepting;
};

class NFA {
 public:
  NFA() = delete;
  NFA(char c);

  NFA(const NFA& other);
  NFA& operator=(const NFA& other);

  NFA(NFA&& other) noexcept = default;
  NFA& operator=(NFA&& other) noexcept = default;

  ~NFA() = default;

  NFA& concat(const char c);
  NFA& concat(NFA& nfa);

  NFA& _or(const char character);
  NFA& _or(NFA& nfa);

  NFA& repeat();

  NFA& repeat(const std::size_t n);
  NFA& atmost(const std::size_t count);
  NFA& atleast(const std::size_t count);
  NFA& between(const std::size_t n, std::size_t m);
  NFA& exclude(const char character);

  bool match(const std::string& str);

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

// 0x00000287dcabfcb0