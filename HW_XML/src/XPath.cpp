/*
 * XPath.cpp
 *
 *  Created on: 2014. 4. 7.
 *      Author: 신동환
 */

#include "XPath.h"

XPath::XPath() {
	printType = print_Value;
	firstCallSearch_All = true;
	cmdIdx = 0;
	cmdBuf = new char[MAX_BUF_SIZE];
	strBuf = new char[MAX_CHAR_SIZE];
}

XPath::~XPath() {
	delete[] cmdBuf;
	delete[] strBuf;
}

//노드 저장용 큐 비우기
void XPath::ClearQ()
{
	while(searchNodeQ.size()) searchNodeQ.pop();
}

//cmd버퍼로부터 단어단위로 잘라서 strBuf에 저장.
void XPath::StrCpyFromCmdBuf()
{
	int _startIdx = cmdIdx; //복사할 문자열의 시작 인덱스 저장.
	while(checkTagName(cmdBuf[cmdIdx])) cmdIdx++; //알파벳,숫자,-_. 이 아닌 문자가 나올때까지 인덱스 이동
	StrCpyNumber(strBuf, &cmdBuf[_startIdx], cmdIdx-_startIdx);//문자열 시작점부터 문자가 아닌것 직전까지 복사함
}

//cmd버퍼로부터 숫자를 잘라서 strBuf에 저장.
//현재 int만 가능, 실수형도 가능하게끔 변경해야함.
void XPath::NumberCpyFromCmdBuf()
{
	int _startIdx = cmdIdx;
	while(checkNumber(cmdBuf[cmdIdx])) cmdIdx++;
	StrCpyNumber(strBuf, &cmdBuf[_startIdx], cmdIdx-_startIdx);
}

//command 읽는중에 발생하는 에러들을 한곳에 모아놓음.
void XPath::ErrorCollection(const char* str)
{
	if(!strcmp(str, "cmd"))			std::cout << "cmd를 다시 입력해 주세요." << std::endl;
	else if(!strcmp(str, "//@"))	std::cout << "잘못입력하였습니다." << std::endl;
	else if(!strcmp(str, "//"))		std::cout << "// 이후 알파벳,*,@ 이외의 문자가 입력되었습니다." << std::endl;
	else if(!strcmp(str, "/"))		std::cout << "/ 이후 처리되지 않은 문자가 나타났습니다." << std::endl;
	else if(!strcmp(str, "[999]"))	std::cout << "입력된 숫자가 너무 큽니다." << std::endl;
	else if(!strcmp(str, "[999"))	std::cout << "[로 시작하면 ]로 끝나야 합니다." << std::endl;
	else if(!strcmp(str, "[func("))	std::cout << ") 는 어디에.." << std::endl;
	else if(!strcmp(str, "[~"))		std::cout << "[ 이후 처리되지 않은 문자가 입력되었습니다." << std::endl;
	else if(!strcmp(str, "[name~$"))std::cout << "[a 이후 처리되지 않은 문자가 입력되었습니다." << std::endl;
	else if(!strcmp(str, "FuncName"))std::cout << "아직 처리 되지 않은 함수명이 입력되었습니다." << std::endl;
	else							std::cout << "error" << std::endl;

	cmdBuf[cmdIdx] = '\0'; //XPathCmdParser()의 while 반복문 정지시키는 기능.
	ClearQ();
}

int XPath::FuncCollection(const char* str)
{
	if(!strcmp(str, "last"))
	{
		return searchNodeQ.size();
	}
	else if(!strcmp(str, "position"))
	{
		std::cout << "position" << std::endl;
	}
	else
	{
		ErrorCollection("FuncName");
	}

	cmdBuf[cmdIdx] = '\0'; //XPathCmdParser()의 while 반복문 정지시키는 기능.
	return -1;
}

