#include "pch.h"
#include "../engine/nfa.h"

namespace NFAStateMachineImplementation {
  TEST(StateOperations, Accepting)
  {
    State state;
    // Default
    EXPECT_EQ(state.accepting(), false);

    state.set_accepting(true);
    EXPECT_EQ(state.accepting(), true);
  }

  TEST(StateOperations, CharTransitions)
  {
    State s1;
    std::shared_ptr<State> s2 = std::make_shared<State>(true);
    s1.add_transition('a', s2);
    ASSERT_EQ(s1.get_transition('a')->second.at(0).get(), s2.get());
    std::shared_ptr<State> s3 = std::make_shared<State>(true);
    s1.add_transition('a', s3);
    ASSERT_EQ(s1.get_transition('a')->second.size(), 2);
    ASSERT_EQ(s1.get_transition('a')->second.at(1).get(), s3.get());
  }

  TEST(StateOperations, EpsilonTransitions)
  {
    State s1;
    std::shared_ptr<State> s2 = std::make_shared<State>(true);
    s1.add_eps_transition(s2);
    ASSERT_EQ(s1.get_eps_transitions().size(), 1);
    ASSERT_EQ(s1.get_eps_transitions().at(0).get(), s2.get());
  }

  TEST(NFAOperations, NFAConstruction)
  {
    NFA nfa('a');

    EXPECT_EQ(nfa.entry()->accepting(), false);
    EXPECT_EQ(nfa.exit()->accepting(), true);
    ASSERT_EQ(nfa.entry()->get_transitions().empty(), false);
    ASSERT_EQ(nfa.entry()->get_eps_transitions().size(), 0);
    EXPECT_EQ(nfa.entry()->get_transition('a')->second.at(0).get(), nfa.exit().get());
    EXPECT_EQ(nfa.exit()->get_transitions().empty(), true);
    EXPECT_EQ(nfa.exit()->get_eps_transitions().empty(), true);
  }

  TEST(NFAOperations, NFAConcatChar)
  {
    NFA nfa('a');;
    nfa.concat('b');
    EXPECT_EQ(nfa.entry()->accepting(), false);
    State::trns_t::iterator first_transtition = nfa.entry()->get_transition('a');
    std::shared_ptr<State> s2 = first_transtition->second.at(0);
    EXPECT_EQ(s2->accepting(), false);
    State::trns_t::iterator second_transition = s2->get_transition('b');
    std::shared_ptr<State> s3 = second_transition->second.at(0);
    EXPECT_EQ(s3->accepting(), true);
    ASSERT_EQ(nfa.exit().get(), s3.get());
  }

  TEST(NFAOperations, NFAConcatNFA)
  {
    NFA nfaa('a');
    NFA nfab('b');

    // s1 - a - s2 - e - s3 - b - s4
    nfaa.concat(nfab);

    EXPECT_EQ(nfaa.entry()->accepting(), false);
    EXPECT_EQ(nfaa.exit()->accepting(), true);
    State::trns_t::iterator tr1 = nfaa.entry()->get_transition('a');
    std::shared_ptr<State> s2 = tr1->second.at(0);

    EXPECT_EQ(s2->accepting(), false);
    std::shared_ptr<State> s3 = s2->get_eps_transitions().at(0);
    EXPECT_EQ(s3->accepting(), false);
    State::trns_t::iterator tr3 = s3->get_transition('b');
    std::shared_ptr<State> s4 = tr3->second.at(0);
    EXPECT_EQ(s4->accepting(), true);
    ASSERT_EQ(nfaa.exit().get(), s4.get());
  }

  TEST(NFAOperations, NFAOrChar)
  {
    NFA nfa('a');
    nfa._or('b');

    /*
      |-- b --|
      s1- a -s2
    */
    EXPECT_EQ(nfa.entry()->accepting(), false);
    EXPECT_EQ(nfa.exit()->accepting(), true);
    State::trns_t::iterator tr_a = nfa.entry()->get_transition('a');
    std::shared_ptr<State> s2_a = tr_a->second.at(0);
    EXPECT_EQ(s2_a->accepting(), true);

    State::trns_t::iterator tr_b = nfa.entry()->get_transition('b');
    std::shared_ptr<State> s2_b = tr_b->second.at(0);
    EXPECT_EQ(s2_b->accepting(), true);
    ASSERT_EQ(s2_a.get(), s2_b.get());
  }

