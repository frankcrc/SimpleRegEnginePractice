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
		string matchStr;
		if (expectedResult)
			isSucceeded = spTest && (spTest->validateString(str, matchStr) == expectedResult);
		else
			isSucceeded = !spTest || (spTest->validateString(str, matchStr) == expectedResult);
		cout << "result : " << left << setw(6) << boolalpha << isSucceeded;
		cout << "expectation : " << setw(6) << expectedResult << noboolalpha << right;
		cout << "Reg Str : \"" << regStr << "\", str : \"" << str;
		if (isSucceeded && expectedResult)
			cout << ", Match Str : \"" << matchStr << "\"";
		cout << endl;
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

	EngineTest::test("^a{1,3}bb$", "abb", true);
	EngineTest::test("^a{1,3}bb$", "abbb", false);

	EngineTest::test("abcd|12345", "abcd", true);
	EngineTest::test("abcd|12345", "12345", true);
	EngineTest::test("abcd|12345", "ab12345", true);

	EngineTest::test("abcd|ab12345", "abcd", true);
	EngineTest::test("abcd|ab12345", "ab12345", true);

	EngineTest::test("abc(1234|defg)xsd", "abc1234xsd", true);
	EngineTest::test("abc(1234|defg)xsd", "abcdefgxsd", true);
	EngineTest::test("abc(1234|defg)xsd", "abcxsd", false);
	EngineTest::test("abc(1234|defg)?xsd", "abcxsd", true);

	EngineTest::test("abcf?(1234?|defg?)xsd", "abc1234xsd", true);
	EngineTest::test("abcf?(1234?|defg?)xsd", "abc123xsd", true);
	EngineTest::test("abcf?(1234?|defg?)xsd", "abcf123xsd", true);
	EngineTest::test("abcf?(1234?|defg?)xsd", "abcdefxsd", true);
	EngineTest::test("abcf?(1234?|defg?)xsd", "abcdefgxsd", true);
	EngineTest::test("abcf?(1234?|defg?)xsd", "abefgxsd", false);

	EngineTest::test("abc(1234|defg)+xsd", "abc1234xsd", true);
	EngineTest::test("abc(1234|defg)+xsd", "abcxsd", false);
	EngineTest::test("abc(1234|defg)+xsd", "abc1234defg12341234xsd", true);

	EngineTest::test("abc(1234|defg)*xsd", "abc1234defg12341234xsd", true);
	EngineTest::test("abc(1234|defg)*xsd", "abcxsd", true);

	EngineTest::test("abc(1234|defg){2,2}xsd", "abcdefgdefgxsd", true);
	EngineTest::test("abc(1234|defg){2,2}xsd", "abc1234defgxsd", true);
	EngineTest::test("abc(1234|defg){2,2}xsd", "abcdefgdefg1234xsd", false);

	EngineTest::test("abc(1234|defg)*1234", "abc1234", true);
	EngineTest::test("abc(1234|defg)*1234", "abc1234defg1234", true);

	EngineTest::test("abc(1234|defg)*", "abc1234defg1234", true);

	EngineTest::test("abc*", "abccccccca", true);

	system("pause");

	return 0;
}