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

	State(State *pParent);

	~State()
	{}

	void addAction(char ch, State *pNextState);
	void addGroupAction(State* pGroupState);
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
	void setParent(State *pParent) { m_pParent = pParent; }
	State *getParent() const { return m_pParent; }
	void setIsGroupState(bool yes) { m_isGroupState = yes; }
	bool getIsGroupState() const { return m_isGroupState; }

private:
	std::string m_id;
	std::unordered_multimap<char, State*> m_actionsMap;
	//�����Ϊnull����ָ��group���
	State *m_pParent;
	bool m_isFinalState;
	bool m_isGroupState;
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
		size_t parentCurOccurs,
		std::unordered_map<State *, size_t> &stateOccursMappingStack);
	/*!
		�ݹ�������ɴ���Group
	 */
	static bool constructDFAImpl(const std::set<State *> prevLevelStates, bool isStateState, State *pGroup,
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