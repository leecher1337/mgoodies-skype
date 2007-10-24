#pragma once

#include <stack>
#include <map>
#include "sigslot.h"

typedef struct {} TEmpty;

template <typename TKey, typename TData = TEmpty, int SizeParam = 4, bool UniqueKeys = true>
class CBTree 
{
public:
	typedef sigslot::signal2< void *, unsigned int> TOnRootChanged;

	class iterator 
	{
	public:
		iterator();
		iterator(CBTree* Tree, unsigned int Node, int Index);
		iterator(const iterator& Other);
		~iterator();

		TKey& Key();
		TData& Data();
		void SetData(const TData & Data);

		/**
			\brief Keeps track of changes in the tree and refresh the iterator

			There are a few possibilities on not UniqueKey-trees where the managing can fail.
			It can occour when deleting an iterated Key and have some other entries with same Key...
			This can cause Values to be enumerated twice.
		**/
		void setManaged();
		bool wasDeleted();

		operator bool() const;
		bool operator !() const;
		
		bool operator ==(const iterator& Other) const;
		iterator& operator =(const iterator& Other);
		iterator& operator ++(); //pre  ++i
		iterator& operator --(); //pre  --i
		iterator operator ++(int); //post i++
		iterator operator --(int); //post i--

	protected:
		friend class CBTree; 

		unsigned int m_Node;
		int m_Index;
		CBTree* m_Tree;		

		bool m_Managed, m_LoadedKey, m_LoadedData;
		TKey m_ManagedKey;
		TData m_ManagedData;
		bool m_ManagedDeleted;

		void Backup();
		void Dec();
		void Inc();
	};


	CBTree(unsigned int RootNode);
	virtual ~CBTree();

	iterator Insert(const TKey & Key, const TData & Data);
	iterator Find(const TKey & Key);
	iterator LowerBound(const TKey & Key);
	iterator UpperBound(const TKey & Key);
	bool Delete(const TKey& Key);
	void Delete(iterator& Item);

	unsigned int getRoot();
	void setRoot(unsigned int NewRoot);

	TOnRootChanged & sigRootChanged() {return m_sigRootChanged;};


protected:
	#pragma pack(push)  /* push current alignment to stack */
	#pragma pack(1)     /* set alignment to 1 byte boundary */

		typedef struct TNode {
			unsigned int Parent;               /// Handle to the parent node
			unsigned char Info;                /// Node information (IsLeaf and stored KeyCount)
			TKey Key[SizeParam * 2 - 1];       /// array with Keys
			TData Data[SizeParam * 2 - 1];     /// array with Data
			unsigned int Child[SizeParam * 2]; /// array with child node handles
		} TNode;

	#pragma pack(pop)

	static const unsigned char cIsLeafMask = 0x80;
	static const unsigned char cKeyCountMask = 0x7F;
	static const unsigned char cFullNode = SizeParam * 2 - 1;
	static const unsigned char cEmptyNode = SizeParam - 1;

	typedef std::multimap<unsigned int, iterator*> TManagedMap;

	unsigned int m_Root;
	TOnRootChanged m_sigRootChanged;
	TManagedMap	m_ManagedIterators;

	virtual unsigned int CreateNewNode();
	virtual void DeleteNode(unsigned int Node);
	virtual void Read(unsigned int Node, int Offset, int Size, TNode & Dest);
	virtual void Write(unsigned int Node, int Offset, int Size, TNode & Source);
	
	virtual void DestroyTree();


private:
	friend class iterator;

	bool InNodeFind(const TNode & Node, const TKey & Key, int & GreaterEqual);
	void SplitNode(unsigned int Node, unsigned int & Left, unsigned int & Right, TKey & UpKey, TData & UpData, unsigned int ParentNode, int ParentIndex);
	unsigned int MergeNodes(unsigned int Left, unsigned int Right, const TKey & DownKey, const TData & DownData, unsigned int ParentNode, int ParentIndex);
	void KeyInsert(unsigned int Node, TNode & NodeData, int Where);
	void KeyDelete(unsigned int Node, TNode & NodeData, int Where);
	void KeyMove(unsigned int Source, int SourceIndex, const TNode & SourceData, unsigned int Dest, int DestIndex, TNode & DestData);
	
	void ReadNode(unsigned int Node, TNode & Dest);
	void WriteNode(unsigned int Node, TNode & Source);

};






