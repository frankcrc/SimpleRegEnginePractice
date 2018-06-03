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
	m_actionsMap.find(ch);
	auto iterState = m_actionsMap.find(ch);
	if (m_actionsMap.end() == iterState)
		return nullptr;
	return iterState->second;
}

pair<State::ActionMap::iterator, State::ActionMap::iterator> State::getNextStates(char ch)
{
	return m_actionsMap.equal_range(ch);
}

SimpleRegExpEngine* SimpleRegExpEngine::constructDFA(const string& regExp)
{
	vector<shared_ptr<State>> states;
	shared_ptr<State> pStartState;
	bool needCheckFromFirstChar = false;
	bool endInLastChar = false;

	pStartState.reset(new State("StartState"));
	states.push_back(pStartState);
	set<State*> previousStates;
	previousStates.insert(pStartState.get());

	size_t i = 0;
	string adjustedRegExp = regExp;
	if (!regExp.empty())
	{
		if ('^' == regExp[0])
		{
			needCheckFromFirstChar = true;
			++ i;
		}
		if ('$' == regExp[regExp.length() - 1])
		{
			adjustedRegExp = regExp.substr(0, regExp.length() - 1);
			endInLastChar = true;
		}
	}

	if (!constructDFAImpl(previousStates, true, adjustedRegExp, i, nullptr, states))
		return false;

	SimpleRegExpEngine *pInstance = new SimpleRegExpEngine();
	pInstance->m_startState = pStartState;
	pInstance->m_states = states;
	pInstance->m_needCheckFromFirstChar = needCheckFromFirstChar;
	pInstance->m_endInLastChar = endInLastChar;

	return pInstance;
}

SimpleRegExpEngine::~SimpleRegExpEngine()
{}

bool SimpleRegExpEngine::validateString(const string& str)
{
	for (size_t i = 0; i < str.length() && (!m_needCheckFromFirstChar || 0 == i); ++i)
	{
		unordered_map<State *, size_t> stateOccursMapping;
		if (validateStringImpl(m_startState.get(), str, i, true, stateOccursMapping))
			return true;
	}

	return false;
}

bool SimpleRegExpEngine::validateStringImpl(State *pCurState, const std::string &str, size_t i, bool isFirstState,
	std::unordered_map<State *, size_t> &stateOccursMapping)
{
	//δ�ﵽ��С����
	if (!isFirstState && str[i] != str[i - 1]
		&& stateOccursMapping[pCurState] < pCurState->getMinOccurs())
		return false;

	pair<State::ActionMap::iterator, State::ActionMap::iterator> nextActions = pCurState->getNextStates(str[i]);
	//�Ҳ�����һ��״̬
	if (nextActions.first == nextActions.second)
		return false;

	for (; nextActions.first != nextActions.second; ++ nextActions.first)
	{
		State *pNextState = nextActions.first->second;
		++ stateOccursMapping[pNextState];

		//����������
		if (stateOccursMapping[pNextState] > pNextState->getMaxOccurs())
			break;

		//������̬
		if (pNextState->isFinalState()
			&& stateOccursMapping[pNextState] >= pNextState->getMinOccurs())
			return m_endInLastChar ? (i == str.length() - 1) : true;
		
		//�ַ���������ɣ���״̬��δ������̬������֤ʧ��
		if (i == str.length() - 1)
			return false;

		//������һ״̬
		if (validateStringImpl(pNextState, str, i + 1, false, stateOccursMapping))
			return true;

		-- stateOccursMapping[pNextState];
	}

	return false;
}

bool SimpleRegExpEngine::constructDFAImpl(set<State *> prevLevelStates, bool isStartState,
	const std::string &regExp, size_t &i,
	set<State *> *pNextLevelEndStates, vector<shared_ptr<State>> &states)
{
	char curChOfState = 0;
	set<State*> prevStates = prevLevelStates;
	set<State*> endStates;
	shared_ptr<State> pCurState;

	for (size_t j = i; j < regExp.length(); ++j)
	{
		shared_ptr<State> pNewState;
		char ch = 0;
		Occurs occurs = Occurs(1, 1);
		bool escaped = false;

		if (!getNextCh(regExp, j, ch, escaped))
			return false;
		if (!parseOccurs(regExp, j, occurs))
			return false;

		if (!escaped)
		{
			bool needContinue = false;
			switch (ch)
			{
			case '&':
				if (i == regExp.length() - 1)
					return true;
			case '^':
				return false;
			case '|':
			{
				if (isStartState)
				{
					for (auto iter = prevStates.begin(); iter != prevStates.end(); ++ iter)
						(*iter)->setIsFinalState(true);
				}
				endStates.insert(prevStates.begin(), prevStates.end());
				prevStates = prevLevelStates;
				needContinue = true;
				break;
			}
			case '(':
			{
				set<State*> nextLevelEndStates;
				++ j;
				if (!constructDFAImpl(prevStates, false, regExp, j, &nextLevelEndStates, states))
					return false;
				prevStates = nextLevelEndStates;
				needContinue = true;
				break;
			}
			case ')':
			{
				endStates.insert(prevStates.begin(), prevStates.end());
				if (pNextLevelEndStates)
					*pNextLevelEndStates = endStates;
				i = j;
				return true;
			}
			}
			if (needContinue)
				continue;
		}

		if (ch != curChOfState)
		{
			pNewState.reset(new State());
			pNewState->setOccurs(occurs);
			//������ظ��������������Ϊ����״̬
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

		if (j + 1 == regExp.length())
			pNewState->setIsFinalState(true);

		for (auto iter = prevStates.begin();
			iter != prevStates.end();
			++ iter)
			(*iter)->addAction(ch, pNewState.get());

		if (0 != pNewState->getMinOccurs())
			prevStates.clear();
		prevStates.insert(pNewState.get());

		pCurState = pNewState;
		curChOfState = ch;
	}

	return true;
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
				//û����С���������߳��ֶ��','
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
				//���û�еڶ������֣����ʾ�����ظ�
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

bool SimpleRegExpEngine::getNextCh(const string& str, size_t &i, char &nextChar, bool &escaped)
{
	switch (str[i])
	{
	case '\\':
		escaped = true;
		++ i;
		//��ʱ���ж�ת���Ƿ���Ч
		nextChar = str[i];
		if (false)
			return false;
		break;
	default:
		nextChar = str[i];
	}
	return true;
}

SimpleRegExpEngine::SimpleRegExpEngine()
	: m_needCheckFromFirstChar(false)
	, m_endInLastChar(false)
{}
