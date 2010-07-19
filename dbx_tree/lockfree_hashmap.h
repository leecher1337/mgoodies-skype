#pragma once

/*

lockfree hash-map based on Ori Shalev and Nir Shavit

implementation
Copyright 2009 Michael "Protogenes" Kunz

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
#pragma intrinsic (_InterlockedCompareExchange)
#pragma intrinsic (_InterlockedIncrement)
#pragma intrinsic (_InterlockedDecrement)
#else
#include "intrin_gcc.h"
#endif

#include <utility>
#include "Hash.h"

#define POINTER(listitem) ((PListItem)(((uint32_t)(listitem)) & ~1))
#define MARK(listitem)    ((bool)     (((uint32_t)(listitem)) & 1))

namespace lockfree
{

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t) = Hash>
	class hash_map
	{
	public:
		typedef std::pair<TKey, TData> value_type;

	public:
		typedef struct TListItem
		{
			TListItem * Next;
			TListItem * NextPurge;
			uint32_t    Hash;
			value_type  Value;
		} TListItem, *PListItem;

		typedef struct THashTable 
		{
			PListItem Table[256];
		} THashTable, *PHashTable;
#pragma pack(push,1)
		typedef union {
			struct {
				volatile PHashTable Table;
				volatile uint32_t Size;
			};
			volatile int64_t Complete;
		} THashTableData;

		typedef union {
			struct {
				volatile long RefCount;
				volatile PListItem Purge;
			};
			volatile int64_t Complete;
		} THashTableReferences;

#pragma pack(pop)

		volatile THashTableReferences m_References;

		volatile uint32_t m_Count;

		volatile THashTableData m_HashTableData;

		PListItem listInsert(PListItem BucketNode, PListItem Node);
		PListItem listDelete(PListItem BucketNode, uint32_t Hash, const TKey & Key);

		PListItem listDelete(PListItem BucketNode, PListItem Node);

		bool listFind(const PListItem BucketNode, const uint32_t Hash, const TKey & Key, const PListItem Node, PListItem * & Prev, PListItem & Curr, PListItem & Next);

		uint32_t getMask(uint32_t Size)
		{
			const uint32_t mask[32] = {0x80000000, 0xc0000000, 0xe0000000, 0xf0000000, 
				0xf8000000, 0xfc000000, 0xfe000000, 0xff000000, 
				0xff800000, 0xffc00000, 0xffe00000, 0xfff00000, 
				0xfff80000, 0xfffc0000, 0xfffe0000, 0xffff0000, 
				0xffff8000, 0xffffc000, 0xffffe000, 0xfffff000, 
				0xfffff800, 0xfffffc00, 0xfffffe00, 0xffffff00, 
				0xffffff80, 0xffffffc0, 0xffffffe0, 0xfffffff0, 
				0xfffffff8, 0xfffffffc, 0xfffffffe, 0xffffffff
			};
			return mask[Size - 1];
		}

		PHashTable makeNewTable()
		{
			PHashTable result = new THashTable;
			memset(result->Table, 0, sizeof(result->Table));
			return result;
		};
		PListItem makeDummyNode(uint32_t Hash)
		{
			PListItem result = new TListItem;
			result->Hash = Hash;
			result->Next = NULL;
			result->NextPurge = NULL;
			return result;
		};

		PListItem initializeBucket(uint32_t Bucket, uint32_t Mask);

		PListItem getBucket(uint32_t Bucket);

		void setBucket(uint32_t Bucket, PListItem Dummy);

		void DeleteTable(THashTable * Table, uint32_t Size)
		{
			if (Size > 8)
			{
				for (uint32_t i = 0; i < 256; ++i)
				{
					if (Table->Table[i])
						DeleteTable((THashTable *)Table->Table[i], Size - 8);
				}
			}

			delete Table;
		};


		void addRef()
		{
			_InterlockedIncrement(&m_References.RefCount);
		};

		void delRef()
		{
			THashTableReferences o, n;
			do {
				o.Complete = m_References.Complete;
				n.Complete = o.Complete;
				n.RefCount--;
				if (n.RefCount == 0)
					n.Purge = NULL;

			}	while (o.Complete != _InterlockedCompareExchange64(&m_References.Complete, n.Complete, o.Complete));
			if (n.Purge == NULL)
			{
				PListItem del = o.Purge;
				while (del)
				{
					PListItem tmp = del->NextPurge;
					delete del;
					del = tmp;
				}
			}
		};
	public:
		class iterator
		{
		protected:
			friend class hash_map<TKey, TData, FHash>;				
			PListItem m_Item;
			hash_map<TKey, TData, FHash> * m_Owner;

			iterator(hash_map<TKey, TData, FHash> * Owner, PListItem Item)
			: m_Owner(Owner)
			{
				m_Owner->addRef();
				m_Item = Item;
				while (m_Item && (!(m_Item->Hash & 1) || MARK(m_Item->Next)))
					m_Item = POINTER(m_Item->Next);
			};
		public:
			iterator(const iterator & Other)
			: m_Owner(Other.m_Owner),
				m_Item(Other.m_Item)
			{
				m_Owner->addRef();
			};
			~iterator()
			{
				m_Owner->delRef();
			};

			operator bool() const
			{
				return m_Item != NULL;
			};
			bool operator !() const
			{
				return m_Item == NULL;
			};

			value_type * operator ->()
			{
				return &m_Item->Value;
			};
			value_type & operator *()
			{
				return m_Item->Value;
			};

			bool operator ==(iterator& Other)
			{
				return m_Item->Value.first == Other.m_Item->Value.first;
			};
			bool operator <  (iterator & Other)
			{
				return m_Item->Value.first < Other.m_Item->Value.first;
			};
			bool operator >  (iterator & Other)
			{
				return m_Item->Value.first > Other.m_Item->Value.first;
			};

			iterator& operator =(const iterator& Other)
			{
				m_Owner = Other.m_Owner;
				m_Item = Other.m_Item;
				return *this;
			};

			iterator& operator ++() //pre  ++i
			{
				if (!m_Item)
					return *this;
				do 
				{
					m_Item = POINTER(m_Item->Next);
				} while (m_Item && (!(m_Item->Hash & 1) || MARK(m_Item->Next)));
				return *this;
			};
			iterator  operator ++(int) //post i++
			{
				iterator bak(*this);
				++(*this);
				return bak;
			};
		};

		iterator begin()
		{
			iterator res(this, getBucket(0));
			++res;
			return res;
		};

		iterator end()
		{
			return iterator(this, NULL);
		};

		hash_map()
		{
			m_Count = 0;
			m_HashTableData.Size = 1;
			m_References.RefCount = 0;
			m_References.Purge = NULL;

			m_HashTableData.Table = makeNewTable();
			setBucket(0x00000000, makeDummyNode(0x00000000));
			setBucket(0x80000000, makeDummyNode(0x80000000));
			m_HashTableData.Table->Table[0]->Next = getBucket(0x80000000);
		};


		~hash_map()
		{
			PListItem h = getBucket(0);
			DeleteTable(m_HashTableData.Table, m_HashTableData.Size);
			while (h)
			{
				PListItem tmp = h;
				h = POINTER(h->Next);
				delete tmp;
			}
		};

		std::pair<iterator, bool> insert(const value_type & Val);

		iterator find(const TKey & Key);

		iterator erase(const iterator & Where);

		size_t erase(const TKey & Key);

	};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	typename hash_map<TKey, TData, FHash>::PListItem hash_map<TKey, TData, FHash>::listInsert(PListItem BucketNode, PListItem Node)
	{
		PListItem *prev, curr, next;
		do 
		{
			if (listFind(BucketNode, Node->Hash, Node->Value.first, NULL, prev, curr, next))
				return POINTER(curr);

			Node->Next = POINTER(curr);

		} while (POINTER(curr) != (TListItem*)_InterlockedCompareExchange((volatile long *)prev, (long)Node, (long)POINTER(curr)));
		return Node;
	};


	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	typename hash_map<TKey, TData, FHash>::PListItem hash_map<TKey, TData, FHash>::listDelete(PListItem BucketNode, uint32_t Hash, const TKey & Key)
	{
		PListItem *prev, curr, next;
		do 
		{
			if (!listFind(BucketNode, Hash, Key, NULL, prev, curr, next))
				return POINTER(curr);

		} while (POINTER(next) != (PListItem)_InterlockedCompareExchange((volatile long *)&curr->Next, (long)next | 1, (long)POINTER(next)));

		if (POINTER(curr) == (PListItem)_InterlockedCompareExchange((volatile long *)prev, (long)POINTER(next), (long)POINTER(curr)))
		{
			PListItem del = POINTER(curr);
			del->NextPurge = del;
			del->NextPurge = (PListItem)_InterlockedExchange((volatile long *)&m_References.Purge, (long)del);

		} else {
			listFind(BucketNode, Hash, Key, NULL, prev, curr, next); // cleanup
		}

		return POINTER(curr);
	};

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	typename hash_map<TKey, TData, FHash>::PListItem hash_map<TKey, TData, FHash>::listDelete(PListItem BucketNode, PListItem Node)
	{
		PListItem *prev, curr, next;
		do 
		{
			if (!listFind(BucketNode, Node->Hash, Node->Value.first, Node, prev, curr, next))
				return POINTER(curr);

		} while (POINTER(next) != (PListItem)_InterlockedCompareExchange((volatile long *)&curr->Next, (long)next | 1, (long)POINTER(next)));

		if (POINTER(curr) == (PListItem)_InterlockedCompareExchange((volatile long *)prev, (long)POINTER(next), (long)POINTER(curr)))
		{
			PListItem del = POINTER(curr);
			del->NextPurge = del;
			del->NextPurge = (PListItem)_InterlockedExchange((volatile long *)&m_References.Purge, (long)del);

		} else {
			listFind(BucketNode, Node->Hash, Node->Value.first, Node, prev, curr, next); // cleanup
		}

		return POINTER(curr);
	};

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	bool hash_map<TKey, TData, FHash>::listFind(const PListItem BucketNode, const uint32_t Hash, const TKey & Key, const PListItem Node, PListItem * & Prev, PListItem & Curr, PListItem & Next)
	{
		Prev = &(BucketNode->Next);
		Curr = *Prev;
		do
		{
			if (POINTER(Curr) == NULL)
				return false;

			Next = POINTER(Curr)->Next;
			uint32_t h = POINTER(Curr)->Hash;

			if (*Prev != POINTER(Curr))
				return listFind(BucketNode, Hash, Key, Node, Prev, Curr, Next);

			if (!MARK(Next))
			{
				if (Node)
				{
					if ((h > Hash) || (Node == POINTER(Curr)))
						return POINTER(Curr) == Node;
				}
				else if ((h > Hash) || ((h == Hash) && !(POINTER(Curr)->Value.first < Key)))
				{
					return (h == Hash) && (POINTER(Curr)->Value.first == Key);
				}

				Prev = &(POINTER(Curr)->Next);
			} else {
				if (POINTER(Curr) == (TListItem *)_InterlockedCompareExchange((volatile long *)Prev, (long)POINTER(Next), (long)POINTER(Curr)))
				{
					PListItem del = POINTER(Curr);
					del->NextPurge = del;
					del->NextPurge = (PListItem)_InterlockedExchange((volatile long *)&m_References.Purge, (long)del);

				} else {
					return listFind(BucketNode, Hash, Key, Node, Prev, Curr, Next);
				}
			}
			Curr = Next;
		} while (true);

	};

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	typename hash_map<TKey, TData, FHash>::PListItem hash_map<TKey, TData, FHash>::initializeBucket(uint32_t Bucket, uint32_t Mask)
	{
		uint32_t parent = Bucket & (Mask << 1);
		PListItem parentnode = getBucket(parent);
		if (parentnode == NULL)
			parentnode = initializeBucket(parent, Mask << 1);

		PListItem dummy = makeDummyNode(Bucket);
		PListItem bucketnode = listInsert(parentnode, dummy);
		if (bucketnode != dummy)
		{
			delete dummy;
			dummy = bucketnode;
		}
		setBucket(Bucket, dummy);

		return dummy;
	}

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	typename hash_map<TKey, TData, FHash>::PListItem hash_map<TKey, TData, FHash>::getBucket(uint32_t Bucket)
	{
		THashTableData table;
		uint32_t mask;

		table.Complete = m_HashTableData.Complete;
		mask = getMask(table.Size);

		uint32_t levelshift = (32 - table.Size) & ~7;

		while (levelshift < 24)
		{
			table.Table = (THashTable *)table.Table->Table[((Bucket & mask) >> levelshift) & 0xff];
			levelshift = levelshift + 8;
			if (!table.Table)
				return NULL;
		}
		return table.Table->Table[(Bucket & mask) >> 24];
	};

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	void hash_map<TKey, TData, FHash>::setBucket(uint32_t Bucket, PListItem Dummy)
	{
		THashTableData table;
		PHashTable *last;
		uint32_t mask;

		table.Complete = m_HashTableData.Complete;
		mask = getMask(table.Size);

		uint32_t levelshift = (32 - table.Size) & ~7;

		while (levelshift < 24)
		{
			last = (THashTable **)&table.Table->Table[((Bucket & mask) >> levelshift) & 0xff];
			table.Table = *last;
			levelshift = levelshift + 8;
			if (!table.Table)
			{
				PHashTable newtable = makeNewTable();
				table.Table = (THashTable *)_InterlockedCompareExchange((volatile long *)last, (long)newtable, NULL);
				if (table.Table)
				{
					delete newtable;
				} else {


					table.Table = newtable;
				}
			}
		}
		table.Table->Table[(Bucket & mask) >> 24] = Dummy;
	};

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	typename std::pair<typename hash_map<TKey, TData, FHash>::iterator, bool> hash_map<TKey, TData, FHash>::insert(const value_type & Val)
	{
		iterator dummyreference(this, NULL);
		PListItem node = new TListItem;
		node->Value = Val;
		node->Hash = FHash(&node->Value.first, sizeof(TKey)) | 1;
		node->NextPurge = NULL;
		node->Next = NULL;

		THashTableData tmp, newdata;
		tmp.Complete = m_HashTableData.Complete;

		uint32_t mask = getMask(tmp.Size);

		uint32_t bucket = node->Hash & mask;
		PListItem bucketnode = getBucket(bucket);

		if (bucketnode == NULL)
			bucketnode = initializeBucket(bucket, mask);
		PListItem retnode = listInsert(bucketnode, node);
		if (retnode != node)
		{
			delete node;
			return std::make_pair(iterator(this, retnode), false);
		}

		if ((_InterlockedIncrement((volatile long *)&m_Count) > (1 << (tmp.Size + 2))) && (tmp.Size == m_HashTableData.Size))
		{
			newdata = tmp;
			newdata.Size++;
			if ((tmp.Size & 0x7) == 0)
			{
				newdata.Table = makeNewTable();
				newdata.Table->Table[0] = (TListItem*)tmp.Table;
			}
			if (tmp.Complete != _InterlockedCompareExchange64(&m_HashTableData.Complete, newdata.Complete, tmp.Complete))
				delete newdata.Table;

		}

		return std::make_pair(iterator(this, node), true);
	};

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	typename hash_map<TKey, TData, FHash>::iterator hash_map<TKey, TData, FHash>::find(const TKey & Key)
	{
		iterator dummyreference(this, NULL);
		uint32_t hash = FHash(&Key, sizeof(TKey)) | 1;
		uint32_t mask = getMask(m_HashTableData.Size);
		uint32_t bucket = hash & mask;
		PListItem bucketnode = getBucket(bucket);
		if (bucketnode == NULL)
			bucketnode = initializeBucket(bucket, mask);

		PListItem *prev, curr, next;		
		if (listFind(bucketnode, hash, Key, NULL, prev, curr, next))
			return iterator(this, POINTER(curr));

		return iterator(this, NULL);
	};

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	typename hash_map<TKey, TData, FHash>::iterator hash_map<TKey, TData, FHash>::erase(const iterator & Where)
	{
		iterator dummyreference(this, NULL);
		uint32_t hash = Where.m_Item->Hash;
		uint32_t mask = getMask(m_HashTableData.Size);
		uint32_t bucket = hash & mask;
		PListItem bucketnode = getBucket(bucket);

		if (bucketnode == NULL)
			bucketnode = initializeBucket(bucket, mask);

		PListItem res = listDelete(bucketnode, Where.m_Item);
		if (Where.m_Item == res)
		{
			_InterlockedDecrement((volatile long*)&m_Count);
			return iterator(this, POINTER(res->Next));
		}
		return iterator(this, res);
	};

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	size_t hash_map<TKey, TData, FHash>::erase(const TKey & Key)
	{
		iterator dummyreference(this, NULL);
		uint32_t hash = FHash(&Key, sizeof(TKey)) | 1;
		uint32_t mask = getMask(m_HashTableData.Size);
		uint32_t bucket = hash & mask;
		PListItem bucketnode = getBucket(bucket);
		if (bucketnode == NULL)
			bucketnode = initializeBucket(bucket, mask);

		PListItem result = listDelete(bucketnode, hash, Key);
		if (result && (result->Value.first == Key))
		{
			_InterlockedDecrement((volatile long*)&m_Count);
			return 1;
		}

		return 0;
	};


}
