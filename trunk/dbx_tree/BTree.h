#pragma once

#include <queue>
#include <stack>

typedef struct {} TEmpty;

template <typename TKey, typename TData = TEmpty, int SizeParam = 4, bool UniqueKeys = true>
class CBTree 
{
private:
	friend class iterator;

	static const unsigned char cIsLeafMask = 0x80;
	static const unsigned char cKeyCountMask = 0x7F;
	static const unsigned char cFullNode = SizeParam * 2 - 1;
	static const unsigned char cEmptyNode = SizeParam - 1;

	#pragma pack(push)  /* push current alignment to stack */
	#pragma pack(1)     /* set alignment to 1 byte boundary */

		typedef struct TNode {
			unsigned int Parent;
			unsigned char Info;
			TKey Key[SizeParam * 2 - 1];
			TData Data[SizeParam * 2 - 1];
			unsigned int Child[SizeParam * 2];
		} TNode;

	#pragma pack(pop)

	bool InNodeFind(const TNode & Node, const TKey & Key, int & GreaterEqual);
	void SplitNode(unsigned int Node, unsigned int & Left, unsigned int & Right, TKey & UpKey, TData & UpData);
	unsigned int MergeNodes(unsigned int Left, unsigned int Right, const TKey & DownKey, const TData & DownData);

	
	void ReadNode(unsigned int Node, TNode & Dest);
	void WriteNode(unsigned int Node, TNode & Source);

protected:
	unsigned int m_Root;

	virtual unsigned int CreateNewNode();
	virtual void DeleteNode(unsigned int Node);
	virtual void RootChanged();
	virtual void Read(unsigned int Node, int Offset, int Size, TNode & Dest);
	virtual void Write(unsigned int Node, int Offset, int Size, TNode & Source);
	
	virtual void ClearTree();
	
public:
	
	class iterator {
	protected:
		friend class CBTree;

		unsigned int m_Node;
		int m_Index;
		CBTree* m_Tree;		
	public:
		iterator();
		iterator(CBTree* Tree, unsigned int Node, int Index);
		iterator(const iterator& Other);
		~iterator();

		TKey& Key() const;
		TData& Data() const;

		operator bool() const;
		bool operator !() const;
		
		bool operator ==(const iterator& Other) const;
		iterator& operator =(const iterator& Other);
		void operator ++(int);
		void operator --(int);
	};


	CBTree(unsigned int RootNode = NULL);
	~CBTree();

	iterator Insert(const TKey & Key, const TData & Data);
	iterator Find(const TKey & Key);
	bool Delete(const TKey& Key);
	void Delete(iterator& Item);

};






