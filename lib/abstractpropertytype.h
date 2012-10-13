/*
	Copyright (C) 2012  Intel Corporation

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _ABSTRACTPROPERTYTYPE_H_
#define _ABSTRACTPROPERTYTYPE_H_

#include <string>
#include <sstream>
#include <stdexcept>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/utility.hpp>
#include <type_traits>

class AbstractPropertyType
{
public:
	virtual std::string toString() = 0;

	virtual void fromString(std::string)= 0;

	void setValue(boost::any val)
	{
		mValue = val;
	}

	template <typename T>
	T value() const
	{
		return boost::any_cast<T>(mValue);
	}

	boost::any anyValue()
	{
		return mValue;
	}

protected:

	boost::any mValue;

};

template <typename T>
class BasicPropertyType: public AbstractPropertyType
{
public:
	BasicPropertyType(BasicPropertyType const &other)
	{
		setValue(other.value<T>());
	}

	BasicPropertyType & operator = (BasicPropertyType const & other)
	{
		setValue(other.value<T>());
		return *this;
	}

	BasicPropertyType(T val)
		:AbstractPropertyType()
	{
		setValue(val);
	}

	BasicPropertyType(std::string val)
	{
		if(!val.empty() && val != "")
		{
			serialize<T>(val);
		}
		else throw std::runtime_error("value cannot be empty");
	}

	void fromString(std::string val)
	{
		if(!val.empty() && val != "")
		{
			serialize<T>(val);
		}
	}

	std::string toString()
	{
		std::stringstream stream;
		stream<<value<T>();

		return stream.str();
	}

private:

	template <class N>
	void serialize(std::string val,  typename std::enable_if<std::is_enum<N>::value, N>::type* = 0)
	{
		int someTemp;

		std::stringstream stream(val);

		stream>>someTemp;
		setValue((N)someTemp);
	}

	template <class N>
	void serialize(std::string  val,  typename std::enable_if<!std::is_enum<N>::value, N>::type* = 0)
	{
		std::stringstream stream(val);
		N someTemp;
		stream>>someTemp;
		setValue(someTemp);
	}
};

class StringPropertyType: public AbstractPropertyType
{
public:
	StringPropertyType(std::string val)
		:AbstractPropertyType()
	{
		setValue(val);
	}

	StringPropertyType(StringPropertyType const & other)
	{
		setValue(other.value<std::string>());
	}

	StringPropertyType & operator = (StringPropertyType const & other)
	{
		setValue(other.value<std::string>());
		return *this;
	}

	void fromString(std::string val)
	{
		setValue(val);
	}


	std::string toString()
	{
		return value<std::string>();
	}

};

#endif
