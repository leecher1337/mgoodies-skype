#pragma once

#include <stack>
#include <map>
#include "sigslot.h"
#include "stdint.h"
#include "Exception.h"

template <typename TKey, uint16_t SizeParam = 4>
class CBTree
{
public:
	typedef uint32_t TNodeRef;
	typedef sigslot::signal2< void *, TNodeRef > TOnRootChanged;
	
	#pragma pack(push, 1)  // push current alignment to stack, set alignment to 1 byte boundary

		typedef struct TNode {
			uint16_t Info;                 /// Node information (IsLeaf and stored KeyCount)
			TNodeRef Parent;               /// Handle to the parent node
			TKey Key[SizeParam * 2 - 1];   /// array with Keys
			TNodeRef Child[SizeParam * 2]; /// array with child node handles
		} TNode;

	#pragma pack(pop)

	class iterator
	{
	public:
		iterator();
		iterator(CBTree* Tree, TNodeRef Node, uint16_t Index);
		iterator(const iterator& Other);
		~iterator();

		CBTree * Tree();

		/**
			\brief Keeps track of changes in the tree and refresh the iterator
		**/
		void setManaged();
		bool wasDeleted();

		operator bool() const;
		bool operator !() const;

		const TKey & operator *();
		const TKey * operator->();


		bool operator ==(iterator& Other);
		bool operator <  (iterator & Other);
		bool operator >  (iterator & Other);

		iterator& operator =(const iterator& Other);

		iterator& operator ++(); //pre  ++i
		iterator& operator --(); //pre  --i
		iterator  operator ++(int); //post i++
		iterator  operator --(int); //post i--


	protected:
		friend class CBTree;

		TNodeRef m_Node;
		uint16_t m_Index;
		CBTree* m_Tree;

		bool m_Managed;
		bool m_LoadedKey;
		TKey m_ManagedKey;
		bool m_ManagedDeleted;

		void Backup();
		void Dec();
		void Inc();
		void RemoveManaged(TNodeRef FromNode);
		void InsertManaged();
	};


	CBTree(TNodeRef RootNode);
	virtual ~CBTree();

	iterator Insert(const TKey & Key);
	iterator Find(const TKey & Key);
	iterator LowerBound(const TKey & Key);
	iterator UpperBound(const TKey & Key);
	bool Delete(const TKey& Key);

	typedef sigslot::signal3<void *, const TKey &, uint32_t> TDeleteCallback;
	void DeleteTree(TDeleteCallback * CallBack, uint32_t Param);

	TNodeRef getRoot();
	void setRoot(TNodeRef NewRoot);

	TOnRootChanged & sigRootChanged() {return m_sigRootChanged;};


protected:
	static const uint16_t cIsLeafMask = 0x8000;
	static const uint16_t cKeyCountMask = 0x7FFF;
	static const uint16_t cFullNode = SizeParam * 2 - 1;
	static const uint16_t cEmptyNode = SizeParam - 1;

	typedef std::multimap<TNodeRef, iterator*> TManagedMap;

	TNodeRef m_Root;
	TOnRootChanged m_sigRootChanged;
	TManagedMap	m_ManagedIterators;

	bool m_DestroyTree;

	virtual TNodeRef CreateNewNode();
	virtual void DeleteNode(TNodeRef Node);
	virtual void Read(TNodeRef Node, uint32_t Offset, uint32_t Size, TNode & Dest);
	virtual void Write(TNodeRef Node, uint32_t Offset, uint32_t Size, TNode & Source);

	void DestroyTree();


private:
	friend class iterator;

	bool InNodeFind(const TNode & Node, const TKey & Key, uint16_t & GreaterEqual);
	void SplitNode(TNodeRef Node, TNodeRef & Left, TNodeRef & Right, TKey & UpKey, TNodeRef ParentNode, uint16_t ParentIndex);
	TNodeRef MergeNodes(TNodeRef Left, TNodeRef Right, const TKey & DownKey, TNodeRef ParentNode, uint16_t ParentIndex);
	void KeyInsert(TNodeRef Node, TNode & NodeData, uint16_t Where);
	void KeyDelete(TNodeRef Node, TNode & NodeData, uint16_t Where);
	void KeyMove(TNodeRef Source, uint16_t SourceIndex, const TNode & SourceData, TNodeRef Dest, uint16_t DestIndex, TNode & DestData);

	void ReadNode(TNodeRef Node, TNode & Dest);
	void WriteNode(TNodeRef Node, TNode & Source);

};






template <typename TKey, uint16_t SizeParam>
CBTree<TKey, SizeParam>::CBTree(TNodeRef RootNode = NULL)
: m_sigRootChanged(),
	m_ManagedIterators()
{
	m_Root = RootNode;
	m_DestroyTree = true;
}

template <typename TKey, uint16_t SizeParam>
CBTree<TKey, SizeParam>::~CBTree()
{
	TManagedMap::iterator i = m_ManagedIterators.begin();
	while (i != m_ManagedIterators.end())
	{
		i->second->m_Tree = NULL;
		i++;
	}
	m_ManagedIterators.clear();

	if (m_DestroyTree)
		DestroyTree();
}

