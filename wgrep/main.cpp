#include <iostream>
#include <cassert>
#include <unordered_map>
#include "../engine/nfa.h";
#include "../parser/parser.h"
#include <queue>
#include <clocale>
#include <vector>
#include <memory>

NFA build_regexp(AST& ast)
{
  switch (ast.type) {
  case AstType::UNION:
  {
    std::unique_ptr<AstUnion> node
      = std::move(std::get<std::unique_ptr<AstUnion>>(ast.node));

    NFA right = build_regexp(node->right);
    return build_regexp(node->left)._or(right);
  }
  break;
  case AstType::CONCATENATION:
  {
    std::unique_ptr<AstConcat> astConcat
      = std::move(std::get<std::unique_ptr<AstConcat>>(ast.node));

    if (astConcat->bres.size() == 0) {
      throw std::runtime_error("empty concatenation expression???");
    }
    else {
      NFA nfa = build_regexp(astConcat->bres[0]);

      for (size_t i = 1; i < astConcat->bres.size(); i++) {
        nfa.concat(build_regexp(astConcat->bres[i]));
      }

      return nfa;
    }
  }
  break;
  case AstType::CHARACTER:
  {
    return NFA(std::get<char>(ast.node));
  }
  break;
  case AstType::FRONT_ANCHOR:
  {
    return NFA(0x0);
  }
  break;
  case AstType::BACK_ANCHOR:
  {
    return NFA(0x1);
  }
  break;
  case AstType::WILDCARD:
  {
    return NFA('.');
  }
  break;
  case AstType::BACKREFERENCE:
  {
    // Implement copy constructor.
  }
  break;
  case AstType::ALTERNATION:
  {
    //range handled here     
  }
  break;
  case AstType::EQUIVALENCE_CLASS:
  {
    throw std::runtime_error("Equivalence class not implemnted yet");
  }
  break;
  case AstType::CHARACTER_CLASS:
  {
    throw std::runtime_error("Character class not implemnted yet");
  }
  break;
  case AstType::COLLATING_SYMBOL:
  {
    throw std::runtime_error("Collating symbol not implemnted yet");
  }
  break;
  /*
  case AstType::RANGE:
      {
      }
      break;
  */
  case AstType::ZERO_OR_MORE:
  {
  }
  break;
  case AstType::ONE_OR_MORE:
  {
  }
  break;
  case AstType::ZERO_OR_ONE:
  {
  }
  break;
  case AstType::OPTIONAL:
  {
  }
  break;
  case AstType::ATLEAST:
  {
  }
  break;
  case AstType::ATMOST:
  {
  }
  break;
  case AstType::BETWEEN:
  {
  }
  break;
  }
  throw std::runtime_error("Unkown Ast node");
}

int main(int argc, char* argv[])
{
  //NFA n1('a');
  //n1.repeat();
  //n1.match("aa");

  std::string input("abcd");
  AST ast = parse(input);
  NFA nfa = build_regexp(ast);

  std::cout << "Matching A => " << (nfa.match("abcd") ? "Matched" : "Didn't Match") << "\n";
  std::cout << "Matching B => " << (nfa.match("bbcd") ? "Matched" : "Didn't Match") << "\n";
};