template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
CBTree<TKey, TData, SizeParam, UniqueKeys>::CBTree(unsigned int RootNode = NULL)
{
	m_Root = RootNode;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
CBTree<TKey, TData, SizeParam, UniqueKeys>::~CBTree()
{
	ClearTree();
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
void CBTree<TKey, TData, SizeParam, UniqueKeys>::SplitNode(unsigned int Node, unsigned int & Left, unsigned int & Right, TKey & UpKey, TData & UpData)
{
	TNode nodedata;
	TNode newnode = {0};
	const int upindex = SizeParam - 1;

	Left = Node;
	Right = CreateNewNode();
	ReadNode(Node, nodedata);
	
	UpKey = nodedata.Key[upindex];
	UpData = nodedata.Data[upindex];
	
	memcpy(&(newnode.Key[0]), &(nodedata.Key[upindex+1]), sizeof(TKey) * (cFullNode - upindex));
	memcpy(&(newnode.Data[0]), &(nodedata.Data[upindex+1]), sizeof(TData) * (cFullNode - upindex));
	memcpy(&(newnode.Child[0]), &(nodedata.Child[upindex+1]), sizeof(unsigned int) * (cFullNode - upindex + 1));

	newnode.Info = (nodedata.Info & cIsLeafMask) | (char)(upindex);
	nodedata.Info = newnode.Info;
	newnode.Parent = nodedata.Parent;

	WriteNode(Left, nodedata);
	WriteNode(Right, newnode);
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
unsigned int CBTree<TKey, TData, SizeParam, UniqueKeys>::MergeNodes(unsigned int Left, unsigned int Right, const TKey & DownKey, const TData & DownData)
{
	TNode ldata, rdata;
	int downindex;

	ReadNode(Left, ldata);
	ReadNode(Right, rdata);

	downindex = ldata.Info & cKeyCountMask;
	ldata.Key[downindex] = DownKey;
	ldata.Data[downindex] = DownData;
	
	memcpy(&(ldata.Key[downindex+1]), &(rdata.Key[0]), sizeof(TKey) * (rdata.Info & cKeyCountMask));
	memcpy(&(ldata.Data[downindex+1]), &(rdata.Data[0]), sizeof(TData) * (rdata.Info & cKeyCountMask));
	memcpy(&(ldata.Child[downindex+1]), &(rdata.Child[0]), sizeof(unsigned int) * ((rdata.Info & cKeyCountMask) + 1));

	ldata.Info =(unsigned char) ((ldata.Info & cIsLeafMask) | (downindex + 1 + (rdata.Info & cKeyCountMask)));

	WriteNode(Left, ldata);
	DeleteNode(Right);

	return Left;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
typename CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator 
CBTree<TKey, TData, SizeParam, UniqueKeys>::Insert(const TKey & Key, const TData & Data)
{
	if (!m_Root)
	{
		m_Root = CreateNewNode();
		RootChanged();
	}

	TNode node, node2;
	unsigned int actnode = m_Root;
	unsigned int nextnode;
	bool exists;
	int ge;

	ReadNode(actnode, node);
	if ((node.Info & cKeyCountMask) == cFullNode)  // root split
	{
		// be a little tricky and let the main code handle the actual splitting.
		// just assign a new root with keycount to zero and one child = old root
		// the InNode test will fail with GreaterEqual = 0
		nextnode = CreateNewNode();
		node.Parent = nextnode;
		Write(actnode, offsetof(TNode, Parent), sizeof(unsigned int), node);

		node.Child[0] = actnode;
		Write(nextnode, offsetof(TNode, Child[0]), sizeof(unsigned int), node);

		ReadNode(nextnode, node);
		actnode = nextnode;

		m_Root = nextnode;
		RootChanged();
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
				memcpy(&(node.Key[ge+1]), &(node.Key[ge]), sizeof(TKey) * ((node.Info & cKeyCountMask) - ge));
				memcpy(&(node.Data[ge+1]), &(node.Data[ge]), sizeof(TData) * ((node.Info & cKeyCountMask) - ge));
				memcpy(&(node.Child[ge+2]), &(node.Child[ge+1]), sizeof(unsigned int) * ((node.Info & cKeyCountMask) - ge));
				
				node.Key[ge] = Key;
				node.Data[ge] = Data;
				node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) + 1);

				WriteNode(actnode, node);

				return iterator(this, actnode, ge);

			} else {  // middle node
				nextnode = node.Child[ge];
				Read(nextnode, offsetof(TNode, Info), sizeof(unsigned char), node2);

				if ((node2.Info & cKeyCountMask) == cFullNode) // split the childnode
				{
					memcpy(&(node.Key[ge+1]), &(node.Key[ge]), sizeof(TKey) * ((node.Info & cKeyCountMask) - ge));
					memcpy(&(node.Data[ge+1]), &(node.Data[ge]), sizeof(TData) * ((node.Info & cKeyCountMask) - ge));
					memcpy(&(node.Child[ge+2]), &(node.Child[ge+1]), sizeof(unsigned int) * ((node.Info & cKeyCountMask) - ge));

					SplitNode(nextnode, node.Child[ge], node.Child[ge+1], node.Key[ge], node.Data[ge]);
					
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

	return iterator(this, foundnode, foundindex);
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
bool CBTree<TKey, TData, SizeParam, UniqueKeys>::Delete(const TKey& Key)
{
	if (!m_Root) return false;

	TNode node, node2, lnode, rnode, tmp;
	unsigned int actnode = m_Root;
	unsigned int nextnode, l, r;
	bool exists, skipread;
	int ge;

	bool foundininnernode = false;
	bool wantleftmost = false;
	unsigned int innernode;
	int innerindex;

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
				memcpy(&(node.Key[ge]), &(node.Key[ge+1]), sizeof(TKey) * ((node.Info & cKeyCountMask) - ge));
				memcpy(&(node.Data[ge]), &(node.Data[ge+1]), sizeof(TData) * ((node.Info & cKeyCountMask) - ge));
				memcpy(&(node.Child[ge+1]), &(node.Child[ge+2]), sizeof(unsigned int) * ((node.Info & cKeyCountMask) - ge));
				
				node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);

				WriteNode(actnode, node);

				return true;

			} else { // delete in inner node
				l = node.Child[ge];
				r = node.Child[ge+1];
				Read(l, offsetof(TNode, Info), sizeof(unsigned char), lnode);
				Read(r, offsetof(TNode, Info), sizeof(unsigned char), rnode);


				if (((rnode.Info & cKeyCountMask) == cEmptyNode) && ((lnode.Info & cKeyCountMask) == cEmptyNode))
				{ // merge childnodes and keep going
					nextnode = MergeNodes(l, r, node.Key[ge], node.Data[ge]);
					
					memcpy(&(node.Key[ge]), &(node.Key[ge+1]), sizeof(TKey) * ((node.Info & cKeyCountMask) - ge));
					memcpy(&(node.Data[ge]), &(node.Data[ge+1]), sizeof(TData) * ((node.Info & cKeyCountMask) - ge));
					memcpy(&(node.Child[ge+1]), &(node.Child[ge+2]), sizeof(unsigned int) * ((node.Info & cKeyCountMask) - ge));
				
					node.Child[ge] = nextnode;
					node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);
					
					if ((actnode == m_Root) && ((node.Info & cKeyCountMask) == 0))
					{ // root node is empty. delete it
						DeleteNode(actnode);
						m_Root = nextnode;
						RootChanged();
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
					node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);

					tmp.Key[innerindex] = node.Key[0];
					tmp.Data[innerindex] = node.Data[0];

					Write(innernode, offsetof(TNode, Key[innerindex]), sizeof(TKey), tmp);
					Write(innernode, offsetof(TNode, Data[innerindex]), sizeof(TData), tmp);					

					memcpy(&(node.Key[0]), &(node.Key[1]), sizeof(TKey) * (node.Info & cKeyCountMask));
					memcpy(&(node.Data[0]), &(node.Data[1]), sizeof(TData) * (node.Info & cKeyCountMask));

					WriteNode(actnode, node);
									
				} else {					
					node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);

					tmp.Key[innerindex] = node.Key[node.Info & cKeyCountMask];
					tmp.Data[innerindex] = node.Data[node.Info & cKeyCountMask];

					Write(innernode, offsetof(TNode, Key[innerindex]), sizeof(TKey), tmp);
					Write(innernode, offsetof(TNode, Data[innerindex]), sizeof(TData), tmp);

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

				if (ge - 1 < (node.Info & cKeyCountMask))	
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
					node2.Key[node2.Info & cKeyCountMask] = node.Key[ge];
					node2.Data[node2.Info & cKeyCountMask] = node.Data[ge];

					// move the child from right to next node
					node2.Child[(node2.Info & cKeyCountMask) + 1] = rnode.Child[0];

					// move key-data-pair up from right to current node
					node.Key[ge] = rnode.Key[0];
					node.Data[ge] = rnode.Data[0];	
					Write(actnode, offsetof(TNode, Key[ge]), sizeof(TKey), node);
					Write(actnode, offsetof(TNode, Data[ge]), sizeof(TData), node);

					// decrement right node key count and remove the first key-data-pair
					rnode.Info = (rnode.Info & cIsLeafMask) | ((rnode.Info & cKeyCountMask) - 1);
					memcpy(&(rnode.Key[0]), &(rnode.Key[1]), sizeof(TKey) * (rnode.Info & cKeyCountMask));
					memcpy(&(rnode.Data[0]), &(rnode.Data[1]), sizeof(TData) * (rnode.Info & cKeyCountMask));
					memcpy(&(rnode.Child[0]), &(rnode.Child[1]), sizeof(unsigned int) * ((rnode.Info & cKeyCountMask) + 1));
				
					// increment KeyCount of the next node
					node2.Info = (node2.Info & cIsLeafMask) | ((node2.Info & cKeyCountMask) + 1);
					if ((node2.Info & cIsLeafMask) == 0) // update the parent property of moved child
					{						
						tmp.Parent = nextnode;
						Write(node2.Child[node.Info & cKeyCountMask], offsetof(TNode, Parent), sizeof(unsigned int), tmp);
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
					memcpy(&(node2.Key[1]), &(node2.Key[0]), sizeof(TKey) * (node2.Info & cKeyCountMask));
					memcpy(&(node2.Data[1]), &(node2.Data[0]), sizeof(TData) * (node2.Info & cKeyCountMask));
					memcpy(&(node2.Child[1]), &(node2.Child[0]), sizeof(unsigned int) * ((node2.Info & cKeyCountMask) + 1));
					node2.Info = (node2.Info & cIsLeafMask) | ((node2.Info & cKeyCountMask) + 1);					

					// move key-data-pair down from current to the next node
					node2.Key[0] = node.Key[ge];
					node2.Data[0] = node.Data[ge];
					
					// move the child from left to next node
					node2.Child[0] = lnode.Child[lnode.Info & cKeyCountMask];

					// move key-data-pair up from left to current node					
					node.Key[ge] = lnode.Key[(lnode.Info & cKeyCountMask) - 1];
					node.Data[ge] = lnode.Data[(lnode.Info & cKeyCountMask) - 1];		
					Write(actnode, offsetof(TNode, Key[ge]), sizeof(TKey), node);
					Write(actnode, offsetof(TNode, Data[ge]), sizeof(TData), node);

					// decrement left node key count
					lnode.Info = (lnode.Info & cIsLeafMask) | ((lnode.Info & cKeyCountMask) - 1);
					Write(l, offsetof(TNode, Info), sizeof(unsigned char), lnode);

					if ((node2.Info & cIsLeafMask) == 0) // update the parent property of moved child
					{
						tmp.Parent = nextnode;
						Write(node2.Child[0], offsetof(TNode, Parent), sizeof(unsigned int), tmp);
					}

					WriteNode(nextnode, node2);
					node = node2;
					skipread = true;			

				} else if (l != 0) // merge with the left node
				{
					nextnode = MergeNodes(l, nextnode, node.Key[ge - 1], node.Data[ge - 1]);
					node.Child[ge - 1] = nextnode;

					memcpy(&(node.Key[ge-1]), &(node.Key[ge]), sizeof(TKey) * ((node.Info & cKeyCountMask) - ge));
					memcpy(&(node.Data[ge-1]), &(node.Data[ge]), sizeof(TData) * ((node.Info & cKeyCountMask) - ge));
					memcpy(&(node.Child[ge]), &(node.Child[ge+1]), sizeof(unsigned int) * ((node.Info & cKeyCountMask) - ge));
					
					node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);				

					WriteNode(actnode, node);

				} else { // merge with the right node
					nextnode = MergeNodes(nextnode, r, node.Key[ge], node.Data[ge]);
					node.Child[ge] = nextnode;

					memcpy(&(node.Key[ge]), &(node.Key[ge+1]), sizeof(TKey) * ((node.Info & cKeyCountMask) - ge - 1));
					memcpy(&(node.Data[ge]), &(node.Data[ge+1]), sizeof(TData) * ((node.Info & cKeyCountMask) - ge - 1));
					memcpy(&(node.Child[ge+1]), &(node.Child[ge+2]), sizeof(unsigned int) * ((node.Info & cKeyCountMask) - ge - 1));
					
					node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);				

					WriteNode(actnode, node);

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

	TNode node;

	ReadNode(Item.m_Node, node);

	if (((node.Info & cKeyCountMask) > cEmptyNode) && (node.Info & cIsLeafMask)) // the easy one: delete in a leaf which is not empty
	{
		node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);
		memcpy(&(node.Key[Item.m_Index]), &(node.Key[Item.m_Index + 1]), sizeof(TKey) * ((node.Info & cKeyCountMask) - Item.m_Index));
		memcpy(&(node.Data[Item.m_Index]), &(node.Data[Item.m_Index + 1]), sizeof(TData) * ((node.Info & cKeyCountMask) - Item.m_Index));

		WriteNode(Item.m_Node, node);

		if ((node.Info & cKeyCountMask) == Item.m_Index) // deleted last item, go up
			Item++;

	} else { // the hard one :(


		TNode node, node2, lnode, rnode, tmp;
		unsigned int actnode = m_Root;
		unsigned int nextnode, l, r;
		bool exists, skipread;
		int ge;

		bool foundininnernode = false;
		bool wantleftmost = false;
		unsigned int innernode;
		int innerindex;

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
					memcpy(&(node.Key[ge]), &(node.Key[ge+1]), sizeof(TKey) * ((node.Info & cKeyCountMask) - ge));
					memcpy(&(node.Data[ge]), &(node.Data[ge+1]), sizeof(TData) * ((node.Info & cKeyCountMask) - ge));
					memcpy(&(node.Child[ge+1]), &(node.Child[ge+2]), sizeof(unsigned int) * ((node.Info & cKeyCountMask) - ge));
					
					node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);

					WriteNode(actnode, node);

					return;

				} else { // delete in inner node
					l = node.Child[ge];
					r = node.Child[ge+1];
					Read(l, offsetof(TNode, Info), sizeof(unsigned char), lnode);
					Read(r, offsetof(TNode, Info), sizeof(unsigned char), rnode);

					if (((rnode.Info & cKeyCountMask) == cEmptyNode) && ((lnode.Info & cKeyCountMask) == cEmptyNode))
					{ // merge childnodes and keep going
						Item.m_Index = lnode.Info & cKeyCountMask;
						
						nextnode = MergeNodes(l, r, node.Key[ge], node.Data[ge]);

						Item.m_Node = nextnode;
						
						memcpy(&(node.Key[ge]), &(node.Key[ge+1]), sizeof(TKey) * ((node.Info & cKeyCountMask) - ge));
						memcpy(&(node.Data[ge]), &(node.Data[ge+1]), sizeof(TData) * ((node.Info & cKeyCountMask) - ge));
						memcpy(&(node.Child[ge+1]), &(node.Child[ge+2]), sizeof(unsigned int) * ((node.Info & cKeyCountMask) - ge));
					
						node.Child[ge] = nextnode;
						node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);
						
						if ((actnode == m_Root) && ((node.Info & cKeyCountMask) == 0))
						{ // root node is empty. delete it
							DeleteNode(actnode);
							m_Root = nextnode;
							RootChanged();
						} else {
							WriteNode(actnode, node);
						}

					} else { // need a key-data-pair from a leaf to replace deleted pair -> save position
						Item++;

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
						node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);

						tmp.Key[innerindex] = node.Key[0];
						tmp.Data[innerindex] = node.Data[0];

						Write(innernode, offsetof(TNode, Key[innerindex]), sizeof(TKey), tmp);
						Write(innernode, offsetof(TNode, Data[innerindex]), sizeof(TData), tmp);					

						memcpy(&(node.Key[0]), &(node.Key[1]), sizeof(TKey) * (node.Info & cKeyCountMask));
						memcpy(&(node.Data[0]), &(node.Data[1]), sizeof(TData) * (node.Info & cKeyCountMask));

						WriteNode(actnode, node);
										
					} else {					
						node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);

						tmp.Key[innerindex] = node.Key[node.Info & cKeyCountMask];
						tmp.Data[innerindex] = node.Data[node.Info & cKeyCountMask];

						Write(innernode, offsetof(TNode, Key[innerindex]), sizeof(TKey), tmp);
						Write(innernode, offsetof(TNode, Data[innerindex]), sizeof(TData), tmp);

						Write(actnode, offsetof(TNode, Info), sizeof(unsigned char), node);
					}				
				}
				return;

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

					if (ge - 1 < (node.Info & cKeyCountMask))	
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
						node2.Key[node2.Info & cKeyCountMask] = node.Key[ge];
						node2.Data[node2.Info & cKeyCountMask] = node.Data[ge];

						// move the child from right to next node
						node2.Child[(node2.Info & cKeyCountMask) + 1] = rnode.Child[0];

						// move key-data-pair up from right to current node
						node.Key[ge] = rnode.Key[0];
						node.Data[ge] = rnode.Data[0];					
						Write(actnode, offsetof(TNode, Key[ge]), sizeof(TKey), node);
						Write(actnode, offsetof(TNode, Data[ge]), sizeof(TData), node);

						// decrement right node key count and remove the first key-data-pair
						rnode.Info = (rnode.Info & cIsLeafMask) | ((rnode.Info & cKeyCountMask) - 1);
						memcpy(&(rnode.Key[0]), &(rnode.Key[1]), sizeof(TKey) * (rnode.Info & cKeyCountMask));
						memcpy(&(rnode.Data[0]), &(rnode.Data[1]), sizeof(TData) * (rnode.Info & cKeyCountMask));
						memcpy(&(rnode.Child[0]), &(rnode.Child[1]), sizeof(unsigned int) * ((rnode.Info & cKeyCountMask) + 1));
					
						// increment KeyCount of the next node
						node2.Info = (node2.Info & cIsLeafMask) | ((node2.Info & cKeyCountMask) + 1);
						if ((node2.Info & cIsLeafMask) == 0) // update the parent property of moved child
						{						
							tmp.Parent = nextnode;
							Write(node2.Child[node.Info & cKeyCountMask], offsetof(TNode, Parent), sizeof(unsigned int), tmp);
						}

						WriteNode(r, rnode);
						WriteNode(nextnode, node2);
						node = node2;
						skipread = true;

					} else if ((l != 0) && ((lnode.Info & cKeyCountMask) > cEmptyNode)) // move a Key-Data-pair from the left
					{					
						ReadNode(nextnode, node2);
						ReadNode(l, lnode);

						if (nextnode == Item.m_Node)
							Item.m_Index++;
										
						// increment next node key count and make new first key-data-pair					
						memcpy(&(node2.Key[1]), &(node2.Key[0]), sizeof(TKey) * (node2.Info & cKeyCountMask));
						memcpy(&(node2.Data[1]), &(node2.Data[0]), sizeof(TData) * (node2.Info & cKeyCountMask));
						memcpy(&(node2.Child[1]), &(node2.Child[0]), sizeof(unsigned int) * ((node2.Info & cKeyCountMask) + 1));
						node2.Info = (node2.Info & cIsLeafMask) | ((node2.Info & cKeyCountMask) + 1);					

						// move key-data-pair down from current to the next node
						node2.Key[0] = node.Key[ge];
						node2.Data[0] = node.Data[ge];
						
						// move the child from left to next node
						node2.Child[0] = lnode.Child[lnode.Info & cKeyCountMask];

						// move key-data-pair up from left to current node					
						node.Key[ge] = lnode.Key[(lnode.Info & cKeyCountMask) - 1];
						node.Data[ge] = lnode.Data[(lnode.Info & cKeyCountMask) - 1];		
						Write(actnode, offsetof(TNode, Key[ge]), sizeof(TKey), node);
						Write(actnode, offsetof(TNode, Data[ge]), sizeof(TData), node);

						// decrement left node key count
						lnode.Info = (lnode.Info & cIsLeafMask) | ((lnode.Info & cKeyCountMask) - 1);
						Write(l, offsetof(TNode, Info), sizeof(unsigned char), lnode);

						if ((node2.Info & cIsLeafMask) == 0) // update the parent property of moved child
						{
							tmp.Parent = nextnode;
							Write(node2.Child[0], offsetof(TNode, Parent), sizeof(unsigned int), tmp);
						}

						WriteNode(nextnode, node2);
						node = node2;
						skipread = true;			

					} else if (l != 0) // merge with the left node
					{
						if (nextnode == Item.m_Node)
						{
							Item.m_Index += (lnode.Info & cKeyCountMask);
							nextnode = MergeNodes(l, nextnode, node.Key[ge - 1], node.Data[ge - 1]);
							Item.m_Node = nextnode;
						} else {
							nextnode = MergeNodes(l, nextnode, node.Key[ge - 1], node.Data[ge - 1]);
						}

						node.Child[ge - 1] = nextnode;

						memcpy(&(node.Key[ge-1]), &(node.Key[ge]), sizeof(TKey) * ((node.Info & cKeyCountMask) - ge));
						memcpy(&(node.Data[ge-1]), &(node.Data[ge]), sizeof(TData) * ((node.Info & cKeyCountMask) - ge));
						memcpy(&(node.Child[ge]), &(node.Child[ge+1]), sizeof(unsigned int) * ((node.Info & cKeyCountMask) - ge));
						
						node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);				

						WriteNode(actnode, node);

					} else { // merge with the right node
						if (nextnode == Item.m_Node)
						{
							nextnode = MergeNodes(nextnode, r, node.Key[ge], node.Data[ge]);
							Item.m_Node = nextnode;
						} else {
							nextnode = MergeNodes(nextnode, r, node.Key[ge], node.Data[ge]);
						}
						node.Child[ge] = nextnode;

						memcpy(&(node.Key[ge]), &(node.Key[ge+1]), sizeof(TKey) * ((node.Info & cKeyCountMask) - ge - 1));
						memcpy(&(node.Data[ge]), &(node.Data[ge+1]), sizeof(TData) * ((node.Info & cKeyCountMask) - ge - 1));
						memcpy(&(node.Child[ge+1]), &(node.Child[ge+2]), sizeof(unsigned int) * ((node.Info & cKeyCountMask) - ge - 1));
						
						node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);				

						WriteNode(actnode, node);

					}
				}
			} // if (exists) else if (node.Info & cIsLeafMask)

			actnode = nextnode;
			if (!skipread)
				ReadNode(actnode, node);

		} // while(actnode)

	}
}