template <typename TKey, uint16_t SizeParam>
inline bool CBTree<TKey, SizeParam>::InNodeFind(const TNode & Node, const TKey & Key, uint16_t & GreaterEqual)
{
	uint16_t l = 0;
	uint16_t r = (Node.Info & cKeyCountMask);
	bool res = false;
	GreaterEqual = 0;
	while ((l < r) && !res)
	{
		GreaterEqual = (l + r) >> 1;
		if (Node.Key[GreaterEqual] < Key)
		{
			GreaterEqual++;
			l = GreaterEqual;
		} else if (Node.Key[GreaterEqual] == Key)
		{
			//r = -1;
			res = true;
		} else {
			r = GreaterEqual;
		}
	}

	return res;
}


template <typename TKey, uint16_t SizeParam>
inline void CBTree<TKey, SizeParam>::SplitNode(TNodeRef Node, TNodeRef & Left, TNodeRef & Right, TKey & UpKey, TNodeRef ParentNode, uint16_t ParentIndex)
{
	TNode nodedata;
	TNode newnode = {0};
	const uint16_t upindex = SizeParam - 1;

	Left = Node;
	Right = CreateNewNode();
	ReadNode(Node, nodedata);

	TManagedMap::iterator it = m_ManagedIterators.find(Node);
	while ((it != m_ManagedIterators.end()) && (it->first == Node))
	{
		if (it->second->m_Index == upindex)
		{
			it->second->m_Index = ParentIndex;
			it->second->m_Node = ParentNode;
			m_ManagedIterators.insert(std::make_pair(ParentNode, it->second));
			m_ManagedIterators.erase(it++);
		} else if (it->second->m_Index > upindex)
		{
			it->second->m_Index = it->second->m_Index - upindex - 1;
			it->second->m_Node = Right;
			m_ManagedIterators.insert(std::make_pair(Right, it->second));
			m_ManagedIterators.erase(it++);
		} else {
			++it;
		}
	}

	UpKey = nodedata.Key[upindex];

	memcpy(&(newnode.Key[0]), &(nodedata.Key[upindex+1]), sizeof(TKey) * (cFullNode - upindex));
	if ((nodedata.Info & cIsLeafMask) == 0)
	{
		TNode tmp;
		tmp.Parent = Right;

		memcpy(&(newnode.Child[0]), &(nodedata.Child[upindex+1]), sizeof(TNodeRef) * (cFullNode - upindex + 1));

		for (int i = 0; i <= upindex; i++)
		{
			Write(newnode.Child[i], offsetof(TNode, Parent), sizeof(TNodeRef), tmp);
		}
	}

	newnode.Info = (nodedata.Info & cIsLeafMask) | upindex;
	nodedata.Info = newnode.Info;
	newnode.Parent = nodedata.Parent;

	WriteNode(Left, nodedata);
	WriteNode(Right, newnode);
}

template <typename TKey, uint16_t SizeParam>
inline unsigned int CBTree<TKey, SizeParam>::MergeNodes(TNodeRef Left, TNodeRef Right, const TKey & DownKey, TNodeRef ParentNode, uint16_t ParentIndex)
{
	TNode ldata, rdata;
	uint16_t downindex;

	ReadNode(Left, ldata);
	ReadNode(Right, rdata);

	downindex = ldata.Info & cKeyCountMask;
	ldata.Key[downindex] = DownKey;

	TManagedMap::iterator it = m_ManagedIterators.find(Right);
	while ((it != m_ManagedIterators.end()) && (it->first == Right))
	{
		it->second->m_Index = it->second->m_Index + downindex + 1;
		it->second->m_Node = Left;
		m_ManagedIterators.insert(std::make_pair(Left, it->second));
		m_ManagedIterators.erase(it++);
	}

	it = m_ManagedIterators.find(ParentNode);
	while ((it != m_ManagedIterators.end()) && (it->first == ParentNode))
	{
		if (it->second->m_Index == ParentIndex)
		{
			it->second->m_Index = downindex;
			it->second->m_Node = Left;
			m_ManagedIterators.insert(std::make_pair(Left, it->second));
			m_ManagedIterators.erase(it++);
		} else {
			++it;
		}
	}

	memcpy(&(ldata.Key[downindex+1]), &(rdata.Key[0]), sizeof(TKey) * (rdata.Info & cKeyCountMask));
	if ((ldata.Info & cIsLeafMask) == 0)
	{
		TNode tmp;
		tmp.Parent = Left;

		memcpy(&(ldata.Child[downindex+1]), &(rdata.Child[0]), sizeof(TNodeRef) * ((rdata.Info & cKeyCountMask) + 1));

		for (int i = 0; i <= (rdata.Info & cKeyCountMask); i++)
		{
			Write(rdata.Child[i], offsetof(TNode, Parent), sizeof(TNodeRef), tmp);
		}
	}

	ldata.Info = ((ldata.Info & cIsLeafMask) | (downindex + 1 + (rdata.Info & cKeyCountMask)));

	WriteNode(Left, ldata);
	DeleteNode(Right);

	return Left;
}


