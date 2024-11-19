#include "pch.h"
#include "../parser/parser.h"

namespace PosixBREParser {

  TEST(StageOne, SubExpressionOpen)
  {
    std::string input{ "\\(" };

    std::vector<Token> token_stream = lex(input);

    ASSERT_EQ(token_stream.size(), 1);
    ASSERT_EQ(token_stream.at(0).type, TokenType::BACK_OPEN_PAREN);
  }

  TEST(StageOne, SubExpressionClose)
  {
    std::string input{ "\\)" };

    std::vector<Token> token_stream = lex(input);

    ASSERT_EQ(token_stream.size(), 1);
    ASSERT_EQ(token_stream.at(0).type, TokenType::BACK_CLOSE_PAREN);
  }

  TEST(StageOne, ZeroOrOneQuantifier)
  {
    std::string input{ "\\?" };
    std::vector<Token> token_stream = lex(input);

    ASSERT_EQ(token_stream.size(), 1);
    ASSERT_EQ(token_stream.at(0).type, TokenType::ZERO_OR_ONE);

  }

  TEST(StageOne, OneOrMoreQuantifier)
  {
    std::string input{ "\\+" };
    std::vector<Token> token_stream = lex(input);

    ASSERT_EQ(token_stream.size(), 1);
    ASSERT_EQ(token_stream.at(0).type, TokenType::ONE_OR_MORE);

  }

  TEST(StageOne, UnionOperator)
  {
    std::string input{ "\\|" };
    std::vector<Token> token_stream = lex(input);

    ASSERT_EQ(token_stream.size(), 1);
    ASSERT_EQ(token_stream.at(0).type, TokenType::UNION_OPERATOR);

  }

  TEST(StageOne, BackReference)
  {
    std::string input{ "\\3" };
    std::vector<Token> token_stream = lex(input);

    ASSERT_EQ(token_stream.size(), 1);
    ASSERT_EQ(token_stream.at(0).type, TokenType::BACK_REFERENCE);
    EXPECT_EQ(std::get<unsigned int>(token_stream.at(0).value), (unsigned int)3);
  }

