#include <iostream>
#include <cassert>
#include <unordered_map>
#include "../engine/nfa.h";
#include <queue>
#include <clocale>

//static std::unordered_map<const State*, std::shared_ptr<State>> visited;

int main(int argc, char* argv[])
{
  NFA n1('a');
  n1.between(2, 4);
};