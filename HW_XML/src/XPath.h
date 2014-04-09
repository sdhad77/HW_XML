/*
 * XPath.h
 *
 *  Created on: 2014. 4. 7.
 *      Author: 신동환
 */

#include <stdlib.h>
#include <list>
#include <string>
#include <queue>
#include "XString.h"
#include "XMLNode.h"

#ifndef XPATH_H_
#define XPATH_H_

#define MAX_CHAR_SIZE 500
#define MAX_BUF_SIZE 800

enum CommandType { search_TagName, search_AttributeName};

class XPath : public XString {
public:
	XPath();
	virtual ~XPath();

private:
	std::queue<XMLNode*> searchNodeQ; // 선택된 노드들을 저장하는 큐
	PrintType printType; //출력하는 타입을 지정하는 변수

	char* cmdBuf; // 커맨드 한 줄의 버퍼
	char* strBuf; // 커맨드 한 줄에서 일정한 기준으로 끊어낸 커맨드 저장용 버퍼
	int cmdIdx; // cmdBuf에서 사용할 인덱스
	bool firstCallSearch_All; //Search_All 함수가 처음 호출됐는지 확인하기 위한 변수

public:
	void ClearQ(); //searchNodeQ 큐를 비움. pop() 사용.
	void StrCpyFromCmdBuf(); //cmdBuf에서 strBuf로 알파벳인 문자열 일부 복사.
	void NumberCpyFromCmdBuf();//cmdBuf에서 strBuf로 숫자인 문자열 일부 복사.
	void ErrorCollection(const char* str); //XPathCmdParser() 함수 호출중에 발생하는 error들을 한곳에서 처리함.
	int FuncCollection(const char* str); //커맨드 중에 함수 호출이 있을 경우 여기서 처리함.

	int XPathCmdParser(char* _cmdBuf, XMLNode* _XpathRoute);//cmd분석 함수.
	void Search_All_NonString(XMLNode* _XpathRoute); //Search_All() 함수를 호출하기 위한 함수.
	void Search_All(XMLNode* _XpathRoute); //문자열에 상관없이 _XpathRoute이하의 전체 노드를 저장하는 함수.
	void Search_All(XMLNode* _XpathRoute, const char* str, CommandType _commandType);//가장 기본적인 탐색함수. 전체 순회 방식
	void Search_Child(const char* str); //searchNodeQ 큐에 있는 노드들의 자식중에 문자열str과 일치하는 노드들을 push함.
	void PrintNodeQ();//searchNodeQ 에 있는 노드들 출력
};

#endif /* XPATH_H_ */