template <typename TKey, uint16_t SizeParam>
inline void CBTree<TKey, SizeParam>::KeyInsert(TNodeRef Node, TNode & NodeData, uint16_t Where)
{
	memcpy(&(NodeData.Key[Where+1]), &(NodeData.Key[Where]), sizeof(TKey) * ((NodeData.Info & cKeyCountMask) - Where));

	if ((NodeData.Info & cIsLeafMask) == 0)
		memcpy(&(NodeData.Child[Where+1]), &(NodeData.Child[Where]), sizeof(TNodeRef) * ((NodeData.Info & cKeyCountMask) - Where + 1));

	NodeData.Info++;

	TManagedMap::iterator it = m_ManagedIterators.find(Node);
	while ((it != m_ManagedIterators.end()) && (it->first == Node))
	{
		if (it->second->m_Index >= Where)
			it->second->m_Index++;

		++it;
	}
}

template <typename TKey, uint16_t SizeParam>
inline void CBTree<TKey, SizeParam>::KeyDelete(TNodeRef Node, TNode & NodeData, uint16_t Where)
{
	NodeData.Info--;

	TManagedMap::iterator it = m_ManagedIterators.find(Node);
	while ((it != m_ManagedIterators.end()) && (it->first == Node))
	{
		if (it->second->m_Index == Where)
		{
			it->second->Backup();
		} else if (it->second->m_Index > Where)
		{
			it->second->m_Index--;
		}

		++it;
	}

	memcpy(&(NodeData.Key[Where]), &(NodeData.Key[Where+1]), sizeof(TKey) * ((NodeData.Info & cKeyCountMask) - Where));

	if ((NodeData.Info & cIsLeafMask) == 0)
		memcpy(&(NodeData.Child[Where]), &(NodeData.Child[Where+1]), sizeof(TNodeRef) * ((NodeData.Info & cKeyCountMask) - Where + 1));
}


template <typename TKey, uint16_t SizeParam>
inline void CBTree<TKey, SizeParam>::KeyMove(TNodeRef Source, uint16_t SourceIndex, const TNode & SourceData, TNodeRef Dest, uint16_t DestIndex, TNode & DestData)
{
	DestData.Key[DestIndex] = SourceData.Key[SourceIndex];

	TManagedMap::iterator it = m_ManagedIterators.find(Source);
	while ((it != m_ManagedIterators.end()) && (it->first == Source))
	{
		if (it->second->m_Index == SourceIndex)
		{
			it->second->m_Index = DestIndex;
			it->second->m_Node = Dest;
			m_ManagedIterators.insert(std::make_pair(Dest, it->second));
			m_ManagedIterators.erase(it++);
		} else {
			++it;
		}
	}
}

template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::iterator
CBTree<TKey, SizeParam>::Insert(const TKey & Key)
{
	TNode node, node2;
	TNodeRef actnode;
	TNodeRef nextnode;
	bool exists;
	uint16_t ge;

	if (!m_Root)
	{
		m_Root = CreateNewNode();
		node.Info = cIsLeafMask;
		Write(m_Root, offsetof(TNode, Info), sizeof(uint16_t), node);
		m_sigRootChanged.emit(this, m_Root);
	}

	actnode = m_Root;
	ReadNode(actnode, node);
	if ((node.Info & cKeyCountMask) == cFullNode)  // root split
	{
		// be a little tricky and let the main code handle the actual splitting.
		// just assign a new root with keycount to zero and one child = old root
		// the InNode test will fail with GreaterEqual = 0
		nextnode = CreateNewNode();
		node.Info = 0;
		node.Child[0] = actnode;
		Write(nextnode, offsetof(TNode, Info), sizeof(uint16_t), node);
		Write(nextnode, offsetof(TNode, Child[0]), sizeof(TNodeRef), node);

		node.Parent = nextnode;
		Write(actnode, offsetof(TNode, Parent), sizeof(TNodeRef), node);

		ReadNode(nextnode, node);
		actnode = nextnode;

		m_Root = nextnode;
		m_sigRootChanged.emit(this, m_Root);
	}

	while (actnode)
	{
		exists = InNodeFind(node, Key, ge);
		if (exists)  // already exists
		{
			return iterator(this, actnode, ge);
		} else {
			if (node.Info & cIsLeafMask) // direct insert to leaf node
			{
				KeyInsert(actnode, node, ge);

				node.Key[ge] = Key;

				WriteNode(actnode, node);

				return iterator(this, actnode, ge);

			} else {  // middle node
				nextnode = node.Child[ge];
				Read(nextnode, offsetof(TNode, Info), sizeof(uint16_t), node2);

				if ((node2.Info & cKeyCountMask) == cFullNode) // split the childnode
				{
					KeyInsert(actnode, node, ge);
					SplitNode(nextnode, node.Child[ge], node.Child[ge+1], node.Key[ge], actnode, ge);

					if (node.Key[ge] == Key)
					{
						WriteNode(actnode, node);
						return iterator(this, actnode, ge);
					} else {
						WriteNode(actnode, node);
						if (node.Key[ge] < Key)
						{
							nextnode = node.Child[ge+1];
						} else {
							nextnode = node.Child[ge];
						}
					}
				} //split complete

				actnode = nextnode;
				ReadNode(actnode, node);
			} // if (node.Info & cIsLeafMask)
		} // if (exists)
	}	// while (actnode)

	// something went wrong
	return iterator(this, 0, 0xFFFF);
}


