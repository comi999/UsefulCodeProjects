#pragma once
#include <typeinfo>
#include <iterator>
#include <array>
#include <vector>
#include <list>
#include <forward_list>
#include <set>
#include <unordered_set>
#include <stack>
#include <deque>
#include <queue>
#include <map>
#include <unordered_map>

#include <string>
#include <sstream>
#include <iostream>
#include <regex>

namespace std
{
	template < typename Lambda                > struct clean_type { typedef Lambda type; };
	template < typename Lambda                > struct clean_type       < Lambda*                                  > : clean_type< Lambda > { };
	template < typename Lambda                > struct clean_type       < Lambda* const                            > : clean_type< Lambda > { };
	template < typename Lambda                > struct clean_type       < Lambda* volatile                         > : clean_type< Lambda > { };
	template < typename Lambda                > struct clean_type       < Lambda* const volatile                   > : clean_type< Lambda > { };
	template < typename Lambda                > struct clean_type       < const Lambda*                            > : clean_type< Lambda > { };
	template < typename Lambda                > struct clean_type       < volatile Lambda*                         > : clean_type< Lambda > { };
	template < typename Lambda                > struct clean_type       < const volatile Lambda*                   > : clean_type< Lambda > { };
	template < typename Lambda                > struct clean_type       < Lambda&                                  > : clean_type< Lambda > { };
	template < typename Lambda                > struct clean_type       < Lambda&&                                 > : clean_type< Lambda > { };
	template < typename Lambda                > struct is_pair                                                  : std::false_type { };
	template < typename... Lambda             > struct is_pair          < std::pair               < Lambda...    > > : std::true_type { };
	template < typename Lambda                > struct is_tuple                                                 : std::false_type { };
	template < typename... Lambda             > struct is_tuple         < std::tuple              < Lambda...    > > : std::true_type { };
	template < typename Lambda                > struct is_stl_container                                         : std::false_type { };
	template < typename Lambda, std::size_t N > struct is_stl_container < std::array              < Lambda, N    > > : std::true_type { };
	template < typename... Args          > struct is_stl_container < std::vector             < Args... > > : std::true_type { };
	template < typename... Args          > struct is_stl_container < std::deque              < Args... > > : std::true_type { };
	template < typename... Args          > struct is_stl_container < std::list               < Args... > > : std::true_type { };
	template < typename... Args          > struct is_stl_container < std::forward_list       < Args... > > : std::true_type { };
	template < typename... Args          > struct is_stl_container < std::set                < Args... > > : std::true_type { };
	template < typename... Args          > struct is_stl_container < std::multiset           < Args... > > : std::true_type { };
	template < typename... Args          > struct is_stl_container < std::map                < Args... > > : std::true_type { };
	template < typename... Args          > struct is_stl_container < std::multimap           < Args... > > : std::true_type { };
	template < typename... Args          > struct is_stl_container < std::unordered_set      < Args... > > : std::true_type { };
	template < typename... Args          > struct is_stl_container < std::unordered_multiset < Args... > > : std::true_type { };
	template < typename... Args          > struct is_stl_container < std::unordered_map      < Args... > > : std::true_type { };
	template < typename... Args          > struct is_stl_container < std::unordered_multimap < Args... > > : std::true_type { };
	template < typename... Args          > struct is_stl_container < std::stack              < Args... > > : std::true_type { };
	template < typename... Args          > struct is_stl_container < std::queue              < Args... > > : std::true_type { };
	template < typename... Args          > struct is_stl_container < std::priority_queue     < Args... > > : std::true_type { };

	template < typename Lambda, bool B = std::is_enum< Lambda >::value >
	struct is_scoped_enum : std::false_type { };
	
	template < typename Lambda >
	struct is_scoped_enum< Lambda, true > : std::integral_constant< bool, !std::is_convertible< Lambda, typename std::underlying_type< Lambda >::type >::value > { };

	template < typename Lambda, bool B = std::is_enum< Lambda >::value >
	struct is_unscoped_enum : std::false_type { };

	template < typename Lambda >
	struct is_unscoped_enum< Lambda, true > : std::integral_constant< bool, std::is_convertible< Lambda, typename std::underlying_type< Lambda >::type >::value > { };

	template < typename Lambda >
	struct unpack_type
	{
		static constexpr int type_count = 0;
		using First = void;
		using Last = void;
		using Tuple = std::tuple<>;

		template < int N >
		using Param = void;
	};

	template < template < typename... > class C, typename... Lambda >
	struct unpack_type< C < Lambda... > >
	{
		static constexpr int type_count = sizeof...( Lambda );
		using First = typename std::tuple_element< 0, std::tuple< Lambda... > >::type;
		using Last = typename std::tuple_element< type_count - 1, std::tuple< Lambda... > >::type;
		using Tuple = typename std::tuple< Lambda... >;

		template < int N >
		using Param = typename std::tuple_element< N, std::tuple< Lambda... > >::type;
	};

	template < typename Lambda, typename = void >
	struct is_iterable : std::false_type { };

	template < typename Lambda >
	struct is_iterable< Lambda, std::void_t< decltype( std::begin( std::declval< Lambda >() ) ),
		decltype( std::end( std::declval< Lambda >() ) ) > > : std::true_type { };

	class has_operator 
	{
		typedef char yes_type;
		typedef long no_type;

	public:

