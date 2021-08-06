#pragma once
#include <list>
#include <vector>

// Callable is a collection of four classes made by Len Farag, and they are nicely wrapped function objects.
// They work similar to std::function, however, are much faster, and employ a helpful serving of syntactic sugar.

template < typename Return, typename... Args >
class Delegate;

template < typename... Args >
class Action;

template < typename... Args >
class Predicate;

/// <summary>
/// Can store a static, member, or lambda function.
/// </summary>
/// <typeparam name="...Args">Argument types for stored function.</typeparam>
/// <typeparam name="Return">Return type for stored function.</typeparam>
template < typename Return = void, typename... Args >
class Invoker
{
	typedef Return( *StaticFunction )( Args... );
	typedef Return( *LambdaFunction )( const Args&... );
	typedef Return( *MemberFunction )( void*, void*, const Args&... );

	template < typename T >
	static constexpr bool m_IsInvoker = false;

	template < typename... Args >
	static constexpr bool m_IsInvoker< Invoker< Args... > > = true;

	template < typename... Args >
	static constexpr bool m_IsInvoker< Action< Args... > > = true;

	template < typename... Args >
	static constexpr bool m_IsInvoker< Predicate< Args... > > = true;

	friend class Delegate< Return, Args... >;

	union Caller
	{
		StaticFunction m_Static;
		LambdaFunction m_Lambda;
		MemberFunction m_Member;
	};

public:

	Invoker() : m_Caller { nullptr }, m_Functor( nullptr ), m_Object( nullptr ), m_IsLambda( false ) { }

	Invoker( const Invoker< Return, Args... >& a_Invoker )
	{
		m_Caller = a_Invoker.m_Caller;
		m_Functor = a_Invoker.m_Functor;
		m_Object = a_Invoker.m_Object;
		m_IsLambda = a_Invoker.m_IsLambda;
	}

	Invoker( Return( *a_Functor )( Args... ) ) : m_Object( nullptr ), m_Functor( nullptr ), m_IsLambda( false )
	{
		m_Caller.m_Static = a_Functor;
	}

	template < typename Lambda, typename = typename std::enable_if< !m_IsInvoker< Lambda > >::type >
	Invoker( Lambda a_Lambda ) : m_IsLambda( true )
	{
		m_Caller.m_Lambda = LambdaPointer( a_Lambda );
		m_Functor = nullptr;
		m_Object = nullptr;
	}

	template < typename Object >
	Invoker( Object* a_Object, Return( Object::* a_Functor )( Args... ) ) : m_IsLambda( false )
	{
		m_Caller.m_Member = &Invocation< Object >;
		m_Functor = ( void*& )a_Functor;
		m_Object = a_Object;
	}

	template < typename Object >
	Invoker( Object& a_Object, Return( Object::* a_Functor )( Args... ) ) : m_IsLambda( false )
	{
		m_Caller.m_Member = &Invocation< Object >;
		m_Functor = ( void*& )a_Functor;
		m_Object = &a_Object;
	}

	template < typename Lambda, typename = typename std::enable_if< !m_IsInvoker< Lambda > >::type >
	void operator =( Lambda a_Lambda )
	{
		m_Caller.m_Lambda = LambdaPointer( a_Lambda );
		m_Functor = nullptr;
		m_Object = nullptr;
		m_IsLambda = true;
	}

	void operator =( StaticFunction a_Functor )
	{
		m_Caller.m_Static = a_Functor;
		m_Functor = nullptr;
		m_Object = nullptr;
		m_IsLambda = false;
	}

	Return operator ()( const Args&... a_Args ) const
	{
		if ( m_Object )
		{
			return ( *m_Caller.m_Member )( m_Object, m_Functor, a_Args... );
		}
		else if ( m_IsLambda )
		{
			return ( *m_Caller.m_Lambda )( a_Args... );
		}

		return ( *m_Caller.m_Static )( a_Args... );
	}

	bool operator ==( const Invoker< Return, Args... >& a_Invoker ) const
	{
		return m_Caller.m_Static == a_Invoker.m_Caller.m_Static &&
			m_Functor == a_Invoker.m_Functor &&
			m_Object == a_Invoker.m_Object;
	}

	bool operator ==( StaticFunction a_Functor ) const
	{
		return m_Caller.m_Static == a_Functor;
	}

	template < typename Object >
	bool operator ==( Return( Object::* a_Member )( Args... ) ) const
	{
		return m_Functor == ( void*& )a_Member;
	}

