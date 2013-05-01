/*
	grive: an GPL program to sync a local directory with Google Drive
	Copyright (C) 2013 Wan Wai Ho

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation version 2
	of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
	MA  02110-1301, USA.
*/

#pragma once

#include "util/Exception.hh"

#include <cstddef>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <iosfwd>

namespace gr {

class Val
{
public :
	enum TypeEnum { null_type, bool_type, double_type, int_type, object_type, array_type, string_type } ;

	struct Error : virtual Exception {} ;
	typedef boost::error_info<struct SrcType,	TypeEnum> SrcType_ ;
	typedef boost::error_info<struct DestType,	TypeEnum> DestType_ ;
	typedef boost::error_info<struct NoKey,		std::string> NoKey_ ;
	typedef boost::error_info<struct OutOfRange,std::size_t> OutOfRange_ ;
	
private :
	template <typename T>
	struct Type2Enum ;

	template <typename T>
	struct SupportType ;

public :
	typedef std::vector<Val>			Array ;
	typedef std::map<std::string, Val>	Object ;

public :
	Val() ;
	Val( const Val& v ) ;
	~Val() ;

	template <typename T>
	explicit Val( const T& t )
	{
		Assign(t) ;
	}

	template <typename T>
	Val& Assign( const T& t ) ;
	void Swap( Val& val ) ;
	
	Val& operator=( const Val& val )
	{
		Val tmp(val) ;
		Swap(tmp) ;
		return *this ;
	}
		
	template <typename T>
	Val& operator=( const T& t )
	{
		return Assign(t) ;
	}

	template <typename T>
	const T& As() const ;

	template <typename T>
	T& As() ;

	template <typename T>
	bool Is() const ;

	TypeEnum Type() const ;

	const Val& operator[]( const std::string& key ) const ;
	const Val& operator[]( std::size_t index ) const ;

	void Add( const std::string& key, const Val& value ) ;

private :
	struct Base ;
	
	template <typename T>
	struct Impl ;
	
	std::auto_ptr<Base>	m_base ;

private :
} ;

template <> struct Val::Type2Enum<void>			{ static const TypeEnum type = null_type ; } ;
template <> struct Val::Type2Enum<long long>	{ static const TypeEnum type = int_type ;  } ;
template <> struct Val::Type2Enum<bool>			{ static const TypeEnum type = bool_type ; } ;
template <> struct Val::Type2Enum<double>		{ static const TypeEnum type = double_type ;} ;
template <> struct Val::Type2Enum<std::string>	{ static const TypeEnum type = string_type ; } ;
template <> struct Val::Type2Enum<Val::Array>	{ static const TypeEnum type = array_type ; } ;
template <> struct Val::Type2Enum<Val::Object>	{ static const TypeEnum type = object_type ; } ;

template <> struct Val::SupportType<int>			{ typedef long long	Type ; } ;
template <> struct Val::SupportType<unsigned>		{ typedef long long	Type ; } ;
template <> struct Val::SupportType<long>			{ typedef long long	Type ; } ;
template <> struct Val::SupportType<unsigned long>	{ typedef long long	Type ; } ;
template <> struct Val::SupportType<short>			{ typedef long long	Type ; } ;
template <> struct Val::SupportType<unsigned short>	{ typedef long long	Type ; } ;
template <> struct Val::SupportType<long long>			{ typedef long long	Type ; } ;
template <> struct Val::SupportType<unsigned long long>	{ typedef long long	Type ; } ;

template <> struct Val::SupportType<bool>			{ typedef bool		Type ; } ;
template <> struct Val::SupportType<double>			{ typedef double	Type ; } ;
template <> struct Val::SupportType<std::string>	{ typedef std::string	Type ; } ;
template <> struct Val::SupportType<const char*>	{ typedef std::string	Type ; } ;
template <> struct Val::SupportType<Val::Array>		{ typedef Val::Array	Type ; } ;
template <> struct Val::SupportType<Val::Object>	{ typedef Val::Object	Type ; } ;

struct Val::Base
{
	virtual ~Base() {}
	virtual Base* Clone() const = 0 ;
	virtual TypeEnum Type() const = 0 ;
} ;

template <typename T>
struct Val::Impl : public Base
{
	T val ;
	Impl( const T& t ) : val(t) {}
	Impl<T>* Clone() const { return new Impl<T>(val); }
	TypeEnum Type() const { return Type2Enum<T>::type ; }
} ;

template <>
struct Val::Impl<void>	: public Base
{
	Impl<void>* Clone() const { return new Impl<void>; }
	TypeEnum Type() const { return null_type ; }
} ;

template <typename T>
Val& Val::Assign( const T& t )
{
	m_base.reset( new Impl<typename SupportType<T>::Type>(t) ) ;
	return *this ;
}

template <typename T>
const T& Val::As() const
{
	try
	{
		const Impl<T> *impl = &dynamic_cast<const Impl<T>&>( *m_base ) ;
		return impl->val ;
	}
	catch ( std::exception& e )
	{
		TypeEnum dest = Type2Enum<T>::type ;
		BOOST_THROW_EXCEPTION(
			Error() << SrcType_(Type()) << DestType_(dest)
		) ;
	}
}

template <typename T>
T& Val::As()
{
	try
	{
		Impl<T> *impl = &dynamic_cast<Impl<T>&>( *m_base ) ;
		return impl->val ;
	}
	catch ( std::exception& e )
	{
		TypeEnum dest = Type2Enum<T>::type ;
		BOOST_THROW_EXCEPTION(
			Error() << SrcType_(Type()) << DestType_(dest)
		) ;
	}
}

template <typename T>
bool Val::Is() const
{
	return Type() == Type2Enum<T>::type ;
}

} // end of namespace

namespace std
{
	void swap( gr::Val& v1, gr::Val& v2 ) ;
	ostream& operator<<( ostream& os, gr::Val::TypeEnum t ) ;
}