template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::iterator
CBTree<TKey, SizeParam>::Find(const TKey & Key)
{
	TNode node;
	TNodeRef actnode = m_Root;
	uint16_t ge;

	if (!m_Root) return iterator(this, 0, 0xFFFF);

	ReadNode(actnode, node);

	while (actnode)
	{
		if (InNodeFind(node, Key, ge))
		{
			return iterator(this, actnode, ge);
		}

		if (!(node.Info & cIsLeafMask))
		{
			actnode = node.Child[ge];
			ReadNode(actnode, node);
		}
	}

	return iterator(this, 0, 0xFFFF);
}

template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::iterator
CBTree<TKey, SizeParam>::LowerBound(const TKey & Key)
{
	TNode node;
	TNodeRef actnode = m_Root;
	uint16_t ge;

	if (!m_Root) return iterator(this, 0, 0xFFFF);

	ReadNode(actnode, node);

	while (actnode)
	{
		if (InNodeFind(node, Key, ge))
		{
			return iterator(this, actnode, ge);
		}

		if (node.Info & cIsLeafMask)
		{
			if (ge >= (node.Info & cKeyCountMask))
			{
				iterator i(this, actnode, ge - 1);
				++i;
				return i;
			} else {
				return iterator(this, actnode, ge);
			}
		} else {
			actnode = node.Child[ge];
			ReadNode(actnode, node);
		}
	}

	return iterator(this, 0, 0xFFFF);
}

template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::iterator
CBTree<TKey, SizeParam>::UpperBound(const TKey & Key)
{
	TNode node;
	TNodeRef actnode = m_Root;
	uint16_t ge;
	if (!m_Root) return iterator(this, 0, 0xFFFF);

	ReadNode(actnode, node);

	while (actnode)
	{
		if (InNodeFind(node, Key, ge))
		{
			return iterator(this, actnode, ge);
		}

		if (node.Info & cIsLeafMask)
		{
			if (ge == 0) 
			{
				iterator i(this, actnode, 0);
				--i;
				return i;
			} else {
				return iterator(this, actnode, ge - 1);
			}
		} else {
			actnode = node.Child[ge];
			ReadNode(actnode, node);
		}
	}

	return iterator(this, 0, 0xFFFF);
}

