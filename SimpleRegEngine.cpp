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
#include <iterator>
#include <algorithm>

using namespace std;

State::State(State *pParent)
	: m_isFinalState(false)
	, m_pParent(pParent)
	, m_isGroupState(false)
	, m_isNormalized(false)
	, m_actionLess(this)
{
	m_occurs.first = 1;
	m_occurs.second = 1;
}

void State::addAction(char ch, State *pNextState)
{
	m_actions.emplace_back(ch, pNextState);
}

void State::addGroupAction(State* pGroupState)
{
	for (auto iter = pGroupState->m_actions.begin(); iter != pGroupState->m_actions.end(); ++iter)
		m_actions.emplace_back(iter->first, pGroupState);
}

pair<State::ActionIter, State::ActionIter> State::getNextStates(char ch)
{
	if (!m_isNormalized)
	{
		normalizeActions();
		m_isNormalized = true;
	}
	Action action = { ch, nullptr };
	return std::equal_range(m_actions.begin(), m_actions.end(), action, m_actionLess);
}

void State::normalizeActions()
{
	std::sort(m_actions.begin(), m_actions.end(), m_actionLess);
}

SimpleRegExpEngine* SimpleRegExpEngine::constructDFA(const string& regExp)
{
	vector<shared_ptr<State>> states;
	shared_ptr<State> pStartState;
	bool needCheckFromFirstChar = false;
	bool endInLastChar = false;

	pStartState.reset(new State(nullptr));
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

	if (!constructDFAImpl(previousStates, true, nullptr, adjustedRegExp, i, nullptr, states))
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

bool SimpleRegExpEngine::validateString(const string& str, std::string &matchStr)
{
	for (size_t i = 0; i < str.length() && (!m_needCheckFromFirstChar || 0 == i); ++i)
	{
		std::string tmp;
		unordered_map<State *, size_t> stateOccursMappingStack;
		if (validateStringImpl(m_startState.get(), str, i, true, 1, stateOccursMappingStack, tmp))
		{
			matchStr = tmp;
			return true;
		}
	}

	return false;
}

bool SimpleRegExpEngine::validateStringImpl(State *pCurState, const string &str, size_t i, bool isFirstState,
	size_t parentOccurs,
	unordered_map<State *, size_t> &stateOccursMapping, std::string &matchStr)
{
	matchStr += str[i];
	if (isFirstState)
		++ stateOccursMapping[pCurState];

	if (pCurState->getIsGroupState() && parentOccurs > pCurState->getMaxOccurs())
	{
		matchStr.pop_back();
		return false;
	}

	pair<State::ActionIter, State::ActionIter> nextActions = pCurState->getNextStates(str[i]);
	//�Ҳ�����һ��״̬
	if (nextActions.first == nextActions.second)
	{
		matchStr.pop_back();
		return false;
	}

	bool jumpOutGroup = false;
	for (; nextActions.first != nextActions.second; ++ nextActions.first)
	{
		State *pNextState = nextActions.first->second;
		//δ�ﵽ��С����
		if (pCurState->getIsGroupState())
		{
			if (parentOccurs < pCurState->getMinOccurs() && pNextState->getParent() != pCurState)
				continue;
			if (pNextState->getParent() != pCurState)
			{
				jumpOutGroup = true;
				matchStr.pop_back();
				matchStr += ')';
				matchStr += str[i];
			}
		}
		else if (stateOccursMapping[pCurState] < pCurState->getMinOccurs()
			&& pCurState != pNextState)
			continue;

		++ stateOccursMapping[pNextState];

		//����������
		if (stateOccursMapping[pNextState] > pNextState->getMaxOccurs())
		{
			-- stateOccursMapping[pNextState];
			continue;
		}

		//������̬
		if (pNextState->isFinalState()
			&& stateOccursMapping[pNextState] >= pNextState->getMinOccurs())
		{
			if (auto *pParent = pNextState->getParent())
			{
				//Group�ս���
				if (pParent->isFinalState()
					&& parentOccurs >= pParent->getMinOccurs()
					&& (!m_endInLastChar || i == str.length() - 1))
					return true;

				if (i == str.length() - 1)
				{
					-- stateOccursMapping[pNextState];
					continue;
				}

				//��Group��ʼ��һ��ƥ��
				{
					unordered_map<State *, size_t> newStateOccursMapping;
					if (validateStringImpl(pParent, str, i + 1, false, parentOccurs + 1, newStateOccursMapping, matchStr))
						return true;
				}
			}
			else if(!m_endInLastChar || i == str.length() - 1)
				//Reg�ս���
				return true;
		}
		
		//�ַ���������ɣ���״̬��δ������̬������֤ʧ��
		if (i == str.length() - 1)
		{
			-- stateOccursMapping[pNextState];
			continue;
		}

		//������һ״̬
		if (pNextState->getIsGroupState())
		{
			unordered_map<State *, size_t> newStateOccursMapping;
			matchStr.pop_back();
			matchStr += '(';
			if (validateStringImpl(pNextState, str, i, false, 0, newStateOccursMapping, matchStr))
				return true;
			matchStr.pop_back();
			matchStr += str[i];
		}
		else if (validateStringImpl(pNextState, str, i + 1, false, parentOccurs, stateOccursMapping, matchStr))
			return true;

		-- stateOccursMapping[pNextState];
	}

	if (jumpOutGroup)
	{
		matchStr.pop_back();
		matchStr.pop_back();
		matchStr += str[i];
	}
	matchStr.pop_back();
	return false;
}

bool SimpleRegExpEngine::constructDFAImpl(const set<State *> prevLevelStates, bool isStartState, State *pGroup,
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
				//����Group״̬
				pNewState.reset(new State(pGroup));
				pNewState->setIsGroupState(true);
				states.push_back(pNewState);
				set<State*> tmpPrevStates;
				tmpPrevStates.insert(pNewState.get());
				//Group�ĳ��ִ������ڲ�����
				if (!constructDFAImpl(tmpPrevStates, false, pNewState.get(), regExp, j, &nextLevelEndStates, states))
					return false;

				for (auto iter = prevStates.begin(); iter != prevStates.end(); ++ iter)
					(*iter)->addGroupAction(pNewState.get());

				curChOfState = 0;
				break;
			}
			case ')':
			{
				//Group�ս���
				prevStates.insert(endStates.begin(), endStates.end());
				for (auto iter = prevStates.begin(); iter != prevStates.end(); iter ++)
					(*iter)->setIsFinalState(true);
				pGroup->setOccurs(occurs);
				i = j;
				return true;
			}
			}
			if (needContinue)
				continue;
		}

		if (nullptr == pNewState || !pNewState->getIsGroupState())
		{
			if (ch != curChOfState)
			{
				pNewState.reset(new State(pGroup));
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

			for (auto iter = prevStates.begin();
				iter != prevStates.end();
				++ iter)
				(*iter)->addAction(ch, pNewState.get());

			curChOfState = ch;
		}

		if (j + 1 == regExp.length())
			pNewState->setIsFinalState(true);

		if (0 != pNewState->getMinOccurs())
			prevStates.clear();
		prevStates.insert(pNewState.get());

		pCurState = pNewState;
	}

	return true;
}

bool SimpleRegExpEngine::parseOccurs(const string &str, size_t &i, Occurs &occurs)
{
	assert(i < str.length());
	if (i == str.length() - 1)
		return true;
	//����Group��Ҫ�ӳٵ�')'�ٴ���
	if ('(' == str[i])
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
