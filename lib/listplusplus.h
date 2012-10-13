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


#ifndef LISTPLUSPLUS_H
#define LISTPLUSPLUS_H

#include <list>
#include <algorithm>

template <class T>
class ListPlusPlus
{
public:
	
	ListPlusPlus(std::list<T> *list)
	:mList(list)
	{
		
	}
	
	
	void removeOne(T value)
	{
		typename std::list<T>::iterator itr = std::find(mList->begin(), mList->end(), value);
		
		if (itr != mList->end())
		{
			mList->erase(itr);
		}
	}
	
	bool contains(T value)
	{
		return (std::find(mList->begin(), mList->end(), value) != mList->end());
	}
	
private:
	std::list<T> *mList;
};

template <class T, class V>
bool contains(T iteratable, V value)
{
	return (std::find(iteratable.begin(), iteratable.end(), value) != iteratable.end());
}

#endif // LISTPLUSPLUS_H
