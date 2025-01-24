#include <iostream>
#include <cassert>
#include <unordered_map>
#include "../engine/nfa.h";
#include "../parser/parser.h"
#include <queue>
#include <clocale>
#include <vector>
#include <memory>
#include "utf8string.h"

NFA build_regexp(AST& ast)
{
  switch (ast.type) {
  case AstType::UNION:
  {
    std::unique_ptr<AstUnion> node
      = std::move(std::get<std::unique_ptr<AstUnion>>(ast.node));

    NFA right = build_regexp(node->right);
    NFA result = build_regexp(node->left)._or(right);
    return result;
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
      NFA first = build_regexp(astConcat->bres[0]);

      for (size_t i = 1; i < astConcat->bres.size(); i++) {
        NFA second = build_regexp(astConcat->bres[i]);
        first.concat(second);
      }

      return first;
    }
  }
  break;
  case AstType::CHARACTER:
  {
    return NFA(std::string(1, std::get<char>(ast.node)));
  }
  break;
  case AstType::FRONT_ANCHOR:
  {
    return NFA(0x0);
  }
  break;
  case AstType::BACK_ANCHOR:
  {
    return NFA(std::string(1, 0x1));
  }
  break;
  case AstType::WILDCARD:
  {
    return NFA(std::string(1, '.'));
  }
  break;
  case AstType::BACKREFERENCE:
  {
    // Implement copy constructor.
    throw std::runtime_error("Backreference not implemnted yet");

  }
  break;
  case AstType::ALTERNATION:
  {
    //range handled here 

    std::unique_ptr<AstAlternation> astAlternation
      = std::move(std::get<std::unique_ptr<AstAlternation>>(ast.node));

    if (astAlternation->bres.size() == 0) {
      throw std::runtime_error("Error cannot have empty alternation!");
    }

    NFA nfa = build_regexp(astAlternation->bres[0]);

    for (size_t i = 1; i < astAlternation->bres.size(); i++) {
      nfa._or(build_regexp(astAlternation->bres[i]));
    }

    return nfa;
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

  case AstType::RANGE:
  {
    throw std::runtime_error("Alternation class not implemnted yet");
  }
  break;

  case AstType::ZERO_OR_MORE:
  {
    std::unique_ptr<AST> Ast = std::move(std::get<std::unique_ptr<AST>>(ast.node));
    NFA nfa = build_regexp(*Ast);
    nfa.repeat();
    return nfa;
  }
  break;
  case AstType::ONE_OR_MORE:
  {

    std::unique_ptr<AST> Ast = std::move(std::get<std::unique_ptr<AST>>(ast.node));
    NFA nfa = build_regexp(*Ast);
    nfa.atleast(1);
    return nfa;
  }
  break;
  case AstType::ZERO_OR_ONE:
  {

    std::unique_ptr<AST> Ast = std::move(std::get<std::unique_ptr<AST>>(ast.node));
    NFA nfa = build_regexp(*Ast);
    nfa.atmost(1);
    return nfa;
  }
  break;
  case AstType::ATLEAST:
  {
    std::unique_ptr<AstQuantifier> astQuantifier 
      = std::move(std::get<std::unique_ptr<AstQuantifier>>(ast.node));
    NFA nfa = build_regexp(astQuantifier->bre);
    nfa.atleast(astQuantifier->bounds.first);
    return nfa;
  }
  break;
  case AstType::ATMOST:
  {
    std::unique_ptr<AstQuantifier> astQuantifier 
      = std::move(std::get<std::unique_ptr<AstQuantifier>>(ast.node));
    NFA nfa = build_regexp(astQuantifier->bre);
    nfa.atmost(astQuantifier->bounds.second);
    return nfa;
  }
  break;
  case AstType::BETWEEN:
  {
  std::unique_ptr<AstQuantifier> astQuantifier 
      = std::move(std::get<std::unique_ptr<AstQuantifier>>(ast.node));
    NFA nfa = build_regexp(astQuantifier->bre);
    nfa.between(astQuantifier->bounds.first, astQuantifier->bounds.second);
    return nfa;
  }
  break;
  }
  throw std::runtime_error("Unkown Ast node");
}

/*
TODO:
  figure out wildcard
  debug example
*/

int main(int argc, char* argv[])
{
  //std::string input("v\\(.[^kj]\\)a\\|[cd]*\\(2348\\)\\{1,2\\}");;
  //std::string input("[^a]");;

  //AST ast = parse(input);
  //NFA nfa = build_regexp(ast);

  //std::cout << "Matching A => " << (nfa.match("a") ? "Matched" : "Didn't Match") << "\n";

  //std::string str = "é";

  //std::cout << "size = " << str.size() << "\n";

  //for (int i = 0; i < str.size(); i++) {
  //  std::cout << str.c_str()[i] << " ";
  //}

  //std::cout << "\n";
  //std::cin >> str;
  //std::cout << "size = " << str.size();

  //for (int i = 0; i < str.size(); i++) {
  //  std::cout << str.c_str()[i] << " ";
  //}


  //UTF8String str("hello");

  //UTF8String::iterator sIt = str.Begin();


  //UTF8String str;
  //UTF8String str2("hello");

  //std::string strstd = "hello";

  //std::cout << strstd << "\n";
  //*(strstd.begin()) = "hello";

  //std::cout<< strstd << "\n";

  /*utf8string a = "console.";

  a += "log()";*/


  return 0;

};