	template < typename Object, typename = typename std::enable_if< std::is_class_v< Object > >::type >
	bool operator ==( Object& a_Object ) const
	{
		return m_Object == &a_Object;
	}

	template < typename Object, typename = typename std::enable_if< std::is_class_v< Object > >::type >
	bool operator ==( Object* a_Object ) const
	{
		return m_Object == a_Object;
	}

	inline bool operator !=( const Invoker< Return, Args... >& a_Invoker ) const
	{
		return !( *this == a_Invoker );
	}

	inline bool operator !=( StaticFunction a_Functor ) const
	{
		return !( *this == a_Functor );
	}

	template < typename Object >
	inline bool operator !=( Return( Object::* a_Member )( Args... ) ) const
	{
		return !( *this == a_Member );
	}

	template < typename Object, typename = typename std::enable_if< std::is_class_v< Object > >::type >
	inline bool operator !=( Object& a_Object ) const
	{
		return !( *this == a_Object );
	}

	template < typename Object, typename = typename std::enable_if< std::is_class_v< Object > >::type >
	inline bool operator !=( Object* a_Object ) const
	{
		return !( *this == a_Object );
	}

	Return Invoke( const Args&... a_Args ) const
	{
		if ( m_Object )
		{
			return ( *m_Caller.m_Member )( m_Object, m_Functor, a_Args... );
		}
		else if ( m_IsLambda )
		{
			return ( *m_Caller.m_Lambda )( a_Args... );
		}

		return ( *m_Caller.m_Static )( a_Args... );
	}

private:

	template < typename Object >
	static Return Invocation( void* a_Object, void* a_Functor, const Args&... a_Args )
	{
		union
		{
			void* in;
			Return( Object::* out )( Args... );
		} converter;
		converter.in = a_Functor;
		return ( static_cast< Object* >( a_Object )->*( converter.out ) )( a_Args... );
	}

	template < typename Lambda >
	static LambdaFunction LambdaPointer( Lambda a_Lambda )
	{
		static auto deducedLambda = a_Lambda;

		return []( const Args&... a_Args ) -> Return
		{
			return deducedLambda( a_Args... );
		};
	}

	Caller m_Caller;
	void* m_Functor;
	void* m_Object;
	bool m_IsLambda;
};

/// <summary>
/// Can store a static, member, or lambda function.
/// </summary>
/// <typeparam name="...Args">Argument types for stored function.</typeparam>
template < typename... Args >
class Invoker< void, Args... >
{
	typedef void( *StaticFunction )( Args... );
	typedef void( *LambdaFunction )( const Args&... );
	typedef void( *MemberFunction )( void*, void*, const Args&... );

	template < typename T >
	static constexpr bool m_IsInvoker = false;

	template < typename... Args >
	static constexpr bool m_IsInvoker< Invoker< Args... > > = true;

	template < typename... Args >
	static constexpr bool m_IsInvoker< Action< Args... > > = true;

	template < typename... Args >
	static constexpr bool m_IsInvoker< Predicate< Args... > > = true;

	friend class Delegate< void, Args... >;

	union Caller
	{
		StaticFunction m_Static;
		LambdaFunction m_Lambda;
		MemberFunction m_Member;
	};

public:

	Invoker() : m_Caller { nullptr }, m_Functor( nullptr ), m_Object( nullptr ), m_IsLambda( false ) { }

	Invoker( const Invoker< void, Args... >& a_Invoker )
	{
		m_Caller = a_Invoker.m_Caller;
		m_Functor = a_Invoker.m_Functor;
		m_Object = a_Invoker.m_Object;
		m_IsLambda = a_Invoker.m_IsLambda;
	}

	Invoker( void( *a_Functor )( Args... ) ) : m_Object( nullptr ), m_Functor( nullptr ), m_IsLambda( false )
	{
		m_Caller.m_Static = a_Functor;
	}

	template < typename Lambda, typename = typename std::enable_if< !m_IsInvoker< Lambda > >::type >
	Invoker( Lambda a_Lambda ) : m_IsLambda( true )
	{
		m_Caller.m_Lambda = LambdaPointer( a_Lambda );
		m_Functor = nullptr;
		m_Object = nullptr;
	}

	template < typename Object >
	Invoker( Object* a_Object, void( Object::* a_Functor )( Args... ) ) : m_IsLambda( false )
	{
		m_Caller.m_Member = &Invocation< Object >;
		m_Functor = ( void*& )a_Functor;
		m_Object = a_Object;
	}

