//============================================================================
// Name        : HW_XML.cpp
// Author      :  신동환
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#include <iostream>
#include <locale>
#include "XMLNode.h"
#include "XMLParser.h"
#include "XPath.h"
using namespace std;

#define MAX_CHAR_SIZE 500
#define MAX_BUF_SIZE 800

int main()
{
	setlocale(LC_ALL,"");
	char* fileName	= 	new char[MAX_CHAR_SIZE];
	char* cmdBuf 	= 	new char[MAX_CHAR_SIZE];

	XPath Xpath;
	XMLParser XParser;
	XMLNode* XpathRoute = new XMLNode;

	while(1)
	{
		cout << "Input FileName(quit) : ";
		cin >> fileName;

		if(!strcmp(fileName, "quit"))
		{
			delete[] fileName;
			delete[] cmdBuf;
			delete XpathRoute;

			return 0;
		}
		else if(!XParser.parser(fileName, XpathRoute)) break;
	}

	while(1)
	{
		cout << "cmd : ";
		cin >> cmdBuf;

		if(!strcmp(cmdBuf, "quit")) break;
		else Xpath.XPathCmdParser(cmdBuf, XpathRoute);
	}

	delete[] fileName;
	delete[] cmdBuf;
	delete XpathRoute;

	return 0;
}
