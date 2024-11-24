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
    s1.add_epsilon_transition(s2);
    ASSERT_EQ(s1.get_epsilon_transitions().size(), 1);
    ASSERT_EQ(s1.get_epsilon_transitions().at(0).get(), s2.get());
  }

  TEST(NFAOperations, NFAConstruction)
  {
    NFA nfa('a');

    EXPECT_EQ(nfa.entry()->accepting(), false);
    EXPECT_EQ(nfa.exit()->accepting(), true);
    ASSERT_EQ(nfa.entry()->get_transitions().empty(), false);
    ASSERT_EQ(nfa.entry()->get_epsilon_transitions().size(), 0);
    EXPECT_EQ(nfa.entry()->get_transition('a')->second.at(0).get(), nfa.exit().get());
    EXPECT_EQ(nfa.exit()->get_transitions().empty(), true);
    EXPECT_EQ(nfa.exit()->get_epsilon_transitions().empty(), true);
  }

  TEST(NFAOperations, NFAConcatChar)
  {
    NFA nfa('a');;
    nfa.concat('b');
    EXPECT_EQ(nfa.entry()->accepting(), false);
    State::transitions_t::iterator first_transtition = nfa.entry()->get_transition('a');
    std::shared_ptr<State> s2 = first_transtition->second.at(0);
    EXPECT_EQ(s2->accepting(), false);
    State::transitions_t::iterator second_transition = s2->get_transition('b');
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
    State::transitions_t::iterator tr1 = nfaa.entry()->get_transition('a');
    std::shared_ptr<State> s2 = tr1->second.at(0);

    EXPECT_EQ(s2->accepting(), false);
    std::shared_ptr<State> s3 = s2->get_epsilon_transitions().at(0);
    EXPECT_EQ(s3->accepting(), false);
    State::transitions_t::iterator tr3 = s3->get_transition('b');
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
    State::transitions_t::iterator tr_a = nfa.entry()->get_transition('a');
    std::shared_ptr<State> s2_a = tr_a->second.at(0);
    EXPECT_EQ(s2_a->accepting(), true);

    State::transitions_t::iterator tr_b = nfa.entry()->get_transition('b');
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

    State::transitions_t::iterator tr_a = nfaa.entry()->get_transition('a');
    std::shared_ptr<State> s2 = tr_a->second.at(0);
    EXPECT_EQ(s2->accepting(), true);

    const State::epsilon_transitions_t& tr_e1 = nfaa.entry()->get_epsilon_transitions();
    std::shared_ptr<State> q1 = tr_e1[0];
    EXPECT_EQ(q1->accepting(), false);

    State::transitions_t::iterator tr_b = q1->get_transition('b');
    std::shared_ptr<State> q2 = tr_b->second.at(0);
    EXPECT_EQ(q2->accepting(), false);

    const State::epsilon_transitions_t& tr_e2 = q2->get_epsilon_transitions();
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

    State::transitions_t::iterator tr_a = nfa.entry()->get_transition('a');
    std::shared_ptr<State> s2 = tr_a->second.at(0);
    EXPECT_EQ(s2->accepting(), true);
    ASSERT_EQ(s2.get(), nfa.exit().get());

    const State::epsilon_transitions_t& s1_e = nfa.entry()->get_epsilon_transitions();
    std::shared_ptr<State> s2_2 = s1_e.at(0);
    EXPECT_EQ(s2_2->accepting(), true);
    ASSERT_EQ(s2_2.get(), s2.get());

    const State::epsilon_transitions_t& s2_e = s2->get_epsilon_transitions();
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

    State::transitions_t::iterator tr_a = nfa.entry()->get_transition('a');
    std::shared_ptr<State> s2 = tr_a->second.at(0);
    EXPECT_EQ(nfa.entry()->accepting(), false);

    const State::epsilon_transitions_t& s2_e = s2->get_epsilon_transitions();
    std::shared_ptr<State> s3 = s2_e.at(0);
    EXPECT_EQ(s3->accepting(), false);

    State::transitions_t::iterator tr_a_2 = s3->get_transition('a');
    std::shared_ptr<State> s4 = tr_a_2->second.at(0);
    EXPECT_EQ(s4->accepting(), true);
    ASSERT_EQ(s4.get(), nfa.exit().get());
  }

  TEST(NFAOperations, NFAAtMost)
  {
    NFA nfa('a');
    nfa.atmost(2);

    /*
    ->s0-a->s1-e->s2-a-s3(v)
    v              v    ^
    |              |    |
    --------e------------
    */

    EXPECT_EQ(nfa.entry()->accepting(), false);
    const State::epsilon_transitions_t& s0_e = nfa.entry()->get_epsilon_transitions();
    EXPECT_EQ(nfa.exit()->accepting(), true);
    ASSERT_EQ(s0_e.at(0).get(), nfa.exit().get());

    State::transitions_t::iterator s0_a = nfa.entry()->get_transition('a');
    std::shared_ptr<State> s1 = s0_a->second.at(0);
    EXPECT_EQ(s1->accepting(), false);

    const State::epsilon_transitions_t& s1_e = s1->get_epsilon_transitions();
    std::shared_ptr<State> s2 = s1_e.at(0);
    EXPECT_EQ(s2->accepting(), false);

    State::transitions_t::iterator s2_a = s2->get_transition('a');
    std::shared_ptr<State> s3_a = s2_a->second.at(0);
    EXPECT_EQ(s3_a->accepting(), true);
    ASSERT_EQ(s3_a.get(), nfa.exit().get());

    State::epsilon_transitions_t s2_e = s2->get_epsilon_transitions();
    std::shared_ptr<State> s3_e = s2_e.at(0);
    EXPECT_EQ(s3_e->accepting(), true);
    ASSERT_EQ(s3_e.get(), nfa.exit().get());

  }

  TEST(NFAOperations, NFAAtLeastOne)
  {
    NFA nfa('a');
    nfa.atleast(1);
    /*
    ->s0- a -s1(v)
       ^      v
       |      |
       ---e----
    */

    EXPECT_EQ(nfa.entry()->accepting(), false);
    State::transitions_t::iterator s0_a = nfa.entry()->get_transition('a');
    std::shared_ptr<State> s1 = s0_a->second.at(0);

    EXPECT_EQ(s1->accepting(), true);
    ASSERT_EQ(s1.get(), nfa.exit().get());

    State::epsilon_transitions_t s1_e = s1->get_epsilon_transitions();
    std::shared_ptr<State> s0_2 = s1_e.at(0);

    ASSERT_EQ(s0_2.get(), nfa.entry().get());
  }

  TEST(NFAOperations, NFAAtLeast)
  {
    NFA nfa('a');
    nfa.atleast(2);
    /*
    ->s0- a -s1- e -s2- a -s3
                     ^      v
                     |      |
                     ---e----
    */
    EXPECT_EQ(nfa.entry()->accepting(), false);
    State::transitions_t::iterator s0_a = nfa.entry()->get_transition('a');
    std::shared_ptr<State> s1 = s0_a->second.at(0);
    EXPECT_EQ(s1->accepting(), false);

    const State::epsilon_transitions_t& s1_e = s1->get_epsilon_transitions();
    std::shared_ptr<State> s2 = s1_e.at(0);

    EXPECT_EQ(s2->accepting(), false);

    State::transitions_t::iterator s2_a = s2->get_transition('a');
    std::shared_ptr<State> s3 = s2_a->second.at(0);
    EXPECT_EQ(s3->accepting(), true);
    ASSERT_EQ(nfa.exit().get(), s3.get());

    const State::epsilon_transitions_t& s3_e = s3->get_epsilon_transitions();
    std::shared_ptr<State> s2_r = s3_e.at(0);
    EXPECT_EQ(s2_r->accepting(), false);
    ASSERT_EQ(s2_r.get(), s2.get());
  }

  TEST(NFAOperations, NFABetween)
  {
    NFA nfa('a');
    nfa.between(2, 4);
    /*
                                                 ________
                                                 |      |
                                                 ^      v
    ->s0- a -s1- e -s2- a -s3- e -s4- a -s5- e -s6- a -s7(v)
                                   v                    ^
                                   |                    |
                                   __________e___________
    */
    std::shared_ptr<State> s0 = nfa.entry();
    EXPECT_EQ(s0->accepting(), false);
    std::shared_ptr<State> s1 = s0->get_transition('a')->second.at(0);
    EXPECT_EQ(s1->accepting(), false);
    std::shared_ptr<State> s2 = s1->get_epsilon_transitions().at(0);
    EXPECT_EQ(s2->accepting(), false);
    std::shared_ptr<State> s3 = s2->get_transition('a')->second.at(0);
    EXPECT_EQ(s3->accepting(), false);
    std::shared_ptr<State> s4 = s3->get_epsilon_transitions().at(0);
    EXPECT_EQ(s4->accepting(), false);
    std::shared_ptr<State> s5 = s4->get_transition('a')->second.at(0);
    EXPECT_EQ(s5->accepting(), false);
    std::shared_ptr<State> s6 = s5->get_epsilon_transitions().at(0);
    EXPECT_EQ(s6->accepting(), false);
    std::shared_ptr<State> s7 = s6->get_transition('a')->second.at(0);
    EXPECT_EQ(s7->accepting(), true);
    ASSERT_EQ(nfa.exit().get(), s7.get());
    std::shared_ptr<State> s7_s4 = s4->get_epsilon_transitions().at(0);
    EXPECT_EQ(s7_s4->accepting(), true);
    ASSERT_EQ(nfa.exit().get(), s7_s4.get());
    std::shared_ptr<State> s7_s6 = s6->get_epsilon_transitions().at(0);
    EXPECT_EQ(s7_s6->accepting(), true);
    ASSERT_EQ(nfa.exit().get(), s7_s6.get());
    // does copying into a shared pointer decrease the ref count?
  }
  TEST(NFAOperations, NFAExclude)
  {
    NFA nfa(0x0);
    nfa.exclude('a');

    /*
       a-s2(d)
       ^
       |
    ->s0- . -s1(v)
    */
    EXPECT_EQ(nfa.entry()->accepting(), false);
    std::shared_ptr<State> s1 = nfa.entry()->get_transition(0x0)->second.at(0);
    EXPECT_EQ(s1->accepting(), true);
    std::shared_ptr<State> s2 = nfa.entry()->get_transition('a')->second.at(0);
    EXPECT_EQ(s2->accepting(), false);
    ASSERT_EQ(nfa.exit().get(), s1.get());
  }

  TEST(NFAOperations, NFAMatchSingleChar)
  {
    NFA a('a');

    EXPECT_EQ(a.match("a"), true);
  }

  TEST(NFAOperations, NFAMatchConcatLetters) {
    NFA ab('a');
    ab.concat('b');

    EXPECT_EQ(ab.match("ab"), true);
  }

  TEST(NFAOperations, NFAMatchConcatNFA) {
    NFA ab('a');
    ab.concat('b');
    NFA ac('a');
    ac.concat('c');
    ab.concat(ac);

    EXPECT_EQ(ab.match("abac"), true);
  }

  TEST(NFAOperations, NFAMatchOrChar) {
    NFA a('a');
    a._or('b');

    EXPECT_EQ(a.match("a"), true);
    EXPECT_EQ(a.match("b"), true);
  }

  TEST(NFAOperations, NFAMatchOrNfa) {
    NFA a('a');
    NFA b('b');
    b.concat('c').concat('d');
    a._or(b);

    EXPECT_EQ(a.match("a"), true);
    EXPECT_EQ(a.match("bcd"), true);
    EXPECT_EQ(a.match("acd"), false);
  }

  TEST(NFAOperations, NFAMatchExclusion) {
    NFA a('a');
    NFA notborc(0x0);
    notborc.exclude('b').exclude('c');
    a.concat(notborc);

    EXPECT_EQ(a.match("ab"), false);
    EXPECT_EQ(a.match("ac"), false);
    EXPECT_EQ(a.match("ad"), true);
    EXPECT_EQ(a.match("ae"), true);
  }

  TEST(NFAOperations, NFAMatchRepeat) {
    NFA a('a');
    a.repeat();

    EXPECT_EQ(a.match(""), true);
    EXPECT_EQ(a.match("a"), true);
    EXPECT_EQ(a.match("aa"), true);
    EXPECT_EQ(a.match("aaa"), true);
    EXPECT_EQ(a.match("aaaa"), true);
  }

  TEST(NFAOperations, NFAMatchRepeatExactNumber) {
    NFA a('a');
    a.repeat(4);

    EXPECT_EQ(a.match("aaa"), false);
    EXPECT_EQ(a.match("aaaa"), true);
    EXPECT_EQ(a.match("aaaaa"), false);
  }

  TEST(NFAOperations, NFAMatchAtmost) {
    NFA a('a');
    a.atmost(3);

    EXPECT_EQ(a.match(""), true);
    EXPECT_EQ(a.match("a"), true);
    EXPECT_EQ(a.match("aa"), true);
    EXPECT_EQ(a.match("aaa"), true);
    EXPECT_EQ(a.match("aaaa"), false);
  }

  TEST(NFAOperations, NFAMatchAleast) {
    NFA a('a');
    a.atleast(3);

    EXPECT_EQ(a.match("aa"), false);
    EXPECT_EQ(a.match("aaa"), true);
    EXPECT_EQ(a.match("aaaa"), true);
    EXPECT_EQ(a.match("aaaaa"), true);
    EXPECT_EQ(a.match("aaaaaa"), true);
  }

  TEST(NFAOperations, NFAMatchBetween)
  {
    NFA d_between_3_5('d');
    d_between_3_5.between(3, 5);

    EXPECT_EQ(d_between_3_5.match("dd"), false);
    EXPECT_EQ(d_between_3_5.match("ddd"), true);
    EXPECT_EQ(d_between_3_5.match("dddd"), true);
    EXPECT_EQ(d_between_3_5.match("ddddd"), true);
    EXPECT_EQ(d_between_3_5.match("dddddd"), false);
  }
}