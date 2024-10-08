#include "nfa.h"
#include <iostream>

static std::unordered_map<const State*, std::shared_ptr<State>> visited;
static std::shared_ptr<State> dead_state = std::make_shared<State>(false);

State::State(const State& other) {

  this->set_accepting(other.accepting());
  for (std::pair<char, std::vector<std::shared_ptr<State>>> old : other.get_transitions()) {
    std::pair<trns_t::iterator, bool> new_trns =
      _trns.insert({ old.first, std::vector<std::shared_ptr<State>>(old.second.size()) });

    for (size_t i = 0; i < old.second.size(); i++) {
      std::unordered_map<const State*, std::shared_ptr<State>>::iterator dest = visited.find(old.second.at(i).get());

      if (dest == visited.end()) {
        new_trns.first->second[i] = std::make_shared<State>(*(old.second.at(i)));
        visited.insert({ old.second.at(i).get(), new_trns.first->second[i] });
      }
      else {
        new_trns.first->second[i] = dest->second;
      }
    }
  }
};

State& State::operator=(const State& other) {
  this->set_accepting(other.accepting());

  for (std::pair<char, std::vector<std::shared_ptr<State>>> old : other.get_transitions()) {
    std::pair<trns_t::iterator, bool> new_trns =
      _trns.insert({ old.first, std::vector<std::shared_ptr<State>>(old.second.size()) });

    for (size_t i = 0; i < old.second.size(); i++) {
      std::unordered_map<const State*, std::shared_ptr<State>>::iterator dest = visited.find(old.second.at(i).get());

      if (dest == visited.end()) {
        new_trns.first->second[i] = std::make_shared<State>(*(old.second.at(i)));
        visited.insert({ old.second.at(i).get(), new_trns.first->second[i] });
      }
      else {
        new_trns.first->second[i] = dest->second;
      }
    }
  }

  return *this;
}

bool State::accepting() { return _accepting; }

const bool State::accepting() const { return _accepting; }

void State::set_accepting(bool accepting) { this->_accepting = accepting; };

/*
  tr: transition.
 */
void State::add_transition(const char c, std::shared_ptr<State> state) {
  // see if there is a transition from char
  std::map<char, std::vector<std::shared_ptr<State>>>::iterator tr_over_char =
    _trns.find(c);
  // if there isn't create one
  // else push_back
  if (tr_over_char == _trns.end()) {
    _trns.insert(
      std::make_pair(c, std::vector<std::shared_ptr<State>>({ state })));
  }
  else {
    tr_over_char->second.push_back(state);
  }
};

State::trns_t::iterator State::get_transition(char character) {
  return _trns.find(character);
};

State::etrns_t& State::get_eps_transitions() { return _etrs; };

const State::etrns_t& State::get_eps_transitions() const { return _etrs; };

State::trns_t& State::get_transitions() { return _trns; };

const State::trns_t& State::get_transitions() const { return _trns; };

void State::add_eps_transition(std::shared_ptr<State> state) {
  _etrs.push_back(state);
};

NFA::NFA(const char character): _start_state {std::make_shared<State>(false)}
{
  std::shared_ptr<State> end_state = std::make_shared<State>(true);
  _start_state->add_transition(character, end_state);
  _end_state = end_state;
};

NFA::NFA(const NFA& other) {
  visited.insert({ other.entry().get(), std::make_shared<State>(*other.entry()) });

  std::unordered_map<const State*, std::shared_ptr<State>>::iterator entry = visited.find(other.entry().get());
  if (entry == visited.end()) {
    throw std::runtime_error("couldn't find entry");
  }
  else {
    _start_state = entry->second;
  }

  std::unordered_map<const State*, std::shared_ptr<State>>::iterator exit = visited.find(other.exit().get());
  if (exit == visited.end()) {
    throw std::runtime_error("couldn't find exit");
  }
  else {
    _end_state = exit->second;
  }
  visited.clear();
}

NFA& NFA::operator=(const NFA& other) {
  visited.insert({ other.entry().get(), std::make_shared<State>(*other.entry()) });

  std::unordered_map<const State*, std::shared_ptr<State>>::iterator entry = visited.find(other.entry().get());
  if (entry == visited.end()) {
    throw std::runtime_error("couldn't find entry");
  }
  else {
    _start_state = entry->second;
  }

  std::unordered_map<const State*, std::shared_ptr<State>>::iterator exit = visited.find(other.exit().get());
  if (exit == visited.end()) {
    throw std::runtime_error("couldn't find exit");
  }
  else {
    _end_state = exit->second;
  }
  visited.clear();
  return *this;
}

