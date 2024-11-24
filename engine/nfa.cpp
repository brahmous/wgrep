#include "nfa.h"
#include <iostream>
#include <stack>
/**/
static std::unordered_map<const State*, std::shared_ptr<State>> visited;
static std::unordered_map<const State*, bool> seen;
static std::map<State*, std::pair<State::transition_key::type, State*>> cycle_map;
static std::map<State*, std::vector<State*>> cycle_map_epsilon;

static std::shared_ptr<State> dead_state = std::make_shared<State>(false);

State::State(const State& other) {
  set_accepting(other.accepting());
  seen.insert(std::make_pair(&other, true));

  auto other_transitions = other.get_transitions();

  for (auto other_transition : other_transitions) {
    //auto transition = _transitions.insert({ other_transition.first, std::vector<std::shared_ptr<State>>(/*other_transition.second.size()*/) });
    //for(auto state: other_transition.second) {
      if (seen.find(other_transition.second.get()) == seen.end()) {
        auto new_transition = _transitions.insert({ other_transition.first, std::make_shared<State>(*other_transition.second) });
        visited.insert(std::make_pair(other_transition.second.get(), (new_transition.first->second)));
      }
      else {
        cycle_map.insert({ this, {other_transition.first.key(), other_transition.second.get()}});
      }
    //}
  }

  auto other_epsilon_transition = other.get_epsilon_transitions();

  for (auto state : other_epsilon_transition) {
    if (seen.find(state.get()) == seen.end()) {
      _epsilon_transitions.push_back(std::make_shared<State>(*state));
      visited.insert(std::make_pair(state.get(), _epsilon_transitions.back()));
    }
    else {
      auto epsilon_cycles = cycle_map_epsilon.find(this);

      if (epsilon_cycles == cycle_map_epsilon.end()) {
        cycle_map_epsilon.insert({ this, std::vector<State*>(1, state.get()) });
      }
      else {
        epsilon_cycles->second.push_back(state.get());
      }
    }
  }
};

State& State::operator=(const State& other) {
  set_accepting(other.accepting());
  seen.insert(std::make_pair(this, true));

  auto other_transitions = other.get_transitions();

  for (auto other_transition : other_transitions) {
    //auto transition = _transitions.insert({ other_transition.first, std::vector<std::shared_ptr<State>>(/*other_transition.second.size()*/) });
    //for(auto state: other_transition.second) {
    if (seen.find(other_transition.second.get()) == seen.end()) {
      auto new_transition = _transitions.insert({ other_transition.first, std::make_shared<State>(*other_transition.second) });
      visited.insert(std::make_pair(other_transition.second.get(), (new_transition.first->second)));
    }
    else {
      cycle_map.insert({ this, {other_transition.first.key(), other_transition.second.get()} });
    }
    //}
  }

  auto other_epsilon_transition = other.get_epsilon_transitions();

  for (auto state : other_epsilon_transition) {
    if (seen.find(state.get()) == seen.end()) {
      _epsilon_transitions.push_back(std::make_shared<State>(*state));
      visited.insert(std::make_pair(state.get(), _epsilon_transitions.back()));
    }
    else {
      auto epsilon_cycles = cycle_map_epsilon.find(this);

      if (epsilon_cycles == cycle_map_epsilon.end()) {
        cycle_map_epsilon.insert({ this, std::vector<State*>(1, state.get()) });
      }
      else {
        epsilon_cycles->second.push_back(state.get());
      }
    }
  }

  return *this;
}

bool State::accepting() {
  return _accepting;
}

const bool State::accepting() const {
  return _accepting;
}

void State::set_accepting(bool accepting) {
  this->_accepting = accepting;
};

/*
  tr: transition.
 */
void State::add_transition(const State::transition_key::type str, std::shared_ptr<State> state) {
  // see if there is a transition from char
  _transitions[str] = state;
};

template <typename C>
State::transitions_t::iterator State::get_transition(C str) {
  return _transitions.find(str);
};

State::epsilon_transitions_t& State::get_epsilon_transitions() {
  return _epsilon_transitions;
};

const State::epsilon_transitions_t& State::get_epsilon_transitions() const {
  return _epsilon_transitions;
};

State::transitions_t& State::get_transitions() {
  return _transitions;
};

const State::transitions_t& State::get_transitions() const {
  return _transitions;
};

void State::add_epsilon_transition(std::shared_ptr<State> state) {
  _epsilon_transitions.push_back(state);
};

NFA::NFA(const State::transition_key::type str) : _start_state{ std::make_shared<State>(false) } {
  std::shared_ptr<State> end_state = std::make_shared<State>(true);
  _start_state->add_transition(str, end_state);
  _end_state = end_state;
};

NFA::NFA(const NFA& other) {
  visited.insert(
    { other.entry().get(), std::make_shared<State>(*other.entry()) });

  std::unordered_map<const State*, std::shared_ptr<State>>::iterator entry =
    visited.find(other.entry().get());
  if (entry == visited.end()) {
    throw std::runtime_error("couldn't find entry");
  }
  else {
    _start_state = entry->second;
  }

  std::shared_ptr<State> sptr = other.exit();
  std::unordered_map<const State*, std::shared_ptr<State>>::iterator exit =
    visited.find(sptr.get());

  if (exit == visited.end()) {
     throw std::runtime_error("couldn't find exit");
  }
  else {
    _end_state = exit->second;
  }

  for (auto cycle : cycle_map) {
    auto state = visited.find(cycle.second.second);
    if (state != visited.end()) {
      cycle.first->add_transition(cycle.second.first, state->second);
    }
    else {
      throw std::runtime_error("couldn't find the cycle in cycle_node map");
    }
  }
  
  for (auto cycle : cycle_map_epsilon) {
    for (State* state_p : cycle.second) {
      auto state = visited.find(state_p);

      if (state != visited.end()) {
        cycle.first->add_epsilon_transition(state->second);
      }
      else {
        throw std::runtime_error("couldn't find the cycle in cycle_node map");
      }
    }
  }

  visited.clear();
  seen.clear();
  cycle_map.clear();
  cycle_map_epsilon.clear();
}

