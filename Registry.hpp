#pragma once
#include <vector>
#include <map>
#include <queue>
#include <typeinfo>

#include <iostream>
#include <string>

using namespace std;

template < typename Base >
class Registry
{
	static_assert( has_virtual_destructor_v< Base >, "Base is not a polymorphic type." );

private:

	template < typename Type >
	struct Node;

	struct INode
	{
		typedef bool( *IsBaseOf_T )( Base* );

		INode( IsBaseOf_T a_IsBaseOf, size_t a_HashCode, INode* a_Parent )
			: m_IsBaseOf( a_IsBaseOf ), m_Parent( a_Parent ), m_HashCode( a_HashCode ), m_ValueCount( 0 ), m_ChildCount( 0 )
		{ }

		template < typename T >
		Node< T >* AsNode()
		{
			return static_cast< Node< T >* >( this );
		}

		IsBaseOf_T m_IsBaseOf;
		INode* m_Parent;
		vector< INode* > m_Children;
		size_t m_HashCode;
		size_t m_ValueCount;
		size_t m_ChildCount;
	};

	template < typename Type >
	struct Node : public INode
	{
		Node( INode* a_Parent ) 
			: INode( IsBaseOf, typeid( Type ).hash_code(), a_Parent )
		{ }

		template < typename T >
		vector< T* > GetValuesAs() const
		{
			vector< T* > values;
			values.reserve( m_Values.size() );

			for ( auto& value : m_Values )
			{
				values.push_back( ( T* )&value );
			}

			return values;
		}

		static bool IsBaseOf( Base* a_Ptr )
		{
			return dynamic_cast< Type* >( a_Ptr );
		}

		vector< Type > m_Values;
	};

public:

	Registry()
		: m_Root( nullptr )
	{
		m_Nodes.emplace( make_pair( typeid( Base ).hash_code(), &m_Root ) );
	}

	~Registry()
	{
		for ( auto& node : m_Nodes )
		{
			delete node.second;
		}
	}
	
	void Debug()
	{
		Print( &m_Root, 5 );
	}

	void Print( INode* node, int level )
	{
		string indent = string( level, '-' );
		string space = string( 5, ' ' );
		cout << indent << '{' << endl;
		cout << indent + space << "AllValues:   " << node->m_ValueCount << endl;
		cout << indent + space << "AllChildren: " << node->m_ChildCount << endl;
		cout << indent + space << "ThisValues:  " << node->AsNode< Base >()->m_Values.size() << endl;
		for ( auto& child : node->m_Children )
		{
			Print( child, level + 5 );
		}
		cout << indent << '}' << endl;
	}

	template < typename T, typename... Args >
	typename enable_if< is_base_of_v< Base, T >, T* >::
	type Emplace( size_t a_Index, Args... a_Args )
	{
		Node< T >* exact = nullptr;
		T* newObject = new T( a_Args... );
		auto iter = m_Nodes.find( typeid( T ).hash_code() );
		bool createdNewNode = false;

		if ( iter != m_Nodes.end() )
		{
			exact = static_cast< Node< T >* >( iter->second );
		}
		else
		{
			queue< INode* > nodes;
			nodes.push( ( INode* )&m_Root );
			INode* closest = nullptr;

			do
			{
				INode* node = nodes.front();
				nodes.pop();

				if ( node->m_IsBaseOf( reinterpret_cast< Base* >( newObject ) ) )
				{
					closest = node;
					nodes = queue< INode* >();

					for ( auto& child : node->m_Children )
					{
						nodes.push( child );
					}
				}

			} while ( !nodes.empty() );

			exact = static_cast< Node< T >* >( m_Nodes.emplace( make_pair( typeid( T ).hash_code(), static_cast< INode* >( new Node< T >( closest ) ) ) ).first->second );
			createdNewNode = true;
			list< INode* > peers;
			list< INode* > children;

			for ( auto& peer : closest->m_Children )
			{
				Base* value = &peer->AsNode< Base >()->m_Values.front();
				if ( !exact->m_IsBaseOf( value ) )
				{
					peers.push_back( peer );
				}
				else
				{
					children.push_back( peer );
					peer->m_Parent = exact;
				}
			}

			if ( peers.size() != closest->m_Children.size() )
			{
				exact->m_Children.reserve( children.size() );

				for ( auto& child : children )
				{
					exact->m_Children.push_back( child );
					exact->m_ValueCount += child->m_ValueCount;
					exact->m_ChildCount += child->m_ChildCount + 1;
				}

				closest->m_Children = vector< INode* >();
				closest->m_Children.reserve( peers.size() + 1 );

				for ( auto& peer : peers )
				{
					closest->m_Children.push_back( peer );
				}
			}

			closest->m_Children.push_back( exact );
		}

		INode* parent = exact->m_Parent;
		while ( parent )
		{
			if ( createdNewNode )
			{
				++parent->m_ChildCount;
			}
			++parent->m_ValueCount;
			parent = parent->m_Parent;
		}

		auto where = exact->m_Values.begin();
		a_Index = a_Index > exact->m_Values.size() ? exact->m_Values.size() : a_Index;
		advance( where, a_Index );
		exact->m_Values.insert( where, *newObject );
		++exact->m_ValueCount;
		operator delete( newObject );
		return &exact->m_Values.at( a_Index );
	}

