#include "parser.h"
#include <iostream>
#include <cctype>
#include <cstdlib>
#include <string>
#include <vector>
#include <memory>
#include "../wgrep/utf8string.h"

#define UINBOUND(I, L) I < L.size()
#define LINBOUND(I) I >= 0
#define INBOUND(I, L) UINBOUND(I, L) && LINBOUND(I)

//inline bool isQuantifier(const Token& token)
//{
//  static TokenType quantifiers[6]{
//    TokenType::ZERO_OR_MORE,
//    TokenType::ONE_OR_MORE,
//    TokenType::ZERO_OR_ONE,
//    TokenType::QUANTIFIER_ATLEAST,
//    TokenType::QUANTIFIER_ATMOST,
//    TokenType::QUANTIFIER_BETWEEN,
//  };
//
//  for (TokenType t : quantifiers) {
//    if (t == token.type) return true;
//  }
//
//  return false;
//}

//AST parse_quantifier(const Token& token, AST& ast)
//{
//  switch (token.type) {
//  case TokenType::ZERO_OR_MORE:
//  {
//    return { AstType::ZERO_OR_MORE, std::make_unique<AST>(std::move(ast)) };
//  }
//  break;
//
//  case TokenType::ONE_OR_MORE:
//  {
//    return { AstType::ONE_OR_MORE, std::make_unique<AST>(std::move(ast)) };
//  }
//  break;
//
//  case TokenType::ZERO_OR_ONE:
//  {
//    return { AstType::ZERO_OR_ONE, std::make_unique<AST>(std::move(ast)) };
//  }
//  break;
//
//  case TokenType::QUANTIFIER_ATLEAST:
//  {
//    std::unique_ptr<AstQuantifier> astQuantifier = std::make_unique<AstQuantifier>();
//    astQuantifier->bounds = std::make_pair(std::get<unsigned int>(token.value), 0);
//    astQuantifier->bre = std::move(ast);
//    return { AstType::ATLEAST, std::move(astQuantifier) };
//  }
//  break;
//
//  case TokenType::QUANTIFIER_ATMOST:
//  {
//    std::unique_ptr<AstQuantifier> astQuantifier = std::make_unique<AstQuantifier>();
//    astQuantifier->bounds = std::make_pair(0, std::get<unsigned int>(token.value));
//    astQuantifier->bre = std::move(ast);
//    return { AstType::ATMOST, std::move(astQuantifier) };
//  }
//  break;
//
//  case TokenType::QUANTIFIER_BETWEEN:
//  {
//    std::unique_ptr<AstQuantifier> astQuantifier = std::make_unique<AstQuantifier>();
//    astQuantifier->bounds = std::get<std::pair<unsigned int, unsigned int>>(token.value);
//    astQuantifier->bre = std::move(ast);
//    return { AstType::BETWEEN, std::move(astQuantifier) };
//  }
//  break;
//  }
//
//  throw std::runtime_error("Expected quantifier found none");
//}

