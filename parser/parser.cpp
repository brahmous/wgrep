#include "parser.h"
#include <iostream>
#include <cctype>
#include <cstdlib>
#include <string>
#include <vector>
#include <memory>

#define UINBOUND(I, L) I < L.size()
#define LINBOUND(I) I >= 0
#define INBOUND(I, L) UINBOUND(I, L) && LINBOUND(I)

std::vector<Token> lex(const std::string& input)
{
  std::vector<Token> token_stream{};
  size_t position = 0;

  bool bracket_open = false;

  while (position < input.size()) {

    if (bracket_open) {
      switch (input.at(position)) {
      case ']':
      {
        if (input.at(position - 1) == '[' || (input.at(position - 2) == '[' && input.at(position - 1) == '^')) {
          token_stream.push_back({ TokenType::CHARACTER, ']' });
          ++position;
        }
        else {
          bracket_open = false;
          token_stream.push_back({ TokenType::CLOSE_BRACKET });
          ++position;
        }
      }
      break;
      case '^':
      {
        if (input.at(position - 1) == '[') {
          token_stream.push_back({ TokenType::CARET });
          ++position;
        }
        else {
          token_stream.push_back({ TokenType::CHARACTER, '^' });
          ++position;
        }
      }
      break;
      case '[':
      {
        ++position;
        switch (input.at(position)) {
        case '.':
        {
          ++position;
          size_t old_position = position;
          while (position + 1 < input.size() && !(input[position] == '.' && input[position + 1] == ']')) ++position;

          if (position == input.size() - 1) throw std::runtime_error("there is no matching .]");

          if (position == old_position) throw std::runtime_error("collating symbol cannot be empty");

          token_stream.push_back({ TokenType::COLLATING_SYMBOL, input.substr(old_position, position - old_position) });
          position += 2;

          if (position < input.size() && input.at(position) == '-') {
            if (position + 1 < input.size() && input.at(position) == ']') {
              token_stream.push_back({ TokenType::CHARACTER, '-' });
              ++position;
            }
            else {
              token_stream.push_back({ TokenType::RANGE_OPERATOR });
              ++position;
            }
          }

        }
        break;
        case ':':
        {
          ++position;
          size_t old_position = position;
          while (position + 1 < input.size() && !(input[position] == ':' && input[position + 1] == ']')) ++position;

          if (position == input.size() - 1) throw std::runtime_error("there is no matching :]");

          if (position == old_position) throw std::runtime_error("character class cannot be empty");

          token_stream.push_back({ TokenType::CHARACTER_CLASS, input.substr(old_position, position - old_position) });
          position += 2;

        }
        break;
        case '=':
        {
          ++position;
          size_t old_position = position;
          if (position + 1 < input.size() && !(input[position] == '=' && input[position + 1] == ']')) ++position;

          if (position == input.size() - 1) throw std::runtime_error("there is no matching =]");

          if (position == old_position) throw std::runtime_error("character class cannot be empty");

          token_stream.push_back({ TokenType::EQUIVCLASS, input.at(old_position) });
          position += 2;
        }
        break;
        default:
        {
          token_stream.push_back({ TokenType::CHARACTER, input.at(position) });
          ++position;
        }
        break;
        }
      }
      break;
      default:
      {
        token_stream.push_back({ TokenType::CHARACTER, input.at(position) });
        ++position;

        if (position < input.size() && input.at(position) == '-') {
          if (position + 1 < input.size() && input.at(position + 1) == ']') {
            token_stream.push_back({ TokenType::CHARACTER, '-' });
            ++position;
          }
          else {
            token_stream.push_back({ TokenType::RANGE_OPERATOR });
            ++position;
          }
        }
      }
      break;
      }
    }
    else {
      switch (input.at(position)) {
      case '[':
      {
        bracket_open = true;
        token_stream.push_back({ TokenType::OPEN_BRACKET });
        ++position;
      }
      break;
      case '\\':
      {
        ++position;
        switch (input.at(position)) {
        case '(':
        {
          token_stream.push_back({ TokenType::BACK_OPEN_PAREN });
          ++position;
        }
        break;
        case ')':
        {
          token_stream.push_back({ TokenType::BACK_CLOSE_PAREN });
          ++position;
        }
        break;
        case '{':
        {
          ++position;
          unsigned int lb = 0, ub = 0;
          bool lb_f = false, ub_f = false;

          if (std::isdigit(input.at(position))) {
            lb_f = true;
            size_t old_position = position;
            while (std::isdigit(input.at(position))) ++position;
            lb = std::stoul(input.substr(old_position, position - old_position + 1));
          }

          if (input.at(position) != ',') {
            throw std::runtime_error("range quantifier missing , at position: ");
          }

          ++position;

          if (std::isdigit(input.at(position))) {
            ub_f = true;
            size_t old_position = position;
            while (std::isdigit(input.at(position))) ++position;
            ub = std::stoul(input.substr(old_position, position - old_position + 1));
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
        break;
        break;
        case '?':
        {
          token_stream.push_back({ TokenType::ZERO_OR_ONE });
          ++position;
        }
        break;
        case '+':
        {
          token_stream.push_back({ TokenType::ONE_OR_MORE });
          ++position;
        }
        break;
        case '|':
        {
          token_stream.push_back({ TokenType::UNION_OPERATOR });
          ++position;
        }
        break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {
          token_stream.push_back({ TokenType::BACK_REFERENCE, std::stoul(input.substr(position, 1)) });
          ++position;
        }
        break;
        case ']':
        {
          token_stream.push_back({ TokenType::CHARACTER, ']' });
          ++position;
        }
        break;
        default:
        {
          //throw
          //throw std::runtime_error("error cannot escape ordinary character");
          token_stream.push_back({ TokenType::CHARACTER, input.at(position) });
          ++position;
        }
        break;
        }
      }
      break;
      case '.':
      {
        token_stream.push_back({ TokenType::WILDCARD });
        ++position;
      }
      break;
      case '*':
      {

        if (position == 0 || (position == 1 && input.at(0) == '^')
          || (position >= 2 && ((input.at(position - 1) == '|' || input.at(position - 1) == '(') && (input.at(position - 2) == '\\')))
          || (position >= 3 && ((input.at(position - 2) == '|' || input.at(position - 2) == '(') && (input.at(position - 3) == '\\') && (input.at(position - 1) == '^')))
          ) {
          token_stream.push_back({ TokenType::CHARACTER, '*' });
          ++position;
        }
        else {
          token_stream.push_back({ TokenType::ZERO_OR_MORE });
          ++position;
        }

      }
      break;
      case '^':
      {
        if (position == 0 || (position >= 2 && (input.substr(position - 2, 2) == "\\(" || input.substr(position - 2, 2) == "\\|"))) {
          token_stream.push_back({ TokenType::FRONT_ANCHOR });
          ++position;
        }
        else {
          token_stream.push_back({ TokenType::CHARACTER, '^' });
          ++position;
        }
      }
      break;
      case '$':
      {
        if (position == input.size() - 1 || (position + 2 < input.size() && (input.substr(position + 1, 2) == "\\)" || input.substr(position + 1, 2) == "\\|"))) {
          token_stream.push_back({ TokenType::BACK_ANCHOR });
          ++position;
        }
        else {
          token_stream.push_back({ TokenType::CHARACTER, '^' });
          ++position;
        }
      }
      break;
      //case ']':
      //{
      //  throw std::runtime_error("malformed bracket expression");
      //}
      //break;
      default:
      {
        token_stream.push_back({ TokenType::CHARACTER, input.at(position) });
        ++position;
      }
      break;
      }
    }
  }
  return token_stream;
}

AST parse_subexpr(std::vector<Token>&, size_t&);

AST parse_bracket(std::vector<Token>& token_stream, size_t& pos)
{
  AST ast{ AstType::ALTERNATION };

  std::unique_ptr<AstAlternation> astAlternation = std::make_unique<AstAlternation>();

  astAlternation->exclude = token_stream[pos].type == TokenType::CARET;

  ++pos;

  while (INBOUND(pos, token_stream)) {

    switch (token_stream[pos].type) {
    case TokenType::EQUIVCLASS:
    {
      astAlternation->bres.push_back({ AstType::EQUIVALENCE_CLASS,  std::get<char>(token_stream[pos++].value) });
    }
    break;
    case TokenType::CHARACTER_CLASS:
    {
      astAlternation->bres.push_back({ AstType::CHARACTER_CLASS, std::get<std::string>(token_stream[pos++].value) });
    }
    break;
    case TokenType::CHARACTER:
    {
      if (INBOUND(pos + 1, token_stream) && token_stream[pos + 1].type == TokenType::RANGE_OPERATOR) {
        std::pair<std::string, std::string> range_def;
        
        range_def.first = std::get<char>(token_stream[pos].value);

        if (INBOUND(pos + 2, token_stream)) {
          if (token_stream[pos + 2].type == TokenType::CHARACTER) {
            range_def.second = std::get<char>(token_stream[pos + 2].value);
          }
          if (token_stream[pos + 2].type == TokenType::COLLATING_SYMBOL) {
            throw std::runtime_error("cannot handle callating symbols yet");
            //range.second = std::get<std::string>(token_stream[pos + 2].value);
          }
          else {
            throw std::runtime_error("ranges only end with characters or collating symbols");
          }
          astAlternation->bres.push_back({ AstType::RANGE, range_def });
          pos += 3;
        }
        else {
          throw std::runtime_error("cannot parse range");
        }
      }
      else {
        astAlternation->bres.push_back({ AstType::CHARACTER, std::get<char>(token_stream[pos++].value) });
      }
    }
    break;
    case TokenType::COLLATING_SYMBOL:
    {
      if (INBOUND(pos + 1, token_stream) && token_stream[pos+1].type == TokenType::RANGE_OPERATOR) {
        
        std::pair<std::string, std::string> range_def;
        range_def.first = std::get<std::string>(token_stream[pos].value);

        if (INBOUND(pos + 2, token_stream)) {
          if (token_stream[pos + 2].type == TokenType::CHARACTER) {
            range_def.second = std::get<char>(token_stream[pos + 2].value);
          }
          else if (token_stream[pos + 2].type == TokenType::COLLATING_SYMBOL) {
            range_def.second = std::get<std::string>(token_stream[pos + 2].value);
          }
          else {
            throw std::runtime_error("Ranges can only end with ordinary character or collating symbol");
          }
          astAlternation->bres.push_back({ AstType::RANGE, range_def });
          pos += 3;
        }
        else {
          throw std::runtime_error("Unexpected end of expression");
        }
      }
      else {
        astAlternation->bres.push_back({ AstType::COLLATING_SYMBOL, std::get<std::string>(token_stream[pos].value) });
      }
    }
    break;
    case TokenType::CLOSE_BRACKET:
    {
      ast.node = std::move(astAlternation);
      return ast;
    }
    break;
    default:
    {
      throw std::runtime_error("unexpected token");
    }
    break;
    }
  }

  throw std::runtime_error("Unexpected ending of expression");

}

AST parse_concat(std::vector<Token>& token_stream, size_t& pos)
{
  bool break_loop = false;
  AST ast{};
  ast.type = AstType::CONCATENATION;

  std::unique_ptr<AstConcat> astConcat = std::make_unique<AstConcat>();

  while (INBOUND(pos, token_stream) && !break_loop) {
    switch (token_stream[pos].type) {
    case TokenType::FRONT_ANCHOR:
    {
      astConcat->bres.push_back({ AstType::FRONT_ANCHOR });
      pos++;
    }
    break;
    case TokenType::BACK_ANCHOR:
    {
      astConcat->bres.push_back({ AstType::BACK_ANCHOR });
      pos++;
    }
    break;
    case TokenType::CHARACTER:
    {
      astConcat->bres.push_back({ AstType::CHARACTER, std::get<char>(token_stream[pos++].value) });
    }
    break;
    case TokenType::WILDCARD:
    {
      astConcat->bres.push_back({ AstType::WILDCARD });
      pos++;
    }
    break;
    case TokenType::OPEN_BRACKET:
    {
      astConcat->bres.push_back(parse_bracket(token_stream, ++pos));
    }
    break;
    case TokenType::BACK_OPEN_PAREN:
    {
      astConcat->bres.push_back(parse_subexpr(token_stream, ++pos));
    }
    break;
    case TokenType::BACK_REFERENCE:
    {
      astConcat->bres.push_back({ AstType::BACKREFERENCE, std::get<unsigned int>(token_stream[pos++].value) });
    }
    break;
    default:
    {
      break_loop = true;
    }
    break;
    }
  }

  if (astConcat->bres.size() == 1) {
    ast = std::move(astConcat->bres[0]);
  }
  else {
    ast.node = std::move(astConcat);
  }
  return ast;
}

AST parse_union(std::vector<Token>& token_stream, size_t& position)
{
  AST ast = parse_concat(token_stream, position);

  while (UINBOUND(position, token_stream) && token_stream[position++].type == TokenType::UNION_OPERATOR) {
    AST newAst{};
    newAst.type = AstType::UNION;
    std::unique_ptr<AstUnion> unionNode = std::make_unique<AstUnion>();
    unionNode->left = std::move(ast); unionNode->right = parse_concat(token_stream, position);
    newAst.node = std::move(unionNode);
    ast = std::move(newAst);
  };

  return ast;
}

AST parse_subexpr(std::vector<Token>& token_stream, size_t& pos)
{
  AST ast = parse_union(token_stream, pos);
  if (token_stream[pos].type != TokenType::BACK_CLOSE_PAREN)
    throw std::runtime_error("no matching close expr found for sub expression");
  ++pos;
  return ast;
}

AST parse(std::string& expression)
{
  std::vector<Token> token_stream = lex(expression);
  size_t position = 0;

  AST node = parse_union(token_stream, position);

  return node;
}



/*
  a|b

  {
    type union
    left a
    right b


*/