//입력받은 커맨드를 분석하고 실행함.
//자료의 형태는 트리이며,
//탐색 방식은 크게 두가지가 존재함.
//1.재귀호출을 이용한 트리 전체 순회 방식
//2.큐를 이용한 깊이별 탐색 방식
int XPath::XPathCmdParser(char* _cmdBuf, XMLNode* _XpathRoute)
{
	cmdIdx = 0;
	strcpy(cmdBuf, _cmdBuf);

	ClearQ(); //탐색한 노드 저장용 큐 초기화

	//루트 부터 탐색하기 위해 루트의 부모 노드가 필요함.
	XMLNode* tempNode = new XMLNode;
	tempNode->setName("Root's Parent"); //이름 설정
	tempNode->setChildNode(_XpathRoute);//루트를 자식으로 설정
	searchNodeQ.push(tempNode);//큐에 push함.

	while(cmdBuf[cmdIdx] != '\0')
	{
		RemoveBlank(cmdBuf, &cmdIdx);

		//cmd : /
		if(cmdBuf[cmdIdx] == '/')
		{
			//cmd : //
			if(cmdBuf[cmdIdx+1] == '/')
			{
				//cmd : //*
				if(cmdBuf[cmdIdx+2] == '*')
				{
					cmdIdx = cmdIdx + 3; // cmd : //* 이후로 인덱스 이동
					Search_All_NonString(_XpathRoute); //문자열에 관계없이 루트노드부터 전부 저장.
					printType = print_Name; //출력 타입을 이름으로 설정.
				}
				//cmd : //@
				else if(cmdBuf[cmdIdx+2] == '@')
				{
					//cmd : //@attributeName
					if(checkAlpha(cmdBuf[cmdIdx+3]))
					{
						cmdIdx = cmdIdx + 3;			//cmd버퍼의 인덱스를 첫번째 알파벳으로 위치시킴.
						StrCpyFromCmdBuf();				//cmd버퍼에서 단어단위로 잘라서 복사함.
						RemoveBlank(cmdBuf, &cmdIdx);	//잘라낸 이후에 공백이 있을수도 있으니 cmd버퍼의 공백 제거
						ClearQ();						//노드를 저장할 큐를 비움.
														//루트부터 잘라낸 str과 일치하는 속성이름을 검색하여 큐에 저장함.
						Search_All(_XpathRoute, strBuf, search_AttributeName);
						printType = print_Value;//출력값을 value로 설정.
					}
					else ErrorCollection("//@");
				}
				//cmd : //tagName
				else if(checkAlpha(cmdBuf[cmdIdx+2]))
				{
					cmdIdx = cmdIdx + 2; //cmd : //이후로 인덱스 이동
					StrCpyFromCmdBuf(); //cmd 버퍼에서 str 버퍼로 커맨드 이동
					RemoveBlank(cmdBuf, &cmdIdx); //cmd버퍼 공백제거
					ClearQ();			//큐 비우기
					Search_All(_XpathRoute, strBuf, search_TagName); //루트 부터 탐색하여 strBuf에 있는 문자열과 일치하는 노드 저장.
					printType = print_Value;//출력값을 value로 설정.
				}
				else ErrorCollection("//");
			}
			//cmd : /*
			else if(cmdBuf[cmdIdx+1] == '*')
			{
				int tempQSize = searchNodeQ.size(); //임시 큐 사이즈 저장. size()함수 그대로 사용하면 값이 계속 바뀌기 때문.
				while(tempQSize--)
				{
					Search_All_NonString(searchNodeQ.front());//큐에 저장된 노드들 기준으로 문자열과 상관없이 탐색하여 모든 노드를 큐에 저장.
					searchNodeQ.pop(); //방금 했던 탐색의 루트가 됐던 큐 제거
				}
				cmdIdx = cmdIdx + 2; //cmd : /* 이후로 인덱스 이동
				printType = print_Name;
			}
			//cmd : /tagName
			else if(checkAlpha(cmdBuf[cmdIdx+1]))
			{
				cmdIdx = cmdIdx + 1; //cmd : /a a로 인덱스 이동
				StrCpyFromCmdBuf(); //strBuf로 문자열 복사
				Search_Child(strBuf);//strBuf에 저장된 문자열을 기준으로 큐에 있는 노드의 자식들을 탐색함.
				printType = print_Value;//출력타입을 값으로 지정.
			}
			else ErrorCollection("/");
		}
		//cmd : [
		else if(cmdBuf[cmdIdx] == '[')
		{
			//cmd : [1
			if(checkNumber(cmdBuf[cmdIdx+1]))
			{
				cmdIdx = cmdIdx + 1; //cmd : [1   1로인덱스 이동
				NumberCpyFromCmdBuf(); //문자열중 연속된 숫자들만 strBuf로 복사
				RemoveBlank(cmdBuf, &cmdIdx);//혹시 모를 공백 제거

				//cmd : [1]
				if(cmdBuf[cmdIdx] == ']') //cmd : [1]
				{
					cmdIdx++;
					int selectCnt = atoi(strBuf); //strBuf에 있는 문자열 -> int형으로 변경. []안에 있는 숫자가 몇인지 저장.
					if((int)searchNodeQ.size() >= selectCnt)
					{
						while(--selectCnt) searchNodeQ.pop(); //[]안의 숫자만큼 큐안의 노드 제거
						searchNodeQ.push(searchNodeQ.front());//원하는 노드를 제일 뒤로 넣음
						while(searchNodeQ.size()-1) searchNodeQ.pop();//원하는 노드 빼고 전부 팝
						printType = print_Value;
					}
					else ErrorCollection("[999]");
				}
				else ErrorCollection("[999");
			}
			//cmd : [a
			else if(checkAlpha(cmdBuf[cmdIdx+1]))
			{
				//cmd : [func(
				if(checkAnyChar(&cmdBuf[cmdIdx+1], '(', ']'))
				{
					//cmd : func( 에서 (위치를 checkFunc에 기록.
					int checkFunc = checkAnyChar(&cmdBuf[cmdIdx+1], '(', ']');

					//cmd : [func()]
					if(cmdBuf[cmdIdx + 1 + checkFunc + 1] == ')')
					{
						cmdIdx = cmdIdx + 1; //cmd : [func( 에서 f위치로 이동.
						StrCpyFromCmdBuf(); //cmd : [func( 에서 f부터 알파벳,숫자,-_. 가 아닌곳 직전까지 복사. (앞까지 복사될것임.
						FuncCollection(strBuf);//위에서 복사한 문자열로 어떤 함수인지 처리.

						cmdBuf[cmdIdx] = '\0';
					}
					else ErrorCollection("[func(");
				}
				else ErrorCollection("[name~$");
			}
			//cmd : [@
			else if(cmdBuf[cmdIdx] == '@')
			{
				cmdBuf[cmdIdx] = '\0';
			}
			else ErrorCollection("[~");
		}
		//cmd가 /,[ 로 시작하지 않을때.
		else ErrorCollection("cmd");
	}

	//searchNodeQ에 있는 노드들 출력
	PrintNodeQ();

	delete tempNode;

	return 0;
}