	template < typename T, typename... Args >
	typename enable_if< is_base_of_v< Base, T >, T* >::
	type EmplaceFront( Args... a_Args )
	{
		Node< T >* exact = nullptr;
		T* newObject = new T( a_Args... );
		auto iter = m_Nodes.find( typeid( T ).hash_code() );
		bool createdNewNode = false;

		if ( iter != m_Nodes.end() )
		{
			exact = static_cast< Node< T >* >( iter->second );
		}
		else
		{
			queue< INode* > nodes;
			nodes.push( ( INode* )&m_Root );
			INode* closest = nullptr;

			do
			{
				INode* node = nodes.front();
				nodes.pop();

				if ( node->m_IsBaseOf( reinterpret_cast< Base* >( newObject ) ) )
				{
					closest = node;
					nodes = queue< INode* >();

					for ( auto& child : node->m_Children )
					{
						nodes.push( child );
					}
				}

			} while ( !nodes.empty() );

			exact = static_cast< Node< T >* >( m_Nodes.emplace( make_pair( typeid( T ).hash_code(), static_cast< INode* >( new Node< T >( closest ) ) ) ).first->second );
			createdNewNode = true;
			list< INode* > peers;
			list< INode* > children;
			
			for ( auto& peer : closest->m_Children )
			{
				Base* value = &reinterpret_cast< Node< Base >* >( peer )->m_Values.front();
				if ( !exact->m_IsBaseOf( value ) )
				{
					peers.push_back( peer );
				}
				else
				{
					children.push_back( peer );
					peer->m_Parent = exact;
				}
			}

			if ( peers.size() != closest->m_Children.size() )
			{
				exact->m_Children.reserve( children.size() );

				for ( auto& child : children )
				{
					exact->m_Children.push_back( child );
					exact->m_ValueCount += child->m_ValueCount;
					exact->m_ChildCount += child->m_ChildCount + 1;
				}

				closest->m_Children = vector< INode* >();
				closest->m_Children.reserve( peers.size() + 1 );

				for ( auto& peer : peers )
				{
					closest->m_Children.push_back( peer );
				}
			}

			closest->m_Children.push_back( exact );
		}

		INode* parent = exact->m_Parent;
		while ( parent )
		{
			if ( createdNewNode )
			{
				++parent->m_ChildCount;
			}
			++parent->m_ValueCount;
			parent = parent->m_Parent;
		}

		auto where = exact->m_Values.insert( exact->m_Values.begin(), *newObject );
		++exact->m_ValueCount;
		operator delete( newObject );
		return &*where;
	}