template <typename TKey, uint16_t SizeParam>
bool CBTree<TKey, SizeParam>::Delete(const TKey& Key)
{
	if (!m_Root) return false;

	TNode node, node2, lnode = {0}, rnode = {0}, tmp;
	TNodeRef actnode = m_Root;
	TNodeRef nextnode, l, r;
	bool exists, skipread;
	uint16_t ge;

	bool foundininnernode = false;
	bool wantleftmost = false;
	TNodeRef innernode = 0;
	uint16_t innerindex = 0xFFFF;

	ReadNode(actnode, node);

	while (actnode)
	{
		skipread = false;

		if (foundininnernode)
		{
			exists = false;
			if (wantleftmost)
				ge = 0;
			else
				ge = node.Info & cKeyCountMask;

		} else {
			exists = InNodeFind(node, Key, ge);
		}

		if (exists)
		{
			if (node.Info & cIsLeafMask)  // delete in leaf
			{
				KeyDelete(actnode, node, ge);
				WriteNode(actnode, node);

				return true;

			} else { // delete in inner node
				l = node.Child[ge];
				r = node.Child[ge+1];
				Read(l, offsetof(TNode, Info), sizeof(uint16_t), lnode);
				Read(r, offsetof(TNode, Info), sizeof(uint16_t), rnode);


				if (((rnode.Info & cKeyCountMask) == cEmptyNode) && ((lnode.Info & cKeyCountMask) == cEmptyNode))
				{ // merge childnodes and keep going
					nextnode = MergeNodes(l, r, node.Key[ge], actnode, ge);

					KeyDelete(actnode, node, ge);
					node.Child[ge] = nextnode;

					if ((actnode == m_Root) && ((node.Info & cKeyCountMask) == 0))
					{ // root node is empty. delete it
						DeleteNode(actnode);
						m_Root = nextnode;
						m_sigRootChanged.emit(this, m_Root);
					} else {
						WriteNode(actnode, node);
					}

				} else { // need a key-data-pair from a leaf to replace deleted pair -> save position
					foundininnernode = true;
					innernode = actnode;
					innerindex = ge;

					if ((lnode.Info & cKeyCountMask) == cEmptyNode)
					{
						wantleftmost = true;
						nextnode = r;
					} else {
						wantleftmost = false;
						nextnode = l;
					}
				}
			}
		} else if (node.Info & cIsLeafMask) { // we are at the bottom. finish it
			if (foundininnernode)
			{
				if (wantleftmost)
				{
					KeyMove(actnode, 0, node, innernode, innerindex, tmp);
					Write(innernode, offsetof(TNode, Key[innerindex]), sizeof(TKey), tmp);

					KeyDelete(actnode, node, 0);
					WriteNode(actnode, node);

				} else {
					KeyMove(actnode, (node.Info & cKeyCountMask) - 1, node, innernode, innerindex, tmp);
					Write(innernode, offsetof(TNode, Key[innerindex]), sizeof(TKey), tmp);

					//KeyDelete(actnode, node, node.Info & cKeyCountMask);
					node.Info--;
					Write(actnode, offsetof(TNode, Info), sizeof(uint16_t), node);
				}
			}
			return foundininnernode;

		} else { // inner node. go on and check if moving or merging is neccessary
			nextnode = node.Child[ge];
			Read(nextnode, offsetof(TNode, Info), sizeof(uint16_t), node2);

			if ((node2.Info & cKeyCountMask) == cEmptyNode) // move or merge
			{
				// set l and r for easier access
				if (ge > 0)
				{
					l = node.Child[ge - 1];
					Read(l, offsetof(TNode, Info), sizeof(uint16_t), lnode);
				} else
					l = 0;

				if (ge < (node.Info & cKeyCountMask))
				{
					r = node.Child[ge + 1];
					Read(r, offsetof(TNode, Info), sizeof(uint16_t), rnode);
				} else
					r = 0;

				if ((r != 0) && ((rnode.Info & cKeyCountMask) > cEmptyNode)) // move a Key-Data-pair from the right
				{
					ReadNode(r, rnode);
					ReadNode(nextnode, node2);

					// move key-data-pair down from current to the next node
					KeyMove(actnode, ge, node, nextnode, node2.Info & cKeyCountMask, node2);

					// move the child from right to next node
					node2.Child[(node2.Info & cKeyCountMask) + 1] = rnode.Child[0];

					// move key-data-pair up from right to current node
					KeyMove(r, 0, rnode, actnode, ge, node);
					Write(actnode, offsetof(TNode, Key[ge]), sizeof(TKey), node);

					// decrement right node key count and remove the first key-data-pair
					KeyDelete(r, rnode, 0);

					// increment KeyCount of the next node
					node2.Info++;

					if ((node2.Info & cIsLeafMask) == 0) // update the parent property of moved child
					{
						tmp.Parent = nextnode;
						Write(node2.Child[node2.Info & cKeyCountMask], offsetof(TNode, Parent), sizeof(TNodeRef), tmp);
					}


					WriteNode(r, rnode);
					WriteNode(nextnode, node2);
					node = node2;
					skipread = true;

				} else if ((l != 0) && ((lnode.Info & cKeyCountMask) > cEmptyNode)) // move a Key-Data-pair from the left
				{
					ReadNode(nextnode, node2);
					ReadNode(l, lnode);

					// increment next node key count and make new first key-data-pair
					KeyInsert(nextnode, node2, 0);

					// move key-data-pair down from current to the next node
					KeyMove(actnode, ge - 1, node, nextnode, 0, node2);

					// move the child from left to next node
					node2.Child[0] = lnode.Child[lnode.Info & cKeyCountMask];

					// move key-data-pair up from left to current node
					KeyMove(l, (lnode.Info & cKeyCountMask) - 1, lnode, actnode, ge - 1, node);
					Write(actnode, offsetof(TNode, Key[ge - 1]), sizeof(TKey), node);

					// decrement left node key count
					lnode.Info--;
					Write(l, offsetof(TNode, Info), sizeof(uint16_t), lnode);

					if ((node2.Info & cIsLeafMask) == 0) // update the parent property of moved child
					{
						tmp.Parent = nextnode;
						Write(node2.Child[0], offsetof(TNode, Parent), sizeof(TNodeRef), tmp);
					}

					WriteNode(nextnode, node2);
					node = node2;
					skipread = true;

				} else {
					if (l != 0) // merge with the left node
					{
						nextnode = MergeNodes(l, nextnode, node.Key[ge - 1], actnode, ge - 1);
						KeyDelete(actnode, node, ge - 1);
						node.Child[ge - 1] = nextnode;

					} else { // merge with the right node
						nextnode = MergeNodes(nextnode, r, node.Key[ge], actnode, ge);
						KeyDelete(actnode, node, ge);
						node.Child[ge] = nextnode;
					}

					if ((actnode == m_Root) && ((node.Info & cKeyCountMask) == 0))
					{
						DeleteNode(actnode);
						m_Root = nextnode;
						m_sigRootChanged(this, nextnode);
					} else {
						WriteNode(actnode, node);
					}
				}
			}
		} // if (exists) else if (node.Info & cIsLeafMask)

		actnode = nextnode;
		if (!skipread)
			ReadNode(actnode, node);

	} // while(actnode)

	return false;
}

