#include <iostream>
#include <cassert>
#include <unordered_map>
#include "../engine/nfa.h";
#include <queue>
#include <clocale>

int main(int argc, char* argv[])
{
  NFA n1('a');
  n1.repeat();
  n1.match("aa");
};