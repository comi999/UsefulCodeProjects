#pragma once
#include <iostream>
#include <type_traits>
#include <typeinfo>
#include <map>
#include "Type.h"

template < typename Base >
class TypeTree
{
	using base_type = typename std::remove_cv< typename std::clean_type< Base >::type >::type;
	static_assert( std::has_virtual_destructor< base_type >::value, "Type 'Base' must have a virtual deconstructor." );

	struct Node
	{
		typedef bool( *is_base_of )( base_type* a_Value );
		is_base_of IsBaseOf;

		std::list< base_type* > m_Values;
		unsigned int m_TypeHash;

		Node* m_Base;
		std::list< Node* > m_Derived;

		template < typename Type = base_type >
		static Node* Create()
		{
			Node* newNode = new Node();

			newNode->IsBaseOf = []( base_type* a_Value ) -> bool
			{
				return dynamic_cast< Type* >( a_Value ) != nullptr;
			};

			newNode->m_TypeHash = typeid( Type ).hash_code();
			newNode->m_Base = nullptr;

			return newNode;
		}

		template < typename Lambda >
		bool operator> ( const Lambda& a_RHS )
		{
			return false;
		}

	};

	struct Entry
	{
	public:

		template < typename Lambda, typename... Args >
		Entry* Create( Args... a_Args )
		{
			Entry* newEntry = new EntryValue< Lambda >( a_Args... );
			newEntry->m_Type = &Type::GetType< Lambda >();

			return newEntry;
		}

		template < typename Lambda >
		Lambda* GetValue()
		{
			return static_cast< EntryValue< Lambda >* >( this )->value;
		}

		const Type& GetType()
		{
			return *m_Type;
		}

	private:

		const Type* m_Type;
	};

	template < typename Lambda >
	struct EntryValue : public Entry
	{
	public:

		template < typename... Args >
		EntryValue( Args... args )
		{
			m_Value = new Type( args... );
		}

	private:

		Lambda* m_Value;
	};

public:

	#pragma region Emplace
	///////////////////////////

	template < typename Type >
	typename std::enable_if< std::is_base_of< base_type, Type >::value, Type* >::type
	Emplace()
	{
		std::cout << "inserted 1 of type: " << typeid( Type ).name() << std::endl;
	}

	template < typename Type >
	typename std::enable_if< std::is_base_of< base_type, Type >::value, std::vector< Type* > >::type
	Emplace( int a_Count )
	{
		std::cout << "inserted " << a_Count << " of type: " << typeid( Type ).name() << std::endl;
	}

	///////////////////////////
	#pragma endregion

	#pragma region InsertObject
	///////////////////////////

	#pragma region PureType
	///////////////////////////