template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::TNodeRef CBTree<TKey, SizeParam>::getRoot()
{
	return m_Root;
}

template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::setRoot(TNodeRef NewRoot)
{
	m_Root = NewRoot;
	return;
}

template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::TNodeRef CBTree<TKey, SizeParam>::CreateNewNode()
{
	TNode* node = new TNode;
	memset(node, 0, sizeof(TNode));
	return (TNodeRef) node;
}

template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::DeleteNode(TNodeRef Node)
{
	delete (TNode*)Node;
}

template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::ReadNode(TNodeRef Node, TNode & Dest)
{
	Read(Node, 0, sizeof(TNode), Dest);
}

template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::WriteNode(TNodeRef Node, TNode & Source)
{
	Write(Node, 0, sizeof(TNode), Source);
}

template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::Read(TNodeRef Node, uint32_t Offset, uint32_t Size, TNode & Dest)
{
	memcpy((uint8_t*)(&Dest) + Offset, (void*)(Node + Offset), Size);
}

template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::Write(TNodeRef Node, uint32_t Offset, uint32_t Size, TNode & Source)
{
	memcpy((void*)(Node + Offset), (uint8_t*)(&Source) + Offset, Size);
}

template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::DestroyTree()
{
	std::stack<TNode*> s;
	TNode* node;
	uint16_t i;

	if (m_Root)
		s.push((TNode*)m_Root);
	while (!s.empty())
	{
		node = s.top();
		s.pop();

		if ((node->Info & cIsLeafMask) == 0)
			for (i = 0; i <= (node->Info & cKeyCountMask); i++)
				s.push((TNode*)node->Child[i]);

		delete node;
	}
}


template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::DeleteTree(TDeleteCallback * CallBack, uint32_t Param)
{
	std::stack<TNodeRef> s;
	TNodeRef actnode;
	TNode node;
	uint16_t i;

	TManagedMap::iterator it = m_ManagedIterators.begin();
	while (it != m_ManagedIterators.end())
	{
		it->second->m_Node = 0;
		it->second->m_Index = 0xffff;
		++it;
	}

	if (m_Root)
		s.push(m_Root);

	m_Root = 0;
	m_sigRootChanged.emit(this, m_Root);

	while (!s.empty())
	{
		actnode = s.top();
		s.pop();

		ReadNode(actnode, node);

		if ((node.Info & cIsLeafMask) == 0)
		{
			for (i = 0; i <= (node.Info & cKeyCountMask); i++)
			{
				s.push(node.Child[i]);
				if (CallBack)
					CallBack->emit(this, node.Key[i], Param);
			}

		} else if (CallBack)
		{
			for (i = 0; i <= (node.Info & cKeyCountMask); i++)
				CallBack->emit(this, node.Key[i], Param);
		}

		DeleteNode(actnode);
	}
}







template <typename TKey, uint16_t SizeParam>
CBTree<TKey, SizeParam>::iterator::iterator()
{
	m_Tree = NULL;
	m_Node = 0;
	m_Index = 0xFFFF;
	m_Managed = false;
	m_ManagedDeleted = false;
	m_LoadedKey = false;
}
template <typename TKey, uint16_t SizeParam>
CBTree<TKey, SizeParam>::iterator::iterator(CBTree* Tree, TNodeRef Node, uint16_t Index)
{
	m_Tree = Tree;
	m_Node = Node;
	m_Index = Index;
	m_Managed = false;
	m_ManagedDeleted = false;
	m_LoadedKey = false;
}
template <typename TKey, uint16_t SizeParam>
CBTree<TKey, SizeParam>::iterator::iterator(const iterator& Other)
{
	m_Tree = Other.m_Tree;
	m_Node = Other.m_Node;
	m_Index = Other.m_Index;
	m_ManagedDeleted = Other.m_ManagedDeleted;
	m_Managed = Other.m_Managed;
	m_LoadedKey = Other.m_LoadedKey;
	m_ManagedKey = Other.m_ManagedKey;

	if (m_Managed)
		InsertManaged();
}

template <typename TKey, uint16_t SizeParam>
CBTree<TKey, SizeParam>::iterator::~iterator()
{
	RemoveManaged(m_Node);
}


template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::iterator::setManaged()
{
	if (!m_Managed)
		InsertManaged();

	m_Managed = true;
}