  TEST(StageOne, EscapedCharacters)
  {
    std::string input{ "\\a\\[\\]" };
    std::vector<Token> token_stream = lex(input);

    ASSERT_EQ(token_stream.size(), 3);
    ASSERT_EQ(token_stream.at(0).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(0).value), 'a');
    EXPECT_EQ(std::get<char>(token_stream.at(1).value), '[');
    EXPECT_EQ(std::get<char>(token_stream.at(2).value), ']');

  }

  TEST(StageOne, OpenBracket)
  {
    std::string input("[");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 1);
    EXPECT_EQ(token_stream.at(0).type, TokenType::OPEN_BRACKET);
  }

  TEST(StageOne, CloseBracketFirst)
  {
    std::string input("[]");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 2);
    EXPECT_EQ(token_stream.at(0).type, TokenType::OPEN_BRACKET);
    EXPECT_EQ(token_stream.at(1).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(1).value), ']');
  }

  TEST(StageOne, CloseBracketAfterCaret)
  {
    std::string input("[^]");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 3);
    EXPECT_EQ(token_stream.at(0).type, TokenType::OPEN_BRACKET);
    EXPECT_EQ(token_stream.at(1).type, TokenType::CARET);
    EXPECT_EQ(token_stream.at(2).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(2).value), ']');
  }

  TEST(StageOne, WildCard)
  {
    std::string input(".");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 1);
    EXPECT_EQ(token_stream.at(0).type, TokenType::WILDCARD);
  }

  TEST(StageOne, DotInBracket)
  {
    std::string input("[.");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 2);
    EXPECT_EQ(token_stream.at(0).type, TokenType::OPEN_BRACKET);
    EXPECT_EQ(token_stream.at(1).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(1).value), '.');
  }

  TEST(StageOne, KleeneStarFirst)
  {
    std::string input("*");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 1);
    EXPECT_EQ(token_stream.at(0).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(0).value), '*');
  }

  TEST(StageOne, KleeneStarAfterCaret)
  {
    std::string input("^*");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 2);
    EXPECT_EQ(token_stream.at(0).type, TokenType::FRONT_ANCHOR);
    EXPECT_EQ(token_stream.at(1).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(1).value), '*');
  }

  TEST(StageOne, KleeneStarZeroOrMore)
  {
    std::string input("a*");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 2);
    EXPECT_EQ(token_stream.at(0).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(0).value), 'a');
    EXPECT_EQ(token_stream.at(1).type, TokenType::ZERO_OR_MORE);
  }

  TEST(StageOne, KleeneStarInBracket)
  {
    std::string input("[*]");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 3);
    EXPECT_EQ(token_stream.at(0).type, TokenType::OPEN_BRACKET);
    EXPECT_EQ(token_stream.at(1).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(1).value), '*');
    EXPECT_EQ(token_stream.at(2).type, TokenType::CLOSE_BRACKET);
  }

  TEST(StageOne, CharacterAlternation)
  {
    std::string input("[ab]");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 4);
    EXPECT_EQ(token_stream.at(0).type, TokenType::OPEN_BRACKET);
    EXPECT_EQ(token_stream.at(1).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(1).value), 'a');
    EXPECT_EQ(token_stream.at(2).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(2).value), 'b');
    EXPECT_EQ(token_stream.at(3).type, TokenType::CLOSE_BRACKET);
  }

  TEST(StageOne, CharacterExclusion)
  {
    std::string input("[^ab]");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 5);
    EXPECT_EQ(token_stream.at(0).type, TokenType::OPEN_BRACKET);
    EXPECT_EQ(token_stream.at(1).type, TokenType::CARET);
    EXPECT_EQ(token_stream.at(2).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(2).value), 'a');
    EXPECT_EQ(token_stream.at(3).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(3).value), 'b');
    EXPECT_EQ(token_stream.at(4).type, TokenType::CLOSE_BRACKET);
  }
  
  TEST(StageOne, CaretCharacterInBracket)
  {
    std::string input("[a^b]");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 5);
    EXPECT_EQ(token_stream.at(0).type, TokenType::OPEN_BRACKET);
    EXPECT_EQ(token_stream.at(1).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(1).value), 'a');
    EXPECT_EQ(token_stream.at(2).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(2).value), '^');
    EXPECT_EQ(token_stream.at(3).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(3).value), 'b');
    EXPECT_EQ(token_stream.at(4).type, TokenType::CLOSE_BRACKET);
  }

  TEST(StageOne, CollatingSymbol) {
    std::string input("[[.].]]");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 3);
    EXPECT_EQ(token_stream.at(0).type, TokenType::OPEN_BRACKET);
    EXPECT_EQ(token_stream.at(1).type, TokenType::COLLATING_SYMBOL);
    EXPECT_EQ(std::get<std::string>(token_stream.at(1).value), "]");
    EXPECT_EQ(token_stream.at(2).type, TokenType::CLOSE_BRACKET);
  }

  TEST(StageOne, CharacterClass)
  {
    std::string input("[[:]:]]");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 3);
    EXPECT_EQ(token_stream.at(0).type, TokenType::OPEN_BRACKET);
    EXPECT_EQ(token_stream.at(1).type, TokenType::CHARACTER_CLASS);
    EXPECT_EQ(std::get<std::string>(token_stream.at(1).value), "]");
    EXPECT_EQ(token_stream.at(2).type, TokenType::CLOSE_BRACKET);
  }

  TEST(StageOne, EquivalenceClass)
  {
    std::string input("[[=]=]]");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 3);
    EXPECT_EQ(token_stream.at(0).type, TokenType::OPEN_BRACKET);
    EXPECT_EQ(token_stream.at(1).type, TokenType::EQUIVCLASS);
    EXPECT_EQ(std::get<char>(token_stream.at(1).value), ']');
    EXPECT_EQ(token_stream.at(2).type, TokenType::CLOSE_BRACKET);
  }


  TEST(StageOne, CharacterZeroOrOne)
  {
    std::string input("a\\?");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 2);
    EXPECT_EQ(token_stream.at(0).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(0).value), 'a');
    EXPECT_EQ(token_stream.at(1).type, TokenType::ZERO_OR_ONE);
  }

  TEST(StageOne, CharacterOneOrMore)
  {
    std::string input("a\\+");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 2);
    EXPECT_EQ(token_stream.at(0).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(0).value), 'a');
    EXPECT_EQ(token_stream.at(1).type, TokenType::ONE_OR_MORE);
  }
  
  TEST(StageOne, QuantifierBoundedRange)
  {
    std::string input("\\{1,2\\}");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 1);
    EXPECT_EQ(token_stream.at(0).type, TokenType::QUANTIFIER_BETWEEN);
    EXPECT_EQ((std::get<std::pair<unsigned int, unsigned int>>(token_stream.at(0).value)).first, 1);
    EXPECT_EQ((std::get<std::pair<unsigned int, unsigned int>>(token_stream.at(0).value)).second, 2);
  }

  TEST(StageOne, QuantifierAtLeast)
  {
    std::string input("\\{1,\\}");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 1);
    EXPECT_EQ(token_stream.at(0).type, TokenType::QUANTIFIER_ATLEAST);
    EXPECT_EQ(std::get<unsigned int>(token_stream.at(0).value), 1);
  }

  TEST(StageOne, QuantifierAtMost)
  {
    std::string input("\\{,2\\}");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 1);
    EXPECT_EQ(token_stream.at(0).type, TokenType::QUANTIFIER_ATMOST);
    EXPECT_EQ(std::get<unsigned int>(token_stream.at(0).value), 2);
  }

  TEST(StageOne, AnchorsStartAndEnd)
  {
    std::string input("^a$");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 3);
    EXPECT_EQ(token_stream.at(0).type, TokenType::FRONT_ANCHOR);
    EXPECT_EQ(token_stream.at(1).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(1).value), 'a');
    EXPECT_EQ(token_stream.at(2).type, TokenType::BACK_ANCHOR);
  }

  TEST(StageOne, AnchorsPreviousAndNextToUnionOp)
  {
    std::string input("a$\\|^b");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 5);
    EXPECT_EQ(token_stream.at(0).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(0).value), 'a');
    EXPECT_EQ(token_stream.at(1).type, TokenType::BACK_ANCHOR);
    EXPECT_EQ(token_stream.at(2).type, TokenType::UNION_OPERATOR);
    EXPECT_EQ(token_stream.at(3).type, TokenType::FRONT_ANCHOR);
    EXPECT_EQ(token_stream.at(4).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(4).value), 'b');

  }

  TEST(StageOne, AnchorInSubExpr)
  {
    std::string input("\\(^a$\\)");
    std::vector<Token> token_stream = lex(input);
    ASSERT_EQ(token_stream.size(), 5);
    EXPECT_EQ(token_stream.at(0).type, TokenType::BACK_OPEN_PAREN);
    EXPECT_EQ(token_stream.at(1).type, TokenType::FRONT_ANCHOR);
    EXPECT_EQ(token_stream.at(2).type, TokenType::CHARACTER);
    EXPECT_EQ(std::get<char>(token_stream.at(2).value), 'a');
    EXPECT_EQ(token_stream.at(3).type, TokenType::BACK_ANCHOR);
    EXPECT_EQ(token_stream.at(4).type, TokenType::BACK_CLOSE_PAREN);
  }

  TEST(Parsing, FullExpression)
  {
    std::string input("^a|b\\?c\\+[][:alpha:]ab(c\\d[=a=]][^(ab\\|c)+]$\\|\\(^ab[cdA-Za-z]\\)$");
    //std::string input("^a|b\\?c\\+[][:alpha:]ab(c\\d[=a=]][^(ab\\|c)+]$\\|\\(^ab[cdA-Za-z]\\)$");



    AST ast = parse(input);

  }
}