	template < typename Object >
	Invoker( Object& a_Object, void( Object::* a_Functor )( Args... ) ) : m_IsLambda( false )
	{
		m_Caller.m_Member = &Invocation< Object >;
		m_Functor = ( void*& )a_Functor;
		m_Object = &a_Object;
	}

	template < typename Lambda, typename = typename std::enable_if< !m_IsInvoker< Lambda > >::type >
	void operator =( Lambda a_Lambda )
	{
		m_Caller.m_Lambda = LambdaPointer( a_Lambda );
		m_Functor = nullptr;
		m_Object = nullptr;
		m_IsLambda = true;
	}

	void operator =( StaticFunction a_Functor )
	{
		m_Caller.m_Static = a_Functor;
		m_Functor = nullptr;
		m_Object = nullptr;
		m_IsLambda = false;
	}

	void operator ()( const Args&... a_Args ) const
	{
		if ( m_Object )
		{
			( *m_Caller.m_Member )( m_Object, m_Functor, a_Args... );
			return;
		}
		else if ( m_IsLambda )
		{
			( *m_Caller.m_Lambda )( a_Args... );
			return;
		}

		( *m_Caller.m_Static )( a_Args... );
	}

	bool operator ==( const Invoker< void, Args... >& a_Invoker ) const
	{
		return m_Caller.m_Static == a_Invoker.m_Caller.m_Static &&
			m_Functor == a_Invoker.m_Functor &&
			m_Object == a_Invoker.m_Object;
	}

	bool operator ==( StaticFunction a_Functor ) const
	{
		return m_Caller.m_Static == a_Functor;
	}

	template < typename Object >
	bool operator ==( void( Object::* a_Member )( Args... ) ) const
	{
		return m_Functor == ( void*& )a_Member;
	}

	template < typename Object, typename = typename std::enable_if< std::is_class_v< Object > >::type >
	bool operator ==( Object& a_Object ) const
	{
		return m_Object == &a_Object;
	}

	template < typename Object, typename = typename std::enable_if< std::is_class_v< Object > >::type >
	bool operator ==( Object* a_Object ) const
	{
		return m_Object == a_Object;
	}

	inline bool operator !=( const Invoker< void, Args... >& a_Invoker ) const
	{
		return !( *this == a_Invoker );
	}

	inline bool operator !=( StaticFunction a_Functor ) const
	{
		return !( *this == a_Functor );
	}

	template < typename Object >
	inline bool operator !=( void( Object::* a_Member )( Args... ) ) const
	{
		return !( *this == a_Member );
	}

	template < typename Object, typename = typename std::enable_if< std::is_class_v< Object > >::type >
	inline bool operator !=( Object& a_Object ) const
	{
		return !( *this == a_Object );
	}

	template < typename Object, typename = typename std::enable_if< std::is_class_v< Object > >::type >
	inline bool operator !=( Object* a_Object ) const
	{
		return !( *this == a_Object );
	}

	void Invoke( const Args&... a_Args ) const
	{
		if ( m_Object )
		{
			( *m_Caller.m_Member )( m_Object, m_Functor, a_Args... );
			return;
		}
		else if ( m_IsLambda )
		{
			( *m_Caller.m_Lambda )( a_Args... );
			return;
		}

		( *m_Caller.m_Static )( a_Args... );
	}

private:

	template < typename Object >
	static void Invocation( void* a_Object, void* a_Functor, const Args&... a_Args )
	{
		union
		{
			void* in;
			void( Object::* out )( Args... );
		} converter;
		converter.in = a_Functor;
		( static_cast< Object* >( a_Object )->*( converter.out ) )( a_Args... );
	}

	template < typename Lambda >
	static LambdaFunction LambdaPointer( Lambda a_Lambda )
	{
		static auto deducedLambda = a_Lambda;

		return []( const Args&... a_Args )
		{
			deducedLambda( a_Args... );
		};
	}

	Caller m_Caller;
	void* m_Functor;
	void* m_Object;
	bool m_IsLambda;
};

/// <summary>
/// Can store a list of invokers of the same type footprint. All stored functions can be
/// called together or seperately.
/// </summary>
/// <typeparam name="Return">Return type for stored functions.</typeparam>
/// <typeparam name="...Args">Argument types for stored functions.</typeparam>
template < typename Return = void, typename... Args >
class Delegate
{
	template < typename T >
	static constexpr bool m_IsInvoker = false;