template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
CBTree<TKey, TData, SizeParam, UniqueKeys>::CBTree(unsigned int RootNode = NULL)
: m_sigRootChanged(),
	m_ManagedIterators()
{
	m_Root = RootNode;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
CBTree<TKey, TData, SizeParam, UniqueKeys>::~CBTree()
{
	TManagedMap::iterator i = m_ManagedIterators.begin();
	while (i != m_ManagedIterators.end())
	{
		i->second->m_Tree = NULL;
		i++;
	}
	m_ManagedIterators.clear();
	DestroyTree();
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
bool CBTree<TKey, TData, SizeParam, UniqueKeys>::InNodeFind(const TNode & Node, const TKey & Key, int & GreaterEqual)
{
	GreaterEqual = 0; 
	while ((GreaterEqual < (Node.Info & cKeyCountMask)) && (Node.Key[GreaterEqual] < Key))
	{
		GreaterEqual++;
	}

	return ((GreaterEqual < (Node.Info & cKeyCountMask)) && (Node.Key[GreaterEqual] == Key));
}


template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
__forceinline void CBTree<TKey, TData, SizeParam, UniqueKeys>::SplitNode(unsigned int Node, unsigned int & Left, unsigned int & Right, TKey & UpKey, TData & UpData, unsigned int ParentNode, int ParentIndex)
{
	TNode nodedata;
	TNode newnode = {0};
	const int upindex = SizeParam - 1;

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
	UpData = nodedata.Data[upindex];
	
	memcpy(&(newnode.Key[0]), &(nodedata.Key[upindex+1]), sizeof(TKey) * (cFullNode - upindex));
	memcpy(&(newnode.Data[0]), &(nodedata.Data[upindex+1]), sizeof(TData) * (cFullNode - upindex));
	if ((nodedata.Info & cIsLeafMask) == 0)
	{
		TNode tmp;
		tmp.Parent = Right;

		memcpy(&(newnode.Child[0]), &(nodedata.Child[upindex+1]), sizeof(unsigned int) * (cFullNode - upindex + 1));
		
		for (int i = 0; i <= upindex; i++)
		{
			Write(newnode.Child[i], offsetof(TNode, Parent), sizeof(unsigned int), tmp);
		}
	}

	newnode.Info = (nodedata.Info & cIsLeafMask) | (char)(upindex);
	nodedata.Info = newnode.Info;
	newnode.Parent = nodedata.Parent;

	WriteNode(Left, nodedata);
	WriteNode(Right, newnode);
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
__forceinline unsigned int CBTree<TKey, TData, SizeParam, UniqueKeys>::MergeNodes(unsigned int Left, unsigned int Right, const TKey & DownKey, const TData & DownData, unsigned int ParentNode, int ParentIndex)
{
	TNode ldata, rdata;
	char downindex;

	ReadNode(Left, ldata);
	ReadNode(Right, rdata);

	downindex = ldata.Info & cKeyCountMask;
	ldata.Key[downindex] = DownKey;
	ldata.Data[downindex] = DownData;

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
	memcpy(&(ldata.Data[downindex+1]), &(rdata.Data[0]), sizeof(TData) * (rdata.Info & cKeyCountMask));
	if ((ldata.Info & cIsLeafMask) == 0)
	{
		TNode tmp;
		tmp.Parent = Left;

		memcpy(&(ldata.Child[downindex+1]), &(rdata.Child[0]), sizeof(unsigned int) * ((rdata.Info & cKeyCountMask) + 1));
		
		for (int i = 0; i <= (rdata.Info & cKeyCountMask); i++)
		{
			Write(rdata.Child[i], offsetof(TNode, Parent), sizeof(unsigned int), tmp);
		}
	}

	ldata.Info = (unsigned char) ((ldata.Info & cIsLeafMask) | (downindex + 1 + (rdata.Info & cKeyCountMask)));
	
	WriteNode(Left, ldata);
	DeleteNode(Right);

	return Left;
}


template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
__forceinline void CBTree<TKey, TData, SizeParam, UniqueKeys>::KeyInsert(unsigned int Node, TNode & NodeData, int Where)
{
	memcpy(&(NodeData.Key[Where+1]), &(NodeData.Key[Where]), sizeof(TKey) * ((NodeData.Info & cKeyCountMask) - Where));
	memcpy(&(NodeData.Data[Where+1]), &(NodeData.Data[Where]), sizeof(TData) * ((NodeData.Info & cKeyCountMask) - Where));

	if ((NodeData.Info & cIsLeafMask) == 0)
		memcpy(&(NodeData.Child[Where+1]), &(NodeData.Child[Where]), sizeof(unsigned int) * ((NodeData.Info & cKeyCountMask) - Where + 1));

	NodeData.Info++;

	TManagedMap::iterator it = m_ManagedIterators.find(Node);
	while ((it != m_ManagedIterators.end()) && (it->first == Node))
	{
		if (it->second->m_Index >= Where)
			it->second->m_Index++;

		++it;
	}
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
__forceinline void CBTree<TKey, TData, SizeParam, UniqueKeys>::KeyDelete(unsigned int Node, TNode & NodeData, int Where)
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
	memcpy(&(NodeData.Data[Where]), &(NodeData.Data[Where+1]), sizeof(TData) * ((NodeData.Info & cKeyCountMask) - Where));

	if ((NodeData.Info & cIsLeafMask) == 0)
		memcpy(&(NodeData.Child[Where]), &(NodeData.Child[Where+1]), sizeof(unsigned int) * ((NodeData.Info & cKeyCountMask) - Where + 1));
}


template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
__forceinline void CBTree<TKey, TData, SizeParam, UniqueKeys>::KeyMove(unsigned int Source, int SourceIndex, const TNode & SourceData, unsigned int Dest, int DestIndex, TNode & DestData)
{
	DestData.Key[DestIndex] = SourceData.Key[SourceIndex];
	DestData.Data[DestIndex] = SourceData.Data[SourceIndex];

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

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
typename CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator 
CBTree<TKey, TData, SizeParam, UniqueKeys>::Insert(const TKey & Key, const TData & Data)
{
	TNode node, node2;
	unsigned int actnode;
	unsigned int nextnode;
	bool exists;
	int ge;

	if (!m_Root)
	{
		m_Root = CreateNewNode();
		node.Info = cIsLeafMask;
		Write(m_Root, offsetof(TNode, Info), sizeof(unsigned char), node);
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
		Write(nextnode, offsetof(TNode, Info), sizeof(unsigned char), node);
		Write(nextnode, offsetof(TNode, Child[0]), sizeof(unsigned int), node);

		node.Parent = nextnode;
		Write(actnode, offsetof(TNode, Parent), sizeof(unsigned int), node);
		
		ReadNode(nextnode, node);
		actnode = nextnode;

		m_Root = nextnode;
		m_sigRootChanged.emit(this, m_Root);
	}

	while (actnode)
	{
		exists = InNodeFind(node, Key, ge);
		if (UniqueKeys && exists)  // already exists
		{
			node.Data[ge] = Data;
			Write(actnode, offsetof(TNode, Data[ge]), sizeof(TData), node); // overwrite data and exit
			return iterator(this, actnode, ge);
		} else {				
			if (node.Info & cIsLeafMask) // direct insert to leaf node
			{
				KeyInsert(actnode, node, ge);

				node.Key[ge] = Key;
				node.Data[ge] = Data;
				
				WriteNode(actnode, node);

				return iterator(this, actnode, ge);

			} else {  // middle node
				nextnode = node.Child[ge];
				Read(nextnode, offsetof(TNode, Info), sizeof(unsigned char), node2);

				if ((node2.Info & cKeyCountMask) == cFullNode) // split the childnode
				{
					KeyInsert(actnode, node, ge);
					SplitNode(nextnode, node.Child[ge], node.Child[ge+1], node.Key[ge], node.Data[ge], actnode, ge);
					
					if (UniqueKeys && (node.Key[ge] == Key))
					{
						node.Data[ge] = Data;  // overwrite data and exit
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
	return iterator(this, 0,-1);
}


template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
typename CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator 
CBTree<TKey, TData, SizeParam, UniqueKeys>::Find(const TKey & Key)
{
	TNode node;
	unsigned int actnode = m_Root;
	int ge;
	unsigned int foundnode = 0;
	int foundindex = -1;

	if (!m_Root) return iterator(this, 0, -1);

	ReadNode(actnode, node);

	while (actnode)
	{
		if (InNodeFind(node, Key, ge))
		{
			if (UniqueKeys)
			{
				return iterator(this, actnode, ge);
			} else {
				foundnode = actnode;
				foundindex = ge;
			}
		} 
		
		if (node.Info & cIsLeafMask)
		{
			return iterator(this, foundnode, foundindex);
		} else {
			actnode = node.Child[ge];
			ReadNode(actnode, node);
		}
	}	

	return iterator(this, 0, -1);
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
typename CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator 
CBTree<TKey, TData, SizeParam, UniqueKeys>::LowerBound(const TKey & Key)
{
	TNode node;
	unsigned int actnode = m_Root;
	int ge;
	unsigned int foundnode = 0;
	int foundindex = -1;

	if (!m_Root) return iterator(this, 0, -1);

	ReadNode(actnode, node);

	while (actnode)
	{
		if (InNodeFind(node, Key, ge))
		{
			if (UniqueKeys)
			{
				return iterator(this, actnode, ge);
			} else {
				foundnode = actnode;
				foundindex = ge;
			}
		} 
		
		if (node.Info & cIsLeafMask)
		{
			if (foundindex > -1)
			{
				return iterator(this, foundnode, foundindex);
			} else if (ge >= (node.Info & cKeyCountMask)) 
			{
				iterator i(this, actnode, ge);
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

	return iterator(this, 0, -1);
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
typename CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator 
CBTree<TKey, TData, SizeParam, UniqueKeys>::UpperBound(const TKey & Key)
{
	TNode node;
	unsigned int actnode = m_Root;
	int ge;
	unsigned int foundnode = 0;
	int foundindex = -1;

	if (!m_Root) return iterator(this, 0, -1);

	ReadNode(actnode, node);

	while (actnode)
	{
		if (InNodeFind(node, Key, ge))
		{
			if (UniqueKeys)
			{
				return iterator(this, actnode, ge);
			} else {
				foundnode = actnode;
				foundindex = ge;
				while ((ge < node.Info & cKeyCountMask) && (node.Key[ge] == Key))
				{
					foundindex = ge;
					ge++;
				}
			}
		} 
		
		if (node.Info & cIsLeafMask)
		{
			if (foundindex > -1)
			{
				return iterator(this, foundnode, foundindex);
			} else if (ge == 0) {
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

	return iterator(this, 0, -1);
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
bool CBTree<TKey, TData, SizeParam, UniqueKeys>::Delete(const TKey& Key)
{
	if (!m_Root) return false;

	TNode node, node2, lnode = {0}, rnode = {0}, tmp;
	unsigned int actnode = m_Root;
	unsigned int nextnode, l, r;
	bool exists, skipread;
	int ge;

	bool foundininnernode = false;
	bool wantleftmost = false;
	unsigned int innernode = 0;
	int innerindex = -1;

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
				Read(l, offsetof(TNode, Info), sizeof(unsigned char), lnode);
				Read(r, offsetof(TNode, Info), sizeof(unsigned char), rnode);


				if (((rnode.Info & cKeyCountMask) == cEmptyNode) && ((lnode.Info & cKeyCountMask) == cEmptyNode))
				{ // merge childnodes and keep going
					nextnode = MergeNodes(l, r, node.Key[ge], node.Data[ge], actnode, ge);
					
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
					Write(innernode, offsetof(TNode, Data[innerindex]), sizeof(TData), tmp);

					KeyDelete(actnode, node, 0);					
					WriteNode(actnode, node);
									
				} else {
					KeyMove(actnode, (node.Info & cKeyCountMask) - 1, node, innernode, innerindex, tmp);
					Write(innernode, offsetof(TNode, Key[innerindex]), sizeof(TKey), tmp);
					Write(innernode, offsetof(TNode, Data[innerindex]), sizeof(TData), tmp);

					//KeyDelete(actnode, node, node.Info & cKeyCountMask);
					node.Info--;
					Write(actnode, offsetof(TNode, Info), sizeof(unsigned char), node);
				}				
			}
			return foundininnernode;

		} else { // inner node. go on and check if moving or merging is neccessary
			nextnode = node.Child[ge];
			Read(nextnode, offsetof(TNode, Info), sizeof(unsigned char), node2);

			if ((node2.Info & cKeyCountMask) == cEmptyNode) // move or merge
			{
				// set l and r for easier access
				if (ge > 0)	
				{
					l = node.Child[ge - 1]; 
					Read(l, offsetof(TNode, Info), sizeof(unsigned char), lnode);
				} else 
					l = 0;

				if (ge < (node.Info & cKeyCountMask))	
				{
					r = node.Child[ge + 1];
					Read(r, offsetof(TNode, Info), sizeof(unsigned char), rnode);
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
					Write(actnode, offsetof(TNode, Data[ge]), sizeof(TData), node);

					// decrement right node key count and remove the first key-data-pair
					KeyDelete(r, rnode, 0);

					// increment KeyCount of the next node
					node2.Info++;

					if ((node2.Info & cIsLeafMask) == 0) // update the parent property of moved child
					{						
						tmp.Parent = nextnode;
						Write(node2.Child[node2.Info & cKeyCountMask], offsetof(TNode, Parent), sizeof(unsigned int), tmp);
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
					Write(actnode, offsetof(TNode, Data[ge - 1]), sizeof(TData), node);

					// decrement left node key count
					lnode.Info--;
					Write(l, offsetof(TNode, Info), sizeof(unsigned char), lnode);

					if ((node2.Info & cIsLeafMask) == 0) // update the parent property of moved child
					{
						tmp.Parent = nextnode;
						Write(node2.Child[0], offsetof(TNode, Parent), sizeof(unsigned int), tmp);
					}

					WriteNode(nextnode, node2);
					node = node2;
					skipread = true;			

				} else {
					if (l != 0) // merge with the left node
					{
						nextnode = MergeNodes(l, nextnode, node.Key[ge - 1], node.Data[ge - 1], actnode, ge - 1);
						KeyDelete(actnode, node, ge - 1);
						node.Child[ge - 1] = nextnode;

					} else { // merge with the right node
						nextnode = MergeNodes(nextnode, r, node.Key[ge], node.Data[ge], actnode, ge);
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

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CBTree<TKey, TData, SizeParam, UniqueKeys>::Delete(iterator& Item)
{
	if (Item.m_Tree != this)
		throw "Iterator from wrong tree!";

	if (!Item)
		throw "Iterator out of range!";

	if (!m_Root) return;

	bool wasmanaged = Item.m_Managed;

	if (Item.m_Managed)
	{
		Item.m_Managed = false;
		TManagedMap::iterator it = m_ManagedIterators.find(Item.m_Node);
		while ((it != m_ManagedIterators.end()) && (it->second != &Item))
			++it;

		if (it != m_ManagedIterators.end())
			m_ManagedIterators.erase(it);
	}

	TNode node;

	ReadNode(Item.m_Node, node);

	if (((node.Info & cKeyCountMask) > cEmptyNode) && (node.Info & cIsLeafMask)) // the easy one: delete in a leaf which is not empty
	{
		KeyDelete(Item.m_Node, node, Item.m_Index);
		WriteNode(Item.m_Node, node);

		if ((node.Info & cKeyCountMask) == Item.m_Index) // deleted last item, go up
		{
			++Item;
		}

	} else { // the hard one :(

		TNode node2, lnode = {0}, rnode = {0}, tmp;
		unsigned int actnode = m_Root;
		unsigned int nextnode, l, r;
		bool exists, skipread;
		int ge;

		bool foundininnernode = false;
		bool wantleftmost = false;
		unsigned int innernode = 0;
		int innerindex = -1;

		std::stack<unsigned int> path;

		actnode = Item.m_Node;
		while (actnode != m_Root)
		{
			path.push(actnode);
			Read(actnode, offsetof(TNode, Parent), sizeof(unsigned int), node);
			actnode = node.Parent;			
		}

		//actnode == m_Root

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

			} else if (actnode == Item.m_Node)
			{				
				exists = true;
				ge = Item.m_Index;
			} else {
				exists = false;
				nextnode = path.top();
				path.pop();

				ge = 0;
				while ((ge <= (node.Info & cKeyCountMask)) && (node.Child[ge] != nextnode))
					ge++;

			}

			if (exists)
			{
				if (node.Info & cIsLeafMask)  // delete in leaf
				{
					KeyDelete(actnode, node, ge);
					WriteNode(actnode, node);

					if ((node.Info & cKeyCountMask) == Item.m_Index) // deleted last item, go up
					{
						Item++;
					}

					if (wasmanaged)
						Item.setManaged();

					return;

				} else { // delete in inner node
					l = node.Child[ge];
					r = node.Child[ge+1];
					Read(l, offsetof(TNode, Info), sizeof(unsigned char), lnode);
					Read(r, offsetof(TNode, Info), sizeof(unsigned char), rnode);

					if (((rnode.Info & cKeyCountMask) == cEmptyNode) && ((lnode.Info & cKeyCountMask) == cEmptyNode))
					{ // merge childnodes and keep going
						nextnode = MergeNodes(l, r, node.Key[ge], node.Data[ge], actnode, ge);
						
						KeyDelete(actnode, node, ge);
						node.Child[ge] = nextnode;					
						
						Item.m_Node = nextnode;
						Item.m_Index = lnode.Info & cKeyCountMask;

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
							++Item;
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
						Write(innernode, offsetof(TNode, Data[innerindex]), sizeof(TData), tmp);

						KeyDelete(actnode, node, 0);					
						WriteNode(actnode, node);
										
					} else {
						KeyMove(actnode, (node.Info & cKeyCountMask) - 1, node, innernode, innerindex, tmp);
						Write(innernode, offsetof(TNode, Key[innerindex]), sizeof(TKey), tmp);
						Write(innernode, offsetof(TNode, Data[innerindex]), sizeof(TData), tmp);

						//KeyDelete(actnode, node, node.Info & cKeyCountMask);
						node.Info--;
						Write(actnode, offsetof(TNode, Info), sizeof(unsigned char), node);
					}				
				}

				if (wasmanaged)
					Item.setManaged();
				return;

			} else { // inner node. go on and check if moving or merging is neccessary
				nextnode = node.Child[ge];
				Read(nextnode, offsetof(TNode, Info), sizeof(unsigned char), node2);

				if ((node2.Info & cKeyCountMask) <= cEmptyNode) // move or merge
				{
					// set l and r for easier access
					if (ge > 0)	
					{
						l = node.Child[ge - 1]; 
						Read(l, offsetof(TNode, Info), sizeof(unsigned char), lnode);
					} else 
						l = 0;

					if (ge < (node.Info & cKeyCountMask))	
					{
						r = node.Child[ge + 1];
						Read(r, offsetof(TNode, Info), sizeof(unsigned char), rnode);
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
						Write(actnode, offsetof(TNode, Data[ge]), sizeof(TData), node);

						// decrement right node key count and remove the first key-data-pair
						KeyDelete(r, rnode, 0);

						// increment KeyCount of the next node
						node2.Info++;

						if ((node2.Info & cIsLeafMask) == 0) // update the parent property of moved child
						{						
							tmp.Parent = nextnode;
							Write(node2.Child[node2.Info & cKeyCountMask], offsetof(TNode, Parent), sizeof(unsigned int), tmp);
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

						if (Item.m_Node == nextnode)
						{
							Item.m_Index++;
						}

						// move key-data-pair down from current to the next node
						KeyMove(actnode, ge - 1, node, nextnode, 0, node2);
						
						// move the child from left to next node
						node2.Child[0] = lnode.Child[lnode.Info & cKeyCountMask];

						// move key-data-pair up from left to current node					
						KeyMove(l, (lnode.Info & cKeyCountMask) - 1, lnode, actnode, ge - 1, node);
						Write(actnode, offsetof(TNode, Key[ge - 1]), sizeof(TKey), node);
						Write(actnode, offsetof(TNode, Data[ge - 1]), sizeof(TData), node);

						// decrement left node key count
						lnode.Info--;
						Write(l, offsetof(TNode, Info), sizeof(unsigned char), lnode);

						if ((node2.Info & cIsLeafMask) == 0) // update the parent property of moved child
						{
							tmp.Parent = nextnode;
							Write(node2.Child[0], offsetof(TNode, Parent), sizeof(unsigned int), tmp);
						}

						WriteNode(nextnode, node2);
						node = node2;
						skipread = true;			

					} else {
						if (l != 0) // merge with the left node
						{
							if (Item.m_Node == nextnode)
							{
								Item.m_Index = (lnode.Info & cKeyCountMask) + 1 + Item.m_Index;
								nextnode = MergeNodes(l, nextnode, node.Key[ge - 1], node.Data[ge - 1], actnode, ge - 1);
								Item.m_Node = nextnode;														
							} else {
								nextnode = MergeNodes(l, nextnode, node.Key[ge - 1], node.Data[ge - 1], actnode, ge - 1);
							}
							KeyDelete(actnode, node, ge - 1);
							node.Child[ge - 1] = nextnode;

						} else { // merge with the right node
							if (Item.m_Node == nextnode)
							{
								nextnode = MergeNodes(nextnode, r, node.Key[ge], node.Data[ge], actnode, ge);
								Item.m_Node = nextnode;
							} else {
								nextnode = MergeNodes(nextnode, r, node.Key[ge], node.Data[ge], actnode, ge);
							}
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
	}

	if (wasmanaged)
		Item.setManaged();
}


template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
unsigned int CBTree<TKey, TData, SizeParam, UniqueKeys>::getRoot()
{
	return m_Root;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CBTree<TKey, TData, SizeParam, UniqueKeys>::setRoot(unsigned int NewRoot)
{
	m_Root = NewRoot;
	return;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
unsigned int CBTree<TKey, TData, SizeParam, UniqueKeys>::CreateNewNode()
{
	TNode* node = new TNode;
	memset(node, 0, sizeof(TNode));
	return (unsigned int) node;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CBTree<TKey, TData, SizeParam, UniqueKeys>::DeleteNode(unsigned int Node)
{
	delete (TNode*)Node;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CBTree<TKey, TData, SizeParam, UniqueKeys>::ReadNode(unsigned int Node, TNode & Dest)
{
	Read(Node, 0, sizeof(TNode), Dest);
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CBTree<TKey, TData, SizeParam, UniqueKeys>::WriteNode(unsigned int Node, TNode & Source)
{
	Write(Node, 0, sizeof(TNode), Source);
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CBTree<TKey, TData, SizeParam, UniqueKeys>::Read(unsigned int Node, int Offset, int Size, TNode & Dest)
{
	memcpy((char*)(&Dest) + Offset, (void*)(Node + Offset), Size);
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CBTree<TKey, TData, SizeParam, UniqueKeys>::Write(unsigned int Node, int Offset, int Size, TNode & Source)
{
	memcpy((void*)(Node + Offset), (char*)(&Source) + Offset, Size);
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CBTree<TKey, TData, SizeParam, UniqueKeys>::DestroyTree()
{
	std::stack<TNode*> s;
	TNode* node;
	int i;

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









template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::iterator()
{
	m_Tree = NULL;
	m_Node = 0;
	m_Index = -1;
	m_Managed = false;
	m_ManagedDeleted = false;
	m_LoadedKey = false;
	m_LoadedData = false;
}
template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::iterator(CBTree* Tree, unsigned int Node, int Index)
{
	m_Tree = Tree;
	m_Node = Node;
	m_Index = Index;
	m_Managed = false;
	m_ManagedDeleted = false;
	m_LoadedKey = false;
	m_LoadedData = false;
}
template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::iterator(const iterator& Other)
{
	m_Tree = Other.m_Tree;
	m_Node = Other.m_Node;
	m_Index = Other.m_Index;
	m_ManagedDeleted = Other.m_ManagedDeleted;
	m_Managed = Other.m_Managed;
	m_LoadedKey = Other.m_LoadedKey;
	m_LoadedData = Other.m_LoadedData;
	m_ManagedKey = Other.m_ManagedKey;
	m_ManagedData = Other.m_ManagedData;
	
	if (m_Managed)
		m_Tree->m_ManagedIterators.insert(std::make_pair(m_Node, this));
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::~iterator()
{
	if (m_Managed && m_Tree)
	{
		TManagedMap::iterator i = m_Tree->m_ManagedIterators.find(m_Node);

		while ((i != m_Tree->m_ManagedIterators.end()) && (i->second != this))
			++i;

		if (i != m_Tree->m_ManagedIterators.end())
			m_Tree->m_ManagedIterators.erase(i);
	}
}


template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::setManaged()
{
	if (!m_Managed)	
	{
		m_Tree->m_ManagedIterators.insert(std::make_pair(m_Node, this));
	}

	m_Managed = true;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
bool CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::wasDeleted()
{
	return m_ManagedDeleted;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::Backup()
{
	if ((!m_ManagedDeleted) && (*this))
	{
		TNode tmp;
		if (!m_LoadedKey)
		{
			m_Tree->Read(m_Node, offsetof(TNode, Key[m_Index]), sizeof(TKey), tmp);
			m_ManagedKey = tmp.Key[m_Index];
		}

		if (!m_LoadedData)
		{
			m_Tree->Read(m_Node, offsetof(TNode, Data[m_Index]), sizeof(TData), tmp);
			m_ManagedData = tmp.Data[m_Index];
		}
		m_LoadedKey = true;
		m_LoadedData = true;
	}

	m_ManagedDeleted = true;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
TKey& CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::Key() 
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

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
TData& CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::Data() 
{
	if (!m_LoadedData)
	{
		TNode node;
		m_Tree->Read(m_Node, offsetof(TNode, Data[m_Index]), sizeof(TData), node);
		m_ManagedData = node.Data[m_Index];
		m_LoadedData = true;
		
	}
	return m_ManagedData;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::SetData(const TData & Data)
{
	m_ManagedData = Data;
	m_LoadedData = true;
	if (!m_ManagedDeleted)
	{
		TNode node;
		node.Data[m_Index] = Data;
		m_Tree->Write(m_Node, offsetof(TNode, Data[m_Index]), sizeof(TData), node);
	}
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::operator bool() const
{
	if (m_Tree && m_Node)
	{
		TNode node;
		m_Tree->Read(m_Node, offsetof(TNode, Info), sizeof(unsigned char), node);
		return (m_Index >= 0) && (m_Index < (node.Info & cKeyCountMask));
	} else
		return false;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
bool CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::operator !() const
{		
	if (m_Tree && m_Node)
	{
		TNode node;
		m_Tree->Read(m_Node, offsetof(TNode, Info), sizeof(unsigned char), node);
		return (m_Index < 0) || (m_Index > (node.Info & cKeyCountMask));
	} else
		return true;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
bool CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::operator ==(const iterator& Other) const
{
	return (m_Tree == Other.m_Tree) && (m_Node == Other.m_Node) && (m_Index == Other.m_Index) && (!m_ManagedDeleted) && (!Other.m_ManagedDeleted);
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
typename CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator& 
CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::operator =(const iterator& Other)
{
	if (m_Managed && (m_Tree != NULL))
	{
		TManagedMap::iterator it = m_Tree->m_ManagedIterators.find(m_Node);
		while ((it != m_Tree->m_ManagedIterators.end()) && (it->second != this))
			++it;

		if (it != m_Tree->m_ManagedIterators.end())
			m_Tree->m_ManagedIterators.erase(it);
	}

	m_Tree = Other.m_Tree;
	m_Node = Other.m_Node;
	m_Index = Other.m_Index;
	m_ManagedDeleted = Other.m_ManagedDeleted;
	m_Managed = Other.m_Managed;
	m_LoadedKey = Other.m_LoadedKey;
	m_LoadedData = Other.m_LoadedData;
	m_ManagedKey = Other.m_ManagedKey;
	m_ManagedData = Other.m_ManagedData;
	
	if (m_Managed)
		m_Tree->m_ManagedIterators.insert(std::make_pair(m_Node, this));

	return *this;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
typename CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator& 
CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::operator ++() //pre  ++i
{
	if (m_Managed && m_ManagedDeleted)
	{
		TKey oldkey = m_ManagedKey;
		m_LoadedKey = false;
		m_LoadedData = false;

		m_ManagedDeleted = false;
		iterator & other = m_Tree->LowerBound(m_ManagedKey);
		m_Node = other.m_Node;
		m_Index = other.m_Index;
		while ((Key() == oldkey) && (*this))
			Inc();
	} else
		Inc();

	return *this;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
typename CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator& 
CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::operator --() //pre  --i
{	
	if (m_Managed && m_ManagedDeleted)
	{
		TKey oldkey = m_ManagedKey;
		m_LoadedKey = false;
		m_LoadedData = false;

		m_ManagedDeleted = false;
		iterator& other = m_Tree->UpperBound(m_ManagedKey);
		m_Node = other.m_Node;
		m_Index = other.m_Index;
		while ((Key() == oldkey) && (*this))
			Dec();
	} else
		Dec();

	return *this;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
typename CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator 
CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::operator ++(int) //post i++
{
	iterator tmp(*this);
	++(*this);
	return tmp;
}
template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
typename CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator 
CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::operator --(int) //post i--
{
	iterator tmp(*this);
	--(*this);
	return tmp;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::Inc()
{
	TNode node;
	unsigned int nextnode;
	m_Tree->Read(m_Node, offsetof(TNode,Info), sizeof(unsigned char), node);
	m_Tree->Read(m_Node, offsetof(TNode,Parent), sizeof(unsigned int), node);

	m_LoadedKey = false;
	m_LoadedData = false;

	if ((node.Info & cIsLeafMask) && ((node.Info & cKeyCountMask) > m_Index + 1)) // leaf
	{
		m_Index++;
		return;
	}

	if ((node.Info & cIsLeafMask) == 0) // inner node. go down
	{
		m_Tree->Read(m_Node, offsetof(TNode, Child[m_Index + 1]), sizeof(unsigned int), node);
		m_Node = node.Child[m_Index + 1];
		m_Tree->Read(m_Node, offsetof(TNode, Info), sizeof(unsigned char), node);

		m_Index = 0;

		while ((node.Info & cIsLeafMask) == 0)  // go down to a leaf
		{
			m_Tree->Read(m_Node, offsetof(TNode, Child[0]), sizeof(unsigned int), node);
			m_Node = node.Child[0];
			m_Tree->Read(m_Node, offsetof(TNode, Info), sizeof(unsigned char), node);
		}

		return;
	}

	while (m_Index >= (node.Info & cKeyCountMask) - 1) // go up
	{
		if (m_Node == m_Tree->m_Root) // the root is the top, we cannot go further
		{
			m_Index = -1;
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

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::Dec()
{
	TNode node;
	unsigned int nextnode;
	m_Tree->Read(m_Node, offsetof(TNode,Info), sizeof(unsigned char), node);
	m_Tree->Read(m_Node, offsetof(TNode,Parent), sizeof(unsigned int), node);

	m_LoadedKey = false;
	m_LoadedData = false;

	if ((node.Info & cIsLeafMask) && (m_Index > 0))
	{
		m_Index--;
		return;
	}

	if ((node.Info & cIsLeafMask) == 0) // inner node. go down
	{
		m_Tree->Read(m_Node, offsetof(TNode, Child[m_Index]), sizeof(unsigned int), node);
		m_Node = node.Child[m_Index];
		m_Tree->Read(m_Node, offsetof(TNode, Info), sizeof(unsigned char), node);
		m_Index = (node.Info & cKeyCountMask) - 1;
		while ((node.Info & cIsLeafMask) == 0)  // go down to a leaf
		{
			m_Tree->Read(m_Node, offsetof(TNode, Child[node.Info & cKeyCountMask]), sizeof(unsigned int), node);
			m_Node = node.Child[node.Info & cKeyCountMask];
			m_Tree->Read(m_Node, offsetof(TNode, Info), sizeof(unsigned char), node);
			m_Index = (node.Info & cKeyCountMask) - 1;
		}

		return;
	}

	while (m_Index <= 0) // go up
	{
		if (m_Node == m_Tree->m_Root) // the root is the top, we cannot go further
		{
			m_Index = -1;
			m_Node = 0;
			return;
		}

		nextnode = node.Parent;
		m_Tree->ReadNode(nextnode, node);		
		m_Index = 0;

		while ((m_Index <= (node.Info & cKeyCountMask)) && (node.Child[m_Index] != m_Node))
			m_Index++;

		m_Index--;
		m_Node = nextnode;

		if (m_Index >= 0)
			return;
	}
}
