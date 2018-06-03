/*!
 * \file SimpleRegEngine.cpp
 *
 * \author frank
 * \date 2018-5-27 17:09
 *
 * 
 */
#include "SimpleRegEngine.h"

#include <string>
#include <set>
#include <cassert>

using namespace std;

State::State()
	: m_isFinalState(false)
{
	m_occurs.first = 1;
	m_occurs.second = 1;
}

State::State(const string &id)
	: m_id(id)
	, m_isFinalState(false)
{
	m_occurs.first = 1;
	m_occurs.second = 1;
}

void State::addAction(char ch, State *pAction)
{
	m_actionsMap.insert(make_pair(ch, pAction));
}

State* State::moveNext(char ch)
{
	{
		auto iterState = m_actionsMap.find(ch);
		if (m_actionsMap.end() == iterState)
			return nullptr;
		return iterState->second;
	}
}

SimpleRegExpEngine* SimpleRegExpEngine::constructDFA(const string& regExp)
{
	vector<shared_ptr<State>> states;
	shared_ptr<State> pStartState, pFinalState;
	shared_ptr<State> pCurState;
	bool needCheckFromFirstChar = false;

	pStartState.reset(new State("StartState"));
	states.push_back(pStartState);
	pCurState = pStartState;
	char curChOfState = 0;
	set<State*> previousStates;
	previousStates.insert(pCurState.get());

	//创建ndfa
	for (size_t i = 0; i < regExp.length(); ++i)
	{
		shared_ptr<State> pNewState;
		char ch = 0;
		Occurs occurs = Occurs(1, 1);

		if ('^' == regExp[i])
		{
			if (0 == i)
			{
				needCheckFromFirstChar = true;
				++ i;
			}
			else
				return nullptr;
		}

		if (!getNextCh(regExp, i, ch))
			return nullptr;
		if (!parseOccurs(regExp, i, occurs))
			return nullptr;

		if (ch != curChOfState)
		{
			pNewState.reset(new State());
			pNewState->setOccurs(occurs);
			if (occurs.second > 1)
				pNewState->addAction(ch, pNewState.get());
			states.push_back(pNewState);
		}
		else
		{
			pNewState = pCurState;
			Occurs oldOccurs = pNewState->getOccurs();
			oldOccurs.first += occurs.first;
			oldOccurs.second += oldOccurs.second;
			pNewState->setOccurs(oldOccurs);
		}

		if (i + 1 == regExp.length())
			pNewState->setIsFinalState(true);

		for (auto iter = previousStates.begin();
			iter != previousStates.end();
			++ iter)
			(*iter)->addAction(ch, pNewState.get());

		if (0 != pNewState->getMinOccurs())
			previousStates.clear();
		previousStates.insert(pNewState.get());

		pCurState = pNewState;
		curChOfState = ch;
	}

	SimpleRegExpEngine *pInstance = new SimpleRegExpEngine();
	pInstance->m_startState = pStartState;
	pInstance->m_states = states;
	pInstance->m_needCheckFromFirstChar = needCheckFromFirstChar;

	return pInstance;
}

SimpleRegExpEngine::~SimpleRegExpEngine()
{}

bool SimpleRegExpEngine::validateString(const string& str)
{
	for (size_t i = 0; i < str.length() && (!m_needCheckFromFirstChar || 0 == i); ++i)
	{
		unordered_map<State *, size_t> stateOccursMapping;
		State* pCurState = m_startState.get();
		size_t j = i;
		for (; j < str.length(); ++j)
		{
			//未达到最小次数
			if ((j - i) > 0 && str[j] != str[j - 1]
				&& stateOccursMapping[pCurState] < pCurState->getMinOccurs())
				break;

			//找不到下一个状态
			if (nullptr == (pCurState = pCurState->moveNext(str[j])))
				break;

			++ stateOccursMapping[pCurState];
			//超过最大次数
			if (stateOccursMapping[pCurState] > pCurState->getMaxOccurs())
				break;

			if (pCurState->isFinalState()
				&& stateOccursMapping[pCurState] >= pCurState->getMinOccurs())
				return true;
		}

		//如果字符串遍历完，状态停留在终结态，则验证失败
		if (str.length() == j && pCurState && !pCurState->isFinalState())
			break;
	}

	return false;
}

bool SimpleRegExpEngine::parseOccurs(const string &str, size_t &i, Occurs &occurs)
{
	assert(i < str.length());
	if (i == str.length() - 1)
		return true;
	switch (str[i + 1])
	{
	case '?':
		occurs.first = 0;
		occurs.second = 1;
		++ i;
		break;
	case '+':
		occurs.first = 1;
		occurs.second = numeric_limits<size_t>::max();
		++ i;
		break;
	case '*':
		occurs.first = 0;
		occurs.second = numeric_limits<size_t>::max();
		++ i;
		break;
	case '{':
	{
		size_t minOccurs = 0;
		size_t maxOccurs = 0;
		bool isParsingOccurs = false;
		bool firstNumber = true;
		for (size_t j = i + 2; j < str.length(); ++ j)
		{
			if (isdigit(str[j]))
			{
				isParsingOccurs = true;
				if (firstNumber)
					minOccurs = minOccurs * 10 + (str[j] - '0');
				else
					maxOccurs = maxOccurs * 10 + (str[j] - '0');
			}
			else if (str[j] == ',')
			{
				//没有最小次数，或者出现多次','
				if (!isParsingOccurs || !firstNumber)
					return false;
				firstNumber = false;
				isParsingOccurs = false;
			}
			else if (str[j] == '}')
			{
				if (isParsingOccurs && minOccurs > maxOccurs)
					return false;
				occurs.first = minOccurs;
				//如果没有第二个数字，则表示无限重复
				occurs.second = isParsingOccurs ? maxOccurs : numeric_limits<size_t>::max();
				i = j;
				break;
			}
			else
				return false;
		}
		break;
	}
	}
	return true;
}

bool SimpleRegExpEngine::getNextCh(const string& str, size_t &i, char &nextChar)
{
	switch (str[i])
	{
	case '\\':

		break;
	default:
		nextChar = str[i];
	}
	return true;
}

SimpleRegExpEngine::SimpleRegExpEngine()
	: m_needCheckFromFirstChar(false)
{}