	template < typename... Args >
	static constexpr bool m_IsInvoker< Invoker< Args... > > = true;

	template < typename... Args >
	static constexpr bool m_IsInvoker< Action< Args... > > = true;

	template < typename... Args >
	static constexpr bool m_IsInvoker< Predicate< Args... > > = true;

public:

	Delegate() = default;

	Delegate( const Invoker< Return, Args... >& a_Invoker )
	{
		m_Invokers.emplace_back( a_Invoker );
	}

	Delegate( const Delegate< Return, Args... >& a_Delegate )
	{
		m_Invokers.assign( a_Delegate.m_Invokers.begin(), a_Delegate.m_Invokers.end() );
	}

	Delegate( Return( *a_Function )( Args... ) )
	{
		m_Invokers.emplace_back( Invoker< Return, Args... >( a_Function ) );
	}

	template < typename Lambda, typename = typename std::enable_if< !m_IsInvoker< Lambda > >::type >
	Delegate( Lambda a_Lambda )
	{
		m_Invokers.emplace_back( Invoker( a_Lambda ) );
	}

	void operator =( const Invoker< Return, Args... >& a_Invoker )
	{
		m_Invokers.clear();
		m_Invokers.emplace_back( a_Invoker );
	}

	void operator =( const Delegate< Return, Args... >& a_Delegate )
	{
		m_Invokers.clear();
		m_Invokers.assign( a_Delegate.m_Invokers.begin(), a_Delegate.m_Invokers.end() );
	}

	void operator =( void( *a_Static )( Args... ) )
	{
		m_Invokers.clear();
		m_Invokers.emplace_back( Invoker( a_Static ) );
	}

	void operator +=( const Invoker< Return, Args... >& a_Invoker )
	{
		m_Invokers.emplace_back( a_Invoker );
	}

	void operator +=( const Delegate< Return, Args... >& a_Delegate )
	{
		std::copy( a_Delegate.m_Invokers.begin(), a_Delegate.m_Invokers.end(),
				   std::back_insert_iterator( m_Invokers ) );
	}

	void operator +=( Return( *a_Static )( Args... ) )
	{
		m_Invokers.emplace_back( Invoker( a_Static ) );
	}

	void operator -=( const Invoker< Return, Args... >& a_Invoker )
	{
		for ( auto iter = m_Invokers.begin(); iter != m_Invokers.end(); )
		{
			if ( *iter == a_Invoker )
			{
				iter = m_Invokers.erase( iter );
			}
			else
			{
				++iter;
			}
		}
	}

	void operator -=( Return( *a_Static )( Args... ) )
	{
		for ( auto iter = m_Invokers.begin(); iter != m_Invokers.end(); )
		{
			if ( iter->m_Caller.m_Static == a_Static )
			{
				iter = m_Invokers.erase( iter );
			}
			else
			{
				++iter;
			}
		}
	}

	template < typename Object >
	void operator -=( Return( Object::*a_Member )( Args... ) )
	{
		for ( auto iter = m_Invokers.begin(); iter != m_Invokers.end(); )
		{
			if ( *iter == a_Member )
			{
				iter = m_Invokers.erase( iter );
			}
			else
			{
				++iter;
			}
		}
	}

	template < typename Object, typename = typename std::enable_if< std::is_class_v< Object > > >
	void operator -=( Object& a_Object )
	{
		for ( auto iter = m_Invokers.begin(); iter != m_Invokers.end(); )
		{
			if ( iter->m_Object == &a_Object )
			{
				iter = m_Invokers.erase( iter );
			}
			else
			{
				++iter;
			}
		}
	}

	template < typename Object, typename = typename std::enable_if< std::is_class_v< Object > > >
	void operator -=( Object* a_Object )
	{
		for ( auto iter = m_Invokers.begin(); iter != m_Invokers.end(); )
		{
			if ( iter->m_Object == a_Object )
			{
				iter = m_Invokers.erase( iter );
			}
			else
			{
				++iter;
			}
		}
	}

	const Invoker< Return, Args... >& operator []( int a_Index ) const
	{
		return m_Invokers[ a_Index ];
	}

	void operator ()( const Args&... a_Args ) const
	{
		for ( auto iterInvokers = m_Invokers.begin(); iterInvokers != m_Invokers.end(); ++iterInvokers )
		{
			iterInvokers->Invoke( a_Args... );
		}
	}