template <typename TKey, uint16_t SizeParam>
inline void CBTree<TKey, SizeParam>::iterator::RemoveManaged(TNodeRef FromNode)
{
	if (m_Managed && m_Tree)
	{
		TManagedMap::iterator i = m_Tree->m_ManagedIterators.find(FromNode);

		while ((i != m_Tree->m_ManagedIterators.end()) && (i->second != this) && (i->first == FromNode))
			++i;

		if ((i != m_Tree->m_ManagedIterators.end()) && (i->second == this))
			m_Tree->m_ManagedIterators.erase(i);
	}
}
template <typename TKey, uint16_t SizeParam>
inline void CBTree<TKey, SizeParam>::iterator::InsertManaged()
{
	if (m_Tree)
		m_Tree->m_ManagedIterators.insert(std::make_pair(m_Node, this));
}

template <typename TKey, uint16_t SizeParam>
bool CBTree<TKey, SizeParam>::iterator::wasDeleted()
{
	return m_ManagedDeleted;
}

template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::iterator::Backup()
{
	if ((!m_ManagedDeleted) && (*this))
	{
		TNode tmp;
		if (!m_LoadedKey)
		{
			m_Tree->Read(m_Node, offsetof(TNode, Key[m_Index]), sizeof(TKey), tmp);
			m_ManagedKey = tmp.Key[m_Index];
		}
		m_LoadedKey = true;
	}

	m_ManagedDeleted = true;
}

template <typename TKey, uint16_t SizeParam>
CBTree<TKey, SizeParam> * CBTree<TKey, SizeParam>::iterator::Tree()
{
	return m_Tree;
}

template <typename TKey, uint16_t SizeParam>
const TKey& CBTree<TKey, SizeParam>::iterator::operator *()
{
	if (!m_LoadedKey)
	{
		TNode node;
		m_Tree->Read(m_Node, offsetof(TNode, Key[m_Index]), sizeof(TKey), node);
		m_ManagedKey = node.Key[m_Index];
		m_LoadedKey = true;
	}
	return m_ManagedKey;
}

template <typename TKey, uint16_t SizeParam>
const TKey* CBTree<TKey, SizeParam>::iterator::operator ->()
{
	if (!m_LoadedKey)
	{
		TNode node;
		m_Tree->Read(m_Node, offsetof(TNode, Key[m_Index]), sizeof(TKey), node);
		m_ManagedKey = node.Key[m_Index];
		m_LoadedKey = true;
	}
	return &m_ManagedKey;
}

template <typename TKey, uint16_t SizeParam>
inline CBTree<TKey, SizeParam>::iterator::operator bool() const
{
	if (m_Tree && m_Node)
	{
		TNode node;
		m_Tree->Read(m_Node, offsetof(TNode, Info), sizeof(uint16_t), node);
		return (m_Index < (node.Info & cKeyCountMask));
	} else
		return false;
}

template <typename TKey, uint16_t SizeParam>
inline bool CBTree<TKey, SizeParam>::iterator::operator !() const
{
	if (m_Tree && m_Node)
	{
		TNode node;
		m_Tree->Read(m_Node, offsetof(TNode, Info), sizeof(uint16_t), node);
		return (m_Index > (node.Info & cKeyCountMask));
	} else
		return true;
}

template <typename TKey, uint16_t SizeParam>
inline bool CBTree<TKey, SizeParam>::iterator::operator ==(iterator & Other)
{
	//return (m_Tree == Other.m_Tree) && (m_Node == Other.m_Node) && (m_Index == Other.m_Index) && (!m_ManagedDeleted) && (!Other.m_ManagedDeleted);
	return Key() == Other.Key();
}

template <typename TKey, uint16_t SizeParam>
inline bool CBTree<TKey, SizeParam>::iterator::operator <  (iterator & Other)
{
	return Key() < Other.Key();
}
template <typename TKey, uint16_t SizeParam>
inline bool CBTree<TKey, SizeParam>::iterator::operator >  (iterator & Other)
{
	return Key() > Other.Key();
}


template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::iterator&
CBTree<TKey, SizeParam>::iterator::operator =(const iterator& Other)
{
	RemoveManaged(m_Node);

	m_Tree = Other.m_Tree;
	m_Node = Other.m_Node;
	m_Index = Other.m_Index;
	m_ManagedDeleted = Other.m_ManagedDeleted;
	m_Managed = Other.m_Managed;
	m_LoadedKey = Other.m_LoadedKey;
	m_ManagedKey = Other.m_ManagedKey;

	if (m_Managed)
		InsertManaged();

	return *this;
}

template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::iterator&
CBTree<TKey, SizeParam>::iterator::operator ++() //pre  ++i
{
	TNodeRef oldnode = m_Node;
	if (m_Managed && m_ManagedDeleted)
	{
		TKey oldkey = m_ManagedKey;
		m_LoadedKey = false;
		m_ManagedDeleted = false;
		iterator other = m_Tree->LowerBound(m_ManagedKey);	
		m_Node = other.m_Node;		
		m_Index = other.m_Index;
		while (((**this) == oldkey) && (*this))
			Inc();

	} else
		Inc();

	if (m_Managed && (oldnode != m_Node))
	{
		RemoveManaged(oldnode);
		InsertManaged();
	}
	return *this;
}

