#include "BTree.h"

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
	memcpy(&(newnode.Childs[0]), &(nodedata.Childs[upindex+1]), sizeof(TNode.Childs[0]) * (cFullNode - upindex + 1));

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
	memcpy(&(ldata.Childs[downindex+1]), &(rdata.Childs[0]), sizeof(TNode.Childs[0]) * ((rdata.Info & cKeyCountMask) + 1));

	ldata.Info = (ldata.Info & cIsLeafMask) | (downindex + 1 + (rdata.Info & cKeyCountMask));

	WriteNode(Left, ldata);
	DeleteNode(Right);

	return Left;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
typename CBTree<TKey, TData, SizeParam, UniqueKeys>::iterator & 
CBTree<TKey, TData, SizeParam, UniqueKeys>::Insert(const TKey & Key, const TData & Data) const
{
	TNode node, node;
	unsigned int actnode = m_Root;
	unsigned int nextnode;
	bool exists;
	int ge;
	
	if (AccessNode(actnode)->Info & cKeyCountMask = cFullNode)  // root split
	{
		// be a little tricky and let the main code handle the actual splitting.
		// just assign a new root with keycount to zero and one child = old root
		// the InNode test will fail with GreaterEqual = 0
		nextnode = CreateNewNode();
		AccessNode(m_Root)->Parent = nextnode;

		AccessNode(nextnode)->Childs[0] = actnode;
		actnode = nextnode;

		m_Root = nextnode;
		RootChanged();
	}

	ReadNode(actnode, node);

	while (actnode)
	{
		exists = InNode(node, Key);
		if (UniqueKeys && exists)  // already exists
		{
			AccessNode(actnode)->Data[ge] = Data;  // overwrite data and exit
			return iterator(this, actnode, ge);
		} else {				
			if (node.Info & cIsLeafMask) // direct insert to leaf node
			{
				memcpy(&(node.Key[ge+1]), &(node.Key[ge]), sizeof(TKey) * ((node.Info & cChildCountMask) - ge));
				memcpy(&(node.Data[ge+1]), &(node.Data[ge]), sizeof(TData) * ((node.Info & cChildCountMask) - ge));
				memcpy(&(node.Childs[ge+2]), &(node.Childs[ge+1]), sizeof(TNode.Childs[0]) * ((node.Info & cChildCountMask) - ge));
				
				node.Key[ge] = Key;
				node.Data[ge] = Data;
				node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) + 1);

				WriteNode(actnode, node);

				return iterator(this, actnode, ge);

			} else {  // middle node
				nextnode = node.Childs[ge];

				if (AccessNode(node2)->Info & cChildCountMask = cFullNode) // split the childnode
				{
					memcpy(&(node.Key[ge+1]), &(node.Key[ge]), sizeof(TKey) * ((node.Info & cChildCountMask) - ge));
					memcpy(&(node.Data[ge+1]), &(node.Data[ge]), sizeof(TData) * ((node.Info & cChildCountMask) - ge));
					memcpy(&(node.Childs[ge+2]), &(node.Childs[ge+1]), sizeof(TNode.Childs[0]) * ((node.Info & cChildCountMask) - ge));

					SplitNode(nextnode, node.Childs[ge], node.Childs[ge+1], node.Key[ge], node.Data[ge]);

					WriteNode(actnode, node);
					
					if (UniqueKeys && (node.Key[ge] == Key))
					{
						AccessNode(actnode)->Data[ge] = Data;  // overwrite data and exit
						return iterator(this, actnode, ge);
					} else if (node.Key[ge] < Key)
					{
						nextnode = node.Childs[ge+1];
					} else {
						nextnode = node.Childs[ge];
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

	ReadNode(actnode, node);

	while (actnode && !InNodeFind(node, Key, ge))
	{
		if (node.Info & cIsLeafMask)
			return iterator(this, 0, -1)
		else
		{
			actnode = node.Childs[ge];
			ReadNode(actnode, node);
		}
	}

	return iterator(this, actnode, ge);
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
bool CBTree<TKey, TData, SizeParam, UniqueKeys>::Delete(const TKey& Key)
{
	TNode node, node2;
	PNode access, access2;
	unsigned int actnode = m_Root;
	unsigned int nextnode, l, r;
	bool exists, skipread;
	int ge;

	bool foundininnernode = false;
	bool wantleftmost = false;
	TKey * innerkey;
	TData * innerdata;

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
				memcpy(&(node.Childs[ge+1]), &(node.Childs[ge+2]), sizeof(TNode.Childs[0]) * ((node.Info & cChildCountMask) - ge));
				
				node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);

				WriteNode(actnode, node);

				return true;

			} else { // delete in inner node
				l = node.Childs[ge];
				r = node.Childs[ge+1];

				if (((AccessNode(r)->Info & cKeyCountMask) == cEmptyNode) && ((AccessNode(l)->Info & cKeyCountMask) == cEmptyNode))
				{ // merge childnodes and keep going
					nextnode = MergeNodes(l, r, node.Key[ge], node.Data[ge]);
					
					memcpy(&(node.Key[ge]), &(node.Key[ge+1]), sizeof(TKey) * ((node.Info & cChildCountMask) - ge));
					memcpy(&(node.Data[ge]), &(node.Data[ge+1]), sizeof(TData) * ((node.Info & cChildCountMask) - ge));
					memcpy(&(node.Childs[ge+1]), &(node.Childs[ge+2]), sizeof(TNode.Childs[0]) * ((node.Info & cChildCountMask) - ge));
				
					node.Childs[ge] = nextnode;
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
					innerkey = &(AccessNode(actnode)->Key[ge]);
					innerdata = &(AccessNode(actnode)->Data[ge]);
					
					if ((AccessNode(l)->Info & cKeyCountMask) == cEmptyNode)
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

					*innerkey = node.Key[0];
					*innerdata = node.Data[0];

					memcpy(&(node.Key[0]), &(node.Key[1]), sizeof(TKey) * (node.Info & cChildCountMask));
					memcpy(&(node.Data[0]), &(node.Data[1]), sizeof(TData) * (node.Info & cChildCountMask));

					WriteNode(actnode, node);
									
				} else {					
					node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);

					*innerkey = node.Key[node.Info & cKeyCountMask];
					*innerdata = node.Data[node.Info & cKeyCountMask];

					//WriteNode(actnode, node);
					AccessNode(actnode)->Info = node.Info;
				}				
			}
			return foundininnernode;

		} else { // inner node. go on and check if moving or merging is neccessary
			nextnode = node.Childs[ge];

			if ((AccessNode(nextnode)->Info & cKeyCountMask) == cEmptyNode) // move or merge
			{
				// set l and r for easier access
				if (ge > 0)	
					l = node.Childs[ge - 1] 
				else 
					l = 0;

				if (ge - 1 < (node.Info & cKeyCountMask))	
					r = node.Childs[ge + 1] 
				else 
					r = 0;


				if ((r != 0) && ((AccessNode(r)->Info & cKeyCountMask) > cEmptyNode)) // move a Key-Data-pair from the right
				{
					ReadNode(r, node2);
					ReadNode(nextnode, node);
					access = AccessNode(actnode);
					
					// move key-data-pair down from current to the next node
					node.Key[node.Info & cKeyCountMask] = access->Key[ge];
					node.Data[node.Info & cKeyCountMask] = access->Data[ge];

					// move the child from right to next node
					node.Childs[(node.Info & cKeyCountMask) + 1] = node2.Childs[0];

					// move key-data-pair up from right to current node
					access->Key[ge] = node2.Key[0];
					access->Data[ge] = node2.Data[0];					

					// decrement right node key count and remove the first key-data-pair
					node2.Info = (node2.Info & cIsLeafMask) | ((node2.Info & cKeyCountMask) - 1);
					memcpy(&(node2.Key[0]), &(node2.Key[1]), sizeof(TKey) * (node2.Info & cChildCountMask));
					memcpy(&(node2.Data[0]), &(node2.Data[1]), sizeof(TData) * (node2.Info & cChildCountMask));
					memcpy(&(node2.Childs[0]), &(node2.Childs[1]), sizeof(TNode.Childs[0]) * ((node2.Info & cChildCountMask) + 1));
				
					// increment KeyCount of the next node
					node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) + 1);
					if ((node.Info & cIsLeafMask) == 0) // update the parent property of moved child
					{
						AccessNode(node.Childs[access->Info & cKeyCountMask])->Parent = nextnode;
					}

					WriteNode(r, node2);
					WriteNode(nextnode, node);
					skipread = true;

				} else if ((l != 0) && ((AccessNode(l)->Info & cKeyCountMask) > cEmptyNode)) // move a Key-Data-pair from the left
				{					
					ReadNode(nextnode, node);
					access = AccessNode(actnode);
					access2 = AccessNode(l);
									
					// increment next node key count and make new first key-data-pair					
					memcpy(&(node.Key[1]), &(node.Key[0]), sizeof(TKey) * (node.Info & cChildCountMask));
					memcpy(&(node.Data[1]), &(node.Data[0]), sizeof(TData) * (node.Info & cChildCountMask));
					memcpy(&(node.Childs[1]), &(node.Childs[0]), sizeof(TNode.Childs[0]) * ((node.Info & cChildCountMask) + 1));
					node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) + 1);					

					// move key-data-pair down from current to the next node
					node.Key[0] = access->Key[ge];
					node.Data[0] = access->Data[ge];
					
					// move the child from left to next node
					node.Childs[0] = access2->Childs[access2->Info & cKeyCountMask];

					// move key-data-pair up from left to current node					
					access->Key[ge] = access2->Key[(access2->Info & cKeyCountMask) - 1];
					access->Data[ge] = access2->Data[(access2->Info & cKeyCountMask) - 1];		

					// decrement left node key count
					access2->Info = (access2->Info & cIsLeafMask) | ((access2->Info & cKeyCountMask) - 1);

					if ((node.Info & cIsLeafMask) == 0) // update the parent property of moved child
					{
						AccessNode(node.Childs[0])->Parent = nextnode;
					}

					WriteNode(nextnode, node);
					skipread = true;			

				} else if (l != 0) // merge with the left node
				{
					nextnode = MergeNodes(l, nextnode, node.Key[ge - 1], node.Data[ge - 1]);
					node.Childs[ge - 1] = nextnode;

					memcpy(&(node.Key[ge-1]), &(node.Key[ge]), sizeof(TKey) * ((node.Info & cChildCountMask) - ge));
					memcpy(&(node.Data[ge-1]), &(node.Data[ge]), sizeof(TData) * ((node.Info & cChildCountMask) - ge));
					memcpy(&(node.Childs[ge]), &(node.Childs[ge+1]), sizeof(TNode.Childs[0]) * ((node.Info & cChildCountMask) - ge));
					
					node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);				

					WriteNode(actnode, node);

				} else { // merge with the right node
					nextnode = MergeNodes(nextnode, r, node.Key[ge], node.Data[ge]);
					node.Childs[ge] = nextnode;

					memcpy(&(node.Key[ge]), &(node.Key[ge+1]), sizeof(TKey) * ((node.Info & cChildCountMask) - ge - 1));
					memcpy(&(node.Data[ge]), &(node.Data[ge+1]), sizeof(TData) * ((node.Info & cChildCountMask) - ge - 1));
					memcpy(&(node.Childs[ge+1]), &(node.Childs[ge+2]), sizeof(TNode.Childs[0]) * ((node.Info & cChildCountMask) - ge - 1));
					
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
				while ((ge < (node.Info & cChildCountMask)) && (node.Childs[ge] != nextnode))
					ge++;

			}

			if (exists) // we reached the node.
			{
				if (node.Info & cIsLeafMask)  // delete in leaf
				{
					memcpy(&(node.Key[ge]), &(node.Key[ge+1]), sizeof(TKey) * ((node.Info & cChildCountMask) - ge));
					memcpy(&(node.Data[ge]), &(node.Data[ge+1]), sizeof(TData) * ((node.Info & cChildCountMask) - ge));
					memcpy(&(node.Childs[ge+1]), &(node.Childs[ge+2]), sizeof(TNode.Childs[0]) * ((node.Info & cChildCountMask) - ge));
					
					node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);

					WriteNode(actnode, node);

					return;

				} else { // delete in inner node
					l = node.Childs[ge];
					r = node.Childs[ge+1];

					if (((AccessNode(r)->Info & cKeyCountMask) == cEmptyNode) && ((AccessNode(l)->Info & cKeyCountMask) == cEmptyNode))
					{ // merge childnodes and keep going

						Item.m_Index = AccessNode(l)->Info & cKeyCountMask;

						nextnode = MergeNodes(l, r, node.Key[ge], node.Data[ge]);

						Item.m_Node = nextnode;						
						
						memcpy(&(node.Key[ge]), &(node.Key[ge+1]), sizeof(TKey) * ((node.Info & cChildCountMask) - ge));
						memcpy(&(node.Data[ge]), &(node.Data[ge+1]), sizeof(TData) * ((node.Info & cChildCountMask) - ge));
						memcpy(&(node.Childs[ge+1]), &(node.Childs[ge+2]), sizeof(TNode.Childs[0]) * ((node.Info & cChildCountMask) - ge));
					
						node.Childs[ge] = nextnode;
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
						innerkey = &(AccessNode(actnode)->Key[ge]);
						innerdata = &(AccessNode(actnode)->Data[ge]);
						
						if ((AccessNode(l)->Info & cKeyCountMask) == cEmptyNode)
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

						*innerkey = node.Key[0];
						*innerdata = node.Data[0];

						memcpy(&(node.Key[0]), &(node.Key[1]), sizeof(TKey) * (node.Info & cChildCountMask));
						memcpy(&(node.Data[0]), &(node.Data[1]), sizeof(TData) * (node.Info & cChildCountMask));

						WriteNode(actnode, node);
										
					} else {					
						node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) - 1);

						*innerkey = node.Key[node.Info & cKeyCountMask];
						*innerdata = node.Data[node.Info & cKeyCountMask];

						//WriteNode(actnode, node);
						AccessNode(actnode)->Info = node.Info;
					}				
				}
				return;

			} else { // inner node. go on and check if moving or merging is neccessary
				nextnode = node.Childs[ge];

				if ((AccessNode(nextnode)->Info & cKeyCountMask) == cEmptyNode) // move or merge
				{
					// set l and r for easier access
					if (ge > 0)	
						l = node.Childs[ge - 1] 
					else 
						l = 0;

					if (ge - 1 < (node.Info & cKeyCountMask))	
						r = node.Childs[ge + 1] 
					else 
						r = 0;


					if ((r != 0) && ((AccessNode(r)->Info & cKeyCountMask) > cEmptyNode)) // move a Key-Data-pair from the right
					{
						ReadNode(r, node2);
						ReadNode(nextnode, node);
						access = AccessNode(actnode);
						
						// move key-data-pair down from current to the next node
						node.Key[node.Info & cKeyCountMask] = access->Key[ge];
						node.Data[node.Info & cKeyCountMask] = access->Data[ge];

						// move the child from right to next node
						node.Childs[(node.Info & cKeyCountMask) + 1] = node2.Childs[0];

						// move key-data-pair up from right to current node
						access->Key[ge] = node2.Key[0];
						access->Data[ge] = node2.Data[0];					

						// decrement right node key count and remove the first key-data-pair
						node2.Info = (node2.Info & cIsLeafMask) | ((node2.Info & cKeyCountMask) - 1);
						memcpy(&(node2.Key[0]), &(node2.Key[1]), sizeof(TKey) * (node2.Info & cChildCountMask));
						memcpy(&(node2.Data[0]), &(node2.Data[1]), sizeof(TData) * (node2.Info & cChildCountMask));
						memcpy(&(node2.Childs[0]), &(node2.Childs[1]), sizeof(TNode.Childs[0]) * ((node2.Info & cChildCountMask) + 1));
					
						// increment KeyCount of the next node
						node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) + 1);
						if ((node.Info & cIsLeafMask) == 0) // update the parent property of moved child
						{
							AccessNode(node.Childs[access->Info & cKeyCountMask])->Parent = nextnode;
						}

						WriteNode(r, node2);
						WriteNode(nextnode, node);
						skipread = true;

					} else if ((l != 0) && ((AccessNode(l)->Info & cKeyCountMask) > cEmptyNode)) // move a Key-Data-pair from the left
					{					
						ReadNode(nextnode, node);
						access = AccessNode(actnode);
						access2 = AccessNode(l);

						if (nextnode == Item.m_Node) 
							Item.m_Index++;
										
						// increment next node key count and make new first key-data-pair					
						memcpy(&(node.Key[1]), &(node.Key[0]), sizeof(TKey) * (node.Info & cChildCountMask));
						memcpy(&(node.Data[1]), &(node.Data[0]), sizeof(TData) * (node.Info & cChildCountMask));
						memcpy(&(node.Childs[1]), &(node.Childs[0]), sizeof(TNode.Childs[0]) * ((node.Info & cChildCountMask) + 1));
						node.Info = (node.Info & cIsLeafMask) | ((node.Info & cKeyCountMask) + 1);					

						// move key-data-pair down from current to the next node
						node.Key[0] = access->Key[ge];
						node.Data[0] = access->Data[ge];
						
						// move the child from left to next node
						node.Childs[0] = access2->Childs[access2->Info & cKeyCountMask];

						// move key-data-pair up from left to current node					
						access->Key[ge] = access2->Key[(access2->Info & cKeyCountMask) - 1];
						access->Data[ge] = access2->Data[(access2->Info & cKeyCountMask) - 1];		

						// decrement left node key count
						access2->Info = (access2->Info & cIsLeafMask) | ((access2->Info & cKeyCountMask) - 1);

						if ((node.Info & cIsLeafMask) == 0) // update the parent property of moved child
						{
							AccessNode(node.Childs[0])->Parent = nextnode;
						}

						WriteNode(nextnode, node);
						skipread = true;			

					} else if (l != 0) // merge with the left node
					{
						if (nextnode == Item.m_Node)
						{
							Item.m_Index += (AccessNode(l)->Info & cKeyCountMask);
							nextnode = MergeNodes(l, nextnode, node.Key[ge - 1], node.Data[ge - 1]);
							Item.m_Node = nextnode;
						} else {
							nextnode = MergeNodes(l, nextnode, node.Key[ge - 1], node.Data[ge - 1]);
						}
						node.Childs[ge - 1] = nextnode;

						memcpy(&(node.Key[ge-1]), &(node.Key[ge]), sizeof(TKey) * ((node.Info & cChildCountMask) - ge));
						memcpy(&(node.Data[ge-1]), &(node.Data[ge]), sizeof(TData) * ((node.Info & cChildCountMask) - ge));
						memcpy(&(node.Childs[ge]), &(node.Childs[ge+1]), sizeof(TNode.Childs[0]) * ((node.Info & cChildCountMask) - ge));
						
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

						node.Childs[ge] = nextnode;

						memcpy(&(node.Key[ge]), &(node.Key[ge+1]), sizeof(TKey) * ((node.Info & cChildCountMask) - ge - 1));
						memcpy(&(node.Data[ge]), &(node.Data[ge+1]), sizeof(TData) * ((node.Info & cChildCountMask) - ge - 1));
						memcpy(&(node.Childs[ge+1]), &(node.Childs[ge+2]), sizeof(TNode.Childs[0]) * ((node.Info & cChildCountMask) - ge - 1));
						
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
virtual unsigned int CBTree<TKey, TData, SizeParam, UniqueKeys>::CreateNewNode()
{
	return new TNode;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
virtual void CBTree<TKey, TData, SizeParam, UniqueKeys>::DeleteNode(unsigned int Node)
{
	delete (PNode)Node;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
virtual void CBTree<TKey, TData, SizeParam, UniqueKeys>::RootChanged()
{
	//nothing to do
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
virtual typename CBTree<TKey, TData, SizeParam, UniqueKeys>::PNode 
CBTree<TKey, TData, SizeParam, UniqueKeys>::AccessNode(unsigned int Node)
{
	return Node;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
virtual void CBTree<TKey, TData, SizeParam, UniqueKeys>::ReadNode(unsigned int Node, TNode & Dest)
{
	memcpy(&Dest, Node, sizeof(TNode));
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
virtual void CBTree<TKey, TData, SizeParam, UniqueKeys>::WriteNode(unsigned int Node, TNode & Source)
{
	memcpy(Node, &Source, sizeof(TNode));
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
virtual void CBTree<TKey, TData, SizeParam, UniqueKeys>::ClearTree()
{
	std::queue<PNode> q;
	PNode node;
	int i;

	q.push(m_Root);
	while (!q.empty())
	{
		node = q.back();
		q.pop();

		if (node->Info & cIsLeafMask == 0)
			for (i = 0; i <= (node->Info & cKeyCountMask); i++)
				q.push(node->Childs[i]);

		delete node;
	}
}