NFA& NFA::operator=(const NFA& other) {
  visited.insert(
    { other.entry().get(), std::make_shared<State>(*other.entry()) });

  std::unordered_map<const State*, std::shared_ptr<State>>::iterator entry =
    visited.find(other.entry().get());
  if (entry == visited.end()) {
    throw std::runtime_error("couldn't find entry");
  }
  else {
    _start_state = entry->second;
  }

  std::shared_ptr<State> sptr = other.exit();
  std::cout << "address: " << sptr.get() << std::endl;
  std::unordered_map<const State*, std::shared_ptr<State>>::iterator exit =
    visited.find(sptr.get());

  if (exit == visited.end()) {
    throw std::runtime_error("couldn't find exit");
  }
  else {
    _end_state = exit->second;
  }

  for (auto cycle : cycle_map) {
    auto state = visited.find(cycle.second.second);
    if (state != visited.end()) {
      cycle.first->add_transition(cycle.second.first, state->second);
    }
    else {
      throw std::runtime_error("couldn't find the cycle in cycle_node map");
    }
  }

  for (auto cycle : cycle_map_epsilon) {
    for (State* cycle_p : cycle.second) {
      auto state = visited.find(cycle_p);

      if (state != visited.end()) {
        cycle.first->add_epsilon_transition(state->second);
      }
      else {
        throw std::runtime_error("couldn't find the cycle in cycle_node map");
      }
    }
  }

  visited.clear();
  seen.clear();
  cycle_map.clear();
  cycle_map_epsilon.clear();
  return *this;
}

NFA& NFA::concat(const State::transition_key::type str)
{
  std::shared_ptr<State> new_accepting_state = std::make_shared<State>(true);
  if (std::shared_ptr<State> old_accepting_state = _end_state.lock()) {
    old_accepting_state->set_accepting(false);
    old_accepting_state->add_transition(str, new_accepting_state);
    _end_state = new_accepting_state;
  };

  return *this;
};

NFA& NFA::concat(NFA& nfa) {
  if (std::shared_ptr<State> old_acc_state = _end_state.lock()) {
    old_acc_state->set_accepting(false);
    old_acc_state->add_epsilon_transition(nfa.entry());
  }
  else {
    throw std::runtime_error("end state cannot be turned into shared pointer");
  }
  _end_state = nfa.exit();
  return *this;
};

NFA& NFA::_or(const State::transition_key::type str)
{
  // take the start state and add transitiosn on it to the end state
  // MAYBE: just pass the weak ptr and there is auto conversion on the fly.
  if (std::shared_ptr<State> exit = _end_state.lock()) {
    _start_state->add_transition(str, exit);
  }
  return *this;
};

NFA& NFA::_or(NFA nfa) {
  // new start and end states
  // add eps transitions from new start to this start and to nfa start
  // then add eps transition from nfa end to new end and add eps transition from
  // this end to new end

  this->entry()->add_epsilon_transition(nfa.entry());
  nfa.exit()->set_accepting(false);
  nfa.exit()->add_epsilon_transition(this->exit());

  return *this;
};

NFA& NFA::repeat() {
  this->exit()->add_epsilon_transition(this->entry());
  this->entry()->add_epsilon_transition(this->exit());
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

  for (std::size_t i = 1; i < n; i++) {
    machines[0].concat(machines[i]);
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

  for (std::size_t i = 0; i < n; i++) {
    machines[i].entry()->add_epsilon_transition(last_machine_exit);
  }

  _start_state = machines[0].entry();
  _end_state = last_machine_exit;

  return *this;
}

NFA& NFA::atleast(const std::size_t n) {
  if (n == 1) {
    this->exit()->add_epsilon_transition(this->entry());
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

NFA& NFA::exclude(const State::transition_key::type str)
{
  this->entry()->add_transition(str, dead_state);
  return *this;
}

bool NFA::match(const std::string& input) {

  std::stack<std::pair<std::shared_ptr<State>, size_t>> stack;
  std::unordered_map<State*, size_t> visited;

  stack.push({ _start_state, 0 });

  while (stack.empty() != true) {
    std::pair<std::shared_ptr<State>, size_t> current = stack.top();
    visited[current.first.get()] = current.second;
    stack.pop();

    size_t position = current.second;
    if (position == input.size() && current.first->accepting() == true) {
      return true;
    }

    if (position < input.size()) {

      State::transitions_t::iterator letter_tr = current.first->get_transition(input[position]);

      // handle epsilon transitions
      // exclusion must be handled as a union either go to dead state or wild card

      if (letter_tr == current.first->get_transitions().end()) {
        State::transitions_t::iterator wild_card_tr = current.first->get_transition(0x0);
        if (wild_card_tr != current.first->get_transitions().end()) {
          stack.push({ wild_card_tr->second, position + 1 });
        }
      }
      else {
          stack.push({ letter_tr->second, position + 1 });
      }

    }
    for (std::shared_ptr<State> s : current.first->get_epsilon_transitions()) {
      std::unordered_map<State*, size_t>::iterator visited_state = visited.find(s.get());

      if ((visited_state == visited.end()) || (visited_state != visited.end() && visited_state->second != position)) {
        stack.push({ s, position });
      }
    }

  }

  return false;
}