std::vector<Token> lex(utf8string& input)
{
  std::vector<Token> token_stream{};
  size_t position = 0;

  bool bracket_open = false;

  while (position < input.size()) {

    if (bracket_open) {
      if (input.at(position) == "]")
      {
        if (input.at(position - 1) == "[" || (input.at(position - 2) == "[" && input.at(position - 1) == "^")) {
          token_stream.push_back({ TokenType::CHARACTER, ']' });
          ++position;
        }
        else {
          bracket_open = false;
          token_stream.push_back({ TokenType::CLOSE_BRACKET });
          ++position;
        }
      }
      else if (input.at(position) == "^")
      {
        if (input.at(position - 1) == "[") {
          token_stream.push_back({ TokenType::CARET });
          ++position;
        }
        else {
          token_stream.push_back({ TokenType::CHARACTER, "^"});
          ++position;
        }
      }
      else if (input.at(position) == "[")
      {
        ++position;
        if (input.at(position) == ".")
        {
          ++position;
          size_t old_position = position;
          while (position + 1 < input.size() && !(input[position] == "." && input[position + 1] == "]")) ++position;

          if (position == input.size() - 1) throw std::runtime_error("there is no matching .]");

          if (position == old_position) throw std::runtime_error("collating symbol cannot be empty");

          token_stream.push_back({ TokenType::COLLATING_SYMBOL, input.substr(old_position, position - old_position).str() });
          position += 2;

          if (position < input.size() && input.at(position) == "-") {
            if (position + 1 < input.size() && input.at(position) == "]") {
              token_stream.push_back({ TokenType::CHARACTER, "-" });
              ++position;
            }
            else {
              token_stream.push_back({ TokenType::RANGE_OPERATOR });
              ++position;
            }
          }

        }
        else if (input.at(position) == ":")
        {
          ++position;
          size_t old_position = position;
          while (position + 1 < input.size() && !(input[position] == ":" && input[position + 1] == "]")) ++position;

          if (position == input.size() - 1) throw std::runtime_error("there is no matching :]");

          if (position == old_position) throw std::runtime_error("character class cannot be empty");

          token_stream.push_back({ TokenType::CHARACTER_CLASS, input.substr(old_position, position - old_position).str() });
          position += 2;

        }
        else if (input.at(position) == "=")
        {
          ++position;
          size_t old_position = position;
          if (position + 1 < input.size() && !(input[position] == "=" && input[position + 1] == "]")) ++position;

          if (position == input.size() - 1) throw std::runtime_error("there is no matching =]");

          if (position == old_position) throw std::runtime_error("character class cannot be empty");

          token_stream.push_back({ TokenType::EQUIVCLASS, input.at(old_position).str() });
          position += 2;
        }
        else
        {
          token_stream.push_back({ TokenType::CHARACTER, input.at(position).str() });
          ++position;
        }
      }
      else
      {
        token_stream.push_back({ TokenType::CHARACTER, input.at(position).str() });
        ++position;

        if (position < input.size() && input.at(position) == "-") {
          if (position + 1 < input.size() && input.at(position + 1) == "]") {
            token_stream.push_back({ TokenType::CHARACTER, '-' });
            ++position;
          }
          else {
            token_stream.push_back({ TokenType::RANGE_OPERATOR });
            ++position;
          }
        }
      }
    }
    else {
      if (input.at(position) == "[")
      {
        bracket_open = true;
        token_stream.push_back({ TokenType::OPEN_BRACKET });
        ++position;
      }
      else if (input.at(position) == "\\")
      {
        ++position;
        if (input.at(position) == "(")
        {
          token_stream.push_back({ TokenType::BACK_OPEN_PAREN });
          ++position;
        }
        else if (input.at(position) == ")")
        {
          token_stream.push_back({ TokenType::BACK_CLOSE_PAREN });
          ++position;
        }
        else if (input.at(position) == "{")
        {
          ++position;
          unsigned int lb = 0, ub = 0;
          bool lb_f = false, ub_f = false;

          // This takes one char to see if it's a digit.
          // I'm assuming that utf8stringRef returns a string of size one.
          // TODO: Add checks to see if the byte size of the returned character is 1
          if (std::isdigit(input.at(position).str()[0])) {
            lb_f = true;
            size_t old_position = position;
            while (std::isdigit(input.at(position).str()[0])) ++position;
            lb = std::stoul(input.substr(old_position, position - old_position + 1).str());
          }

          if (input.at(position) != ",") {
            throw std::runtime_error("range quantifier missing , at position: ");
          }

          ++position;

          if (std::isdigit(input.at(position).str().c_str()[0])) {
            ub_f = true;
            size_t old_position = position;
            while (std::isdigit(input.at(position).str().c_str()[0])) ++position;
            ub = std::stoul(input.substr(old_position, position - old_position + 1).str());
          }

          if (input.substr(position, 2) != "\\}") {
            throw std::runtime_error("quantifier missing \\} to close it ");
          }

          position += 2;

          if (lb_f && ub_f) {
            token_stream.push_back({ TokenType::QUANTIFIER_BETWEEN, std::make_pair(lb, ub) });
          }
          else if (lb_f && !ub_f) {
            token_stream.push_back({ TokenType::QUANTIFIER_ATLEAST, lb });
          }
          else if (!lb_f && ub_f) {
            token_stream.push_back({ TokenType::QUANTIFIER_ATMOST, ub });
          }
          else {
            throw std::runtime_error("unacceptable range quantifier pattern");
          }
        }
        else if (input.at(position) == "?")
        {
          token_stream.push_back({ TokenType::ZERO_OR_ONE });
          ++position;
        }
        else if (input.at(position) == "+")
        {
          token_stream.push_back({ TokenType::ONE_OR_MORE });
          ++position;
        }
        else if (input.at(position) == "*")
        {
          token_stream.push_back({ TokenType::ZERO_OR_MORE });
          ++position;
        }
        else if (input.at(position) == "|")
        {
          token_stream.push_back({ TokenType::UNION_OPERATOR });
          ++position;
        }
        else if (
          input.at(position) == "1" ||
          input.at(position) == "2" ||
          input.at(position) == "3" ||
          input.at(position) == "4" ||
          input.at(position) == "5" ||
          input.at(position) == "6" ||
          input.at(position) == "7" ||
          input.at(position) == "8" ||
          input.at(position) == "9"
          )
        {
          token_stream.push_back({ TokenType::BACK_REFERENCE, std::stoul(input.substr(position, 1).str()) });
          ++position;
        }
        else if (input.at(position) == "]")
        {
          token_stream.push_back({ TokenType::CHARACTER, ']' });
          ++position;
        }
        else
        {
          //throw
          //throw std::runtime_error("error cannot escape ordinary character");
          token_stream.push_back({ TokenType::CHARACTER, input.at(position).str()});
          ++position;
        }
      }
      else if (input.at(position) == ".")
      {
        token_stream.push_back({ TokenType::WILDCARD });
        ++position;
      }
      else if (input.at(position) == "*")
      {

        if (position == 0 || (position == 1 && input.at(0) == "^")
          || (position >= 2 && ((input.at(position - 1) == "|" || input.at(position - 1) == "(") && (input.at(position - 2) == "\\")))
          || (position >= 3 && ((input.at(position - 2) == "|" || input.at(position - 2) == "(") && (input.at(position - 3) == "\\") && (input.at(position - 1) == "^")))
          ) {
          token_stream.push_back({ TokenType::CHARACTER, '*' });
          ++position;
        }
        else {
          token_stream.push_back({ TokenType::ZERO_OR_MORE });
          ++position;
        }

      }
      else if (input.at(position) == "^")
      {
        if (position == 0 || (position >= 2 && (input.substr(position - 2, 2) == "\\(" || input.substr(position - 2, 2) == "\\|"))) {
          token_stream.push_back({ TokenType::FRONT_ANCHOR });
          ++position;
        }
        else {
          token_stream.push_back({ TokenType::CHARACTER, "^"});
          ++position;
        }
      }
      else if (input.at(position) == "$")
      {
        if (position == input.size() - 1 || (position + 2 < input.size() && (input.substr(position + 1, 2) == "\\)" || input.substr(position + 1, 2) == "\\|"))) {
          token_stream.push_back({ TokenType::BACK_ANCHOR });
          ++position;
        }
        else {
          token_stream.push_back({ TokenType::CHARACTER, "^"});
          ++position;
        }
      }
      //case ']':
      //{
      //  throw std::runtime_error("malformed bracket expression");
      //}
      //break;
      else
      {
        token_stream.push_back({ TokenType::CHARACTER, input.at(position).str() });
        ++position;
      }
    }
  }
  return token_stream;
}