	Return Invoke( int a_Index, const Args&... a_Args ) const
	{
		auto iterInvokers = m_Invokers.begin();
		std::advance( iterInvokers, a_Index );
		return ( *iterInvokers ).Invoke( a_Args... );
	}

	void InvokeAll( std::vector< Return >& a_Return, const Args&... a_Args ) const
	{
		if ( a_Return.size() < m_Invokers.size() )
		{
			a_Return.resize( m_Invokers.size() );
		}
		else
		{
			a_Return.clear();
		}

		auto iterReturns = a_Return.begin();
		for ( auto iterInvokers = m_Invokers.begin(); iterInvokers != m_Invokers.end(); ++iterInvokers, ++iterReturns )
		{
			*iterReturns = iterInvokers->Invoke( a_Args... );
		}
	}

	void InvokeAll( const Args&... a_Args ) const
	{
		for ( auto iterInvokers = m_Invokers.begin(); iterInvokers != m_Invokers.end(); ++iterInvokers )
		{
			iterInvokers->Invoke( a_Args... );
		}
	}

	int Count() const
	{
		return m_Invokers.size();
	}

	const std::list< Invoker< Return, Args... > >&
		GetInvocationList() const
	{
		return m_Invokers;
	}

private:

	std::list< Invoker< Return, Args... > > m_Invokers;
};

/// <summary>
/// Can store a list of invokers of the same type footprint. All stored functions can be
/// called together or seperately.
/// </summary>
/// <typeparam name="...Args">Argument types for stored functions.</typeparam>
template < typename... Args >
class Delegate< void, Args... >
{
	template < typename T >
	static constexpr bool m_IsInvoker = false;

	template < typename... Args >
	static constexpr bool m_IsInvoker< Invoker< Args... > > = true;

	template < typename... Args >
	static constexpr bool m_IsInvoker< Action< Args... > > = true;

	template < typename... Args >
	static constexpr bool m_IsInvoker< Predicate< Args... > > = true;

public:

	Delegate() = default;

	Delegate( const Invoker< void, Args... >& a_Invoker )
	{
		m_Invokers.emplace_back( a_Invoker );
	}

	Delegate( const Delegate< void, Args... >& a_Delegate )
	{
		m_Invokers.assign( a_Delegate.m_Invokers.begin(), a_Delegate.m_Invokers.end() );
	}

	Delegate( void( *a_Static )( Args... ) )
	{
		m_Invokers.emplace_back( Invoker< void, Args... >( a_Static ) );
	}

	void operator =( const Invoker< void, Args... >& a_Invoker )
	{
		m_Invokers.clear();
		m_Invokers.emplace_back( a_Invoker );
	}

	void operator =( const Delegate< void, Args... >& a_Delegate )
	{
		m_Invokers.clear();
		m_Invokers.assign( a_Delegate.m_Invokers.begin(), a_Delegate.m_Invokers.end() );
	}

	void operator =( void( *a_Static )( Args... ) )
	{
		m_Invokers.clear();
		m_Invokers.emplace_back( Invoker( a_Static ) );
	}

	void operator +=( const Invoker< void, Args... >& a_Invoker )
	{
		m_Invokers.emplace_back( a_Invoker );
	}

	void operator +=( const Delegate< void, Args... >& a_Delegate )
	{
		std::copy( a_Delegate.m_Invokers.begin(), a_Delegate.m_Invokers.end(),
				   std::back_insert_iterator( m_Invokers ) );
	}

	void operator +=( void( *a_Static )( Args... ) )
	{
		m_Invokers.emplace_back( Invoker( a_Static ) );
	}

	void operator -=( const Invoker< void, Args... >& a_Invoker )
	{
		for ( auto iter = m_Invokers.begin(); iter != m_Invokers.end(); )
		{
			if ( *iter == a_Invoker )
			{
				iter = m_Invokers.erase( iter );
			}
			else
			{
				++iter;
			}
		}
	}

	void operator -=( void( *a_Static )( Args... ) )
	{
		for ( auto iter = m_Invokers.begin(); iter != m_Invokers.end(); )
		{
			if ( iter->m_Caller.m_Static == a_Static )
			{
				iter = m_Invokers.erase( iter );
			}
			else
			{
				++iter;
			}
		}
	}

	template < typename Object >
	void operator -=( void( Object::* a_Member )( Args... ) )
	{
		for ( auto iter = m_Invokers.begin(); iter != m_Invokers.end(); )
		{
			if ( *iter == a_Member )
			{
				iter = m_Invokers.erase( iter );
			}
			else
			{
				++iter;
			}
		}
	}

