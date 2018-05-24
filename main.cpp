/*!
 * \file main.cpp
 *
 * \author frank
 * \date 2018-5-13 19:54
 *
 * 简单正则表达式引擎
 */
#include <iostream>
#include <unordered_map>
#include <memory>
#include <string>

using namespace std;

//暂时先写这里，后面再移到文件里
/*!
 * \class State
 *
 * \brief 状态类
 *
 * \author frank
 * \date 五月 2018
 */
class State
{
public:
	State(const string &id)
		: m_id(id)
		, m_isFinalState(false)
	{}

	~State()
	{}

	void addAction(char ch, State *pAction)
	{
		m_actionsMap.insert(make_pair(ch, pAction));
	}

	State* moveNext(char ch)
	{
		auto iterState = m_actionsMap.find(ch);
		if (m_actionsMap.end() == iterState)
			return nullptr;
		return iterState->second;
	}

	void setIsFinalState(bool yes)
	{
		m_isFinalState = yes;
	}

	bool isFinalState() const
	{
		return m_isFinalState;
	}

private:
	string m_id;
	unordered_map<char, State*> m_actionsMap;
	bool m_isFinalState;
};

class SimpleRegExpEngine
{
public:
	static SimpleRegExpEngine* constructDFA(const string& regExp)
	{
		unordered_map<string, shared_ptr<State>> stateHelper;
		shared_ptr<State> pStartState, pFinalState;
		shared_ptr<State> pCurState;

		pStartState.reset(new State("StartState"));
		pCurState = pStartState;

		//创建ndfa
		for (size_t i = 0; i < regExp.length(); ++i)
		{
			//尾字符不参与后面的dfa转换和化简
			if (regExp.length() == i + 1)
			{
				pFinalState.reset(new State({regExp[i]}));
				pFinalState->setIsFinalState(true);
				pCurState->addAction(regExp[i], pFinalState.get());
			}
			else
			{
				shared_ptr<State> pNewState;
				auto iterState = stateHelper.find({regExp[i]});
				if (stateHelper.end() == iterState)
				{
					pNewState.reset(new State({regExp[i]}));
					stateHelper.insert(make_pair(string({regExp[i]}), pNewState));
				}
				else
					pNewState = iterState->second;
				pCurState->addAction(regExp[i], pNewState.get());
				pCurState = pNewState;
			}
		}

		SimpleRegExpEngine *pInstance = new SimpleRegExpEngine();
		pInstance->m_startState = pStartState;
		pInstance->m_states = stateHelper;

		return pInstance;
	}

public:
	~SimpleRegExpEngine()
	{

	}

	bool validateString(const string& str)
	{
		for (size_t i = 0; i < str.length(); ++i)
		{
			State* pCurState = m_startState.get();
			size_t j = i;
			for (; j < str.length(); ++j)
			{
				if (nullptr == (pCurState = pCurState->moveNext(str[j])))
					break;

				if (pCurState->isFinalState())
					return true;
			}

			//如果字符串遍历完，状态停留在终结态，则验证失败
			if (str.length() == j && pCurState && !pCurState->isFinalState())
				break;
		}

		return false;
	}

private:
	SimpleRegExpEngine()
	{

	}

	shared_ptr<State> m_startState;
	unordered_map<string, shared_ptr<State>> m_states;
};

int main()
{
	SimpleRegExpEngine *pTest = SimpleRegExpEngine::constructDFA("abbacbf");

	cout << boolalpha
		<< pTest->validateString("abbacbf") << endl
		<< pTest->validateString("1234abbacbf") << endl
		<< pTest->validateString("1234abbacbf4321") << endl
		<< pTest->validateString("abbacbf4321") << endl
		<< pTest->validateString("abbacb") << endl
		<< endl;

	return 0;
}