template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
unsigned int CBTree<TKey, TData, SizeParam, UniqueKeys>::CreateNewNode()
{
	return (unsigned int) new TNode;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CBTree<TKey, TData, SizeParam, UniqueKeys>::DeleteNode(unsigned int Node)
{
	delete (TNode*)Node;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CBTree<TKey, TData, SizeParam, UniqueKeys>::RootChanged()
{
	//nothing to do
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
void CBTree<TKey, TData, SizeParam, UniqueKeys>::ClearTree()
{
	std::queue<TNode*> q;
	TNode* node;
	int i;

	q.push((TNode*)m_Root);
	while (!q.empty())
	{
		node = q.back();
		q.pop();

		if ((node->Info & cIsLeafMask) == 0)
			for (i = 0; i <= (node->Info & cKeyCountMask); i++)
				q.push((TNode*)node->Child[i]);

		delete node;
	}
}









template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::iterator()
{
	m_Tree = NULL;
	m_Node = 0;
	m_Index = -1;
}
template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::iterator(CBTree* Tree, unsigned int Node, int Index)
{
	m_Tree = Tree;
	m_Node = Node;
	m_Index = Index;
}
template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::iterator(const iterator& Other)
{
	m_Tree = Other.m_Tree;
	m_Node = Other.m_Node;
	m_Index = Other.m_Index;

}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::~iterator()
{

}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
TKey& CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::Key() const
{
	TNode node;
	m_Tree->Read(m_Node, offsetof(TNode, Key[m_Index]), sizeof(TKey), node);
	return node.Key[m_Index];
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
TData& CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::Data() const
{
	TNode node;
	m_Tree->Read(m_Node, offsetof(TNode, Data[m_Index]), sizeof(TData), node);
	return node.Data[m_Index];
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::operator bool() const
{
	if (m_Tree && m_Node)
	{
		TNode node;
		m_Tree->Read(m_Node, offsetof(TNode, Info), sizeof(unsigned char), node);
		return (m_Index >= 0) && (m_Index <= (node.Info & cKeyCountMask));
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
	return (m_Tree == Other.m_Tree) && (m_Node == Other.m_Node) && (m_Index == Other.m_Index);
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
typename CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator& CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::operator =(const iterator& Other)
{
	m_Tree = Other.m_Tree;
	m_Node = Other.m_Node;
	m_Index = Other.m_Index;
	return *this;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::operator ++(int)
{
	TNode node;
	unsigned int nextnode;
	m_Tree->Read(m_Node, offsetof(TNode,Info), sizeof(unsigned char), node);
	m_Tree->Read(m_Node, offsetof(TNode,Parent), sizeof(unsigned int), node);

	while (m_Index >= (node.Info & cKeyCountMask)) // go up
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

		if (m_Index <= (node.Info & cKeyCountMask))
			return;
	}

	if (node.Info & cIsLeafMask) // we are in a leaf.
	{
		m_Index++;
		return;
	}

	m_Index = 0;
	while ((node.Info & cIsLeafMask) == 0)  // go down to a leaf
	{
		m_Tree->Read(m_Node, offsetof(TNode, Child[0]), sizeof(unsigned int), node);
		m_Node = node.Child[0];
		m_Tree->Read(m_Node, offsetof(TNode, Info), sizeof(unsigned char), node);
	}

	return;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator::operator --(int)
{
	TNode node;
	unsigned int nextnode;
	m_Tree->Read(m_Node, offsetof(TNode,Info), sizeof(unsigned char), node);
	m_Tree->Read(m_Node, offsetof(TNode,Parent), sizeof(unsigned int), node);

	while (m_Index == 0) // go up
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

	if (node.Info & cIsLeafMask) // we are in a leaf.
	{
		m_Index--;
		return;
	}

	while ((node.Info & cIsLeafMask) == 0)  // go down to a leaf
	{
		m_Tree->Read(m_Node, offsetof(TNode, Child[(node.Info & cKeyCountMask) + 1]), sizeof(unsigned int), node);
		m_Node = node.Child[(node.Info & cKeyCountMask) + 1];
		m_Tree->Read(m_Node, offsetof(TNode, Info), sizeof(unsigned char), node);
		m_Index = node.Info & cKeyCountMask;
	}

	return;
}