//전체순회탐색인 Search_All을 호출하는 함수.
//Search_All을 바로 호출할 경우 큐에 불필요한 값이 저장되기에 이 함수를 거쳐서 호출해야함.
void XPath::Search_All_NonString(XMLNode* _XpathRoute)
{
	firstCallSearch_All = true;
	Search_All(_XpathRoute);
}

//전체순회탐색 함수. cmd의 문자열과 상관없이 _XpathRoute 이하의 모든 노드를 저장하는 함수.
void XPath::Search_All(XMLNode* _XpathRoute)
{
	//첫 호출인지 아닌지 검사
	if(!firstCallSearch_All) searchNodeQ.push(_XpathRoute);
	else firstCallSearch_All = false;

	std::list<XMLNode>::iterator _iter;
	for(_iter = _XpathRoute->getChildNode()->begin(); _iter != _XpathRoute->getChildNode()->end(); _iter++)
			Search_All(&(*_iter)); // 재귀 호출
}

//전체순회탐색 함수. 검사할 문자열인 str과 일치하는 노드들만 저장함.
void XPath::Search_All(XMLNode* _XpathRoute, const char* str, CommandType _commandType)
{
	std::list<XMLNode>::iterator _iter;
	std::list<XMLNode>::iterator _iter2;

	if(_commandType == search_TagName)
	{
		if(!strcmp(_XpathRoute->getName(), str)) searchNodeQ.push(_XpathRoute);
	}

	else if(_commandType == search_AttributeName)
	{
		for(_iter2 = _XpathRoute->getAttribute()->begin(); _iter2 != _XpathRoute->getAttribute()->end(); _iter2++)
		{
			if(!strcmp(_iter2->getName(), str)) searchNodeQ.push(&(*_iter2));
		}
	}

	for(_iter = _XpathRoute->getChildNode()->begin(); _iter != _XpathRoute->getChildNode()->end(); _iter++)
		Search_All(&(*_iter),str, _commandType);
}

//searchNodeQ 를 이용한 탐색
//문자열 str과 일치하는 자식 노드들만 큐에 저장함.
void XPath::Search_Child(const char* str)
{
	std::list<XMLNode>::iterator _iter;
	int tempCnt = searchNodeQ.size();

	while(tempCnt--)
	{
		for(_iter = searchNodeQ.front()->getChildNode()->begin(); _iter != searchNodeQ.front()->getChildNode()->end(); _iter++)
		{
			if(!strcmp(_iter->getName(),str)) searchNodeQ.push(&(*_iter));
		}
		searchNodeQ.pop();
	}
}

//printType
void XPath::PrintNodeQ()
{
	while(searchNodeQ.size())
	{
		searchNodeQ.front()->PrintNode(printType);
		searchNodeQ.pop();
	}
}
