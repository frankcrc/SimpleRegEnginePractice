/*!
 * \file SimpleRegEngine.h
 *
 * \author frank
 * \date 2018-5-27 17:03
 *
 * 
 */
#ifndef SIMPLEREGENGINE_H
#define SIMPLEREGENGINE_H

#include <unordered_map>
#include <set>
#include <vector>
#include <memory>
#include <string>

//暂时先写这里，后面再移到文件里
/*!
* \class State
*
* \brief 状态类
*
* \author frank
* \date 五月 2018
*/

/*!
	\typedef 状态可出现次数

	first	最小出现次数
	second	最大出现次数
	-1表示无限次
*/
using Occurs = std::pair<size_t, size_t>;

/*!
	\class 状态类
 */
class State
{
public:
	typedef std::unordered_multimap<char, State*> ActionMap;

	using Action = std::pair<char, State*>;
	using Actions = std::vector<Action>;
	using ActionIter = Actions::iterator;
	struct ActionLess
	{
		ActionLess(const State *pState)
			: pThis(pState)
		{}

		bool operator()(const Action &lhs, const Action &rhs)
		{
			if (lhs.first == rhs.first && lhs.second != nullptr && rhs.second != nullptr)
				return lhs.second == pThis;
			else
				return lhs.first < rhs.first;
		}

		const State *pThis;
	};

	State(State *pParent);

	~State()
	{}

	void addAction(char ch, State *pNextState);
	void addGroupAction(State* pGroupState);
	std::pair<ActionIter, ActionIter> getNextStates(char ch);

	void setIsFinalState(bool yes) { m_isFinalState = yes; }
	bool isFinalState() const { return m_isFinalState; }
	void setOccurs(const Occurs &occurs) { m_occurs = occurs; }
	Occurs getOccurs() const { return m_occurs; }
	void setMinOccurs(size_t value) { m_occurs.first = value; }
	size_t getMinOccurs() const { return m_occurs.first; }
	void setMaxOccurs(size_t value) { m_occurs.second = value; }
	size_t getMaxOccurs() const { return m_occurs.second; }
	void setParent(State *pParent) { m_pParent = pParent; }
	State *getParent() const { return m_pParent; }
	void setIsGroupState(bool yes) { m_isGroupState = yes; }
	bool getIsGroupState() const { return m_isGroupState; }

private:
	void normalizeActions();

	std::string m_id;
	std::unordered_multimap<char, State*> m_actionsMap;
	Actions m_actions;
	//如果不为null，则指向group结点
	State *m_pParent;
	bool m_isFinalState;
	bool m_isGroupState;
	bool m_isNormalized;
	Occurs m_occurs;
	ActionLess m_actionLess;
};

/*!
\class 简单正则表达式引擎
*/
class SimpleRegExpEngine
{
public:
	/*!
		构造匹配引擎
	 */
	static SimpleRegExpEngine* constructDFA(const std::string &regExp);

public:
	~SimpleRegExpEngine();

	/*!
		验证字符串是否匹配
	 */
	bool validateString(const std::string &str, std::string &matchStr);

private:
	bool validateStringImpl(State *pCurState, const std::string &str, size_t i, bool isFirstState,
		size_t parentCurOccurs,
		std::unordered_map<State *, size_t> &stateOccursMappingStack, std::string &matchStr);
	/*!
		递归解析，可处理Group
	 */
	static bool constructDFAImpl(const std::set<State *> prevLevelStates, bool isStateState, State *pGroup,
		const std::string &regExp, size_t &i,
		std::set<State *> *pNextLevelEndStates, std::vector<std::shared_ptr<State>> &states);
	/*!
	解析最大最小次数

	\return	表示是否解析成功
	*/
	static bool parseOccurs(const std::string &str, size_t &i, Occurs &occurs);
	/*!
	获取下一个字符，包含转义处理
	*/
	static bool getNextCh(const std::string &str, size_t &i, char &nextChar, bool &escaped);

	SimpleRegExpEngine();

	std::shared_ptr<State> m_startState;
	std::vector<std::shared_ptr<State>> m_states;
	bool m_needCheckFromFirstChar;
	bool m_endInLastChar;
};

#endif	//SIMPLEREGENGINE_H