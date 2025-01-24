#pragma once
#include <vector>
#include <string>
#include <variant>
#include <memory>
#include "../wgrep/utf8string.h"

enum class TokenType {
  BACK_OPEN_PAREN,//subexpression
  BACK_CLOSE_PAREN,
  OPEN_BRACKET,// bracket expression
  CLOSE_BRACKET,
  ZERO_OR_MORE,// quantifier 
  ONE_OR_MORE,
  ZERO_OR_ONE,
  BACK_REFERENCE, 
  UNION_OPERATOR,// UNION
  CHARACTER,// CHARACTER
  WILDCARD,// WILDCARD
  FRONT_ANCHOR,// ANCHOR - type could be front or back
  BACK_ANCHOR,
  EQUIVCLASS,// EQUIVALENT CLASS 
  COLLATING_SYMBOL, // COLL SYM b
  CHARACTER_CLASS, // CHAR - 3 o b
  RANGE_OPERATOR, // RANGE - 2 b
  CARET, // EXCLUDE - bracket expression with flag set to exclude - 2 b
  QUANTIFIER_BETWEEN, // Quantifier - 1 b
  QUANTIFIER_ATLEAST, // Quantifier - 1 b
  QUANTIFIER_ATMOST // Quantifier   - 1 b
};

/*
  Text editor with operational transform.
  Database 
  Matrix
  C++ parser
  C with templates
  Immedate UI
  Allocation stuff
*/

struct Token {
  TokenType type;
  std::variant<
    unsigned int,
    char,
    std::string,
    std::pair<unsigned int, unsigned int>> value;
};

enum class AstType {
  UNION,
  CONCATENATION,
  CHARACTER, //*
  FRONT_ANCHOR,
  BACK_ANCHOR,
  WILDCARD, 
  BACKREFERENCE,
  ALTERNATION,
  EQUIVALENCE_CLASS, //* add flag to the node
  CHARACTER_CLASS, //* add flag to the node
  //* related to the key of the map and comparison method
  COLLATING_SYMBOL,  
  RANGE, //*
  ZERO_OR_MORE,
  ONE_OR_MORE,
  ZERO_OR_ONE,
  ATLEAST,
  ATMOST,
  BETWEEN
};

struct AstConcat;
struct AstUnion;
struct AstAlternation;
struct AstQuantifier;

struct AST {
  AstType type;
  std::variant<std::pair<std::string,std::string>,
	      std::string,
	      unsigned int,
	      char,
	      std::unique_ptr<AstConcat>,
	      std::unique_ptr<AstUnion>,
	      std::unique_ptr<AstAlternation>,
	      std::unique_ptr<AST>,
	      std::unique_ptr<AstQuantifier>> node;
};

struct AstUnion {
  AST left;
  AST right;
};

struct AstConcat {
  std::vector<AST> bres;
};

struct AstAlternation {
  bool exclude;
  std::vector<AST> bres;
};

struct AstQuantifier {
  AST bre;
  std::pair<unsigned int, unsigned int> bounds;
};

std::vector<Token> lex(utf8string&);
AST parse(std::string&);