NFA& NFA::concat(const char character) {
  std::shared_ptr<State> new_accepting_state = std::make_shared<State>(true);
  if (std::shared_ptr<State> old_accepting_state = _end_state.lock()) {
    old_accepting_state->set_accepting(false);
    old_accepting_state->add_transition(character, new_accepting_state);
    _end_state = new_accepting_state;
  };

  return *this;
};

NFA& NFA::concat(NFA& nfa) {
  if (std::shared_ptr<State> old_acc_state = _end_state.lock()) {
    old_acc_state->set_accepting(false);
    old_acc_state->add_eps_transition(nfa.entry());
  }
  else {
    throw std::runtime_error("end state cannot be turned into shared pointer");
  }
  _end_state = nfa.exit();
  return *this;
};

NFA& NFA::_or(const char character) {
  // take the start state and add transitiosn on it to the end state
  // MAYBE: just pass the weak ptr and there is auto conversion on the fly.
  if (std::shared_ptr<State> exit = _end_state.lock()) {
    _start_state->add_transition(character, exit);
  }
  return *this;
};

NFA& NFA::_or(NFA& nfa) {
  // new start and end states
  // add eps transitions from new start to this start and to nfa start
  // then add eps transition from nfa end to new end and add eps transition from
  // this end to new end

  this->entry()->add_eps_transition(nfa.entry());
  nfa.exit()->set_accepting(false);
  nfa.exit()->add_eps_transition(this->exit());

  return *this;
};

NFA& NFA::repeat() {
  this->exit()->add_eps_transition(this->entry());
  this->entry()->add_eps_transition(this->exit());
  return *this;
};

// NFA& NFA::repeat(std::size_t n)
//{
//   // make n copies and connect them with epsilon transitions.
//   std::vector<NFA> machines(n, *this);
//
//   std::shared_ptr<State> machine_exit;
//
//   for(std::size_t i=0; i<n-1; i++) {
//     machine_exit = machines[i].exit();
//     machine_exit->add_eps_transition(machines[i + 1].entry());
//     machine_exit->set_accepting(false);
//   }
//
//   return *this;
// }

NFA& NFA::repeat(std::size_t n) {
  // make n copies and connect them with epsilon transitions.
  std::vector<NFA> machines;

  for (size_t i = 0; i < n; i++) {
    machines.push_back(*this);
  }

  for (std::size_t i = 0; i < n - 1; i++) {
    machines[i].concat(machines[i + 1]);
  }

  // could be faster by using current machine as start without copy.
  _start_state = machines[0].entry();
  _end_state = machines[n - 1].exit();

  return *this;
}

NFA& NFA::atmost(const std::size_t n) {
  std::vector<NFA> machines;

  for (size_t i = 0; i < n; i++) {
    machines.push_back(*this);
  }

  std::shared_ptr<State> last_machine_exit = machines[n - 1].exit();

  for (std::size_t i = 0; i < n - 1; i++) {
    machines[i].concat(machines[i + 1]);
  }

  for (std::size_t i = 0; i < n; i++)
  {
    machines[i].entry()->add_eps_transition(last_machine_exit);
  }

  _start_state = machines[0].entry();
  _end_state = last_machine_exit;

  return *this;
}

NFA& NFA::atleast(const std::size_t n) {
  if (n == 1) {
    this->exit()->add_eps_transition(this->entry());
  }
  else {
    std::vector<NFA> machines;

    for (int i = 0; i < n; i++) {
      machines.push_back(*this);
    }

    for (std::size_t i = 0; i < n - 1; i++) {
      machines[i].concat(machines[i + 1]);
    }

    machines[n - 1].atleast(1);
    _start_state = machines[0].entry();
    _end_state = machines[n - 1].exit();
  }

  return *this;
}

NFA& NFA::between(const std::size_t n, const std::size_t m) {
  if (m < n) {
    throw std::runtime_error("n must be smaller than n!");
  }
  else if (n == m) {
    this->repeat(n);
  }
  else {
    NFA atmost = *this;
    atmost.atmost(m - n);

    this->repeat(n);
    this->concat(atmost);
  }

  return *this;
}

NFA& NFA::exclude(const char character)
{
  this->entry()->add_transition(character, dead_state);
  return *this;
}

bool match(const std::string& str) { return true; }