	template < typename T, typename... Args >
	typename enable_if< is_base_of_v< Base, T >, T* >::
	type EmplaceBack( const Args&... a_Args )
	{
		Node< T >* exact = nullptr;
		T* newObject = new T( a_Args... );
		auto iter = m_Nodes.find( typeid( T ).hash_code() );
		bool createdNewNode = false;

		if ( iter != m_Nodes.end() )
		{
			exact = static_cast< Node< T >* >( iter->second );
		}
		else
		{
			queue< INode* > nodes;
			nodes.push( ( INode* )&m_Root );
			INode* closest = nullptr;

			do
			{
				INode* node = nodes.front();
				nodes.pop();

				if ( node->m_IsBaseOf( reinterpret_cast< Base* >( newObject ) ) )
				{
					closest = node;
					nodes = queue< INode* >();

					for ( auto& child : node->m_Children )
					{
						nodes.push( child );
					}
				}

			} while ( !nodes.empty() );

			exact = static_cast< Node< T >* >( m_Nodes.emplace( make_pair( typeid( T ).hash_code(), static_cast< INode* >( new Node< T >( closest ) ) ) ).first->second );
			createdNewNode = true;
			list< INode* > peers;
			list< INode* > children;
			
			for ( auto& peer : closest->m_Children )
			{
				Base* value = &reinterpret_cast< Node< Base >* >( peer )->m_Values.front();
				if ( !exact->m_IsBaseOf( value ) )
				{
					peers.push_back( peer );
				}
				else
				{
					children.push_back( peer );
					peer->m_Parent = exact;
				}
			}

			if ( peers.size() != closest->m_Children.size() )
			{
				exact->m_Children.reserve( children.size() );

				for ( auto& child : children )
				{
					exact->m_Children.push_back( child );
					exact->m_ValueCount += child->m_ValueCount;
					exact->m_ChildCount += child->m_ChildCount + 1;
				}

				closest->m_Children = vector< INode* >();
				closest->m_Children.reserve( peers.size() + 1 );

				for ( auto& peer : peers )
				{
					closest->m_Children.push_back( peer );
				}
			}

			closest->m_Children.push_back( exact );
		}

		INode* parent = exact->m_Parent;
		while ( parent )
		{
			if ( createdNewNode )
			{
				++parent->m_ChildCount;
			}
			++parent->m_ValueCount;
			parent = parent->m_Parent;
		}

		exact->m_Values.push_back( *newObject );
		++exact->m_ValueCount;
		operator delete( newObject );
		return &exact->m_Values.back();
	}

	template < typename T >
	typename enable_if< is_base_of_v< Base, T >, T* >::
	type PushFront( const T& a_Object )
	{
		Node< T >* exact = nullptr;
		auto iter = m_Nodes.find( typeid( T ).hash_code() );
		bool createdNewNode = false;

		if ( iter != m_Nodes.end() )
		{
			exact = static_cast< Node< T >* >( iter->second );
		}
		else
		{
			queue< INode* > nodes;
			nodes.push( ( INode* )&m_Root );
			INode* closest = nullptr;

			do
			{
				INode* node = nodes.front();
				nodes.pop();

				if ( node->m_IsBaseOf( reinterpret_cast< Base* >( &a_Object ) ) )
				{
					closest = node;
					nodes = queue< INode* >();

					for ( auto& child : node->m_Children )
					{
						nodes.push( child );
					}
				}

			} while ( !nodes.empty() );

			exact = static_cast< Node< T >* >( m_Nodes.emplace( make_pair( typeid( T ).hash_code(), static_cast< INode* >( new Node< T >( closest ) ) ) ).first->second );
			createdNewNode = true;
			list< INode* > peers;
			list< INode* > children;

			for ( auto& peer : closest->m_Children )
			{
				Base* value = &reinterpret_cast< Node< Base >* >( peer )->m_Values.front();
				if ( !exact->m_IsBaseOf( value ) )
				{
					peers.push_back( peer );
				}
				else
				{
					children.push_back( peer );
					peer->m_Parent = exact;
				}
			}

			if ( peers.size() != closest->m_Children.size() )
			{
				exact->m_Children.reserve( children.size() );

				for ( auto& child : children )
				{
					exact->m_Children.push_back( child );
					exact->m_ValueCount += child->m_ValueCount;
					exact->m_ChildCount += child->m_ChildCount + 1;
				}

				closest->m_Children = vector< INode* >();
				closest->m_Children.reserve( peers.size() + 1 );

				for ( auto& peer : peers )
				{
					closest->m_Children.push_back( peer );
				}
			}

			closest->m_Children.push_back( exact );
		}

		INode* parent = exact->m_Parent;
		while ( parent )
		{
			if ( createdNewNode )
			{
				++parent->m_ChildCount;
			}
			++parent->m_ValueCount;
			parent = parent->m_Parent;
		}

		exact->m_Values.insert( exact->m_Values.begin(), a_Object );
		++exact->m_ValueCount;
		return &exact->m_Values.front();
	}

