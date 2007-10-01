#include "BTree.h"

#define NODEREAD(node, type, dest) Read(node, offsetof(TNode,type), sizeof(TNode.type), dest)
#define NODEWRITE(node, type, source) Write(node, offsetof(TNode,type), sizeof(TNode.type), source)

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
	while ((GreaterEqual < (Node.Info & cChildCountMask)) && (Node.Key[GreaterEqual] < Key))
	{
		GreaterEqual++;
	}

	return ((GreaterEqual < (Node.Info & cChildCountMask)) && (Node.Key[GreaterEqual] == Key));
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
	memcpy(&(newnode.Child[0]), &(nodedata.Child[upindex+1]), sizeof(TNode.Child[0]) * (cFullNode - upindex + 1));

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
	memcpy(&(ldata.Child[downindex+1]), &(rdata.Child[0]), sizeof(TNode.Child[0]) * ((rdata.Info & cKeyCountMask) + 1));

	ldata.Info = (ldata.Info & cIsLeafMask) | (downindex + 1 + (rdata.Info & cKeyCountMask));

	WriteNode(Left, ldata);
	DeleteNode(Right);

	return Left;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
typename CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator & 
CBTree<TKey, TData, SizeParam, UniqueKeys>::Insert(const TKey & Key, const TData & Data) const
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
	if ((node.Info & cKeyCountMask) = cFullNode)  // root split
	{
		// be a little tricky and let the main code handle the actual splitting.
		// just assign a new root with keycount to zero and one child = old root
		// the InNode test will fail with GreaterEqual = 0
		nextnode = CreateNewNode();
		node.Parent = nextnode;
		NODEWRITE(actnode, Parent, node);

		node.Child[0] = actnode;
		NODEWRITE(nextnode, Child[0], node);

		ReadNode(nextnode, node);
		actnode = nextnode;

		m_Root = nextnode;
		RootChanged();
	}

	while (actnode)
	{
		exists = InNode(node, Key);
		if (UniqueKeys && exists)  // already exists
		{
			node.Data[ge] = Data;
			NODEWRITE(actnode, Data[ge], node);  // overwrite data and exit 
			return iterator(this, actnode, ge);
		} else {				
			if (node.Info & cIsLeafMask) // direct insert to leaf node
			{
				memcpy(&(node.Key[ge+1]), &(node.Key[ge]), sizeof(TKey) * ((node.Info & cChildCountMask) - ge));
				memcpy(&(node.Data[ge+1]), &(node.Data[ge]), sizeof(TData) * ((node.Info & cChildCountMask) - ge));
				memcpy(&(node.Child[ge+2]), &(node.Child[ge+1]), sizeof(TNode.Child[0]) * ((node.Info & cChildCountMask) - ge));
				
				node.Key[ge] = Key;
				node.Data[ge] = Data;
				node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) + 1);

				WriteNode(actnode, node);

				return iterator(this, actnode, ge);

			} else {  // middle node
				nextnode = node.Child[ge];
				NODEREAD(nextnode, Info, node2);

				if (node2.Info & cChildCountMask = cFullNode) // split the childnode
				{
					memcpy(&(node.Key[ge+1]), &(node.Key[ge]), sizeof(TKey) * ((node.Info & cChildCountMask) - ge));
					memcpy(&(node.Data[ge+1]), &(node.Data[ge]), sizeof(TData) * ((node.Info & cChildCountMask) - ge));
					memcpy(&(node.Child[ge+2]), &(node.Child[ge+1]), sizeof(TNode.Child[0]) * ((node.Info & cChildCountMask) - ge));

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
typename CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator & 
CBTree<TKey, TData, SizeParam, UniqueKeys>::Find(const TKey & Key) const
{
	TNode node;
	unsigned int actnode = m_Root;
	int ge;

	if (!m_Root) return iterator(this, 0, -1);

	ReadNode(actnode, node);

	while (actnode && !InNodeFind(node, Key, ge))
	{
		if (node.Info & cIsLeafMask)
			return iterator(this, 0, -1)
		else
		{
			actnode = node.Child[ge];
			ReadNode(actnode, node);
		}
	}

	return iterator(this, actnode, ge);
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
				ge = 0
			else
				ge = node.Info & cKeyCountMask;

		} else {
			exists = InNodeFind(node, Key, ge);
		}

		if (exists)
		{
			if (node.Info & cIsLeafMask)  // delete in leaf
			{
				memcpy(&(node.Key[ge]), &(node.Key[ge+1]), sizeof(TKey) * ((node.Info & cChildCountMask) - ge));
				memcpy(&(node.Data[ge]), &(node.Data[ge+1]), sizeof(TData) * ((node.Info & cChildCountMask) - ge));
				memcpy(&(node.Child[ge+1]), &(node.Child[ge+2]), sizeof(TNode.Child[0]) * ((node.Info & cChildCountMask) - ge));
				
				node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);

				WriteNode(actnode, node);

				return true;

			} else { // delete in inner node
				l = node.Child[ge];
				r = node.Child[ge+1];
				NODEREAD(l, Info, lnode);
				NODEREAD(r, Info, rnode);


				if (((rnode.Info & cKeyCountMask) == cEmptyNode) && ((lnode.Info & cKeyCountMask) == cEmptyNode))
				{ // merge childnodes and keep going
					nextnode = MergeNodes(l, r, node.Key[ge], node.Data[ge]);
					
					memcpy(&(node.Key[ge]), &(node.Key[ge+1]), sizeof(TKey) * ((node.Info & cChildCountMask) - ge));
					memcpy(&(node.Data[ge]), &(node.Data[ge+1]), sizeof(TData) * ((node.Info & cChildCountMask) - ge));
					memcpy(&(node.Child[ge+1]), &(node.Child[ge+2]), sizeof(TNode.Child[0]) * ((node.Info & cChildCountMask) - ge));
				
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

					NODEWRITE(innernode, Key[innerindex], tmp);
					NODEWRITE(innernode, Data[innerindex], tmp);					

					memcpy(&(node.Key[0]), &(node.Key[1]), sizeof(TKey) * (node.Info & cChildCountMask));
					memcpy(&(node.Data[0]), &(node.Data[1]), sizeof(TData) * (node.Info & cChildCountMask));

					WriteNode(actnode, node);
									
				} else {					
					node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);

					tmp.Key[innerindex] = node.Key[node.Info & cKeyCountMask];
					tmp.Data[innerindex] = node.Data[node.Info & cKeyCountMask];

					NODEWRITE(innernode, Key[innerindex], tmp);
					NODEWRITE(innernode, Data[innerindex], tmp);

					NODEWRITE(actnode, Info, node);
				}				
			}
			return foundininnernode;

		} else { // inner node. go on and check if moving or merging is neccessary
			nextnode = node.Child[ge];
			NODEREAD(nextnode, Info, node2);

			if ((node2.Info & cKeyCountMask) == cEmptyNode) // move or merge
			{
				// set l and r for easier access
				if (ge > 0)	
				{
					l = node.Child[ge - 1] 
					NODEREAD(l, Info, lnode);
				} else 
					l = 0;

				if (ge - 1 < (node.Info & cKeyCountMask))	
				{
					r = node.Child[ge + 1] 
					NODEREAD(r, Info, rnode);
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
					NODEWRITE(actnode, Key[ge], node);
					NODEWRITE(actnode, Data[ge], node);

					// decrement right node key count and remove the first key-data-pair
					rnode.Info = (rnode.Info & cIsLeafMask) | ((rnode.Info & cKeyCountMask) - 1);
					memcpy(&(rnode.Key[0]), &(rnode.Key[1]), sizeof(TKey) * (rnode.Info & cChildCountMask));
					memcpy(&(rnode.Data[0]), &(rnode.Data[1]), sizeof(TData) * (rnode.Info & cChildCountMask));
					memcpy(&(rnode.Child[0]), &(rnode.Child[1]), sizeof(TNode.Child[0]) * ((rnode.Info & cChildCountMask) + 1));
				
					// increment KeyCount of the next node
					node2.Info = (node2.Info & cIsLeafMask) | ((node2.Info & cKeyCountMask) + 1);
					if ((node2.Info & cIsLeafMask) == 0) // update the parent property of moved child
					{						
						tmp.Parent = nextnode;
						NODEWrite(node2.Child[node.Info & cKeyCountMask], Parent, tmp);
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
					memcpy(&(node2.Key[1]), &(node2.Key[0]), sizeof(TKey) * (node2.Info & cChildCountMask));
					memcpy(&(node2.Data[1]), &(node2.Data[0]), sizeof(TData) * (node2.Info & cChildCountMask));
					memcpy(&(node2.Child[1]), &(node2.Child[0]), sizeof(TNode.Child[0]) * ((node2.Info & cChildCountMask) + 1));
					node2.Info = (node2.Info & cIsLeafMask) | ((node2.Info & cKeyCountMask) + 1);					

					// move key-data-pair down from current to the next node
					node2.Key[0] = node.Key[ge];
					node2.Data[0] = node.Data[ge];
					
					// move the child from left to next node
					node2.Child[0] = lnode.Child[lnode.Info & cKeyCountMask];

					// move key-data-pair up from left to current node					
					node.Key[ge] = lnode.Key[(lnode.Info & cKeyCountMask) - 1];
					node.Data[ge] = lnode.Data[(lnode.Info & cKeyCountMask) - 1];		
					NODEWRITE(actnode, Key[ge], node);
					NODEWRITE(actnode, Data[ge], node);

					// decrement left node key count
					lnode.Info = (lnode.Info & cIsLeafMask) | ((lnode.Info & cKeyCountMask) - 1);
					NODEWRITE(l, Info, lnode);

					if ((node2.Info & cIsLeafMask) == 0) // update the parent property of moved child
					{
						tmp.Parent = nextnode;
						NODEWRITE(node2.Child[0], Parent, tmp);
					}

					WriteNode(nextnode, node2);
					node = node2;
					skipread = true;			

				} else if (l != 0) // merge with the left node
				{
					nextnode = MergeNodes(l, nextnode, node.Key[ge - 1], node.Data[ge - 1]);
					node.Child[ge - 1] = nextnode;

					memcpy(&(node.Key[ge-1]), &(node.Key[ge]), sizeof(TKey) * ((node.Info & cChildCountMask) - ge));
					memcpy(&(node.Data[ge-1]), &(node.Data[ge]), sizeof(TData) * ((node.Info & cChildCountMask) - ge));
					memcpy(&(node.Child[ge]), &(node.Child[ge+1]), sizeof(TNode.Child[0]) * ((node.Info & cChildCountMask) - ge));
					
					node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);				

					WriteNode(actnode, node);

				} else { // merge with the right node
					nextnode = MergeNodes(nextnode, r, node.Key[ge], node.Data[ge]);
					node.Child[ge] = nextnode;

					memcpy(&(node.Key[ge]), &(node.Key[ge+1]), sizeof(TKey) * ((node.Info & cChildCountMask) - ge - 1));
					memcpy(&(node.Data[ge]), &(node.Data[ge+1]), sizeof(TData) * ((node.Info & cChildCountMask) - ge - 1));
					memcpy(&(node.Child[ge+1]), &(node.Child[ge+2]), sizeof(TNode.Child[0]) * ((node.Info & cChildCountMask) - ge - 1));
					
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
void CBTree<TKey, TData, SizeParam, UniqueKeys>::Delete(const iterator& Item)
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


		TNode node2;
		PNode access, access2;
		unsigned int actnode = m_Root;
		unsigned int nextnode, l, r;
		bool exists, skipread;
		int ge;

		bool foundininnernode = false;
		bool wantleftmost = false;
		TKey * innerkey;
		TData * innerdata;

		std::stack<unsigned int> path;

		actnode = Item.m_Node;
		while (actnode != m_Root)
		{
			path.push(actnode);
			actnode = AccessNode(actnode)->Parent;			
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
					ge = 0
				else
					ge = node.Info & cKeyCountMask;

			} else if (actnode = Item.m_Node)
			{				
				exists = true;
				ge = Item.m_Index;
			} else {
				exists = false;
				nextnode = path.top();
				path.pop();

				ge = 0;
				while ((ge < (node.Info & cChildCountMask)) && (node.Child[ge] != nextnode))
					ge++;

			}

			if (exists)
			{
				if (node.Info & cIsLeafMask)  // delete in leaf
				{
					memcpy(&(node.Key[ge]), &(node.Key[ge+1]), sizeof(TKey) * ((node.Info & cChildCountMask) - ge));
					memcpy(&(node.Data[ge]), &(node.Data[ge+1]), sizeof(TData) * ((node.Info & cChildCountMask) - ge));
					memcpy(&(node.Child[ge+1]), &(node.Child[ge+2]), sizeof(TNode.Child[0]) * ((node.Info & cChildCountMask) - ge));
					
					node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);

					WriteNode(actnode, node);

					return true;

				} else { // delete in inner node
					l = node.Child[ge];
					r = node.Child[ge+1];
					NODEREAD(l, Info, lnode);
					NODEREAD(r, Info, rnode);

					if (((rnode.Info & cKeyCountMask) == cEmptyNode) && ((lnode.Info & cKeyCountMask) == cEmptyNode))
					{ // merge childnodes and keep going
						Item.m_Index = lnode.Info & cKeyCountMask;
						
						nextnode = MergeNodes(l, r, node.Key[ge], node.Data[ge]);

						Item.m_Node = nextnode;
						
						memcpy(&(node.Key[ge]), &(node.Key[ge+1]), sizeof(TKey) * ((node.Info & cChildCountMask) - ge));
						memcpy(&(node.Data[ge]), &(node.Data[ge+1]), sizeof(TData) * ((node.Info & cChildCountMask) - ge));
						memcpy(&(node.Child[ge+1]), &(node.Child[ge+2]), sizeof(TNode.Child[0]) * ((node.Info & cChildCountMask) - ge));
					
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

						NODEWRITE(innernode, Key[innerindex], tmp);
						NODEWRITE(innernode, Data[innerindex], tmp);					

						memcpy(&(node.Key[0]), &(node.Key[1]), sizeof(TKey) * (node.Info & cChildCountMask));
						memcpy(&(node.Data[0]), &(node.Data[1]), sizeof(TData) * (node.Info & cChildCountMask));

						WriteNode(actnode, node);
										
					} else {					
						node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);

						tmp.Key[innerindex] = node.Key[node.Info & cKeyCountMask];
						tmp.Data[innerindex] = node.Data[node.Info & cKeyCountMask];

						NODEWRITE(innernode, Key[innerindex], tmp);
						NODEWRITE(innernode, Data[innerindex], tmp);

						//WriteNode(actnode, node);
						//AccessNode(actnode)->Info = node.Info;
						NODEWRITE(actnode, Info, node);
					}				
				}
				return;

			} else { // inner node. go on and check if moving or merging is neccessary
				nextnode = node.Child[ge];
				NODEREAD(nextnode, Info, node2);

				if ((node2.Info & cKeyCountMask) == cEmptyNode) // move or merge
				{
					// set l and r for easier access
					if (ge > 0)	
					{
						l = node.Child[ge - 1] 
						NODEREAD(l, Info, lnode);
					} else 
						l = 0;

					if (ge - 1 < (node.Info & cKeyCountMask))	
					{
						r = node.Child[ge + 1] 
						NODEREAD(r, Info, rnode);
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
						NODEWRITE(actnode, Key[ge], node);
						NODEWRITE(actnode, Data[ge], node);

						// decrement right node key count and remove the first key-data-pair
						rnode.Info = (rnode.Info & cIsLeafMask) | ((rnode.Info & cKeyCountMask) - 1);
						memcpy(&(rnode.Key[0]), &(rnode.Key[1]), sizeof(TKey) * (rnode.Info & cChildCountMask));
						memcpy(&(rnode.Data[0]), &(rnode.Data[1]), sizeof(TData) * (rnode.Info & cChildCountMask));
						memcpy(&(rnode.Child[0]), &(rnode.Child[1]), sizeof(TNode.Child[0]) * ((rnode.Info & cChildCountMask) + 1));
					
						// increment KeyCount of the next node
						node2.Info = (node2.Info & cIsLeafMask) | ((node2.Info & cKeyCountMask) + 1);
						if ((node2.Info & cIsLeafMask) == 0) // update the parent property of moved child
						{						
							tmp.Parent = nextnode;
							NODEWrite(node2.Child[node.Info & cKeyCountMask], Parent, tmp);
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
						memcpy(&(node2.Key[1]), &(node2.Key[0]), sizeof(TKey) * (node2.Info & cChildCountMask));
						memcpy(&(node2.Data[1]), &(node2.Data[0]), sizeof(TData) * (node2.Info & cChildCountMask));
						memcpy(&(node2.Child[1]), &(node2.Child[0]), sizeof(TNode.Child[0]) * ((node2.Info & cChildCountMask) + 1));
						node2.Info = (node2.Info & cIsLeafMask) | ((node2.Info & cKeyCountMask) + 1);					

						// move key-data-pair down from current to the next node
						node2.Key[0] = node.Key[ge];
						node2.Data[0] = node.Data[ge];
						
						// move the child from left to next node
						node2.Child[0] = lnode.Child[lnode.Info & cKeyCountMask];

						// move key-data-pair up from left to current node					
						node.Key[ge] = lnode.Key[(lnode.Info & cKeyCountMask) - 1];
						node.Data[ge] = lnode.Data[(lnode.Info & cKeyCountMask) - 1];		
						NODEWRITE(actnode, Key[ge], node);
						NODEWRITE(actnode, Data[ge], node);

						// decrement left node key count
						lnode.Info = (lnode.Info & cIsLeafMask) | ((lnode.Info & cKeyCountMask) - 1);
						NODEWRITE(l, Info, lnode);

						if ((node2.Info & cIsLeafMask) == 0) // update the parent property of moved child
						{
							tmp.Parent = nextnode;
							NODEWRITE(node2.Child[0], Parent, tmp);
						}

						WriteNode(nextnode, node2);
						node = node2;
						skipread = true;			

					} else if (l != 0) // merge with the left node
					{
						if (nextnod == Item.m_Node)
						{
							Item.m_Index += (lnode.Info & cKeyCountMask);
							nextnode = MergeNodes(l, nextnode, node.Key[ge - 1], node.Data[ge - 1]);
							Item.m_Node = nextnode;
						} else {
							nextnode = MergeNodes(l, nextnode, node.Key[ge - 1], node.Data[ge - 1]);
						}

						node.Child[ge - 1] = nextnode;

						memcpy(&(node.Key[ge-1]), &(node.Key[ge]), sizeof(TKey) * ((node.Info & cChildCountMask) - ge));
						memcpy(&(node.Data[ge-1]), &(node.Data[ge]), sizeof(TData) * ((node.Info & cChildCountMask) - ge));
						memcpy(&(node.Child[ge]), &(node.Child[ge+1]), sizeof(TNode.Child[0]) * ((node.Info & cChildCountMask) - ge));
						
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

						memcpy(&(node.Key[ge]), &(node.Key[ge+1]), sizeof(TKey) * ((node.Info & cChildCountMask) - ge - 1));
						memcpy(&(node.Data[ge]), &(node.Data[ge+1]), sizeof(TData) * ((node.Info & cChildCountMask) - ge - 1));
						memcpy(&(node.Child[ge+1]), &(node.Child[ge+2]), sizeof(TNode.Child[0]) * ((node.Info & cChildCountMask) - ge - 1));
						
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
	Read(Node, 0, sizeof(TNode), Source);
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

		if (node->Info & cIsLeafMask == 0)
			for (i = 0; i <= (node->Info & cKeyCountMask); i++)
				q.push((TNode*)node->Child[i]);

		delete node;
	}
}