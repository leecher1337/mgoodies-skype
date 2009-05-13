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

#include "lockfree_list.h"
#include <windows.h>

template <typename T>
class CThreadLocalStorage
{
	protected:
		typedef struct TListItem
		{
			long ThreadID;
			T Content;
		} TListItem, *PListItem;

		typedef lockfree_list<TListItem> THashList;

		THashList m_HashTable[0x10];
		unsigned long HashFunc(unsigned long Thread) 
		{
			return (Thread ^ (Thread >> 8)) & 0x0f;
		};
	public:

		class iterator
		{
		protected:
			friend class CThreadLocalStorage;
			typename THashList::iterator m_Item;

			iterator(typename THashList::iterator & Item)
				: m_Item(Item)
			{
				
			};
		public:
			iterator(const iterator & Other)
				: m_Item(Other.m_Item)
			{
				
			}
			~iterator() {};

			T & operator *() {return m_Item->Content;};
			T * operator->() {return &(m_Item->Content);};
		};

		CThreadLocalStorage() {};
		~CThreadLocalStorage() {};

		iterator Open(const T & InitValue, unsigned long ThreadID = 0)
		{
			if (ThreadID == 0)
				ThreadID = GetCurrentThreadId();
			unsigned long index = HashFunc(ThreadID);
			THashList::iterator i = m_HashTable[index].begin();
			while ((i != m_HashTable[index].end()) && (i->ThreadID != ThreadID))
				++i;
			
			if (i == m_HashTable[index].end())
			{
				TListItem ins;
				ins.ThreadID = ThreadID;
				ins.Content = InitValue;
				i = m_HashTable[index].insert(ins);
			}

			return iterator(i);
		};

		void Delete(iterator & Item)
		{
			 m_HashTable[HashFunc(Item.m_Item->ThreadID)].erase(Item.m_Item);
		};
};
