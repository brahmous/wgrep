#include <iostream>
#include <cassert>
#include "../engine/nfa.h"

int main(int argc, char* argv[])
{
	State s1;
	State s2;

	Fragment f1(s1, s2);

	State s3;
	State s4;

	Fragment f2(s3, s4);

	NFA<CONCATENATION>(f1, f2, '&');
	std::cout << f1.start_state->epsilon_transitions.size() << std::endl;

};