template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::iterator&
CBTree<TKey, SizeParam>::iterator::operator --() //pre  --i
{
	TNodeRef oldnode = m_Node;
	if (m_Managed && m_ManagedDeleted)
	{
		TKey oldkey = m_ManagedKey;
		m_LoadedKey = false;

		m_ManagedDeleted = false;
		iterator other = m_Tree->UpperBound(m_ManagedKey);
		m_Node = other.m_Node;
		m_Index = other.m_Index;
		while (((**this) == oldkey) && (*this))
			Dec();
	} else
		Dec();

	if (m_Managed && (oldnode != m_Node))
	{
		RemoveManaged(oldnode);
		InsertManaged();
	}
	return *this;
}

template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::iterator
CBTree<TKey, SizeParam>::iterator::operator ++(int) //post i++
{
	iterator tmp(*this);
	++(*this);
	return tmp;
}
template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::iterator
CBTree<TKey, SizeParam>::iterator::operator --(int) //post i--
{
	iterator tmp(*this);
	--(*this);
	return tmp;
}

template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::iterator::Inc()
{
	TNode node;
	TNodeRef nextnode;
	m_Tree->Read(m_Node, offsetof(TNode,Info), sizeof(uint16_t), node);
	m_Tree->Read(m_Node, offsetof(TNode,Parent), sizeof(TNodeRef), node);

	m_LoadedKey = false;

	if ((node.Info & cIsLeafMask) && ((node.Info & cKeyCountMask) > m_Index + 1)) // leaf
	{
		m_Index++;
		return;
	}

	if ((node.Info & cIsLeafMask) == 0) // inner node. go down
	{
		m_Tree->Read(m_Node, offsetof(TNode, Child[m_Index + 1]), sizeof(TNodeRef), node);
		m_Node = node.Child[m_Index + 1];
		m_Tree->Read(m_Node, offsetof(TNode, Info), sizeof(uint16_t), node);

		m_Index = 0;

		while ((node.Info & cIsLeafMask) == 0)  // go down to a leaf
		{
			m_Tree->Read(m_Node, offsetof(TNode, Child[0]), sizeof(TNodeRef), node);
			m_Node = node.Child[0];
			m_Tree->Read(m_Node, offsetof(TNode, Info), sizeof(uint16_t), node);
		}

		return;
	}

	while (m_Index >= (node.Info & cKeyCountMask) - 1) // go up
	{
		if (m_Node == m_Tree->m_Root) // the root is the top, we cannot go further
		{
			m_Index = 0xFFFF;
			m_Node = 0;
			return;
		}

		nextnode = node.Parent;
		m_Tree->ReadNode(nextnode, node);
		m_Index = 0;

		while ((m_Index <= (node.Info & cKeyCountMask)) && (node.Child[m_Index] != m_Node))
			m_Index++;

		m_Node = nextnode;

		if (m_Index < (node.Info & cKeyCountMask))
			return;
	}

}

template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::iterator::Dec()
{
	TNode node;
	TNodeRef nextnode;
	m_Tree->Read(m_Node, offsetof(TNode,Info), sizeof(uint16_t), node);
	m_Tree->Read(m_Node, offsetof(TNode,Parent), sizeof(TNodeRef), node);

	m_LoadedKey = false;

	if ((node.Info & cIsLeafMask) && (m_Index > 0)) // leaf
	{
		m_Index--;
		return;
	}

	if ((node.Info & cIsLeafMask) == 0) // inner node. go down
	{
		m_Tree->Read(m_Node, offsetof(TNode, Child[m_Index]), sizeof(TNodeRef), node);
		m_Node = node.Child[m_Index];
		m_Tree->Read(m_Node, offsetof(TNode, Info), sizeof(uint16_t), node);
		m_Index = (node.Info & cKeyCountMask) - 1;

		while ((node.Info & cIsLeafMask) == 0)  // go down to a leaf
		{
			m_Tree->Read(m_Node, offsetof(TNode, Child[node.Info & cKeyCountMask]), sizeof(TNodeRef), node);
			m_Node = node.Child[node.Info & cKeyCountMask];
			m_Tree->Read(m_Node, offsetof(TNode, Info), sizeof(uint16_t), node);
			m_Index = (node.Info & cKeyCountMask) - 1;
		}

		return;
	}

	while (m_Index == 0) // go up
	{
		if (m_Node == m_Tree->m_Root) // the root is the top, we cannot go further
		{
			m_Index = 0xFFFF;
			m_Node = 0;
			return;
		}

		nextnode = node.Parent;
		m_Tree->ReadNode(nextnode, node);
		m_Index = 0;

		while ((m_Index <= (node.Info & cKeyCountMask)) && (node.Child[m_Index] != m_Node))
			m_Index++;
		
		m_Node = nextnode;

		if (m_Index > 0)
		{
			m_Index--;
			return;
		}
	}
}
