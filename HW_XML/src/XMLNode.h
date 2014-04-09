/*
 * XMLNode.h
 *
 *  Created on: 2014. 4. 2.
 *      Author: 신동환
 */
#include <iostream>
#include <list>
#include <string>

#ifndef XMLNODE_H_
#define XMLNODE_H_

#define MAX_CHAR_SIZE 500
#define MAX_BUF_SIZE 800

enum PrintType { print_Name, print_Value};

class XMLNode {
public:
	XMLNode();
	XMLNode(const XMLNode& node); //default copy constructor. list의 push 때문에 작성.
	virtual ~XMLNode();

private:
	char* name;
	char* value;
	XMLNode* parentNode;
	std::list<XMLNode> attribute;
	std::list<XMLNode> childNode;

public:
	char* getValue() {return value;}
	char* getName() {return name;}
	XMLNode* getParentNode() {return parentNode;}
	std::list<XMLNode>* getChildNode() {return &childNode;}
	std::list<XMLNode>* getAttribute() {return &attribute;}

	void setValue(const char* _value);
	void setName(const char* _name) {strcpy(name, _name);}
	void setParentNode(XMLNode* _parentNode) {parentNode = _parentNode;}
	void setChildNode(const XMLNode* _childNode) {childNode.push_back(*_childNode);}
	void setAttribute(const XMLNode* _attribute) {attribute.push_back(*_attribute);}

	void PrintNode(PrintType _type);
	void PrintName() {std::cout << name << std::endl;}
	void PrintValue() {std::cout << value << std::endl;}
};

#endif /* XMLNODE_H_ */
