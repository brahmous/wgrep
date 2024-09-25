#include <iostream>
#include <cassert>
#include <unordered_map>
#include "../engine/nfa.h"
#include <queue>
#include <clocale>

//#include <iostream>
//#include <fcntl.h>
//#include <io.h>
//
//
//class Num {
//public:
//	int value;
//	Num(int number) : value{ number }
//	{}
//};
//
//class Obj {
//public:
//	std::weak_ptr<Num> number;
//
//	Obj(int n) : number{ Num(n) }
//	{}
//};

int main(int argc, char* argv[])
{

	Fragment f1('a');
	Fragment f2('b');
	NFA<UNION> n1{ f1, f2 };
	Fragment f3('c');
	Fragment f4('d');
	NFA<UNION> n2{ f3, f4 };
	NFA<CONCATENATION> n3(n1, n2);
	NFA<KLEENE> n4(n3);

	std::cout << "result: " << n4.match("acacaac") << std::endl;
	//std::cout << "result: " << n4.match("ac") << std::endl;
	//std::cout << "result: " << n4.match("bc") << std::endl;
	//std::cout << "result: " << n4.match("bd") << std::endl;

	std::cout << n4 << "\n";

 
	//_setmode(_fileno(stdout), _O_U16TEXT);
	//wchar_t c = L'\u03B5';
	//std::wcout << c << std::endl;
	//std::string str = u8"\u03b5";
	//std::cout << str << std::endl;

	return 0;
};