		template< typename Lambda >
		class add
		{
			template < typename U > static yes_type test( decltype( &U::operator+ ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class add_overload
		{
			template < typename U, typename = decltype( std::declval< U >() + std::declval< RHS >() ) >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS, typename Return >
		class add_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >() + std::declval< RHS >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class subtract
		{
			template < typename U > static yes_type test( decltype( &U::operator- ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class subtract_overload
		{
			template < typename U, typename = decltype( std::declval< U >() - std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS, typename Return >
		class subtract_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >() - std::declval< RHS >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class multiply
		{
			template < typename U, typename = decltype( std::declval< U >() * std::declval< U >() )>
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class multiply_overload
		{
			template < typename U, typename = decltype( std::declval< U >() * std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS, typename Return >
		class multiply_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >() * std::declval< RHS >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class divide
		{
			template < typename U > static yes_type test( decltype( &U::operator/ ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class divide_overload
		{
			template < typename U, typename = decltype( std::declval< U >() / std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS, typename Return >
		class divide_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >() / std::declval< RHS >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class modulo
		{
			template < typename U > static yes_type test( decltype( &U::operator% ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class modulo_overload
		{
			template < typename U, typename = decltype( std::declval< U >() % std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS, typename Return >
		class modulo_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >() % std::declval< RHS >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template < typename Lambda >
		class pre_increment
		{
			template < typename U, typename = decltype( ++std::declval< U >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template < typename Lambda, typename Return >
		class pre_increment_ret
		{
			typedef typename std::clean_type< Return >::type CleanRet;
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::clean_type< 
				decltype( ++std::declval< U >() ) >::type, CleanRet >::value, void >::type >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template < typename Lambda >
		class post_increment
		{
			template < typename U > static yes_type test( decltype( std::declval< U >()++ ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template < typename Lambda, typename Return >
		class post_increment_ret
		{
			typedef typename std::clean_type< Return >::type CleanRet;
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::clean_type< 
				decltype( std::declval< U >()++ ) >::type, CleanRet >::value, void >::type >
			static yes_type test( decltype( &U::operator++ ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template < typename Lambda >
		class pre_decrement
		{
			template < typename U, typename = decltype( --std::declval< U >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template < typename Lambda, typename Return >
		class pre_decrement_ret
		{
			typedef typename std::clean_type< Return >::type CleanRet;
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::clean_type< 
				decltype( --std::declval< U >() ) >::type, CleanRet >::value, void >::type >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template < typename Lambda >
		class post_decrement
		{
			template < typename U > static yes_type test( decltype( std::declval< U >()-- ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template < typename Lambda, typename Return >
		class post_decrement_ret
		{
			typedef typename std::clean_type< Return >::type CleanRet;
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::clean_type< 
				decltype( std::declval< U >()-- ) >::type, CleanRet >::value, void >::type >
			static yes_type test( decltype( &U::operator++ ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class equality
		{
			template < typename U > static yes_type test( decltype( &U::operator== ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class equality_overload
		{
			template < typename U, typename = decltype( std::declval< U >() == std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS, typename Return >
		class equality_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >() == std::declval< RHS >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class inequality
		{
			template < typename U > static yes_type test( decltype( &U::operator!= ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class inequality_overload
		{
			template < typename U, typename = decltype( std::declval< U >() != std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS, typename Return >
		class inequality_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >() != std::declval< RHS >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class greater_than
		{
			template < typename U > static yes_type test( decltype( &U::operator> ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class greater_than_overload
		{
			template < typename U, typename = decltype( std::declval< U >() > std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS, typename Return >
		class greater_than_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >() > std::declval< RHS >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class less_than
		{
			template < typename U > static yes_type test( decltype( &U::operator< ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class less_than_overload
		{
			template < typename U, typename = decltype( std::declval< U >() < std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS, typename Return >
		class less_than_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >() < std::declval< RHS >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template < typename Lambda >
		class greater_equal_than
		{
			template < typename U > static yes_type test( decltype( &U::operator>= ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template < typename Lambda, typename RHS >
		class greater_equal_than_overload
		{
			template < typename U, typename = decltype( std::declval< U >() >= std::declval< RHS >() ) >
				static yes_type test( const U&& );
				template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template < typename Lambda, typename RHS, typename Return >
		class greater_equal_than_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >() >= std::declval< RHS >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template < typename Lambda >
		class less_equal_than
		{
			template < typename U > static yes_type test( decltype( &U::operator<= ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template < typename Lambda, typename RHS >
		class less_equal_than_overload
		{
			template < typename U, typename = decltype( std::declval< U >() <= std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS, typename Return >
		class less_equal_than_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >() <= std::declval< RHS >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template < typename Lambda >
		class assign
		{
			template < typename U, typename = decltype( &U::operator= ) > static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class assign_overload
		{
			template < typename U, typename = decltype( std::declval< U >() = std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class assign_add
		{
			template < typename U > static yes_type test( decltype( &U::operator+= ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class assign_add_overload
		{
			template < typename U, typename = decltype( std::declval< U >() += std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};
		// Up To Here
		template< typename Lambda >
		class assign_subtract
		{
			template < typename U > static yes_type test( decltype( &U::operator-= ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class assign_subtract_overload
		{
			template < typename U, typename = decltype( std::declval< U >() -= std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class assign_multiply
		{
			template < typename U > static yes_type test( decltype( &U::operator*= ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class assign_multiply_overload
		{
			template < typename U, typename = decltype( std::declval< U >() *= std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class assign_divide
		{
			template < typename U > static yes_type test( decltype( &U::operator/= ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class assign_divide_overload
		{
			template < typename U, typename = decltype( std::declval< U >() /= std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class assign_modulo
		{
			template < typename U > static yes_type test( decltype( &U::operator%= ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class assign_modulo_overload
		{
			template < typename U, typename = decltype( std::declval< U >() %= std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class assign_binary_and
		{
			template < typename U > static yes_type test( decltype( &U::operator&= ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class assign_binary_and_overload
		{
			template < typename U, typename = decltype( std::declval< U >() &= std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class assign_binary_or
		{
			template < typename U > static yes_type test( decltype( &U::operator|= ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class assign_binary_or_overload
		{
			template < typename U, typename = decltype( std::declval< U >() |= std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class assign_binary_xor
		{
			template < typename U > static yes_type test( decltype( &U::operator^= ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class assign_binary_xor_overload
		{
			template < typename U, typename = decltype( std::declval< U >() ^= std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class assign_binary_shift_left
		{
			template < typename U > static yes_type test( decltype( &U::operator<<= ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class assign_binary_shift_left_overload
		{
			template < typename U, typename = decltype( std::declval< U >() <<= std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class assign_binary_shift_right
		{
			template < typename U > static yes_type test( decltype( &U::operator>>= ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class assign_binary_shift_right_overload
		{
			template < typename U, typename = decltype( std::declval< U >() >>= std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};
		// Up to here
		template< typename Lambda >
		class logical_and
		{
			template < typename U > static yes_type test( decltype( &U::operator&& ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class logical_and_overload
		{
			template < typename U, typename = decltype( std::declval< U >() && std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS, typename Return >
		class logical_and_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >() && std::declval< RHS >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class logical_or
		{
			template < typename U > static yes_type test( decltype( &U::operator|| ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class logical_or_overload
		{
			template < typename U, typename = decltype( std::declval< U >() || std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS, typename Return >
		class logical_or_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >() || std::declval< RHS >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class logical_not
		{
			template < typename U > static yes_type test( decltype( &U::operator! ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename Return >
		class logical_not_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( !std::declval< U >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};
		// Up to here
		template< typename Lambda, typename Arg = int >
		class binary_and
		{
			template <  typename U, typename = decltype( std::declval< U >() & std::declval< Arg >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class binary_and_overload
		{
			template < typename U, typename = decltype( std::declval< U >() || std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS, typename Return >
		class binary_and_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >() & std::declval< RHS >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class binary_or
		{
			template < typename U > static yes_type test( decltype( &U::operator| ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class binary_or_overload
		{
			template < typename U, typename = decltype( std::declval< U >() | std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS, typename Return >
		class binary_or_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >() | std::declval< RHS >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class binary_xor
		{
			template < typename U > static yes_type test( decltype( &U::operator^ ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class binary_xor_overload
		{
			template < typename U, typename = decltype( std::declval< U >() ^ std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS, typename Return >
		class binary_xor_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >() ^ std::declval< RHS >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class binary_ones_compliment
		{
			template < typename U > static yes_type test( decltype( &U::operator^ ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename Return >
		class binary_ones_compliment_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( ~std::declval< U >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class binary_shift_left
		{
			template < typename U > static yes_type test( decltype( &U::operator<< ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class binary_shift_left_overload
		{
			template < typename U, typename = decltype( std::declval< U >() << std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS, typename Return >
		class binary_shift_left_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >() << std::declval< RHS >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class binary_shift_right
		{
			template < typename U > static yes_type test( decltype( &U::operator>> ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class binary_shift_right_overload
		{
			template < typename U, typename = decltype( std::declval< U >() >> std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS, typename Return >
		class binary_shift_right_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >() >> std::declval< RHS >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class indirection
		{
			template < typename U, typename = decltype( *std::declval< U >() )>
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename Return >
		class indirection_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( *std::declval< U >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class invoke
		{
			template < typename U > static yes_type test( decltype( &U::operator() ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename... Args >
		class invoke_overload
		{
			template < typename U, typename = decltype( std::declval< Lambda >()( std::declval< Args >()... ) ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename Return, typename... Args >
		class invoke_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >()( std::declval< Args >()... ) ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class subscript
		{
			template < typename U > static yes_type test( decltype( &U::operator[] ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename Ind >
		class subscript_overload
		{
			template < typename U, typename = decltype( std::declval< U >()[ std::declval< Ind >() ] ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename Ind, typename Return >
		class subscript_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >()[ std::declval< Ind >() ] ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class address_of
		{
			template < typename U, typename = decltype( &std::declval< U >() )>
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename Return >
		class address_of_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( &std::declval< U >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class member_access
		{
			template < typename U > static yes_type test( decltype( &U::operator-> ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename Return >
		class member_access_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >()-> ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda >
		class member_access_pointer
		{
			template < typename U > static yes_type test( decltype( &U::operator->* ) );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( 0 ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS >
		class member_access_pointer_overload
		{
			template < typename U, typename = decltype( std::declval< U >()->*std::declval< RHS >() ) >
			static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};

		template< typename Lambda, typename RHS, typename Return >
		class member_access_pointer_overload_ret
		{
			template < typename U, typename = typename std::enable_if< std::is_same< typename std::remove_reference<
				decltype( std::declval< U >()->*std::declval< RHS >() ) >::type, Return >::value, void >::type >
				static yes_type test( const U&& );
			template < typename U > static no_type test( ... );
		public:
			static constexpr bool value = sizeof( test< Lambda >( std::declval< Lambda >() ) ) == sizeof( yes_type );
		};
	};
}

class Type
{
public:

	enum class Specifier
	{
		PRIMITIVE,
		FUNCTION,
		UNION,
		STRUCT,
		CLASS,
		ENUM,
		ENUM_CLASS
	};

	enum class Trait
	{
		POD,
		FUNDAMENTAL,
		INTEGRAL,
		FLOATING_POINT,
		SIGNED,
		UNSIGNED,
		AGGREGATE,
		UNSCOPED_ENUM,
		STANDARD_LAYOUT,
		EMPTY,
		FINAL,
		ABSTRACT,
		POLYMORPHIC,
		TRIVIAL,
		ITERABLE,
		INVOCABLE,
		TUPLE,
		PAIR,
		STL_CONTAINER,
		VOID,
		NULL_POINTER,
		HAS_ADD,
		HAS_SUBTRACT,
		HAS_MULTIPLY,
		HAS_DIVIDE,
		HAS_MODULO,
		HAS_PRE_INCREMENT,
		HAS_PRE_DECREMENT,
		HAS_POST_INCREMENET,
		HAS_POST_DECREMENT,
		HAS_EQUALITY,
		HAS_INEQUALITY,
		HAS_GREATER,
		HAS_GREATER_EQUAL,
		HAS_LESS,
		HAS_LESS_EQUAL,
		HAS_LOGIC_AND,
		HAS_LOGIC_OR,
		HAS_LOGIC_NOT,
		HAS_BINARY_AND,
		HAS_BINARY_OR,
		HAS_BINARY_XOR,
		HAS_BINARY_ONES_COMPLIMENT,
		HAS_BINARY_SHIFT_LEFT,
		HAS_BINARY_SHIFT_RIGHT,
		HAS_ASSIGN,
		HAS_ASSIGN_ADD,
		HAS_ASSIGN_SUBTRACT,
		HAS_ASSIGN_MULTIPLY,
		HAS_ASSIGN_DIVIDE,
		HAS_ASSIGN_MODULO,
		HAS_ASSIGN_BINARY_AND,
		HAS_ASSIGN_BINARY_OR,
		HAS_ASSIGN_BINARY_XOR,
		HAS_ASSIGN_BINARY_SHIFT_LEFT,
		HAS_ASSIGN_BINARY_SHIFT_RIGHT,
		HAS_INDIRECTION,
		HAS_INVOKE,
		HAS_SUBSCRIPT,
		HAS_ADDRESS_OF,
		HAS_MEMBER_ACCESS,
		HAS_MEMBER_ACCESS_POINTER
	};

	template < typename Lambda, typename CleanType = typename std::clean_type< Lambda >::type >
	static const Type& GetType()
	{
		unsigned int hash = typeid( CleanType ).hash_code();
		auto result = m_TypesMap.find( hash );

		if ( result == m_TypesMap.end() )
		{
			const Type* newType = GenerateNew< Lambda >();
			m_TypesMap[ hash ] = newType;
			m_TypesSet.insert( newType );
			return *m_TypesMap[ hash ];
		}

		return *result->second;
	}

	static const std::set< const Type*, Type >& GetCache()
	{
		return m_TypesSet;
	}

	#pragma region Non-Static Functions
	///////////////////////////////////

	inline const std::string& Name() const
	{
		return m_Name;
	}

	inline const std::string& TemplatedName() const
	{
		return m_TemplatedName;
	}

	inline const char* FullName() const
	{
		return m_TypeInfo->name();
	}

	inline const char* RawName() const
	{
		return m_TypeInfo->raw_name();
	}

	inline const std::string& Namespace() const
	{
		return m_Namespace;
	}

	inline unsigned int Hash() const
	{
		return m_Hash;
	}

	inline bool Before( const Type& a_OtherType ) const
	{
		return m_TypeInfo->before( *a_OtherType.m_TypeInfo );
	}

	inline bool Before( const Type* a_OtherType ) const
	{
		return m_TypeInfo->before( *a_OtherType->m_TypeInfo );
	}

	inline const std::type_info& TypeInfo() const
	{
		return *m_TypeInfo;
	}

	inline const Type* UnderlyingType() const
	{
		return m_UnderlyingType;
	}

	inline unsigned int ParamaterCount() const
	{
		return m_TemplateParameters->size();
	}

	inline const Type* GetParameter( int a_Index ) const
	{
		if ( m_TemplateParameters == nullptr || a_Index < 0 || a_Index >= m_TemplateParameters->size() )
		{
			return nullptr;
		}

		return m_TemplateParameters->at( a_Index );
	}

	inline Specifier GetSpecifier() const
	{
		return m_SpecifierType;
	}

	std::string GetPrintout( int a_Indent = 0 ) const
	{
		std::string printOut;

		auto indent = [&]( int indentCount ) -> void
		{
			for ( int i = 0; i < indentCount; ++i )
			{
				printOut += "    ";
			}
		};

		indent( a_Indent );
		printOut += "----" + m_Name + "----" + '\n';
		indent( a_Indent );
		printOut += "{\n";

		for ( int i = 0; m_TemplateParameters != nullptr && i < m_TemplateParameters->size(); ++i )
		{
			printOut += m_TemplateParameters->at( i )->GetPrintout( a_Indent + 1 );
		}

		indent( a_Indent );
		printOut += "}\n";

		return printOut;
	}

	void Print() const 
	{
		std::cout << GetPrintout() << std::endl;
	}

	inline bool IsPod() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::POD ) );
	}

	inline bool IsIntegral() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::INTEGRAL ) );
	}

	inline bool IsDecimal() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::FLOATING_POINT ) );
	}

	inline bool IsArithmetic() const
	{
		return IsIntegral() || IsDecimal();
	}

	inline bool IsFundamental() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::FUNDAMENTAL ) );
	}

	inline bool IsCompound() const
	{
		return !IsFundamental();
	}

	inline bool IsScalar() const
	{
		return IsArithmetic() || IsEnum() || IsNullPointer();
	}

	inline bool IsSigned() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::SIGNED ) );
	}

	inline bool IsUnsigned() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::UNSIGNED ) );
	}

	inline bool IsAggregate() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::AGGREGATE ) );
	}

	inline bool IsEnum() const
	{
		return m_SpecifierType == Specifier::ENUM || m_SpecifierType == Specifier::ENUM_CLASS;
	}

	inline bool IsUnscopedEnum() const
	{
		return m_SpecifierType == Specifier::ENUM;
	}

	inline bool IsScopedEnum() const
	{
		return m_SpecifierType == Specifier::ENUM_CLASS;
	}

	inline bool IsStruct() const
	{
		return m_SpecifierType == Specifier::STRUCT;
	}

	inline bool IsClass() const
	{
		return m_SpecifierType == Specifier::CLASS;
	}

	inline bool IsUnion() const
	{
		return m_SpecifierType == Specifier::UNION;
	}

	inline bool IsFunction() const
	{
		return m_SpecifierType == Specifier::FUNCTION;
	}

	inline bool IsPrimitive() const
	{
		return m_SpecifierType == Specifier::PRIMITIVE;
	}

	inline bool IsStandardLayout() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::STANDARD_LAYOUT ) );
	}
	
	inline bool IsEmpty() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::EMPTY ) );
	}

	inline bool IsFinal() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::FINAL ) );
	}

	inline bool IsAbstract() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::ABSTRACT ) );
	}

	inline bool IsPolymorphic() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::POLYMORPHIC ) );
	}

	inline bool IsTrivial() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::TRIVIAL ) );
	}

	inline bool IsIterable() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::ITERABLE ) );
	}

	inline bool IsInvocable() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::INVOCABLE ) );
	}

	inline bool IsTuple() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::TUPLE ) );
	}

	inline bool IsPair() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::PAIR ) );
	}

	inline bool IsSTLContainer() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::STL_CONTAINER ) );
	}

	inline bool IsVoid() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::VOID ) );
	}

	inline bool IsNullPointer() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::NULL_POINTER ) );
	}

	inline bool HasAdd() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_ADD ) );
	}

	inline bool HasSubtract() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_SUBTRACT ) );
	}

	inline bool HasMultiply() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_MULTIPLY ) );
	}

	inline bool HasDivide() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_DIVIDE ) );
	}

	inline bool HasModulo() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_MODULO ) );
	}

	inline bool HasPreIncrement() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_PRE_INCREMENT ) );
	}

	inline bool HasPreDecrement() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_PRE_DECREMENT ) );
	}

	inline bool HasPostIncrement() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_POST_INCREMENET ) );
	}

	inline bool HasPostDecrement() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_POST_DECREMENT ) );
	}

	inline bool HasEquality() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_EQUALITY ) );
	}

	inline bool HasInequality() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_INEQUALITY ) );
	}

	inline bool HasGreaterThan() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_GREATER ) );
	}

	inline bool HasGreaterEqualThan() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_GREATER_EQUAL ) );
	}

	inline bool HasLessThan() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_LESS ) );
	}

	inline bool HasLessEqualThan() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_LESS_EQUAL ) );
	}

	inline bool HasLogicAND() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_LOGIC_AND ) );
	}

	inline bool HasLogicOR() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_LOGIC_OR ) );
	}

	inline bool HasLogicNOT() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_LOGIC_NOT ) );
	}

	inline bool HasBinaryAND() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_BINARY_AND ) );
	}

	inline bool HasBinaryOR() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_BINARY_OR ) );
	}

	inline bool HasBinaryXOR() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_BINARY_XOR ) );
	}

	inline bool HasBinaryOnesCompliment() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_BINARY_ONES_COMPLIMENT ) );
	}

	inline bool HasBinaryShiftLeft() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_BINARY_SHIFT_LEFT ) );
	}

	inline bool HasBinaryShiftRight() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_BINARY_SHIFT_RIGHT ) );
	}

	inline bool HasAssign() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_ASSIGN ) );
	}

	inline bool HasAssignAdd() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_ASSIGN_ADD ) );
	}

	inline bool HasAssignSubtract() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_ASSIGN_SUBTRACT ) );
	}

	inline bool HasAssignMultiply() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_ASSIGN_MULTIPLY ) );
	}

	inline bool HasAssignDivide() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_ASSIGN_DIVIDE ) );
	}

	inline bool HasAssignModulo() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_ASSIGN_MODULO ) );
	}

	inline bool HasAssignBinaryAND() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_ASSIGN_BINARY_AND ) );
	}

	inline bool HasAssignBinaryOR() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_ASSIGN_BINARY_OR ) );
	}

	inline bool HasAssignBinaryXOR() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_ASSIGN_BINARY_XOR ) );
	}

	inline bool HasAssignBinaryShiftLeft() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_ASSIGN_BINARY_SHIFT_LEFT ) );
	}

	inline bool HasAssignBinaryShiftRight() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_ASSIGN_BINARY_SHIFT_RIGHT ) );
	}

	inline bool HasIndirection() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_INDIRECTION ) );
	}

	inline bool HasInvoke() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_INVOKE ) );
	}

	inline bool HasSubscript() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_SUBSCRIPT ) );
	}

	inline bool HasAddressOf() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_ADDRESS_OF ) );
	}

	inline bool HasMemberAccess() const
	{
		return m_Flags & ( ( uint64_t )1 << static_cast< char >( Trait::HAS_MEMBER_ACCESS ) );
	}

	inline bool HasMemberAccessPointer() const
	{
		return m_Flags & ( (uint64_t)1 << static_cast< char >( Trait::HAS_MEMBER_ACCESS_POINTER ) );
	}
	
	///////////////////////////////////
	#pragma endregion

	#pragma region Static Functions
	///////////////////////////////////
	
	template < typename Lambda >
	inline static constexpr bool IsAbstract()
	{
		return std::is_abstract_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsAggregate()
	{
		return std::is_aggregate_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsArray()
	{
		return std::is_array_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsArithmetic()
	{
		return std::is_arithmetic_v< Lambda >;
	}

	template < typename From, typename To >
	inline static constexpr bool IsAssignable()
	{
		return std::is_assignable_v< To, From >;
	}
	
	template < typename Base, typename Derived >
	inline static constexpr bool IsBaseOf()
	{
		return std::is_base_of_v< Base, Derived >;
	}
	
	template < typename Lambda >
	inline static constexpr bool IsCompound()
	{
		return std::is_compound_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsConst()
	{
		return std::is_const_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsConstructible()
	{
		return std::is_constructible_v< Lambda >;
	}

	template < typename From, typename To >
	inline static constexpr bool IsConvertible()
	{
		return std::is_convertible_v< From, To >;
	}
	
	template < typename Lambda >
	inline static constexpr bool IsCopyAssignable()
	{
		return std::is_copy_assignable_v< Lambda >;
	}
	
	template < typename Lambda >
	inline static constexpr bool IsCopyConstructible()
	{
		return std::is_copy_constructible_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsDecimal()
	{
		return std::is_floating_point_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsDefaultConstructible()
	{
		return std::is_default_constructible_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsDestructible()
	{
		return std::is_destructible_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsEmpty()
	{
		return std::is_empty_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsEnum()
	{
		return std::is_enum_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsFinal()
	{
		return std::is_final_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsFunction()
	{
		return std::is_function_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsFundamental()
	{
		return std::is_fundamental_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsIntegral()
	{
		return std::is_integral_v< Lambda >;
	}

	template < typename Lambda, typename... Args >
	inline static constexpr bool IsInvocable()
	{
		return std::is_invocable_v< Lambda, Args... >;
	}

	template < typename Lambda, typename Return, typename... Args >
	inline static constexpr bool IsInvocableR()
	{
		return std::is_invocable_r_v< Return, Lambda, Args... >;
	}

	template < typename Lambda >
	inline static constexpr bool IsIterable()
	{
		return std::is_iterable< Lambda >::value;
	}
	
	template < typename Lambda >
	inline static constexpr bool IsLValueReference()
	{
		return std::is_lvalue_reference_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsMemberFunctionPointer()
	{
		return std::is_member_function_pointer_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsMemberObjectPointer()
	{
		return std::is_member_object_pointer_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsMemberPointer()
	{
		return std::is_member_pointer_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsMoveAssignable()
	{
		return std::is_move_assignable< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsMoveConstructivle()
	{
		return std::is_move_constructible_v< Lambda >;
	}
	
	template < typename Lambda >
	inline static constexpr bool IsNullPointer()
	{
		return std::is_null_pointer_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsObject()
	{
		return std::is_object_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsPair()
	{
		return std::is_pair< Lambda >::value;
	}

	template < typename Lambda >
	inline static constexpr bool IsPod()
	{
		return std::is_pod_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsPointer()
	{
		return std::is_pointer_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsPolymorphic()
	{
		return std::is_polymorphic_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsReference()
	{
		return std::is_reference_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsRValueReference()
	{
		return std::is_rvalue_reference_v< Lambda >;
	}

	template < typename Lambda, typename U >
	inline static constexpr bool IsSameAs()
	{
		return std::is_same_v< Lambda, U >;
	}

	template < typename Lambda >
	inline static constexpr bool IsScalar()
	{
		return std::is_scalar_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsScopedEnum()
	{
		return std::is_scoped_enum< Lambda >::value;
	}

	template < typename Lambda >
	inline static constexpr bool IsSigned()
	{
		return std::is_signed_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsStandardLayout()
	{
		return std::is_standard_layout_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsSTLContainer()
	{
		return std::is_stl_container< Lambda >::value;
	}

	template < typename Lambda >
	inline static constexpr bool IsStructOrClass()
	{
		return std::is_class_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsSwappable()
	{
		return std::is_swappable_v< Lambda >;
	}

	template < typename Lambda, typename U >
	inline static constexpr bool IsSwappableWith()
	{
		return std::is_swappable_with_v< Lambda, U >;
	}
	
	template < typename Lambda >
	inline static constexpr bool IsTrivial()
	{
		return std::is_trivial_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsTriviallyAssignable()
	{
		return std::is_trivially_assignable_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsTriviallyConstructible()
	{
		return std::is_trivially_constructible_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsTriviallyCopyable()
	{
		return std::is_trivially_copyable_v< Lambda >;
	}
	
	template < typename Lambda >
	inline static constexpr bool IsTriviallyCopyAssignable()
	{
		return std::is_trivially_copy_assignable_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsTriviallyCopyConstructible()
	{
		return std::is_trivially_copy_constructible_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsTriviallyDefaultConstructible()
	{
		return std::is_trivially_default_constructible_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsTriviallyDestructible()
	{
		return std::is_trivially_destructible_v< Lambda >;
	}
	
	template < typename Lambda >
	inline static constexpr bool IsTriviallyMoveAssignable()
	{
		return std::is_trivially_move_assignable_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsTriviallyMoveConstructible()
	{
		return std::is_trivially_move_constructible_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsTuple()
	{
		return std::is_tuple< Lambda >::value;
	}

	template < typename Lambda >
	inline static constexpr bool IsUnion()
	{
		return std::is_union_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsUnscopedEnum()
	{
		return std::is_unscoped_enum< Lambda >::value;
	}

	template < typename Lambda >
	inline static constexpr bool IsUnsigned()
	{
		return std::is_unsigned_v< Lambda >;
	}
	
	template < typename Lambda >
	inline static constexpr bool IsVirtuallyDestructible()
	{
		return std::has_virtual_destructor_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsVoid()
	{
		return std::is_void_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool IsVolatile()
	{
		return std::is_volatile_v< Lambda >;
	}
	
	template < typename Lambda >
	inline static constexpr int GetAllignmentOf()
	{
		return std::alignment_of_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr int GetRankOf()
	{
		return std::rank_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr int GetExtentOf()
	{
		return std::extent_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool HasUniqueObjectRepresentation()
	{
		return std::has_unique_object_representations_v< Lambda >;
	}

	template < typename Lambda >
	inline static constexpr bool HasAdd()
	{
		return std::has_operator::add< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasAddOverload()
	{
		return std::has_operator::add_overload< Lambda, RHS >::value;
	}

	template < typename Lambda, typename RHS, typename Return >
	inline static constexpr bool HasAddOverloadR()
	{
		return std::has_operator::add_overload_ret< Lambda, RHS, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasSubtract()
	{
		return std::has_operator::subtract< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasSubtractOverload()
	{
		return std::has_operator::subtract_overload< Lambda, RHS >::value;
	}

	template < typename Lambda, typename RHS, typename Return >
	inline static constexpr bool HasSubtractOverloadR()
	{
		return std::has_operator::subtract_overload_ret< Lambda, RHS, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasMultiply()
	{
		return std::has_operator::multiply< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasMultiplyOverload()
	{
		return std::has_operator::multiply_overload< Lambda, RHS >::value;
	}

	template < typename Lambda, typename RHS, typename Return >
	inline static constexpr bool HasMultiplyOverloadR()
	{
		return std::has_operator::multiply_overload_ret< Lambda, RHS, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasDivide()
	{
		return std::has_operator::divide< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasDivideOverload()
	{
		return std::has_operator::divide_overload< Lambda, RHS >::value;
	}

	template < typename Lambda, typename RHS, typename Return >
	inline static constexpr bool HasDivideOverloadR()
	{
		return std::has_operator::divide_overload_ret< Lambda, RHS, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasModulo()
	{
		return std::has_operator::modulo< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasModuloOverload()
	{
		return std::has_operator::modulo_overload< Lambda, RHS >::value;
	}

	template < typename Lambda, typename RHS, typename Return >
	inline static constexpr bool HasModulo()
	{
		return std::has_operator::modulo_overload_ret< Lambda, RHS, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasPreIncrement()
	{
		return std::has_operator::pre_increment< Lambda >::value;
	}

	template < typename Lambda, typename Return >
	inline static constexpr bool HasPreIncrementR()
	{
		return std::has_operator::pre_increment_ret< Lambda, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasPreDecrement()
	{
		return std::has_operator::pre_decrement< Lambda >::value;
	}

	template < typename Lambda, typename Return >
	inline static constexpr bool HasPreDecrementR()
	{
		return std::has_operator::pre_decrement_ret< Lambda, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasPostIncrement()
	{
		return std::has_operator::post_increment< Lambda >::value;
	}

	template < typename Lambda, typename Return >
	inline static constexpr bool HasPostIncrementR()
	{
		return std::has_operator::post_increment_ret< Lambda, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasPostDecrement()
	{
		return std::has_operator::post_decrement< Lambda >::value;
	}

	template < typename Lambda, typename Return >
	inline static constexpr bool HasPostDecrement()
	{
		return std::has_operator::post_decrement_ret< Lambda, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasEquality()
	{
		return std::has_operator::equality< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasEqualityOverload()
	{
		return std::has_operator::equality_overload< Lambda, RHS >::value;
	}

	template < typename Lambda, typename RHS, typename Return >
	inline static constexpr bool HasEqualityOverloadR()
	{
		return std::has_operator::equality_overload_ret< Lambda, RHS, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasInequality()
	{
		return std::has_operator::inequality< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasInequalityOverload()
	{
		return std::has_operator::inequality_overload< Lambda, RHS >::value;
	}

	template < typename Lambda, typename RHS, typename Return >
	inline static constexpr bool HasInequalityOverloadR()
	{
		return std::has_operator::inequality_overload_ret< Lambda, RHS, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasGreaterThan()
	{
		return std::has_operator::greater_than< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasGreaterThanOverload()
	{
		return std::has_operator::greater_than_overload< Lambda, RHS >::value;
	}

	template < typename Lambda, typename RHS, typename Return >
	inline static constexpr bool HasGreaterThanOverloadR()
	{
		return std::has_operator::greater_than_overload_ret< Lambda, RHS, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasGreaterEqualThan()
	{
		return std::has_operator::greater_equal_than< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasGreaterEqualThanOverload()
	{
		return std::has_operator::greater_equal_than_overload< Lambda, RHS >::value;
	}

	template < typename Lambda, typename RHS, typename Return >
	inline static constexpr bool HasGreaterEqualThanOverloadR()
	{
		return std::has_operator::greater_equal_than_overload_ret< Lambda, RHS, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasLessThan()
	{
		return std::has_operator::less_than< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasLessThanOverload()
	{
		return std::has_operator::less_than_overload< Lambda, RHS >::value;
	}

	template < typename Lambda, typename RHS, typename Return >
	inline static constexpr bool HasLessThanOverloadR()
	{
		return std::has_operator::less_than_overload_ret< Lambda, RHS, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasLessEqualThan()
	{
		return std::has_operator::less_equal_than< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasLessEqualThanOverload()
	{
		return std::has_operator::less_equal_than_overload< Lambda, RHS >::value;
	}

	template < typename Lambda, typename RHS, typename Return >
	inline static constexpr bool HasLessEqualThanOverloadR()
	{
		return std::has_operator::less_equal_than_overload_ret< Lambda, RHS, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasLogicAND()
	{
		return std::has_operator::logical_and< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasLogicANDOverload()
	{
		return std::has_operator::logical_and_overload< Lambda, RHS >::value;
	}

	template < typename Lambda, typename RHS, typename Return >
	inline static constexpr bool HasLogicANDOverloadR()
	{
		return std::has_operator::logical_and_overload_ret< Lambda, RHS, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasLogicOR()
	{
		return std::has_operator::logical_or< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasLogicOROverload()
	{
		return std::has_operator::logical_or_overload< Lambda, RHS >::value;
	}

	template < typename Lambda, typename RHS, typename Return >
	inline static constexpr bool HasLogicOROverloadR()
	{
		return std::has_operator::logical_or_overload_ret< Lambda, RHS, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasLogicNOT()
	{
		return std::has_operator::logical_not< Lambda >::value;
	}

	template < typename Lambda, typename Return >
	inline static constexpr bool HasLogicNOTR()
	{
		return std::has_operator::logical_not_ret< Lambda, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasBinaryAND()
	{
		return std::has_operator::binary_and< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasBinaryANDOverload()
	{
		return std::has_operator::binary_and_overload< Lambda, RHS >::value;
	}

	template < typename Lambda, typename RHS, typename Return >
	inline static constexpr bool HasBinaryANDOverloadR()
	{
		return std::has_operator::binary_and_overload_ret< Lambda, RHS, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasBinaryOR()
	{
		return std::has_operator::binary_or< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasBinaryOROverload()
	{
		return std::has_operator::binary_or_overload< Lambda, RHS >::value;
	}

	template < typename Lambda, typename RHS, typename Return >
	inline static constexpr bool HasBinaryOROverloadR()
	{
		return std::has_operator::binary_or_overload_ret< Lambda, RHS, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasBinaryXOR()
	{
		return std::has_operator::binary_xor< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasBinaryXOROverload()
	{
		return std::has_operator::binary_xor_overload< Lambda, RHS >::value;
	}

	template < typename Lambda, typename RHS, typename Return >
	inline static constexpr bool HasBinaryXOROverloadR()
	{
		return std::has_operator::binary_xor_overload_ret< Lambda, RHS, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasBinaryOnesCompliment()
	{
		return std::has_operator::binary_ones_compliment< Lambda >::value;
	}

	template < typename Lambda, typename Return >
	inline static constexpr bool HasBinaryOnesComplimentR()
	{
		return std::has_operator::binary_ones_compliment_ret< Lambda, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasBinaryShiftLeft()
	{
		return std::has_operator::binary_shift_left< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasBinaryShiftLeftOverload()
	{
		return std::has_operator::binary_shift_left_overload< Lambda, RHS >::value;
	}

	template < typename Lambda, typename RHS, typename Return >
	inline static constexpr bool HasBinaryShiftLeftOverloadR()
	{
		return std::has_operator::binary_shift_left_overload_ret< Lambda, RHS, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasBinaryShiftRight()
	{
		return std::has_operator::binary_shift_right< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasBinaryShiftRightOverload()
	{
		return std::has_operator::binary_shift_right_overload< Lambda, RHS >::value;
	}

	template < typename Lambda, typename RHS, typename Return >
	inline static constexpr bool HasBinaryShiftRightOverloadR()
	{
		return std::has_operator::binary_shift_right_overload_ret< Lambda, RHS, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasAssign()
	{
		return std::has_operator::assign< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasAssignOverload()
	{
		return std::has_operator::assign_overload< Lambda, RHS >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasAssignAdd()
	{
		return std::has_operator::assign_add< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasAssignAddOverload()
	{
		return std::has_operator::assign_add_overload< Lambda, RHS >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasAssignSubtract()
	{
		return std::has_operator::assign_subtract< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasAssignSubtractOverload()
	{
		return std::has_operator::assign_subtract_overload< Lambda, RHS >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasAssignMultiply()
	{
		return std::has_operator::assign_multiply< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasAssignMultiplyOverload()
	{
		return std::has_operator::assign_multiply_overload< Lambda, RHS >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasAssignDivide()
	{
		return std::has_operator::assign_divide< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasAssignDivideOverload()
	{
		return std::has_operator::assign_divide_overload< Lambda, RHS >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasAssignModulo()
	{
		return std::has_operator::assign_modulo< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasAssignModuloOverload()
	{
		return std::has_operator::assign_modulo_overload< Lambda, RHS >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasAssignBinaryAND()
	{
		return std::has_operator::assign_binary_and< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasAssignBinaryANDOverload()
	{
		return std::has_operator::assign_binary_and_overload< Lambda, RHS >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasAssignBinaryOR()
	{
		return std::has_operator::assign_binary_or< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasAssignBinaryOROverload()
	{
		return std::has_operator::assign_binary_or_overload< Lambda, RHS >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasAssignBinaryXOR()
	{
		return std::has_operator::assign_binary_xor< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasAssignBinaryXOROverload()
	{
		return std::has_operator::assign_binary_xor_overload< Lambda, RHS >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasAssignBinaryShiftLeft()
	{
		return std::has_operator::assign_binary_shift_left< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasAssignBinaryShiftLeftOverload()
	{
		return std::has_operator::assign_binary_shift_left_overload< Lambda, RHS >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasAssignBinaryShiftRight()
	{
		return std::has_operator::assign_binary_shift_right< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasAssignBinaryShiftRightOverload()
	{
		return std::has_operator::assign_binary_shift_right_overload< Lambda, RHS >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasIndirection()
	{
		return std::has_operator::indirection< Lambda >::value;
	}

	template < typename Lambda, typename Return >
	inline static constexpr bool HasIndirectionR()
	{
		return std::has_operator::indirection_ret< Lambda, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasInvoke()
	{
		return std::has_operator::invoke< Lambda >::value;
	}

	template < typename Lambda, typename... Args >
	inline static constexpr bool HasInvokeOverload()
	{
		return std::has_operator::invoke_overload< Lambda, Args... >::value;
	}

	template < typename Lambda, typename Return, typename... Args >
	inline static constexpr bool HasInvokeOverloadR()
	{
		return std::has_operator::invoke_overload_ret< Lambda, Return, Args... >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasSubscript()
	{
		return std::has_operator::subscript< Lambda >::value;
	}

	template < typename Lambda, typename Ind >
	inline static constexpr bool HasSubscriptOverload()
	{
		return std::has_operator::subscript_overload< Lambda, Ind >::value;
	}

	template < typename Lambda, typename Ind, typename Return >
	inline static constexpr bool HasSubscriptOverloadR()
	{
		return std::has_operator::subscript_overload_ret< Lambda, Ind, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasAddressOf()
	{
		return std::has_operator::address_of< Lambda >::value;
	}

	template < typename Lambda, typename Return >
	inline static constexpr bool HasAddressOfR()
	{
		return std::has_operator::address_of_ret< Lambda, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasMemberAccess()
	{
		return std::has_operator::member_access< Lambda >::value;
	}

	template < typename Lambda, typename Return >
	inline static constexpr bool HasMemberAccessR()
	{
		return std::has_operator::member_access_ret< Lambda, Return >::value;
	}

	template < typename Lambda >
	inline static constexpr bool HasMemberAccessPointer()
	{
		return std::has_operator::member_access_pointer< Lambda >::value;
	}

	template < typename Lambda, typename RHS >
	inline static constexpr bool HasMemberAccessPointerOverload()
	{
		return std::has_operator::member_access_pointer_overload< Lambda, RHS >::value;
	}

	template < typename Lambda, typename RHS, typename Return >
	inline static constexpr bool HasMemberAccessPointerOverloadR()
	{
		return std::has_operator::member_access_pointer_overload_ret< Lambda, RHS, Return >::value;
	}

	template < typename Lambda >
	inline static void PrintTypeInfo()
	{
		std::cout << typeid( Lambda ).name() << std::endl;
	}
	
	///////////////////////////////////
	#pragma endregion

	inline bool operator==( const Type& m_Right ) const
	{
		return m_Hash == m_Right.m_Hash;
	}

	inline bool operator!=( const Type& m_Right ) const
	{
		return m_Hash != m_Right.m_Hash;
	}

	inline bool operator==( const std::type_info& a_Right ) const
	{
		return m_Hash == a_Right.hash_code();
	}

	inline bool operator!=( const std::type_info& a_Right ) const
	{
		return m_Hash != a_Right.hash_code();
	}

	inline bool operator==( const unsigned int& a_Right ) const
	{
		return m_Hash == a_Right;
	}

	inline bool operator!=( const unsigned int& a_Right ) const
	{
		return m_Hash != a_Right;
	}

	inline bool operator() ( const Type* a_LHS, const Type* a_RHS ) const
	{
		return a_LHS->m_Name < a_RHS->m_Name;
	}
	
private:

	friend class std::set< const Type*, Type >;
	
	Type() : m_UnderlyingType( nullptr ), m_TemplateParameters( nullptr ) { }

	template < typename Lambda, typename CleanType = typename std::clean_type< Lambda >::type >
	static Type* GenerateNew()
	{
		// Assign preliminary information.
		Type* newType = new Type();
		newType->m_TypeInfo = &typeid( CleanType );
		newType->m_Hash = newType->m_TypeInfo->hash_code();
		newType->m_Flags = GenerateFlags< Lambda >();

		std::string fullName( newType->m_TypeInfo->name() );

		// Is Primitive
		if ( std::is_fundamental_v< CleanType > )
		{
			newType->m_SpecifierType = Specifier::PRIMITIVE;
			newType->m_Name = fullName;
		}

		// Is Function
		else if ( std::is_function_v< CleanType > )
		{
			newType->m_SpecifierType = Specifier::FUNCTION;
			newType->m_Name = fullName;
		}

		// Is Struct or Class
		else if ( std::is_class_v< CleanType > )
		{
			std::regex regex( "struct |class " );
			std::smatch match;
			std::regex_search( fullName, match, regex );

			if ( match[ 0 ] == "struct " )
			{
				newType->m_SpecifierType = Specifier::STRUCT;
			}
			else
			{
				newType->m_SpecifierType = Specifier::CLASS;
			}

			fullName = match.suffix().str();
			regex.assign( "<" );
			std::regex_search( fullName, match, regex );

			if ( match.size() != 0 )
			{
				fullName = match.prefix().str();
			}
			
			int index = fullName.size() - 1;
			for ( auto iter = fullName.rbegin(); iter != fullName.rend(); iter++ )
			{
				--index;
				if ( *iter == ':' )
				{
					break;
				}
			}

			if ( index != -1 )
			{
				newType->m_Namespace = fullName.substr( 0, index );
				newType->m_Name = fullName.substr( index + 2, fullName.size() - index + 1 );
			}
			else
			{
				newType->m_Name = fullName;
			}
		}

		// Is Union
		else if ( std::is_union_v< CleanType > )
		{
			newType->m_SpecifierType = Specifier::UNION;
			fullName.erase( 0, 6 );
			std::regex regex( "<" );
			std::smatch match;
			std::regex_search( fullName, match, regex );

			if ( match.size() != 0 )
			{
				fullName = match.prefix().str();
			}

			int index = fullName.size() - 1;
			for ( auto iter = fullName.rbegin(); iter != fullName.rend(); iter++ )
			{
				--index;
				if ( *iter == ':' )
				{
					break;
				}
			}

			if ( index != -1 )
			{
				newType->m_Namespace = fullName.substr( 0, index );
				newType->m_Name = fullName.substr( index + 2, fullName.size() - index + 1 );
			}
			else
			{
				newType->m_Name = fullName;
			}
		}

		// Is Enum Class
		else if ( std::is_scoped_enum< CleanType >::value )
		{
			newType->m_SpecifierType = Specifier::ENUM_CLASS;
			fullName.erase( 0, 5 );

			int index = fullName.size() - 1;
			for ( auto iter = fullName.rbegin(); iter != fullName.rend(); iter++ )
			{
				--index;
				if ( *iter == ':' )
				{
					break;
				}
			}

			if ( index != -1 )
			{
				newType->m_Namespace = fullName.substr( 0, index );
				newType->m_Name = fullName.substr( index + 2, fullName.size() - index + 1 );
			}
			else
			{
				newType->m_Name = fullName;
			}
		}

		// Is Enum
		else if ( std::is_unscoped_enum< CleanType >::value )
		{
			newType->m_SpecifierType = Specifier::ENUM;
			fullName.erase( 0, 5 );

			int index = fullName.size() - 1;
			for ( auto iter = fullName.rbegin(); iter != fullName.rend(); iter++ )
			{
				--index;
				if ( *iter == ':' )
				{
					break;
				}
			}

			if ( index != -1 )
			{
				newType->m_Namespace = fullName.substr( 0, index );
				newType->m_Name = fullName.substr( index + 2, fullName.size() - index + 1 );
			}
			else
			{
				newType->m_Name = fullName;
			}
		}
		
		ProcessArguments< typename std::unpack_type< CleanType >::Tuple >( newType );

		newType->m_TemplatedName = newType->m_Name + '<';

		if ( newType->m_TemplateParameters == nullptr )
		{
			newType->m_TemplatedName += '>';
			return newType;
		}
		
		for ( int i = 0; i < newType->m_TemplateParameters->size(); ++i )
		{
			newType->m_TemplatedName += newType->m_TemplateParameters->at( i )->m_Name + ( i == newType->m_TemplateParameters->size() - 1 ? "" : "," );
		}

		newType->m_TemplatedName += '>';

		return newType;
	}

	template < typename Lambda, typename CleanType = typename std::clean_type< Lambda >::type >
	static constexpr uint64_t GenerateFlags()
	{
		return 
			( static_cast< uint64_t >( std::is_pod                                  < CleanType >::value ) << static_cast< char >( Trait::POD                           ) ) |
			( static_cast< uint64_t >( std::is_fundamental                          < CleanType >::value ) << static_cast< char >( Trait::FUNDAMENTAL                   ) ) |
			( static_cast< uint64_t >( std::is_integral                             < CleanType >::value ) << static_cast< char >( Trait::INTEGRAL                      ) ) |
			( static_cast< uint64_t >( std::is_floating_point                       < CleanType >::value ) << static_cast< char >( Trait::FLOATING_POINT                ) ) |
			( static_cast< uint64_t >( std::is_signed                               < CleanType >::value ) << static_cast< char >( Trait::SIGNED                        ) ) |
			( static_cast< uint64_t >( std::is_unsigned                             < CleanType >::value ) << static_cast< char >( Trait::UNSIGNED                      ) ) |
			( static_cast< uint64_t >( std::is_aggregate                            < CleanType >::value ) << static_cast< char >( Trait::AGGREGATE                     ) ) |
			( static_cast< uint64_t >( std::is_unscoped_enum                        < CleanType >::value ) << static_cast< char >( Trait::UNSCOPED_ENUM                 ) ) |
			( static_cast< uint64_t >( std::is_standard_layout                      < CleanType >::value ) << static_cast< char >( Trait::STANDARD_LAYOUT               ) ) |
			( static_cast< uint64_t >( std::is_empty                                < CleanType >::value ) << static_cast< char >( Trait::EMPTY                         ) ) |
			( static_cast< uint64_t >( std::is_final                                < CleanType >::value ) << static_cast< char >( Trait::FINAL                         ) ) |
			( static_cast< uint64_t >( std::is_abstract                             < CleanType >::value ) << static_cast< char >( Trait::ABSTRACT                      ) ) |
			( static_cast< uint64_t >( std::is_polymorphic                          < CleanType >::value ) << static_cast< char >( Trait::POLYMORPHIC                   ) ) |
			( static_cast< uint64_t >( std::is_trivial                              < CleanType >::value ) << static_cast< char >( Trait::TRIVIAL                       ) ) |
			( static_cast< uint64_t >( std::is_iterable                             < CleanType >::value ) << static_cast< char >( Trait::ITERABLE                      ) ) |
			( static_cast< uint64_t >( std::is_invocable                            < CleanType >::value ) << static_cast< char >( Trait::INVOCABLE                     ) ) |
			( static_cast< uint64_t >( std::is_tuple                                < CleanType >::value ) << static_cast< char >( Trait::TUPLE                         ) ) |
			( static_cast< uint64_t >( std::is_pair                                 < CleanType >::value ) << static_cast< char >( Trait::PAIR                          ) ) |
			( static_cast< uint64_t >( std::is_stl_container                        < CleanType >::value ) << static_cast< char >( Trait::STL_CONTAINER                 ) ) |
			( static_cast< uint64_t >( std::is_void                                 < CleanType >::value ) << static_cast< char >( Trait::VOID                          ) ) |
			( static_cast< uint64_t >( std::is_null_pointer                         < CleanType >::value ) << static_cast< char >( Trait::NULL_POINTER                  ) ) |
			( static_cast< uint64_t >( std::has_operator::add                       < CleanType >::value ) << static_cast< char >( Trait::HAS_ADD                		  ) ) |
			( static_cast< uint64_t >( std::has_operator::subtract                  < CleanType >::value ) << static_cast< char >( Trait::HAS_SUBTRACT				  ) ) |
			( static_cast< uint64_t >( std::has_operator::multiply                  < CleanType >::value ) << static_cast< char >( Trait::HAS_MULTIPLY				  ) ) |
			( static_cast< uint64_t >( std::has_operator::divide                    < CleanType >::value ) << static_cast< char >( Trait::HAS_DIVIDE					  ) ) |
			( static_cast< uint64_t >( std::has_operator::modulo                    < CleanType >::value ) << static_cast< char >( Trait::HAS_MODULO					  ) ) |
			( static_cast< uint64_t >( std::has_operator::pre_increment             < CleanType >::value ) << static_cast< char >( Trait::HAS_PRE_INCREMENT			  ) ) |
			( static_cast< uint64_t >( std::has_operator::pre_decrement             < CleanType >::value ) << static_cast< char >( Trait::HAS_PRE_DECREMENT			  ) ) |
			( static_cast< uint64_t >( std::has_operator::post_increment            < CleanType >::value ) << static_cast< char >( Trait::HAS_POST_INCREMENET 		  ) ) |
			( static_cast< uint64_t >( std::has_operator::post_decrement            < CleanType >::value ) << static_cast< char >( Trait::HAS_POST_DECREMENT			  ) ) |
			( static_cast< uint64_t >( std::has_operator::equality                  < CleanType >::value ) << static_cast< char >( Trait::HAS_EQUALITY				  ) ) |
			( static_cast< uint64_t >( std::has_operator::inequality                < CleanType >::value ) << static_cast< char >( Trait::HAS_INEQUALITY				  ) ) |
			( static_cast< uint64_t >( std::has_operator::greater_than              < CleanType >::value ) << static_cast< char >( Trait::HAS_GREATER					  ) ) |
			( static_cast< uint64_t >( std::has_operator::greater_equal_than        < CleanType >::value ) << static_cast< char >( Trait::HAS_GREATER_EQUAL			  ) ) |
			( static_cast< uint64_t >( std::has_operator::less_than                 < CleanType >::value ) << static_cast< char >( Trait::HAS_LESS					  ) ) |
			( static_cast< uint64_t >( std::has_operator::less_equal_than           < CleanType >::value ) << static_cast< char >( Trait::HAS_LESS_EQUAL				  ) ) |
			( static_cast< uint64_t >( std::has_operator::logical_and               < CleanType >::value ) << static_cast< char >( Trait::HAS_LOGIC_AND				  ) ) |
			( static_cast< uint64_t >( std::has_operator::logical_or                < CleanType >::value ) << static_cast< char >( Trait::HAS_LOGIC_OR				  ) ) |
			( static_cast< uint64_t >( std::has_operator::logical_not               < CleanType >::value ) << static_cast< char >( Trait::HAS_LOGIC_NOT				  ) ) |
			( static_cast< uint64_t >( std::has_operator::binary_and                < CleanType >::value ) << static_cast< char >( Trait::HAS_BINARY_AND				  ) ) |
			( static_cast< uint64_t >( std::has_operator::binary_or                 < CleanType >::value ) << static_cast< char >( Trait::HAS_BINARY_OR				  ) ) |
			( static_cast< uint64_t >( std::has_operator::binary_xor                < CleanType >::value ) << static_cast< char >( Trait::HAS_BINARY_XOR				  ) ) |
			( static_cast< uint64_t >( std::has_operator::binary_ones_compliment    < CleanType >::value ) << static_cast< char >( Trait::HAS_BINARY_ONES_COMPLIMENT	  ) ) |
			( static_cast< uint64_t >( std::has_operator::binary_shift_left         < CleanType >::value ) << static_cast< char >( Trait::HAS_BINARY_SHIFT_LEFT		  ) ) |
			( static_cast< uint64_t >( std::has_operator::binary_shift_right        < CleanType >::value ) << static_cast< char >( Trait::HAS_BINARY_SHIFT_RIGHT		  ) ) |
			( static_cast< uint64_t >( std::has_operator::assign                    < CleanType >::value ) << static_cast< char >( Trait::HAS_ASSIGN					  ) ) |
			( static_cast< uint64_t >( std::has_operator::assign_add                < CleanType >::value ) << static_cast< char >( Trait::HAS_ASSIGN_ADD				  ) ) |
			( static_cast< uint64_t >( std::has_operator::assign_subtract           < CleanType >::value ) << static_cast< char >( Trait::HAS_ASSIGN_SUBTRACT			  ) ) |
			( static_cast< uint64_t >( std::has_operator::assign_multiply           < CleanType >::value ) << static_cast< char >( Trait::HAS_ASSIGN_MULTIPLY			  ) ) |
			( static_cast< uint64_t >( std::has_operator::assign_divide             < CleanType >::value ) << static_cast< char >( Trait::HAS_ASSIGN_DIVIDE			  ) ) |
			( static_cast< uint64_t >( std::has_operator::assign_modulo             < CleanType >::value ) << static_cast< char >( Trait::HAS_ASSIGN_MODULO			  ) ) |
			( static_cast< uint64_t >( std::has_operator::assign_binary_and         < CleanType >::value ) << static_cast< char >( Trait::HAS_ASSIGN_BINARY_AND		  ) ) |
			( static_cast< uint64_t >( std::has_operator::assign_binary_or          < CleanType >::value ) << static_cast< char >( Trait::HAS_ASSIGN_BINARY_OR		  ) ) |
			( static_cast< uint64_t >( std::has_operator::assign_binary_xor         < CleanType >::value ) << static_cast< char >( Trait::HAS_ASSIGN_BINARY_XOR		  ) ) |
			( static_cast< uint64_t >( std::has_operator::assign_binary_shift_left  < CleanType >::value ) << static_cast< char >( Trait::HAS_ASSIGN_BINARY_SHIFT_LEFT  ) ) |
			( static_cast< uint64_t >( std::has_operator::assign_binary_shift_right < CleanType >::value ) << static_cast< char >( Trait::HAS_ASSIGN_BINARY_SHIFT_RIGHT ) ) |
			( static_cast< uint64_t >( std::has_operator::indirection               < CleanType >::value ) << static_cast< char >( Trait::HAS_INDIRECTION				  ) ) |
			( static_cast< uint64_t >( std::has_operator::invoke                    < CleanType >::value ) << static_cast< char >( Trait::HAS_INVOKE					  ) ) |
			( static_cast< uint64_t >( std::has_operator::subscript                 < CleanType >::value ) << static_cast< char >( Trait::HAS_SUBSCRIPT				  ) ) |
			( static_cast< uint64_t >( std::has_operator::address_of                < CleanType >::value ) << static_cast< char >( Trait::HAS_ADDRESS_OF				  ) ) |
			( static_cast< uint64_t >( std::has_operator::member_access             < CleanType >::value ) << static_cast< char >( Trait::HAS_MEMBER_ACCESS			  ) ) |
			( static_cast< uint64_t >( std::has_operator::member_access_pointer     < CleanType >::value ) << static_cast< char >( Trait::HAS_MEMBER_ACCESS_POINTER	  ) );
	}

	template < typename Lambda >
	static typename std::enable_if< !std::is_same< Lambda, std::tuple<> >::value, void >::
	type ProcessArguments( Type* a_Type )
	{
		const Type& thisType = GetType< typename Lambda::_This_type >();

		if ( a_Type->m_TemplateParameters == nullptr )
		{
			a_Type->m_TemplateParameters = new std::vector< const Type* >();
		}

		a_Type->m_TemplateParameters->push_back( &thisType );
		ProcessArguments< typename Lambda::_Mybase >( a_Type );
	}

	template < typename Lambda >
	static typename std::enable_if< std::is_same< Lambda, std::tuple<> >::value, void >::
	type ProcessArguments( Type* a_Type ) { }

	template < typename CleanType >
	static typename std::enable_if< !std::is_enum< CleanType >::value, const Type* >::
	type GetUnderlying()
	{
		return nullptr;
	}

	template < typename CleanType >
	static typename std::enable_if< std::is_enum< CleanType >::value, const Type* >::
	type GetUnderlying()
	{
		return &GetType< typename std::underlying_type< CleanType >::type >();
	}

	static std::map< unsigned int, const Type* > m_TypesMap;
	static std::set< const Type*, Type > m_TypesSet;
	std::string m_Name;
	std::string m_TemplatedName;
	std::string m_Namespace;
	unsigned int m_Hash;
	const std::type_info* m_TypeInfo;
	std::vector< const Type* >* m_TemplateParameters;
	const Type* m_UnderlyingType;
	Specifier m_SpecifierType;
	uint64_t m_Flags;

};