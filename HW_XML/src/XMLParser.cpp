/*
 * XMLParser.cpp
 *
 *  Created on: 2014. 4. 2.
 *      Author: 신동환
 */

#include "XMLParser.h"

XMLParser::XMLParser() {
	tempElement			=	new wchar_t[MAX_CHAR_SIZE];
	tempAttributeName	=	new wchar_t[MAX_CHAR_SIZE];
	tempAttributeValue	=	new wchar_t[MAX_CHAR_SIZE];
	buf					= 	new wchar_t[MAX_BUF_SIZE];
	tempBuf				= 	new wchar_t[MAX_BUF_SIZE];

	idx 		= 	0;
	startIdx 	= 	0;
	endIdx 		= 	0;
	isRoute 	= 	true; //루트 노드인지 검사하기 위함
	isEmptyTag 	= 	false;//빈태그인지 검사하기 위함

	XpathRoute = new XMLNode;
}

XMLParser::~XMLParser() {
	delete[] tempElement;
	delete[] tempAttributeName;
	delete[] tempAttributeValue;
	delete[] buf;
	delete[] tempBuf;
}

int XMLParser::checkAnyChar(const wchar_t* str, const wchar_t _ch, const wchar_t _last)
{
	int _idx = 0;
	while (str[_idx] != _last)
	{
		if(str[_idx] == _ch) return _idx;
		_idx++;
	}

	if(_ch == L'>') return -1; // >태그가 등장하지 않았을 경우, 다음 라인을 추가로 읽어오기 위해 존재함.
	if(_ch == L'&') return -2; //&quot; 등을 검사하는 checkAmp() 함수에서 벗어나기 위해 존재함.
	if(_ch == L'"') return -3; // "기호가 아닌 '기호로 시작되는 attribute value를 검사하기 위함.

	return _idx;
}

//문자열str에 &quot;와 같은 Entity들을 검사하고 바꿔주는 함수.
//이 함수를 거치면 &quot; -> " 이런 형태로 문자가 바뀜.
wchar_t* XMLParser::checkAmp(wchar_t* str)
{
	int _idx = checkAnyChar(str, L'&',L'\0');
	if(_idx == -2) return str;

	int ampEnd;
	int charEnd;

	while(1)
	{
		//aaa&quot;bbb -> 기본 형태. &quot;을 예제로 설명.
		wchar_t* tempCheckAmp = new wchar_t[MAX_CHAR_SIZE];
		wchar_t* tempCheckAmp2 = new wchar_t[MAX_CHAR_SIZE];

		ampEnd	= checkAnyChar(str, L';', L'\0');	//;가 존재하는지 검사
		charEnd	= checkAnyChar(str, L'\0', L'\0');//문자열 끝나는 지점 검사

		StrCpyNumber(tempCheckAmp, &str[_idx+1], ampEnd -_idx - 1);//&quot; -> quot 만 따로 저장

		wcsncpy(tempCheckAmp2, &str[ampEnd+1], charEnd-ampEnd);  //bbb만 분리

		//문자열에서 &와 ;사이에 있던 문자열이 무엇인지 검사 후 기호로 변경
		if(!wcscmp(tempCheckAmp, L"lt"))		str[_idx] = L'<';
		else if(!wcscmp(tempCheckAmp, L"gt"))	str[_idx] = L'>';
		else if(!wcscmp(tempCheckAmp, L"amp"))	str[_idx] = L'&';
		else if(!wcscmp(tempCheckAmp, L"apos"))	str[_idx] = L'\'';
		else if(!wcscmp(tempCheckAmp, L"quot"))	str[_idx] = L'"';
		else
		{
			wprintf(L"존재하지 않는 Entity입니다.\n");
			wprintf(L"%s", str);
			break;
		}

		//기호로 변경한 문자 바로 뒤에 아까 분리한 bbb 부분을 이어 붙임
		wcsncpy(&str[_idx+1], tempCheckAmp2, charEnd-ampEnd);

		delete[] tempCheckAmp;
		delete[] tempCheckAmp2;

		//남은 문자열에서 &가 있는지 검사하고, 없을경우 break; 기호로 수정한 이후의 문자열을 검사하기 때문에 &amp; -> &변환에 대해 안전함
		if(checkAnyChar(&str[_idx+1], L'&', L'\0') == -2) break;
		_idx = _idx + checkAnyChar(&str[_idx+1], L'&', L'\0') + 1;//새로운 & 시작점을 찾아서 _idx에 저장
	}

	return str;
}

void XMLParser::parserPI()
{
	wprintf(L"Processing Instruction\n");
}

void XMLParser::parserDTD()
{
	if(tempBuf[1] == L'-' && tempBuf[2] == L'-') parserComment();
	else wprintf(L"DTD\n");
}

void XMLParser::parserComment()
{
	wprintf(L"Comment\n");
}