	// const* - Derived
	template < typename Type >
	typename std::enable_if< std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const Type* a_Value )
	{
		std::cout << "not taking ownership of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// const* array - Derived
	template < typename Type >
	typename std::enable_if< std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const Type* a_Values, int a_Count )
	{
		std::cout << "not taking ownership of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// borrowed * - Accepted
	template < typename Type, bool a_TakeOwnership = true >
	typename std::enable_if< std::is_base_of< base_type, Type >::value && !a_TakeOwnership, void >::
	type InsertObject( Type* a_Value )
	{
		std::cout << "not taking ownership of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// owned * - Accepted
	template < typename Type, bool a_TakeOwnership = true >
	typename std::enable_if< std::is_base_of< base_type, Type >::value && a_TakeOwnership, void >::
	type InsertObject( Type* a_Value )
	{
		std::cout << "taking ownership of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// borrowed * array - Accepted
	template < typename Type, bool a_TakeOwnership = true >
	typename std::enable_if< std::is_base_of< base_type, Type >::value && !a_TakeOwnership, void >::
	type InsertObject( Type* a_Values, int a_Count )
	{
		std::cout << "not taking ownership of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// owned * array - Accepted
	template < typename Type, bool a_TakeOwnership = true >
	typename std::enable_if< std::is_base_of< base_type, Type >::value&& a_TakeOwnership, void >::
	type InsertObject( Type* a_Values, int a_Count )
	{
		std::cout << "taking ownership of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// * - Rejected
	template < typename Type >
	typename std::enable_if< !std::is_base_of< base_type, Type >::value >::
	type InsertObject( Type* a_Value )
	{
		std::cout << "didn't insert pointer of type: " << typeid( Type ).name() << std::endl;
	}

	// * array - Rejected
	template < typename Type >
	typename std::enable_if< !std::is_base_of< base_type, Type >::value >::
	type InsertObject( Type* a_Values, int a_Count )
	{
		std::cout << "didn't insert pointer of type: " << typeid( Type ).name() << std::endl;
	}

	// & - Accepted
	template < typename Type >
	typename std::enable_if< std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const Type& a_Value )
	{
		std::cout << "emplaced type: " << typeid( Type ).name() << std::endl;
	}

	// & - Rejected
	template < typename Type >
	typename std::enable_if< !std::is_base_of< base_type, Type >::value >::
	type InsertObject( const Type& a_Value )
	{
		std::cout << "didn't emplace type: " << typeid( Type ).name() << std::endl;
		#pragma message( "Attempted to insert incorrect type." )
	}

	///////////////////////////
	#pragma endregion

	#pragma region MultiType
	///////////////////////////

	// Empty tuple*
	void InsertObject( std::tuple<>* a_Tuple )
	{

	}

	// Empty tuple
	void InsertObject( const std::tuple<>& a_Tuple )
	{
		std::cout << "didn't emplace empty tuple" << std::endl;
	}

	// Filled tuple*
	template < typename... Types >
	void InsertObject( std::tuple< Types... >* a_Tuple )
	{
		InsertObject( a_Tuple->_Myfirst._Val );
		InsertObject( a_Tuple->_Get_rest() );
	}

	// Filled tuple
	template < typename... Types >
	void InsertObject( const std::tuple< Types... >& a_Tuple )
	{
		InsertObject( a_Tuple._Myfirst._Val );
		InsertObject( a_Tuple._Get_rest() );
	}

	// Filled Pair*
	template < typename... Types >
	void InsertObject( std::pair< Types... >* a_Pair )
	{
		InsertObject( a_Pair->first );
		InsertObject( a_Pair->second );
	}

	// Filled Pair
	template < typename... Types >
	void InsertObject( const std::pair< Types... >& a_Pair )
	{
		InsertObject( a_Pair.first );
		InsertObject( a_Pair.second );
	}

	///////////////////////////
	#pragma endregion

	#pragma region Array
	///////////////////////////

	// & Array* - Pair, Tuple, or Container - Accepted
	template < typename Type, int Count >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::array< Type, Count >* a_Array )
	{

	}

	// & Array* - Not Pair, Tuple or Container - Accepted
	template < typename Type, int Count >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::array< Type, Count >* a_Array )
	{
		std::cout << "inserted array pointer of reference type: " << typeid( Type ).name() << "  x" << Count << std::endl;
	}

	// & Array* - Not Pair, Tuple or Container - Rejected
	template < typename Type, int Count >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::array< Type, Count >* a_Array )
	{
		std::cout << "didn't inserted array pointer of reference type: " << typeid( Type ).name() << "  x" << Count << std::endl;
	}

	// & Array& - Pair, Tuple, or Container - Accepted
	template < typename Type, int Count >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::array< Type, Count >& a_Array )
	{

	}

	// & Array& - Not Pair, Tuple or Container - Accepted
	template < typename Type, int Count >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::array< Type, Count >& a_Array )
	{
		std::cout << "inserted array reference of reference type: " << typeid( Type ).name() << "  x" << Count << std::endl;
	}

	// & Array& - Not Pair, Tuple or Container - Rejected
	template < typename Type, int Count >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::array< Type, Count >& a_Array )
	{
		std::cout << "didn't inserted array reference of reference type: " << typeid( Type ).name() << "  x" << Count << std::endl;
	}

	// const* Array* - Pair, Tuple, or Container - Accepted
	template < typename Type, int Count >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::array< const Type*, Count >* a_Array )
	{

	}

	// const* Array* - Not Pair, Tuple or Container - Accepted
	template < typename Type, int Count >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::array< const Type*, Count >* a_Array )
	{
		std::cout << "inserted array pointer of const pointer type: " << typeid( Type ).name() << "  x" << Count << std::endl;
	}

	// const* Array* - Not Pair, Tuple or Container - Rejected
	template < typename Type, int Count >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::array< const Type*, Count >* a_Array )
	{
		std::cout << "didn't inserted array pointer of const pointer type: " << typeid( Type ).name() << "  x" << Count << std::endl;
	}

	// * Array* - Pair, Tuple, or Container - Accepted
	template < typename Type, int Count >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::array< Type*, Count >* a_Array )
	{

	}

	// * Array* - Not Pair, Tuple or Container - Accepted
	template < typename Type, int Count >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::array< Type*, Count >* a_Array )
	{
		std::cout << "inserted array point of point type: " << typeid( Type ).name() << "  x" << Count << std::endl;
	}

	// * Array* - Not Pair, Tuple or Container - Rejected
	template < typename Type, int Count >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::array< Type*, Count >* a_Array )
	{
		std::cout << "didn't inserted array pointer of pointer type: " << typeid( Type ).name() << "  x" << Count << std::endl;
	}

	// const* Array& - Pair, Tuple, or Container - Accepted
	template < typename Type, int Count >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::array< const Type*, Count >& a_Array )
	{

	}

	// const* Array& - Not Pair, Tuple or Container - Accepted
	template < typename Type, int Count >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::array< const Type*, Count >& a_Array )
	{
		std::cout << "inserted array reference of reference type: " << typeid( Type ).name() << "  x" << Count << std::endl;
	}

	// const* Array& - Not Pair, Tuple or Container - Rejected
	template < typename Type, int Count >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::array< const Type*, Count >& a_Array )
	{
		std::cout << "didn't inserted array reference of reference type: " << typeid( Type ).name() << "  x" << Count << std::endl;
	}

	// * Array& - Pair, Tuple, or Container - Accepted
	template < typename Type, int Count >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::array< Type*, Count >& a_Array )
	{

	}

	// * Array& - Not Pair, Tuple or Container - Accepted
	template < typename Type, int Count >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::array< Type*, Count >& a_Array )
	{
		std::cout << "inserted array reference of reference type: " << typeid( Type ).name() << "  x" << Count << std::endl;
	}

	// * Array& - Not Pair, Tuple or Container - Rejected
	template < typename Type, int Count >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::array< Type*, Count >& a_Array )
	{
		std::cout << "didn't inserted array reference of reference type: " << typeid( Type ).name() << "  x" << Count << std::endl;
	}

	///////////////////////////
	#pragma endregion

	#pragma region Vector
	///////////////////////////

	// & Vector* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::vector< Type >* a_Vector )
	{

	}

	// & Vector* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::vector< Type >* a_Vector )
	{
		std::cout << "inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & Vector* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::vector< Type >* a_Vector )
	{
		std::cout << "didn't inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & Vector& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::vector< Type >& a_Vector )
	{

	}

	// & Vector& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::vector< Type >& a_Vector )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & Vector& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::vector< Type >& a_Vector )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* Vector* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::vector< const Type* >* a_Vector )
	{

	}

	// const* Vector* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::vector< const Type* >* a_Vector )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// const* Vector* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::vector< const Type* >* a_Vector )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// * Vector* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::vector< Type* >* a_Vector )
	{

	}

	// * Vector* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::vector< Type* >* a_Vector )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// * Vector* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::vector< Type* >* a_Vector )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// const* Vector& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::vector< const Type* >& a_Vector )
	{

	}

	// const* Vector& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::vector< const Type* >& a_Vector )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* Vector& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::vector< const Type* >& a_Vector )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * Vector& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::vector< Type* >& a_Vector )
	{

	}

	// * Vector& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::vector< Type* >& a_Vector )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * Vector& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::vector< Type* >& a_Vector )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	///////////////////////////
	#pragma endregion

	#pragma region List
	///////////////////////////

	// & List* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::list< Type >* a_List )
	{

	}

	// & List* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::list< Type >* a_List )
	{
		std::cout << "inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & List* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::list< Type >* a_List )
	{
		std::cout << "didn't inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & List& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::list< Type >& a_List )
	{

	}

	// & List& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::list< Type >& a_List )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & List& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::list< Type >& a_List )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* List* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::list< const Type* >* a_List )
	{

	}

	// const* List* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::list< const Type* >* a_List )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// const* List* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::list< const Type* >* a_List )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// * List* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::list< Type* >* a_List )
	{

	}

	// * List* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::list< Type* >* a_List )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// * List* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::list< Type* >* a_List )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// const* List& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::list< const Type* >& a_List )
	{

	}

	// const* List& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::list< const Type* >& a_List )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* List& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::list< const Type* >& a_List )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * List& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::list< Type* >& a_List )
	{

	}

	// * List& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::list< Type* >& a_List )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * List& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::list< Type* >& a_List )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	///////////////////////////
	#pragma endregion

	#pragma region ForwardList
	///////////////////////////

	// & ForwardList* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::forward_list< Type >* a_ForwardList )
	{

	}

	// & ForwardList* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::forward_list< Type >* a_ForwardList )
	{
		std::cout << "inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & ForwardList* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::forward_list< Type >* a_ForwardList )
	{
		std::cout << "didn't inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & ForwardList& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::forward_list< Type >& a_ForwardList )
	{

	}

	// & ForwardList& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::forward_list< Type >& a_ForwardList )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & ForwardList& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::forward_list< Type >& a_ForwardList )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* ForwardList* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::forward_list< const Type* >* a_ForwardList )
	{

	}

	// const* ForwardList* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::forward_list< const Type* >* a_ForwardList )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// const* ForwardList* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::forward_list< const Type* >* a_ForwardList )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// * ForwardList* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::forward_list< Type* >* a_ForwardList )
	{

	}

	// * ForwardList* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::forward_list< Type* >* a_ForwardList )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// * ForwardList* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::forward_list< Type* >* a_ForwardList )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// const* ForwardList& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::forward_list< const Type* >& a_ForwardList )
	{

	}

	// const* ForwardList& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::forward_list< const Type* >& a_ForwardList )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* ForwardList& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::forward_list< const Type* >& a_ForwardList )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * ForwardList& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::forward_list< Type* >& a_ForwardList )
	{

	}

	// * ForwardList& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::forward_list< Type* >& a_ForwardList )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * ForwardList& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::forward_list< Type* >& a_ForwardList )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	///////////////////////////
	#pragma endregion

	#pragma region Set
	///////////////////////////

	// & Set* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::set< Type >* a_Set )
	{

	}

	// & Set* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::set< Type >* a_Set )
	{
		std::cout << "inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & Set* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::set< Type >* a_Set )
	{
		std::cout << "didn't inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & Set& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::set< Type >& a_Set )
	{

	}

	// & Set& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::set< Type >& a_Set )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & Set& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::set< Type >& a_Set )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* Set* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::set< const Type* >* a_Set )
	{

	}

	// const* Set* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::set< const Type* >* a_Set )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// const* Set* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::set< const Type* >* a_Set )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// * Set* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::set< Type* >* a_Set )
	{

	}

	// * Set* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::set< Type* >* a_Set )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// * Set* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::set< Type* >* a_Set )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// const* Set& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::set< const Type* >& a_Set )
	{

	}

	// const* Set& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::set< const Type* >& a_Set )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* Set& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::set< const Type* >& a_Set )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * Set& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::set< Type* >& a_Set )
	{

	}

	// * Set& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::set< Type* >& a_Set )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * Set& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::set< Type* >& a_Set )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	///////////////////////////
	#pragma endregion

	#pragma region UnorderedSet
	///////////////////////////

	// & UnorderedSet* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::unordered_set< Type >* a_UnorderedSet )
	{

	}

	// & UnorderedSet* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::unordered_set< Type >* a_UnorderedSet )
	{
		std::cout << "inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & UnorderedSet* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::unordered_set< Type >* a_UnorderedSet )
	{
		std::cout << "didn't inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & UnorderedSet& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::unordered_set< Type >& a_UnorderedSet )
	{

	}

	// & UnorderedSet& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::unordered_set< Type >& a_UnorderedSet )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & UnorderedSet& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::unordered_set< Type >& a_UnorderedSet )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* UnorderedSet* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::unordered_set< const Type* >* a_UnorderedSet )
	{

	}

	// const* UnorderedSet* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::unordered_set< const Type* >* a_UnorderedSet )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// const* UnorderedSet* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::unordered_set< const Type* >* a_UnorderedSet )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// * UnorderedSet* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::unordered_set< Type* >* a_UnorderedSet )
	{

	}

	// * UnorderedSet* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::unordered_set< Type* >* a_UnorderedSet )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// * UnorderedSet* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::unordered_set< Type* >* a_UnorderedSet )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// const* UnorderedSet& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::unordered_set< const Type* >& a_UnorderedSet )
	{

	}

	// const* UnorderedSet& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::unordered_set< const Type* >& a_UnorderedSet )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* UnorderedSet& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::unordered_set< const Type* >& a_UnorderedSet )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * UnorderedSet& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::unordered_set< Type* >& a_UnorderedSet )
	{

	}

	// * UnorderedSet& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::unordered_set< Type* >& a_UnorderedSet )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * UnorderedSet& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::unordered_set< Type* >& a_UnorderedSet )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	///////////////////////////
	#pragma endregion

	#pragma region MultiSet
	///////////////////////////

	// & MultiSet* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::multiset< Type >* a_MultiSet )
	{

	}

	// & MultiSet* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::multiset< Type >* a_MultiSet )
	{
		std::cout << "inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & MultiSet* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
		type InsertObject( std::multiset< Type >* a_MultiSet )
	{
		std::cout << "didn't inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & MultiSet& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::multiset< Type >& a_MultiSet )
	{

	}

	// & MultiSet& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::multiset< Type >& a_MultiSet )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & MultiSet& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::multiset< Type >& a_MultiSet )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* MultiSet* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::multiset< const Type* >* a_MultiSet )
	{

	}

	// const* MultiSet* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::multiset< const Type* >* a_MultiSet )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// const* MultiSet* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::multiset< const Type* >* a_MultiSet )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// * MultiSet* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::multiset< Type* >* a_MultiSet )
	{

	}

	// * MultiSet* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::multiset< Type* >* a_MultiSet )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// * MultiSet* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::multiset< Type* >* a_MultiSet )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// const* MultiSet& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::multiset< const Type* >& a_MultiSet )
	{

	}

	// const* MultiSet& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::multiset< const Type* >& a_MultiSet )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* MultiSet& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::multiset< const Type* >& a_MultiSet )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * MultiSet& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::multiset< Type* >& a_MultiSet )
	{

	}

	// * MultiSet& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::multiset< Type* >& a_MultiSet )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * MultiSet& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::multiset< Type* >& a_MultiSet )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	///////////////////////////
	#pragma endregion

	#pragma region UnorderedMultiSet
	///////////////////////////

	// & UnorderedMultiSet* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::unordered_multiset< Type >* a_UnorderedMultiSet )
	{

	}

	// & UnorderedMultiSet* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::unordered_multiset< Type >* a_UnorderedMultiSet )
	{
		std::cout << "inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & UnorderedMultiSet* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::unordered_multiset< Type >* a_UnorderedMultiSet )
	{
		std::cout << "didn't inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & UnorderedMultiSet& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::unordered_multiset< Type >& a_UnorderedMultiSet )
	{

	}

	// & UnorderedMultiSet& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::unordered_multiset< Type >& a_UnorderedMultiSet )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & UnorderedMultiSet& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::unordered_multiset< Type >& a_UnorderedMultiSet )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* UnorderedMultiSet* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::unordered_multiset< const Type* >* a_UnorderedMultiSet )
	{

	}

	// const* UnorderedMultiSet* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::unordered_multiset< const Type* >* a_UnorderedMultiSet )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// const* UnorderedMultiSet* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::unordered_multiset< const Type* >* a_UnorderedMultiSet )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// * UnorderedMultiSet* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::unordered_multiset< Type* >* a_UnorderedMultiSet )
	{

	}

	// * UnorderedMultiSet* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::unordered_multiset< Type* >* a_UnorderedMultiSet )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// * UnorderedMultiSet* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::unordered_multiset< Type* >* a_UnorderedMultiSet )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// const* UnorderedMultiSet& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::unordered_multiset< const Type* >& a_UnorderedMultiSet )
	{

	}

	// const* UnorderedMultiSet& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::unordered_multiset< const Type* >& a_UnorderedMultiSet )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* UnorderedMultiSet& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::unordered_multiset< const Type* >& a_UnorderedMultiSet )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * UnorderedMultiSet& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::unordered_multiset< Type* >& a_UnorderedMultiSet )
	{

	}

	// * UnorderedMultiSet& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::unordered_multiset< Type* >& a_UnorderedMultiSet )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * UnorderedMultiSet& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::unordered_multiset< Type* >& a_UnorderedMultiSet )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	///////////////////////////
	#pragma endregion

	#pragma region Stack
	///////////////////////////

	// & Stack* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::stack< Type >* a_Stack )
	{

	}

	// & Stack* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::stack< Type >* a_Stack )
	{
		std::cout << "inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & Stack* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::stack< Type >* a_Stack )
	{
		std::cout << "didn't inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & Stack& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::stack< Type >& a_Stack )
	{

	}

	// & Stack& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::stack< Type >& a_Stack )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & Stack& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::stack< Type >& a_Stack )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* Stack* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::stack< const Type* >* a_Stack )
	{

	}

	// const* Stack* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::stack< const Type* >* a_Stack )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// const* Stack* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::stack< const Type* >* a_Stack )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// * Stack* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::stack< Type* >* a_Stack )
	{

	}

	// * Stack* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::stack< Type* >* a_Stack )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// * Stack* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::stack< Type* >* a_Stack )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// const* Stack& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::stack< const Type* >& a_Stack )
	{

	}

	// const* Stack& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::stack< const Type* >& a_Stack )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* Stack& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::stack< const Type* >& a_Stack )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * Stack& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::stack< Type* >& a_Stack )
	{

	}

	// * Stack& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::stack< Type* >& a_Stack )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * Stack& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::stack< Type* >& a_Stack )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	///////////////////////////
	#pragma endregion

	#pragma region Deque
	///////////////////////////

	// & Deque* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::deque< Type >* a_Deque )
	{

	}

	// & Deque* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::deque< Type >* a_Deque )
	{
		std::cout << "inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & Deque* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::deque< Type >* a_Deque )
	{
		std::cout << "didn't inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & Deque& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::deque< Type >& a_Deque )
	{

	}

	// & Deque& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::deque< Type >& a_Deque )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & Deque& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::deque< Type >& a_Deque )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* Deque* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::deque< const Type* >* a_Deque )
	{

	}

	// const* Deque* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::deque< const Type* >* a_Deque )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// const* Deque* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::deque< const Type* >* a_Deque )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// * Deque* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::deque< Type* >* a_Deque )
	{

	}

	// * Deque* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::deque< Type* >* a_Deque )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// * Deque* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::deque< Type* >* a_Deque )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// const* Deque& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::deque< const Type* >& a_Deque )
	{

	}

	// const* Deque& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::deque< const Type* >& a_Deque )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* Deque& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::deque< const Type* >& a_Deque )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * Deque& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::deque< Type* >& a_Deque )
	{

	}

	// * Deque& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::deque< Type* >& a_Deque )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * Deque& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::deque< Type* >& a_Deque )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	///////////////////////////
	#pragma endregion

	#pragma region Queue
	///////////////////////////

	// & Queue* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::queue< Type >* a_Queue )
	{

	}

	// & Queue* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::queue< Type >* a_Queue )
	{
		std::cout << "inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & Queue* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::queue< Type >* a_Queue )
	{
		std::cout << "didn't inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & Queue& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::queue< Type >& a_Queue )
	{

	}

	// & Queue& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::queue< Type >& a_Queue )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & Queue& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::queue< Type >& a_Queue )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* Queue* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::queue< const Type* >* a_Queue )
	{

	}

	// const* Queue* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::queue< const Type* >* a_Queue )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// const* Queue* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::queue< const Type* >* a_Queue )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// * Queue* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::queue< Type* >* a_Queue )
	{

	}

	// * Queue* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::queue< Type* >* a_Queue )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// * Queue* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::queue< Type* >* a_Queue )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// const* Queue& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::queue< const Type* >& a_Queue )
	{

	}

	// const* Queue& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::queue< const Type* >& a_Queue )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* Queue& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::queue< const Type* >& a_Queue )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * Queue& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::queue< Type* >& a_Queue )
	{

	}

	// * Queue& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::queue< Type* >& a_Queue )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * Queue& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::queue< Type* >& a_Queue )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	///////////////////////////
	#pragma endregion

	#pragma region PriorityQueue
	///////////////////////////

	// & PriorityQueue* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::priority_queue< Type >* a_PriorityQueue )
	{

	}

	// & PriorityQueue* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::priority_queue< Type >* a_PriorityQueue )
	{
		std::cout << "inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & PriorityQueue* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::priority_queue< Type >* a_PriorityQueue )
	{
		std::cout << "didn't inserted vector pointer of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & PriorityQueue& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::priority_queue< Type >& a_PriorityQueue )
	{

	}

	// & PriorityQueue& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::priority_queue< Type >& a_PriorityQueue )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// & PriorityQueue& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::priority_queue< Type >& a_PriorityQueue )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* PriorityQueue* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::priority_queue< const Type* >* a_PriorityQueue )
	{

	}

	// const* PriorityQueue* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::priority_queue< const Type* >* a_PriorityQueue )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// const* PriorityQueue* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::priority_queue< const Type* >* a_PriorityQueue )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// * PriorityQueue* - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( std::priority_queue< Type* >* a_PriorityQueue )
	{

	}

	// * PriorityQueue* - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::priority_queue< Type* >* a_PriorityQueue )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type ).name() << std::endl;
	}

	// * PriorityQueue* - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( std::priority_queue< Type* >* a_PriorityQueue )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type ).name() << std::endl;
	}

	// const* PriorityQueue& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::priority_queue< const Type* >& a_PriorityQueue )
	{

	}

	// const* PriorityQueue& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::priority_queue< const Type* >& a_PriorityQueue )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// const* PriorityQueue& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::priority_queue< const Type* >& a_PriorityQueue )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * PriorityQueue& - Pair, Tuple, or Container - Accepted
	template < typename Type >
	typename std::enable_if< std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value, void >::
	type InsertObject( const std::priority_queue< Type* >& a_PriorityQueue )
	{

	}

	// * PriorityQueue& - Not Pair, Tuple or Container - Accepted
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::priority_queue< Type* >& a_PriorityQueue )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	// * PriorityQueue& - Not Pair, Tuple or Container - Rejected
	template < typename Type >
	typename std::enable_if< !( std::is_pair< Type >::value || std::is_tuple< Type >::value || std::is_stl_container< Type >::value ) && !std::is_base_of< base_type, Type >::value, void >::
	type InsertObject( const std::priority_queue< Type* >& a_PriorityQueue )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type ).name() << std::endl;
	}

	///////////////////////////
	#pragma endregion

	#pragma region Map
	///////////////////////////

	// & Map* - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( std::map< Type1, Type2 >* a_Map )
	{

	}

	// & Map* - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::map< Type1, Type2 >* a_Map )
	{
		std::cout << "inserted vector pointer of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// & Map* - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::map< Type1, Type2 >* a_Map )
	{
		std::cout << "didn't inserted vector pointer of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// & Map& - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( const std::map< Type1, Type2 >& a_Map )
	{

	}

	// & Map& - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::map< Type1, Type2 >& a_Map )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// & Map& - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::map< Type1, Type2 >& a_Map )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// const* Map* - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( std::map< Type1, const Type2* >* a_Map )
	{

	}

	// const* Map* - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::map< Type1, const Type2* >* a_Map )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type2 ).name() << std::endl;
	}

	// const* Map* - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::map< Type1, const Type2* >* a_Map )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type2 ).name() << std::endl;
	}

	// * Map* - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( std::map< Type1, Type2* >* a_Map )
	{

	}

	// * Map* - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::map< Type1, Type2* >* a_Map )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type2 ).name() << std::endl;
	}

	// * Map* - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::map< Type1, Type2* >* a_Map )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type2 ).name() << std::endl;
	}

	// const* Map& - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( const std::map< Type1, const Type2* >& a_Map )
	{

	}

	// const* Map& - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::map< Type1, const Type2* >& a_Map )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// const* Map& - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::map< Type1, const Type2* >& a_Map )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// * Map& - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( const std::map< Type1, Type2* >& a_Map )
	{

	}

	// * Map& - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::map< Type1, Type2* >& a_Map )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// * Map& - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::map< Type1, Type2* >& a_Map )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	///////////////////////////
	#pragma endregion

	#pragma region UnorderedMap
	///////////////////////////

	// & UnorderedMap* - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( std::unordered_map< Type1, Type2 >* a_UnorderedMap )
	{

	}

	// & UnorderedMap* - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::unordered_map< Type1, Type2 >* a_UnorderedMap )
	{
		std::cout << "inserted vector pointer of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// & UnorderedMap* - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::unordered_map< Type1, Type2 >* a_UnorderedMap )
	{
		std::cout << "didn't inserted vector pointer of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// & UnorderedMap& - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( const std::unordered_map< Type1, Type2 >& a_UnorderedMap )
	{

	}

	// & UnorderedMap& - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::unordered_map< Type1, Type2 >& a_UnorderedMap )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// & UnorderedMap& - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::unordered_map< Type1, Type2 >& a_UnorderedMap )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// const* UnorderedMap* - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( std::unordered_map< Type1, const Type2* >* a_UnorderedMap )
	{

	}

	// const* UnorderedMap* - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::unordered_map< Type1, const Type2* >* a_UnorderedMap )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type2 ).name() << std::endl;
	}

	// const* UnorderedMap* - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::unordered_map< Type1, const Type2* >* a_UnorderedMap )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type2 ).name() << std::endl;
	}

	// * UnorderedMap* - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( std::unordered_map< Type1, Type2* >* a_UnorderedMap )
	{

	}

	// * UnorderedMap* - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::unordered_map< Type1, Type2* >* a_UnorderedMap )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type2 ).name() << std::endl;
	}

	// * UnorderedMap* - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::unordered_map< Type1, Type2* >* a_UnorderedMap )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type2 ).name() << std::endl;
	}

	// const* UnorderedMap& - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( const std::unordered_map< Type1, const Type2* >& a_UnorderedMap )
	{

	}

	// const* UnorderedMap& - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::unordered_map< Type1, const Type2* >& a_UnorderedMap )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// const* UnorderedMap& - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::unordered_map< Type1, const Type2* >& a_UnorderedMap )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// * UnorderedMap& - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( const std::unordered_map< Type1, Type2* >& a_UnorderedMap )
	{

	}

	// * UnorderedMap& - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::unordered_map< Type1, Type2* >& a_UnorderedMap )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// * UnorderedMap& - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::unordered_map< Type1, Type2* >& a_UnorderedMap )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	///////////////////////////
	#pragma endregion

	#pragma region MultiMap
	///////////////////////////

	// & MultiMap* - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( std::multimap< Type1, Type2 >* a_MultiMap )
	{

	}

	// & MultiMap* - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::multimap< Type1, Type2 >* a_MultiMap )
	{
		std::cout << "inserted vector pointer of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// & MultiMap* - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::multimap< Type1, Type2 >* a_MultiMap )
	{
		std::cout << "didn't inserted vector pointer of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// & MultiMap& - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( const std::multimap< Type1, Type2 >& a_MultiMap )
	{

	}

	// & MultiMap& - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::multimap< Type1, Type2 >& a_MultiMap )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// & MultiMap& - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::multimap< Type1, Type2 >& a_MultiMap )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// const* MultiMap* - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( std::multimap< Type1, const Type2* >* a_MultiMap )
	{

	}

	// const* MultiMap* - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::multimap< Type1, const Type2* >* a_MultiMap )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type2 ).name() << std::endl;
	}

	// const* MultiMap* - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::multimap< Type1, const Type2* >* a_MultiMap )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type2 ).name() << std::endl;
	}

	// * MultiMap* - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( std::multimap< Type1, Type2* >* a_MultiMap )
	{

	}

	// * MultiMap* - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::multimap< Type1, Type2* >* a_MultiMap )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type2 ).name() << std::endl;
	}

	// * MultiMap* - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::multimap< Type1, Type2* >* a_MultiMap )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type2 ).name() << std::endl;
	}

	// const* MultiMap& - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( const std::multimap< Type1, const Type2* >& a_MultiMap )
	{

	}

	// const* MultiMap& - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::multimap< Type1, const Type2* >& a_MultiMap )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// const* MultiMap& - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::multimap< Type1, const Type2* >& a_MultiMap )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// * MultiMap& - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( const std::multimap< Type1, Type2* >& a_MultiMap )
	{

	}

	// * MultiMap& - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::multimap< Type1, Type2* >& a_MultiMap )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// * MultiMap& - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::multimap< Type1, Type2* >& a_MultiMap )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	///////////////////////////
	#pragma endregion

	#pragma region UnorderedMultiMap
	///////////////////////////

	// & UnorderedMultiMap* - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( std::unordered_multimap< Type1, Type2 >* a_UnorderedMultiMap )
	{

	}

	// & UnorderedMultiMap* - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::unordered_multimap< Type1, Type2 >* a_UnorderedMultiMap )
	{
		std::cout << "inserted vector pointer of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// & UnorderedMultiMap* - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::unordered_multimap< Type1, Type2 >* a_UnorderedMultiMap )
	{
		std::cout << "didn't inserted vector pointer of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// & UnorderedMultiMap& - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( const std::unordered_multimap< Type1, Type2 >& a_UnorderedMultiMap )
	{

	}

	// & UnorderedMultiMap& - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::unordered_multimap< Type1, Type2 >& a_UnorderedMultiMap )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// & UnorderedMultiMap& - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::unordered_multimap< Type1, Type2 >& a_UnorderedMultiMap )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// const* UnorderedMultiMap* - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( std::unordered_multimap< Type1, const Type2* >* a_UnorderedMultiMap )
	{

	}

	// const* UnorderedMultiMap* - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::unordered_multimap< Type1, const Type2* >* a_UnorderedMultiMap )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type2 ).name() << std::endl;
	}

	// const* UnorderedMultiMap* - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::unordered_multimap< Type1, const Type2* >* a_UnorderedMultiMap )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type2 ).name() << std::endl;
	}

	// * UnorderedMultiMap* - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( std::unordered_multimap< Type1, Type2* >* a_UnorderedMultiMap )
	{

	}

	// * UnorderedMultiMap* - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::unordered_multimap< Type1, Type2* >* a_UnorderedMultiMap )
	{
		std::cout << "inserted vector point of point type: " << typeid( Type2 ).name() << std::endl;
	}

	// * UnorderedMultiMap* - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( std::unordered_multimap< Type1, Type2* >* a_UnorderedMultiMap )
	{
		std::cout << "didn't inserted vector pointer of pointer type: " << typeid( Type2 ).name() << std::endl;
	}

	// const* UnorderedMultiMap& - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( const std::unordered_multimap< Type1, const Type2* >& a_UnorderedMultiMap )
	{

	}

	// const* UnorderedMultiMap& - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::unordered_multimap< Type1, const Type2* >& a_UnorderedMultiMap )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// const* UnorderedMultiMap& - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::unordered_multimap< Type1, const Type2* >& a_UnorderedMultiMap )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// * UnorderedMultiMap& - Pair, Tuple, or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value, void >::
	type InsertObject( const std::unordered_multimap< Type1, Type2* >& a_UnorderedMultiMap )
	{

	}

	// * UnorderedMultiMap& - Not Pair, Tuple or Container - Accepted
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::unordered_multimap< Type1, Type2* >& a_UnorderedMultiMap )
	{
		std::cout << "inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	// * UnorderedMultiMap& - Not Pair, Tuple or Container - Rejected
	template < typename Type1, typename Type2 >
	typename std::enable_if< !( std::is_pair< Type2 >::value || std::is_tuple< Type2 >::value || std::is_stl_container< Type2 >::value ) && !std::is_base_of< base_type, Type2 >::value, void >::
	type InsertObject( const std::unordered_multimap< Type1, Type2* >& a_UnorderedMultiMap )
	{
		std::cout << "didn't inserted vector reference of reference type: " << typeid( Type2 ).name() << std::endl;
	}

	///////////////////////////
	#pragma endregion

	#pragma region Variadic
	///////////////////////////

	// Variadic list - const*
	template < typename First, typename... Rest >
	void InsertObject( const First* a_First, Rest... a_Rest )
	{
		InsertObject( a_First );
		InsertObject( a_Rest... );
	}

	// Variadic list - *
	template < typename First, typename... Rest >
	void InsertObject( First* a_First, Rest... a_Rest )
	{
		InsertObject( a_First );
		InsertObject( a_Rest... );
	}

	// Variadic list - &
	template < typename First, typename... Rest >
	void InsertObject( const First& a_First, Rest... a_Rest )
	{
		InsertObject( a_First );
		InsertObject( a_Rest... );
	}

	void InsertObject()
	{
		std::cout << "inserted nothing" << std::endl;
	}

	///////////////////////////
	#pragma endregion

	///////////////////////////
	#pragma endregion

	#pragma region FindObject
	///////////////////////////

	template < typename Type >
	typename std::enable_if< std::is_base_of< base_type, Type >::value, Type* >::type
	FindObjectOfType()
	{

	}

	template < typename Type >
	typename std::enable_if< std::is_base_of< base_type, Type >::value, Type* >::type
	FindObjectOfType( int a_Index )
	{

	}

	template < typename Type >
	typename std::enable_if< std::is_base_of< base_type, Type >::value, Type* >::type
	FindObjectOfExactType()
	{

	}

	template < typename Type >
	typename std::enable_if< std::is_base_of< base_type, Type >::value, Type* >::type
	FindObjectOfExactType( int a_Index )
	{

	}

	template < typename Type >
	typename std::enable_if< std::is_base_of< base_type, Type >::value, Type* >::type
	FindObjectOfDerivedType()
	{

	}

	template < typename Type >
	typename std::enable_if< std::is_base_of< base_type, Type >::value, Type* >::type
	FindObjectOfDerivedType( int a_Index )
	{

	}

	template < typename Type >
	typename std::enable_if< std::is_base_of< base_type, Type >::value, std::vector< Type* > >::type
	FindObjectsOfType()
	{

	}

	template < typename Type >
	typename std::enable_if< std::is_base_of< base_type, Type >::value, std::vector< Type* > >::type
	FindObjectsOfExactType()
	{

	}

	template < typename Type >
	typename std::enable_if< std::is_base_of< base_type, Type >::value, std::vector< Type* > >::type
	FindObjectsOfDerivedType()
	{

	}

	///////////////////////////
	#pragma endregion

private:

	// This will store nodes
	std::map< unsigned int, Node* > m_Nodes;

};