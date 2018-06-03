/*!
 * \file main.cpp
 *
 * \author frank
 * \date 2018-5-13 19:54
 *
 * 简单正则表达式引擎
 */
#include <iostream>
#include <string>
#include <iomanip>
#include "SimpleRegEngine.h"

using namespace std;

class EngineTest
{
public:
	static void test(const string &regStr, const string &str, bool expectedResult)
	{
		unique_ptr<SimpleRegExpEngine> spTest(SimpleRegExpEngine::constructDFA(regStr));
		bool isSucceeded = true;
		if (expectedResult)
			isSucceeded = spTest && (spTest->validateString(str) == expectedResult);
		else
			isSucceeded = !spTest || (spTest->validateString(str) == expectedResult);
		cout << "result : " << left << setw(6) << boolalpha << isSucceeded;
		cout << "expectation : " << setw(6) << expectedResult << noboolalpha << right;
		cout << "Reg Str : \"" << regStr << "\", str : \"" << str << endl;
	}
};

int main()
{
	EngineTest::test("abbacbf", "abbacbf", true);
	EngineTest::test("abbacbf", "1234abbacbf", true);
	EngineTest::test("abbacbf", "abbacbf4321", true);
	EngineTest::test("abbacbf", "1234abbacbf4321", true);
	EngineTest::test("abbacbf", "abbacb", false);
	EngineTest::test("abbacbf", "abacb", false);
	EngineTest::test("abbacbf", "abbbacb", false);

	EngineTest::test("a?bb", "abb", true);
	EngineTest::test("a?bb", "bb", true);
	EngineTest::test("^a?bb", "aabb", false);

	EngineTest::test("^a+bb", "abb", true);
	EngineTest::test("^a+bb", "bb", false);
	EngineTest::test("^a+bb", "aabb", true);

	EngineTest::test("^a*bb", "abb", true);
	EngineTest::test("^a*bb", "bb", true);
	EngineTest::test("^a*bb", "aabb", true);

	EngineTest::test("^a{1,}bb", "abb", true);
	EngineTest::test("^a{1,}bb", "bb", false);
	EngineTest::test("^a{1,}bb", "aabb", true);

	EngineTest::test("^a{1,3}bb", "abb", true);
	EngineTest::test("^a{1,3}bb", "bb", false);
	EngineTest::test("^a{1,3}bb", "aaabb", true);
	EngineTest::test("^a{1,3}bb", "aaaabb", false);

	system("pause");

	return 0;
}