	template < typename Object, typename = typename std::enable_if< std::is_class_v< Object > > >
	void operator -=( Object& a_Object )
	{
		for ( auto iter = m_Invokers.begin(); iter != m_Invokers.end(); )
		{
			if ( iter->m_Object == &a_Object )
			{
				iter = m_Invokers.erase( iter );
			}
			else
			{
				++iter;
			}
		}
	}

	template < typename Object, typename = typename std::enable_if< std::is_class_v< Object > > >
	void operator -=( Object* a_Object )
	{
		for ( auto iter = m_Invokers.begin(); iter != m_Invokers.end(); )
		{
			if ( iter->m_Object == a_Object )
			{
				iter = m_Invokers.erase( iter );
			}
			else
			{
				++iter;
			}
		}
	}

	const Invoker< void, Args... >& operator []( int a_Index ) const
	{
		return m_Invokers[ a_Index ];
	}

	void operator ()( const Args&... a_Args ) const
	{
		for ( auto iterInvokers = m_Invokers.begin(); iterInvokers != m_Invokers.end(); ++iterInvokers )
		{
			iterInvokers->Invoke( a_Args... );
		}
	}

	void Invoke( int a_Index, const Args&... a_Args ) const
	{
		auto iterInvokers = m_Invokers.begin();
		std::advance( iterInvokers, a_Index );
		( *iterInvokers ).Invoke( a_Args... );
	}

	void InvokeAll( const Args&... a_Args ) const
	{
		for ( auto iterInvokers = m_Invokers.begin(); iterInvokers != m_Invokers.end(); ++iterInvokers )
		{
			iterInvokers->Invoke( a_Args... );
		}
	}

	int Count() const
	{
		return m_Invokers.size();
	}

	const std::list< Invoker< void, Args... > >
		GetInvocationList() const
	{
		return m_Invokers;
	}

private:

	std::list< Invoker< void, Args... > > m_Invokers;
};

/// <summary>
/// Can store a static, member or lambda function with a void return type.
/// </summary>
/// <typeparam name="...Args">Argument types for stored function.</typeparam>
template < typename... Args >
class Action : public Invoker< void, Args... >
{
public:
	inline Action() : Invoker< void, Args... >() { }

	template < typename T >
	inline Action( const T& a_Object ) : Invoker< void, Args... >( a_Object ) { }

	template < typename T, typename U >
	inline Action( const T& a_Object1, const U& a_Object2 ) : Invoker< void, Args... >( a_Object1, a_Object2 ) { }

	template < typename T >
	inline void operator =( const T& a_Object )
	{
		Invoker< void, Args... >::operator =( a_Object );
	}

	inline void operator ()( const Args&... a_Args ) const
	{
		Invoker< void, Args... >( a_Args... );
	}

	template < typename T >
	inline bool operator ==( const T& a_Object ) const 
	{
		return Invoker< void, Args... >::operator ==( a_Object );
	}

	template < typename T >
	inline bool operator !=( const T& a_Object ) const
	{
		return Invoker< void, Args... >::operator !=( a_Object );
	}
};

/// <summary>
/// Can store a static, member or lambda function with a bool return type.
/// </summary>
/// <typeparam name="...Args">Argument types for stored function.</typeparam>
template < typename... Args >
class Predicate : public Invoker< bool, Args... >
{
public:
	inline Predicate() : Invoker< bool, Args... >() { }

	template < typename T >
	inline Predicate( const T& a_Object ) : Invoker< bool, Args... >( a_Object ) { }

	template < typename T, typename U >
	inline Predicate( const T& a_Object1, const U& a_Object2 ) : Invoker< bool, Args... >( a_Object1, a_Object2 ) { }

	template < typename T >
	inline void operator =( const T& a_Object )
	{
		Invoker< bool, Args... >::operator =( a_Object );
	}

	inline bool operator ()( const Args&... a_Args ) const
	{
		return Invoker< bool, Args... >::operator ()( a_Args... );
	}

	template < typename T >
	inline bool operator ==( const T& a_Object ) const
	{
		return Invoker< bool, Args... >::operator ==( a_Object );
	}

	template < typename T >
	inline bool operator !=( const T& a_Object ) const
	{
		return Invoker< bool, Args... >::operator !=( a_Object );
	}
};
