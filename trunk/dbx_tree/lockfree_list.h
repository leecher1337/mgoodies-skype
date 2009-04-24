/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2009 Michael "Protogenes" Kunz,

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#pragma once

#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic (_InterlockedExchange)
#else
#include "intrin_gcc.h"
#endif

template <typename T>
class lockfree_list
{
	protected:
		typedef struct TListItem
		{
			TListItem * Next;
			volatile long Used;
			T Content;
		} TListItem, *PListItem;

		volatile PListItem m_Head;
	public:

		class iterator
		{
			protected:
				friend class lockfree_list<T>;
				PListItem m_Item;

				iterator(PListItem Item)
				{
					m_Item = Item;
				};
			public:
				iterator(const iterator & Other)
				{
					m_Item = Other.m_Item;
				}
				~iterator() {};

				T & operator *() {return m_Item->Content;};
				T * operator->() {return &(m_Item->Content);};

				bool operator == (const iterator & Other)
				{
					return m_Item == Other.m_Item;
				};
				bool operator != (const iterator & Other)
				{
					return m_Item != Other.m_Item;
				};
				iterator & operator ++()
				{
					if (!m_Item) return *this;
					do 
					{
						m_Item = m_Item->Next;
					} while ((m_Item) && (m_Item->Used == 0));

					return *this;
				};
				iterator & operator ++(int)
				{
					iterator j(*this);
					++(*this);
					return j;
				};
		};

		lockfree_list()
		{
			m_Head = NULL;
		};
		~lockfree_list()
		{
			PListItem i = m_Head, j;
			while (i)
			{
				j = i->Next;
				delete i;
				i = j;
			}
		};
		
		iterator insert(const T & Item)
		{
			PListItem i = m_Head;

			while (i && (_InterlockedExchange(&(i->Used), 1) == 1))
				i = i->Next;

			if (!i)
			{
				i = new TListItem();
				i->Content = Item;
				i->Used = 1;
				i->Next = i;
				i->Next = (PListItem)_InterlockedExchange((long volatile *)&m_Head, (long)i);
			} else {
				i->Content = Item;
			}
			
			return iterator(i);
		};
		void erase(iterator & Item)
		{
			memset(&(Item.m_Item->Content), 0, sizeof(T));
			InterlockedExchange(&(Item.m_Item->Used), 0);
		};

		iterator begin()
		{
			return iterator(m_Head);
		};
		iterator end()
		{
			return iterator(NULL);
		};
		
};