	template < typename T >
	typename enable_if< is_base_of_v< Base, T >, T* >::
	type PushBack( const T& a_Object )
	{
		Node< T >* exact = nullptr;
		auto iter = m_Nodes.find( typeid( T ).hash_code() );
		bool createdNewNode = false;

		if ( iter != m_Nodes.end() )
		{
			exact = static_cast< Node< T >* >( iter->second );
		}
		else
		{
			queue< INode* > nodes;
			nodes.push( ( INode* )&m_Root );
			INode* closest = nullptr;

			do
			{
				INode* node = nodes.front();
				nodes.pop();

				if ( node->m_IsBaseOf( reinterpret_cast< Base* >( &a_Object ) ) )
				{
					closest = node;
					nodes = queue< INode* >();

					for ( auto& child : node->m_Children )
					{
						nodes.push( child );
					}
				}

			} while ( !nodes.empty() );

			exact = static_cast< Node< T >* >( m_Nodes.emplace( make_pair( typeid( T ).hash_code(), static_cast< INode* >( new Node< T >( closest ) ) ) ).first->second );
			createdNewNode = true;
			list< INode* > peers;
			list< INode* > children;

			for ( auto& peer : closest->m_Children )
			{
				Base* value = &reinterpret_cast< Node< Base >* >( peer )->m_Values.front();
				if ( !exact->m_IsBaseOf( value ) )
				{
					peers.push_back( peer );
				}
				else
				{
					children.push_back( peer );
					peer->m_Parent = exact;
				}
			}

			if ( peers.size() != closest->m_Children.size() )
			{
				exact->m_Children.reserve( children.size() );

				for ( auto& child : children )
				{
					exact->m_Children.push_back( child ); 
					exact->m_ValueCount += child->m_ValueCount;
					exact->m_ChildCount += child->m_ChildCount + 1;
				}

				closest->m_Children = vector< INode* >();
				closest->m_Children.reserve( peers.size() + 1 );

				for ( auto& peer : peers )
				{
					closest->m_Children.push_back( peer );
				}
			}

			closest->m_Children.push_back( exact );
		}

		INode* parent = exact->m_Parent;
		while ( parent )
		{
			if ( createdNewNode )
			{
				++parent->m_ChildCount;
			}
			++parent->m_ValueCount;
			parent = parent->m_Parent;
		}

		exact->m_Values.insert( exact->m_Values.end(), a_Object );
		++exact->m_ValueCount;
		return &exact->m_Values.back();
	}

	template < typename T >
	typename enable_if< is_base_of_v< Base, T >, bool >::
	type PopFront()
	{
		auto iter = m_Nodes.find( typeid( T ).hash_code() );

		if ( iter != m_Nodes.end() )
		{
			Node< T >* node = iter->second->AsNode< T >();
			if ( node->m_Values.size() == 1 && !is_same_v< Base, T > )
			{
				INode* parent = iter->second->m_Parent;

				while ( parent )
				{
					parent->m_ValueCount -= iter->second->AsNode< T >()->m_Values.size();
					--parent->m_ChildCount;
					parent = parent->m_Parent;
				}

				parent = iter->second->m_Parent;

				if ( iter->second->m_Children.size() )
				{
					parent->m_Children.erase( find( parent->m_Children.begin(), parent->m_Children.end(), iter->second ) );
					vector< INode* > children = iter->second->m_Children;
					delete iter->second;
					parent->m_Children.reserve( parent->m_Children.size() + children.size() );
					m_Nodes.erase( iter );
					for ( auto& child : children )
					{
						parent->m_Children.push_back( child );
					}
				}
			}
			else if ( node->m_Values.size() )
			{
				node->m_Values.erase( node->m_Values.begin() );
				--node->m_ValueCount;
				INode* parent = iter->second->m_Parent;

				while ( parent )
				{
					parent->m_ValueCount -= iter->second->AsNode< T >()->m_Values.size();
					--parent->m_ChildCount;
					parent = parent->m_Parent;
				}
			}

			return true;
		}

		return false;
	}
	
