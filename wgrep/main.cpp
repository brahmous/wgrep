#include <iostream>
#include <cassert>
#include <unordered_map>
#include "../engine/nfa.h";
#include "../parser/parser.h"
#include <queue>
#include <clocale>
#include <vector>
#include <memory>

int main(int argc, char* argv[])
{
  //NFA n1('a');
  //n1.repeat();
  //n1.match("aa");
  //std::string input("ab[[=@=]]d\\|\\(a[ab])");

  std::string input("a\\|a");
  AST ast = parse(input);

};