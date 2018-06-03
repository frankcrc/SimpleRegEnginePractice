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

//��ʱ��д����������Ƶ��ļ���
/*!
* \class State
*
* \brief ״̬��
*
* \author frank
* \date ���� 2018
*/

/*!
	\typedef ״̬�ɳ��ִ���

	first	��С���ִ���
	second	�����ִ���
	-1��ʾ���޴�
*/
using Occurs = std::pair<size_t, size_t>;

/*!
	\class ״̬��
 */
class State
{
public:
	typedef std::unordered_multimap<char, State*> ActionMap;

	State();

	State(const std::string &id);

	~State()
	{}

	void addAction(char ch, State *pAction);
	State* moveNext(char ch);
	std::pair<ActionMap::iterator, ActionMap::iterator> getNextStates(char ch);

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
	std::unordered_multimap<char, State*> m_actionsMap;
	bool m_isFinalState;
	Occurs m_occurs;
};

/*!
\class ��������ʽ����
*/
class SimpleRegExpEngine
{
public:
	/*!
		����ƥ������
	 */
	static SimpleRegExpEngine* constructDFA(const std::string& regExp);

public:
	~SimpleRegExpEngine();

	/*!
		��֤�ַ����Ƿ�ƥ��
	 */
	bool validateString(const std::string& str);

private:
	bool validateStringImpl(State *pCurState, const std::string &str, size_t i, bool isFirstState,
		std::unordered_map<State *, size_t>  &stateOccursMapping);
	/*!
		�ݹ�������ɴ���Group
	 */
	static bool constructDFAImpl(std::set<State *> prevLevelStates, bool isStateState,
		const std::string &regExp, size_t &i,
		std::set<State *> *pNextLevelEndStates, std::vector<std::shared_ptr<State>> &states);
	/*!
	���������С����

	\return	��ʾ�Ƿ�����ɹ�
	*/
	static bool parseOccurs(const std::string &str, size_t &i, Occurs &occurs);
	/*!
	��ȡ��һ���ַ�������ת�崦��
	*/
	static bool getNextCh(const std::string& str, size_t &i, char &nextChar, bool &escaped);

	SimpleRegExpEngine();

	std::shared_ptr<State> m_startState;
	std::vector<std::shared_ptr<State>> m_states;
	bool m_needCheckFromFirstChar;
	bool m_endInLastChar;
};

#endif	//SIMPLEREGENGINE_H