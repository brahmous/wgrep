#pragma once
#include <vector>
#include <string>
#include <variant>
#include <memory>

enum class TokenType {
  BACK_OPEN_PAREN,//subexpression
  BACK_CLOSE_PAREN,
  BACK_OPEN_BRACE, // quantifier
  BACK_CLOSE_BRACE,
  OPEN_BRACKET,// bracket expression
  CLOSE_BRACKET,
  ZERO_OR_MORE,// quantifier 
  ONE_OR_MORE,
  ZERO_OR_ONE,
  BACK_REFERENCE,// backref - syntactically valid to have just a back reference semantically means nothing 
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
  std::variant<unsigned int, char, std::string, std::pair<unsigned int, unsigned int>> value;
};

enum class AstType {
  UNION,
  CONCATENATION,
  CHARACTER,
  FRONT_ANCHOR,
  BACK_ANCHOR,
  WILDCARD,
  BACKREFERENCE,
  ALTERNATION,
  EQUIVALENCE_CLASS,
  CHARACTER_CLASS,
  COLLATING_SYMBOL,
  RANGE
};

struct AstConcat;
struct AstUnion;
struct AstAlternation;

/*
            [ast| 
              type: 
              pointers
            ]

            (p1: pointer// heap) 

            {
              left 
              [
                type:
                pointer
              ]
              right
              [
                
              ]
            }

*/

struct AST {
  AstType type;
  std::variant<std::pair<std::string, std::string>, std::string, unsigned int, char, std::unique_ptr<AstConcat>, std::unique_ptr<AstUnion>, std::unique_ptr<AstAlternation>, std::unique_ptr<AST>> node;
};

struct AstUnion {
  struct AST left;
  struct AST right;
};

struct AstConcat {
  std::vector<AST> bres;
};

struct AstAlternation {
  bool exclude;
  std::vector<AST> bres;
};


std::vector<Token> lex(const std::string&);
AST parse(std::string&);