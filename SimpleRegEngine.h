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
	State();

	State(const std::string &id);

	~State()
	{}

	void addAction(char ch, State *pAction);
	State* moveNext(char ch);

	void setIsFinalState(bool yes) { m_isFinalState = yes; }
	bool isFinalState() const { return m_isFinalState; }
	void setOccurs(const Occurs &occurs) { m_occurs = occurs; }
	Occurs getOccurs() const { return m_occurs; }
	void setMinOccurs(size_t value) { m_occurs.first = value; }
	size_t getMinOccurs() const { return m_occurs.first; }
	void setMaxOccurs(size_t value) { m_occurs.second = value; }
	size_t getMaxOccurs() const { return m_occurs.second; }

private:
	std::string m_id;
	std::unordered_map<char, State*> m_actionsMap;
	bool m_isFinalState;
	Occurs m_occurs;
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
	static SimpleRegExpEngine* constructDFA(const std::string& regExp);

public:
	~SimpleRegExpEngine();

	/*!
		验证字符串是否匹配
	 */
	bool validateString(const std::string& str);

private:
	/*!
	解析最大最小次数

	\return	表示是否解析成功
	*/
	static bool parseOccurs(const std::string &str, size_t &i, Occurs &occurs);
	/*!
	获取下一个字符，包含转义处理
	*/
	static bool getNextCh(const std::string& str, size_t &i, char &nextChar);

	SimpleRegExpEngine();

	std::shared_ptr<State> m_startState;
	std::vector<std::shared_ptr<State>> m_states;
	bool m_needCheckFromFirstChar;
};

#endif	//SIMPLEREGENGINE_H