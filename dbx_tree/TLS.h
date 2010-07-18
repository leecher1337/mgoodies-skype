/*

dbx_tree: tree database driver for Miranda IM

Copyright 2010 Michael "Protogenes" Kunz,

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

template <typename TAdministrator, typename TData>
class CThreadLocalStorage
{
	private:
		typedef struct TListElem
		{
			TListElem *      Next;
			TAdministrator * Admin;
			TData            Data;
			TListElem(TListElem * ANext, TAdministrator * AAdmin, const TData & AData)
				:	Next(ANext), Admin(AAdmin), Data(AData)
				{	};

		} TListElem, *PListElem;
		static __declspec(thread) PListElem m_Head;
	public:
		CThreadLocalStorage()
			{	};
		~CThreadLocalStorage()
			{	};

		TData & Open(TAdministrator * Admin, const TData & Default);
		TData * Find(TAdministrator * Admin);
		void    Remove(TAdministrator * Admin);
};



template <typename TAdministrator, typename TData>
typename CThreadLocalStorage<TAdministrator, TData>::PListElem CThreadLocalStorage<TAdministrator, TData>::m_Head = NULL;

template <typename TAdministrator, typename TData>
typename TData & CThreadLocalStorage<TAdministrator, TData>::Open(typename TAdministrator * Admin, const TData & Default)
{
	PListElem * last = &m_Head;
	PListElem i = m_Head;
	while (i && (i->Admin != Admin))
	{
		last = &i->Next;
		i = i->Next;
	}

	if (i)
	{
		*last = i->Next;
		i->Next = m_Head;
		m_Head = i;
	} else {
		m_Head = new TListElem(m_Head, Admin, Default);
	}
	return m_Head->Data;
}

template <typename TAdministrator, typename TData>
typename TData * CThreadLocalStorage<TAdministrator, TData>::Find(typename TAdministrator * Admin)
{
	PListElem * last = &m_Head;
	PListElem i = m_Head;
	while (i && (i->Admin != Admin))
	{
		last = &i->Next;
		i = i->Next;
	}

	if (i)
	{
		*last = i->Next;
		i->Next = m_Head;
		m_Head = i;
	} else {
		return NULL;
	}
	return &m_Head->Data;
}

template <typename TAdministrator, typename TData>
void    CThreadLocalStorage<TAdministrator, TData>::Remove(typename TAdministrator * Admin)
{
	PListElem * last = &m_Head;
	PListElem i = m_Head;
	while (i && (i->Admin != Admin))
	{
		last = &i->Next;
		i = i->Next;
	}

	if (i)
	{
		*last = i->Next;
		delete i;
	}
}