  TEST(NFAOperations, NFAOrNFA)
  {
    /*
  ->s1- a -s2(x)
    |       ^
    |       |
    e1      e2
    |       |
    q1- b -q2
  */
    NFA nfaa('a');
    NFA nfab('b');

    nfaa._or(nfab);
    EXPECT_EQ(nfaa.entry()->accepting(), false);
    EXPECT_EQ(nfaa.exit()->accepting(), true);

    State::trns_t::iterator tr_a = nfaa.entry()->get_transition('a');
    std::shared_ptr<State> s2 = tr_a->second.at(0);
    EXPECT_EQ(s2->accepting(), true);

    const State::etrns_t& tr_e1 = nfaa.entry()->get_eps_transitions();
    std::shared_ptr<State> q1 = tr_e1[0];
    EXPECT_EQ(q1->accepting(), false);

    State::trns_t::iterator tr_b = q1->get_transition('b');
    std::shared_ptr<State> q2 = tr_b->second.at(0);
    EXPECT_EQ(q2->accepting(), false);

    const State::etrns_t& tr_e2 = q2->get_eps_transitions();
    std::shared_ptr<State> s2_2 = tr_e2[0];
    EXPECT_EQ(s2_2->accepting(), true);
    ASSERT_EQ(s2_2.get(), s2.get());
    ASSERT_EQ(nfaa.exit().get(), s2_2.get());


  }

  TEST(NFAOperations, NFARepeatZeroOrMore)
  {
    /*
    -----e-----
    |         |
    v         ^
    ->s1- a -s2(v)
    |         ^
    v         |
    -----e-----
    */

    NFA nfa('a');
    nfa.repeat();

    EXPECT_EQ(nfa.entry()->accepting(), false);
    EXPECT_EQ(nfa.exit()->accepting(), true);

    State::trns_t::iterator tr_a = nfa.entry()->get_transition('a');
    std::shared_ptr<State> s2 = tr_a->second.at(0);
    EXPECT_EQ(s2->accepting(), true);
    ASSERT_EQ(s2.get(), nfa.exit().get());

    const State::etrns_t& s1_e = nfa.entry()->get_eps_transitions();
    std::shared_ptr<State> s2_2 = s1_e.at(0);
    EXPECT_EQ(s2_2->accepting(), true);
    ASSERT_EQ(s2_2.get(), s2.get());

    const State::etrns_t& s2_e = s2->get_eps_transitions();
    std::shared_ptr<State> s1 = s2_e.at(0);
    EXPECT_EQ(s1->accepting(), false);
    ASSERT_EQ(s1.get(), nfa.entry().get());
  }

  TEST(NFAOperations, NFARepeatNTimes)
  {

    NFA nfa('a');
    nfa.repeat(2);
    /*
    ->s1-a-s2-e-s3-a-s4(v)
    */

    EXPECT_EQ(nfa.entry()->accepting(), false);
    EXPECT_EQ(nfa.exit()->accepting(), true);

    State::trns_t::iterator tr_a = nfa.entry()->get_transition('a');
    std::shared_ptr<State> s2 = tr_a->second.at(0);
    EXPECT_EQ(nfa.entry()->accepting(), false);

    const State::etrns_t& s2_e = s2->get_eps_transitions();
    std::shared_ptr<State> s3 = s2_e.at(0);
    EXPECT_EQ(s3->accepting(), false);

    State::trns_t::iterator tr_a_2 = s3->get_transition('a');
    std::shared_ptr<State> s4 = tr_a_2->second.at(0);
    EXPECT_EQ(s4->accepting(), true);
    ASSERT_EQ(s4.get(), nfa.exit().get());
  }

}

TEST(NFAOperations, NFAAtMost)
{
  /**/
}

TEST(NFAOperations, NFAAtLeast)
{

}

TEST(NFAOperations, NFABetween)
{

}

TEST(NFAOperations, NFAExclude)
{

}

TEST(NFAOperations, NFAMatch)
{

}