//AST parse_subexpr(std::vector<Token>&, size_t&);

//AST parse_bracket(std::vector<Token>& token_stream, size_t& pos)
//{
//  AST ast{ AstType::ALTERNATION };
//
//  std::unique_ptr<AstAlternation> astAlternation = std::make_unique<AstAlternation>();
//
//  if (token_stream[pos].type == TokenType::CARET) {
//    astAlternation->exclude = true;
//    ++pos;
//  }
//
//  while (INBOUND(pos, token_stream)) {
//
//    switch (token_stream[pos].type) {
//    case TokenType::EQUIVCLASS:
//    {
//      astAlternation->bres.push_back({ AstType::EQUIVALENCE_CLASS,  std::get<char>(token_stream[pos++].value) });
//    }
//    break;
//    case TokenType::CHARACTER_CLASS:
//    {
//      astAlternation->bres.push_back({ AstType::CHARACTER_CLASS, std::get<std::string>(token_stream[pos++].value) });
//    }
//    break;
//    case TokenType::CHARACTER:
//    {
//      if (INBOUND(pos + 1, token_stream) && token_stream[pos + 1].type == TokenType::RANGE_OPERATOR) {
//        std::pair<std::string, std::string> range_def;
//
//        range_def.first = std::get<char>(token_stream[pos].value);
//
//        if (INBOUND(pos + 2, token_stream)) {
//          if (token_stream[pos + 2].type == TokenType::CHARACTER) {
//            range_def.second = std::get<char>(token_stream[pos + 2].value);
//          }
//          else if (token_stream[pos + 2].type == TokenType::COLLATING_SYMBOL) {
//            throw std::runtime_error("cannot handle callating symbols yet");
//            //range.second = std::get<std::string>(token_stream[pos + 2].value);
//          }
//          else {
//            throw std::runtime_error("ranges only end with characters or collating symbols");
//          }
//          astAlternation->bres.push_back({ AstType::RANGE, range_def });
//          pos += 3;
//        }
//        else {
//          throw std::runtime_error("cannot parse range");
//        }
//      }
//      else {
//        astAlternation->bres.push_back({ AstType::CHARACTER, std::get<char>(token_stream[pos++].value) });
//      }
//    }
//    break;
//    case TokenType::COLLATING_SYMBOL:
//    {
//      if (INBOUND(pos + 1, token_stream) && token_stream[pos + 1].type == TokenType::RANGE_OPERATOR) {
//
//        std::pair<std::string, std::string> range_def;
//        range_def.first = std::get<std::string>(token_stream[pos].value);
//
//        if (INBOUND(pos + 2, token_stream)) {
//          if (token_stream[pos + 2].type == TokenType::CHARACTER) {
//            range_def.second = std::get<char>(token_stream[pos + 2].value);
//          }
//          else if (token_stream[pos + 2].type == TokenType::COLLATING_SYMBOL) {
//            range_def.second = std::get<std::string>(token_stream[pos + 2].value);
//          }
//          else {
//            throw std::runtime_error("Ranges can only end with ordinary character or collating symbol");
//          }
//          astAlternation->bres.push_back({ AstType::RANGE, range_def });
//          pos += 3;
//        }
//        else {
//          throw std::runtime_error("Unexpected end of expression");
//        }
//      }
//      else {
//        astAlternation->bres.push_back({ AstType::COLLATING_SYMBOL, std::get<std::string>(token_stream[pos].value) });
//      }
//    }
//    break;
//    case TokenType::CLOSE_BRACKET:
//    {
//      ast.node = std::move(astAlternation);
//      ++pos;
//      return ast;
//    }
//    break;
//    default:
//    {
//      throw std::runtime_error("unexpected token");
//    }
//    break;
//    }
//  }
//
//  throw std::runtime_error("Unexpected ending of expression");
//
//}
//
//AST parse_concat(std::vector<Token>& token_stream, size_t& pos)
//{
//  bool break_loop = false;
//  AST ast{};
//  ast.type = AstType::CONCATENATION;
//
//  std::unique_ptr<AstConcat> astConcat = std::make_unique<AstConcat>();
//
//  while (INBOUND(pos, token_stream) && !break_loop) {
//
//    switch (token_stream[pos].type) {
//    case TokenType::FRONT_ANCHOR:
//    {
//      astConcat->bres.push_back({ AstType::FRONT_ANCHOR });
//      pos++;
//    }
//    break;
//    case TokenType::BACK_ANCHOR:
//    {
//      astConcat->bres.push_back({ AstType::BACK_ANCHOR });
//      pos++;
//    }
//    break;
//    case TokenType::CHARACTER:
//    {
//      AST character = { AstType::CHARACTER, std::get<char>(token_stream[pos++].value) };
//
//      if (INBOUND(pos, token_stream) && isQuantifier(token_stream[pos])) {
//        astConcat->bres.push_back(parse_quantifier(token_stream[pos++], character));
//      }
//      else {
//        astConcat->bres.push_back(std::move(character));
//      }
//    }
//    break;
//    case TokenType::WILDCARD:
//    {
//      AST wildcard = { AstType::WILDCARD };
//      pos++;
//
//      if (INBOUND(pos, token_stream) && isQuantifier(token_stream[pos])) {
//        astConcat->bres.push_back(parse_quantifier(token_stream[pos++], wildcard));
//      }
//      else {
//        astConcat->bres.push_back(std::move(wildcard));
//      }
//    }
//    break;
//    case TokenType::OPEN_BRACKET:
//    {
//      AST bracket_expr = parse_bracket(token_stream, ++pos);
//
//      if (INBOUND(pos, token_stream) && isQuantifier(token_stream[pos])) {
//        astConcat->bres.push_back(parse_quantifier(token_stream[pos++], bracket_expr));
//      }
//      else {
//        astConcat->bres.push_back(std::move(bracket_expr));
//      }
//    }
//    break;
//    case TokenType::BACK_OPEN_PAREN:
//    {
//      AST sub_expr = parse_subexpr(token_stream, ++pos);
//      if (INBOUND(pos, token_stream) && isQuantifier(token_stream[pos])) {
//        astConcat->bres.push_back(parse_quantifier(token_stream[pos++], sub_expr));
//      }
//      else {
//        astConcat->bres.push_back(std::move(sub_expr));
//      }
//    }
//    break;
//    case TokenType::BACK_REFERENCE:
//    {
//      AST sub_expr = parse_subexpr(token_stream, ++pos);
//      if (INBOUND(pos, token_stream) && isQuantifier(token_stream[pos])) {
//        astConcat->bres.push_back(parse_quantifier(token_stream[pos++], (sub_expr)));
//      }
//      else {
//        astConcat->bres.push_back(std::move(sub_expr));
//      }
//    }
//    break;
//    default:
//    {
//      break_loop = true;
//    }
//    break;
//    }
//  }
//
//  if (astConcat->bres.size() == 1) {
//    ast = std::move(astConcat->bres[0]);
//  }
//  else {
//    ast.node = std::move(astConcat);
//  }
//  return ast;
//}

//AST parse_union(std::vector<Token>& token_stream, size_t& position)
//{
//  AST ast = parse_concat(token_stream, position);
//
//  while (UINBOUND(position, token_stream) && token_stream[position].type == TokenType::UNION_OPERATOR) {
//    ++position;
//    AST newAst{};
//    newAst.type = AstType::UNION;
//    std::unique_ptr<AstUnion> unionNode = std::make_unique<AstUnion>();
//    unionNode->left = std::move(ast); unionNode->right = parse_concat(token_stream, position);
//    newAst.node = std::move(unionNode);
//    ast = std::move(newAst);
//  };
//
//  return ast;
//}
//
//AST parse_subexpr(std::vector<Token>& token_stream, size_t& pos)
//{
//  AST ast = parse_union(token_stream, pos);
//  if (token_stream[pos].type != TokenType::BACK_CLOSE_PAREN)
//    throw std::runtime_error("no matching close expr found for sub expression");
//  ++pos;
//  return ast;
//}
//
//AST parse(std::string& expression)
//{
//  std::vector<Token> token_stream = lex(expression);
//  size_t position = 0;
//
//  AST node = parse_union(token_stream, position);
//
//  return node;
//}



/*
  a|b

  {
    type union
    left a
    right b


*/
