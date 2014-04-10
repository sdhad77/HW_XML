/*
 * XString.h
 *
 *  Created on: 2014. 4. 9.
 *      Author: 신동환
 */
#include <iostream>
#include <string>

#ifndef XSTRING_H_
#define XSTRING_H_

#define MAX_CHAR_SIZE 500
#define MAX_BUF_SIZE 800

class XString {
public:
	XString();
	virtual ~XString();

public:
	bool checkAlpha(const wchar_t ch); //문자 ch가 알파벳인지
	bool checkNumber(const wchar_t ch);//문자 ch가 숫자인지
	bool checkTagName(const wchar_t ch);//문자 ch가 태그 이름으로 사용가능한지
	int checkAnyChar(const wchar_t* str, const wchar_t _ch, const wchar_t _last); //문자열 str에서 _ch문자 찾기. _last 문자가 등장하기 전까지 찾음.
	bool checkByteOrderMark(wchar_t* str, int* _idx);

	void StrCpyNumber(wchar_t* dest, const wchar_t* src, int _num);

	void RemoveBlank(wchar_t* _str, int* _idx); //공백제거
};

#endif /* XSTRING_H_ */