	template < typename T >
	typename enable_if< is_base_of_v< Base, T >, bool >::
	type PopBack()
	{
		auto iter = m_Nodes.find( typeid( T ).hash_code() );

		if ( iter != m_Nodes.end() )
		{
			Node< T >* node = iter->second->AsNode< T >();
			if ( node->m_Values.size() == 1 && !is_same_v< Base, T > )
			{
				INode* parent = iter->second->m_Parent;

				while ( parent )
				{
					parent->m_ValueCount -= iter->second->AsNode< T >()->m_Values.size();
					--parent->m_ChildCount;
					parent = parent->m_Parent;
				}

				parent = iter->second->m_Parent;

				if ( iter->second->m_Children.size() )
				{
					parent->m_Children.erase( find( parent->m_Children.begin(), parent->m_Children.end(), iter->second ) );
					vector< INode* > children = iter->second->m_Children;
					delete iter->second;
					parent->m_Children.reserve( parent->m_Children.size() + children.size() );
					m_Nodes.erase( iter );
					for ( auto& child : children )
					{
						parent->m_Children.push_back( child );
					}
				}
			}
			else if ( node->m_Values.size() )
			{
				node->m_Values.pop_back();
				--node->m_ValueCount;
				INode* parent = iter->second->m_Parent;

				while ( parent )
				{
					parent->m_ValueCount -= iter->second->AsNode< T >()->m_Values.size();
					--parent->m_ChildCount;
					parent = parent->m_Parent;
				}
			}

			return true;
		}

		return false;
	}

	template < typename T >
	typename enable_if< is_base_of_v< Base, T >, T* >::
	type Insert( const T& a_Object, size_t a_Index )
	{
		Node< T >* exact = nullptr;
		auto iter = m_Nodes.find( typeid( T ).hash_code() );
		bool createdNewNode = false;

		if ( iter != m_Nodes.end() )
		{
			exact = static_cast< Node< T >* >( iter->second );
		}
		else
		{
			queue< INode* > nodes;
			nodes.push( ( INode* )&m_Root );
			INode* closest = nullptr;

			do
			{
				INode* node = nodes.front();
				nodes.pop();

				if ( node->m_IsBaseOf( reinterpret_cast< Base* >( &a_Object ) ) )
				{
					closest = node;
					nodes = queue< INode* >();

					for ( auto& child : node->m_Children )
					{
						nodes.push( child );
					}
				}

			} while ( !nodes.empty() );

			exact = static_cast< Node< T >* >( m_Nodes.emplace( make_pair( typeid( T ).hash_code(), static_cast< INode* >( new Node< T >( closest ) ) ) ).first->second );
			createdNewNode = true;
			list< INode* > peers;
			list< INode* > children;

			for ( auto& peer : closest->m_Children )
			{
				Base* value = &reinterpret_cast< Node< Base >* >( peer )->m_Values.front();
				if ( !exact->m_IsBaseOf( value ) )
				{
					peers.push_back( peer );
				}
				else
				{
					children.push_back( peer );
					peer->m_Parent = exact;
				}
			}

			if ( peers.size() != closest->m_Children.size() )
			{
				exact->m_Children.reserve( children.size() );

				for ( auto& child : children )
				{
					exact->m_Children.push_back( child );
					exact->m_ValueCount += child->m_ValueCount;
					exact->m_ChildCount += child->m_ChildCount + 1;
				}

				closest->m_Children = vector< INode* >();
				closest->m_Children.reserve( peers.size() + 1 );

				for ( auto& peer : peers )
				{
					closest->m_Children.push_back( peer );
				}
			}

			closest->m_Children.push_back( exact );
		}

		INode* parent = exact->m_Parent;
		while ( parent )
		{
			if ( createdNewNode )
			{
				++parent->m_ChildCount;
			}
			++parent->m_ValueCount;
			parent = parent->m_Parent;
		}

		auto where = exact->m_Values.begin();
		a_Index = a_Index > exact->m_Values.size() ? exact->m_Values.size() : a_Index;
		advance( where, a_Index );
		exact->m_Values.insert( where, a_Object );
		++exact->m_ValueCount;
		return &exact->m_Values.at( a_Index );
	}