void XMLParser::parserStartTag()
{
	int blankNum = checkAnyChar(&tempBuf[0], L' ', L'\0');
	StrCpyNumber(tempElement, &tempBuf[0], blankNum);
	checkAmp(tempElement);

	isEmptyTag = false;
	if(tempBuf[endIdx-1] == '/') isEmptyTag = true;

	if(isRoute)
	{
		isRoute = false;
		XpathRoute->setParentNode(XpathRoute);
		XpathRoute->setName(tempElement);
	}
	else
	{
		XMLNode temp;
		XMLNode* currentNode = XpathRoute;
		temp.setName(tempElement);
		XpathRoute->setChildNode(&temp);
		XpathRoute = &XpathRoute->getChildNode()->back();
		XpathRoute->setParentNode(currentNode);
	}
	parserAttribute(blankNum); // attribute 처리

	if(isEmptyTag) XpathRoute = XpathRoute->getParentNode(); // 빈 태그일경우, 부모 노드로 바로 복귀시킴
}

void XMLParser::parserEndTag()
{
	XpathRoute = XpathRoute->getParentNode();
}

void XMLParser::parserContent()
{
	endIdx = checkAnyChar(&buf[idx], L'<', L'\0');
	StrCpyNumber(tempBuf, &buf[idx], endIdx);
	idx = idx + endIdx;
	XpathRoute->setValue(tempBuf);
}

void XMLParser::parserAttribute(int _blankNum)
{
	while(1)
	{
		RemoveBlank(tempBuf, &_blankNum);

		if(tempBuf[_blankNum] != L'\0')
		{
			XMLNode tempAttribute;
			int _startIdx = _blankNum;
			int _endIdx = checkAnyChar(&tempBuf[_startIdx], L'=', L'\0');

			StrCpyNumber(tempAttributeName, &tempBuf[_startIdx], _endIdx);
			checkAmp(tempAttributeName);
			tempAttribute.setName(tempAttributeName);

			_startIdx = _startIdx + _endIdx + 2;
			_endIdx = checkAnyChar(&tempBuf[_startIdx], L'"', L'\0');
			if(_endIdx == -3) _endIdx = checkAnyChar(&tempBuf[_startIdx], L'\'', L'\0');

			StrCpyNumber(tempAttributeValue, &tempBuf[_startIdx], _endIdx);
			checkAmp(tempAttributeValue);
			tempAttribute.setValue(tempAttributeValue);

			XpathRoute->setAttribute(&tempAttribute);
			XpathRoute->getAttribute()->back().setParentNode(XpathRoute);

			_blankNum = _startIdx + _endIdx + 1;
		}

		RemoveBlank(tempBuf, &_blankNum);

		if(tempBuf[_blankNum] == L'\0') break;
		else if(tempBuf[_blankNum] == L'/' && tempBuf[_blankNum+1] == L'\0') break; //빈 태그일 경우 멈추기 위함.
	}
}

int XMLParser::parser(const char* fileName, XMLNode* _XMLNode)
{
	XpathRoute = _XMLNode;

/*	FILE* inFile = _wfopen(fileName, L"r, ccs=UTF-8");

	if(!inFile)
	{
		wprintf(L"파일이 존재하지 않습니다.\n");
		return 1;
	}

	int index = 0;
	wchar_t c;

	while((fwscanf(inFile, L"%c", &c) == 1) && (c != L'\0'))
	{
		buf[index++] = c;
	}
*/
	wifstream fin(fileName);
	wofstream fout("jjj.xml");

	if(!fin)
	{
		wcout << fileName << L" 파일을 찾을수 없습니다.\n";
		return 1;
	}

	fin.getline(buf, MAX_BUF_SIZE);
	wcout << L"ddd " << buf << " fff" << endl;

	/*
	//byte order mark 검사==================
	fin.getline(buf, MAX_BUF_SIZE);
	if(checkByteOrderMark(buf, &idx))fin.seekg(0);
	//====================================

	while(fin.getline(buf, MAX_BUF_SIZE))
	{
		while(buf[idx] != L'\0')
		{
			RemoveBlank(buf, &idx);

			if(buf[idx] == L'<')
			{
				idx++;
				startIdx = idx;
				endIdx = checkAnyChar(&buf[startIdx], L'>', L'\0');
				while(endIdx == -1)
				{
					fin.getline(tempBuf, MAX_BUF_SIZE);
					wcscat(buf,tempBuf);
					endIdx = checkAnyChar(&buf[startIdx], L'>', L'\0');
				}
				StrCpyNumber(tempBuf, &buf[startIdx], endIdx);
				idx = endIdx + startIdx + 1; //다음 문자열로 idx를 이동시킴

				if		(tempBuf[0] == L'?')	parserPI();
				else if	(tempBuf[0] == L'!')	parserDTD(); // 주석은 parserDTD 함수 내에서 처리
				else if	(tempBuf[0] == L'/')	parserEndTag();
				else							parserStartTag();
			}
			else if(checkAlpha(buf[idx]) || checkNumber(buf[idx])) parserContent();
			else
			{
				parserContent();
				wprintf(L"알파벳과 숫자 이외의 문자가 입력되었습니다.\n");
				wprintf(L"%s\n", buf);
			}
		}

		idx = 0;
	}

	*/

	fin.close();

	return 0;
}
