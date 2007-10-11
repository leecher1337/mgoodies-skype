#pragma once
#include "BTree.h"
#include "FileAccess.h"

template <typename TKey, typename TData = TEmpty, int SizeParam = 4, bool UniqueKeys = true>
class CFileBTree :	public CBTree<TKey, TData, SizeParam, UniqueKeys>
{
private:

protected:
	CFileAccess & m_FileAccess;

	virtual unsigned int CreateNewNode();
	virtual void DeleteNode(unsigned int Node);
	virtual void Read(unsigned int Node, int Offset, int Size, TNode & Dest);
	virtual void Write(unsigned int Node, int Offset, int Size, TNode & Source);
	
	virtual void ClearTree();
public:
	CFileBTree(CFileAccess & FileAccess, unsigned int RootNode);
	virtual ~CFileBTree();
};




template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
CFileBTree<TKey, TData, SizeParam, UniqueKeys>::CFileBTree(CFileAccess & FileAccess, unsigned int RootNode)
:	CBTree(RootNode),
	m_FileAccess(FileAccess)
{

}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
CFileBTree<TKey, TData, SizeParam, UniqueKeys>::~CFileBTree()
{

}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
unsigned int CFileBTree<TKey, TData, SizeParam, UniqueKeys>::CreateNewNode()
{
	return m_FileAccess.Alloc(sizeof(TNode));
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CFileBTree<TKey, TData, SizeParam, UniqueKeys>::DeleteNode(unsigned int Node)
{
	m_FileAccess.Free(Node, sizeof(TNode));
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CFileBTree<TKey, TData, SizeParam, UniqueKeys>::Read(unsigned int Node, int Offset, int Size, TNode & Dest)
{
	m_FileAccess.Read((char*)(&Dest) + Offset, Node + Offset, Size);
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CFileBTree<TKey, TData, SizeParam, UniqueKeys>::Write(unsigned int Node, int Offset, int Size, TNode & Source)
{
	m_FileAccess.Write((char*)(&Source) + Offset, Node + Offset, Size);
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CFileBTree<TKey, TData, SizeParam, UniqueKeys>::ClearTree()
{
	//nothing to do
}