	template < typename T >
	typename enable_if< is_base_of_v< Base, T >, T* >::
	type Front() const
	{
		auto iter = m_Nodes.find( typeid( T ).hash_code() );
		if ( iter != m_Nodes.end() )
		{
			return &iter->AsNode< T >()->m_Values.front();
		}

		return nullptr;
	}

	template < typename T >
	typename enable_if< is_base_of_v< Base, T >, T* >::
	type Back() const
	{
		auto iter = m_Nodes.find( typeid( T ).hash_code() );
		if ( iter != m_Nodes.end() )
		{
			return &iter->AsNode< T >()->m_Values.back();
		}

		return nullptr;
	}

	template < typename T >
	typename enable_if< is_base_of_v< Base, T >, T* >::
	type At( size_t a_Index ) const
	{
		auto iter = m_Nodes.find( typeid( T ).hash_code() );
		if ( iter != m_Nodes.end() )
		{
			Node< T >* node = iter->AsNode< T >();

			if ( a_Index >= node->m_Values.size() )
			{
				return &node->m_Values.at( a_Index );
			}
			else
			{
				return nullptr;
			}
		}

		return nullptr;
	}

	template < typename T >
	typename enable_if< is_base_of_v< Base, T >, T* >::
	type FindOfType() const
	{
		auto iter = m_Nodes.find( typeid( T ).hash_code() );

		if ( iter != m_Nodes.end() )
		{
			return &iter->second->AsNode< T >()->m_Values.front();
		}
		
		queue< INode* > nodes;
		nodes.push( ( INode* )&m_Root );
		list< INode* > children;

		do
		{
			INode* node = nodes.front();
			nodes.pop();

			for ( auto& child : node->m_Children )
			{
				nodes.push( child );

				Base* value = &child->AsNode< Base >()->m_Values.front();
				if ( dynamic_cast< T* >( value ) )
				{
					children.push_back( child );
					break;
				}
			}

			if ( children.size() > 0 )
			{
				break;
			}

		} while ( !nodes.empty() );

		if ( children.size() > 0 )
		{
			return &children.front()->AsNode< T >()->m_Values.front();
		}
		
		return nullptr;
	}

	template < typename T >
	typename enable_if< is_base_of_v< Base, T >, vector< T* > >::
	type FindAllOfType() const
	{
		queue< INode* > nodes;
		size_t foundCount = 0;
		auto iter = m_Nodes.find( typeid( T ).hash_code() );

		if ( iter != m_Nodes.end() )
		{
			nodes.push( iter->second );
			foundCount += iter->second->m_ValueCount;
		}
		else
		{
			nodes.push( ( INode* )&m_Root );
			list< INode* > children;

			do
			{
				INode* node = nodes.front();
				nodes.pop();

				for ( auto& child : node->m_Children )
				{
					nodes.push( child );

					Base* value = &child->AsNode< Base >()->m_Values.front();
					if ( dynamic_cast< T* >( value ) )
					{
						children.push_back( child );
						foundCount += child->m_ValueCount;
					}
				}

				if ( children.size() > 0 )
				{
					break;
				}

			} while ( !nodes.empty() );

			nodes = queue< INode* >();

			for ( auto& child : children )
			{
				nodes.push( child );
			}
		}

		if ( foundCount > 0 )
		{
			vector< T* > found;
			found.reserve( foundCount );

			do
			{
				INode* node = nodes.front();
				nodes.pop();
				vector< T* >&& values = node->AsNode< T >()->GetValuesAs< T >();

				for ( T* ptr : values )
				{
					found.push_back( ptr );
				}

				for ( auto& child : node->m_Children )
				{
					nodes.push( child );
				}

			} while ( !nodes.empty() );

			return found;
		}

		return vector< T* >();
	}

