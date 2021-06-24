#include <iostream>
#include <thread>
#include "Callable.h"
#include "Type.h"

union Pointer
{
	int* intPointer;
	double* doublePounter;
	bool* boolPointer;
	std::string* stringPointer;
};

//enum Type
//{
//	INT, DOUBLE, BOOL, STRING
//};
//
//class BlackBoardEntry
//{
//	Type type;
//	Pointer pointer;
//
//	int* GetInt()
//	{
//		if ( type == Type::INT )
//		{
//			return pointer.intPointer;
//		}
//	}
//};

int main()
{
	return 0;
}