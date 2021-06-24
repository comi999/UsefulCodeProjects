#include "Type.h"

std::map< unsigned int, const Type* > Type::m_TypesMap;
std::set< const Type*, Type > Type::m_TypesSet;