	template < typename T >
	typename enable_if< is_base_of_v< Base, T >, vector< T* > >::
	type FindAllOfExactType() const
	{
		auto iter = m_Nodes.find( typeid( T ).hash_code() );

		if ( iter != m_Nodes.end() )
		{
			return iter->second->AsNode< T >()->GetValuesAs< T >();
		}

		return vector< T* >();
	}

	bool Clear()
	{
		if ( !m_Root.m_ValueCount )
		{
			return false;
		}

		size_t baseHashCode = typeid( Base ).hash_code();

		for ( auto& node : m_Nodes )
		{
			if ( node.first != baseHashCode )
			{
				delete node.second;
			}
		}

		m_Nodes.clear();
		m_Nodes.emplace( make_pair( baseHashCode, ( INode* )&m_Root ) );
		m_Root.m_Values.clear();
		m_Root.m_Children.clear();
		m_Root.m_ValueCount = 0;
		m_Root.m_ChildCount = 0;
		return true;
	}

	template < typename T >
	typename enable_if< is_base_of_v< Base, T >, bool >::
	type ClearType()
	{
		list< INode* > children;
		auto iter = m_Nodes.find( typeid( T ).hash_code() );
		size_t valueCount = 0;
		size_t childCount = 0;


		if ( iter != m_Nodes.end() )
		{
			children.push_back( iter->second );
			valueCount += iter->second->m_ValueCount;
			childCount += iter->second->m_ChildCount + 1;
		}
		else
		{
			queue< INode* > nodes;
			nodes.push( ( INode* )&m_Root );

			do
			{
				INode* node = nodes.front();
				nodes.pop();

				for ( auto& child : node->m_Children )
				{
					nodes.push( child );
					Base* value = &child->AsNode< Base >()->m_Values.front();
					if ( dynamic_cast< T* >( value ) )
					{
						children.push_back( child );
						valueCount += child->m_ValueCount;
						childCount += child->m_ChildCount + 1;
					}
				}

				if ( children.size() )
				{
					break;
				}

			} while ( !nodes.empty() );
		}

		if ( children.size() )
		{
			INode* parent = children.front()->m_Parent;
			
			if ( parent )
			{
				for ( auto& child : children )
				{
					parent->m_Children.erase( find( parent->m_Children.begin(), parent->m_Children.end(), child ) );
				}
			}

			while ( parent )
			{
				parent->m_ValueCount -= valueCount;
				parent->m_ChildCount -= childCount;
				parent = parent->m_Parent;
			}

			size_t baseHashCode = typeid( Base ).hash_code();

			do
			{
				INode* node = children.front();
				children.pop_front();

				for ( auto& child : node->m_Children )
				{
					children.push_back( child );
				}

				if ( node->m_HashCode != baseHashCode )
				{
					m_Nodes.erase( m_Nodes.find( node->m_HashCode ) );
					delete node;
				}
				else
				{
					node->AsNode< Base >()->m_Values.clear();
					node->m_Children.clear();
					node->m_ValueCount = 0;
					node->m_ChildCount = 0;
				}

			} while ( !children.empty() );

			return true;
		}

		return false;
	}

	template < typename T >
	typename enable_if< is_base_of_v< Base, T >, bool >::
	type ClearExactType()
	{
		if ( is_same_v< Base, T > )
		{
			m_Root.m_ValueCount -= m_Root.m_Values.size();
			m_Root.m_Values.clear();
			return true;
		}

		auto iter = m_Nodes.find( typeid( T ).hash_code() );

		if ( iter != m_Nodes.end() )
		{
			INode* parent = iter->second->m_Parent;

			while ( parent )
			{
				parent->m_ValueCount -= iter->second->AsNode< T >()->m_Values.size();
				--parent->m_ChildCount;
				parent = parent->m_Parent;
			}

			parent = iter->second->m_Parent;
			
			if ( iter->second->m_Children.size() )
			{
				parent->m_Children.erase( find( parent->m_Children.begin(), parent->m_Children.end(), iter->second ) );
				vector< INode* > children = iter->second->m_Children;
				delete iter->second;
				parent->m_Children.reserve( parent->m_Children.size() + children.size() );
				m_Nodes.erase( iter );
				for ( auto& child : children )
				{
					parent->m_Children.push_back( child );
				}
			}

			return true;
		}

		return false;
	}

	bool Empty() const
	{
		return !m_Root->m_ValueCount;
	}

	size_t TypeCount() const
	{
		return m_Nodes.size();
	}

	template < typename T >
	size_t TypeCount() const
	{
		auto iter = m_Nodes.find( typeid( T ).hash_code() );

		if ( iter != m_Nodes.end() )
		{
			return iter->second->m_ChildCount + 1;
		}
		else
		{
			queue< INode* > nodes;
			nodes.push( ( INode* )&m_Root );
			list< INode* > children;
			size_t typeCount = 0;

			do
			{
				INode* node = nodes.front();
				nodes.pop();

				for ( auto& child : node->m_Children )
				{
					nodes.push( child );

					Base* value = &child->AsNode< Base >()->m_Values.front();
					if ( dynamic_cast< T* >( value ) )
					{
						children.push_back( child );
						typeCount += child->m_ChildCount + 1;
					}
				}

				if ( typeCount > 0 )
				{
					break;
				}

			} while ( !nodes.empty() );

			return typeCount;
		}
	}

	template < typename T >
	typename enable_if< is_base_of_v< Base, T >, bool >::
	type HasType() const
	{
		auto iter = m_Nodes.find( typeid( T ).hash_code() );

		if ( iter != m_Nodes.end() )
		{
			return &iter->second->AsNode< T >()->m_Values.front();
		}

		queue< INode* > nodes;
		nodes.push( ( INode* )&m_Root );

		do
		{
			INode* node = nodes.front();
			nodes.pop();

			for ( auto& child : node->m_Children )
			{
				nodes.push( child );

				Base* value = &child->AsNode< Base >()->m_Values.front();
				if ( dynamic_cast< T* >( value ) )
				{
					return true;
				}
			}

		} while ( !nodes.empty() );

		return false;
	}

	template < typename T >
	typename enable_if< is_base_of_v< Base, T >, bool >::
	type HasExactType() const
	{
		return m_Nodes.find( typeid( T ).hash_code() ) != m_Nodes.end();
	}

	size_t CountAll() const
	{
		return m_Root.m_ValueCount;
	}

	template < typename T >
	typename enable_if< is_base_of_v< Base, T >, size_t >::
	type CountOfType() const
	{
		auto iter = m_Nodes.find( typeid( T ).hash_code() );

		if ( iter != m_Nodes.end() )
		{
			return iter->second->m_ValueCount;
		}
		else
		{
			queue< INode* > nodes;
			nodes.push( ( INode* )&m_Root );
			size_t foundCount = 0;

			do
			{
				INode* node = nodes.front();
				nodes.pop();

				for ( auto& child : node->m_Children )
				{
					nodes.push( child );

					Base* value = &child->AsNode< Base >()->m_Values.front();
					if ( dynamic_cast< T* >( value ) )
					{
						foundCount += child->m_ValueCount;
					}
				}

				if ( foundCount > 0 )
				{
					return foundCount;
				}

			} while ( !nodes.empty() );

			return foundCount;
		}
	}

	template < typename T >
	typename enable_if< is_base_of_v< Base, T >, size_t >::
	type CountOfExactType() const
	{
		auto iter = m_Nodes.find( typeid( T ).hash_code() );

		if ( iter != m_Nodes.end() )
		{
			return iter->second->AsNode< T >()->m_Values.size();
		}

		return 0;
	}

private:

	Node< Base > m_Root;
	map< size_t, INode* > m_Nodes;

};