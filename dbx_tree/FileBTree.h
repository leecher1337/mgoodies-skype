#pragma once
#include "BTree.h"
#include "BlockManager.h"

template <typename TKey, typename TData = TEmpty, int SizeParam = 4, bool UniqueKeys = true>
class CFileBTree :	public CBTree<TKey, TData, SizeParam, UniqueKeys>
{
private:
	
protected:
	CBlockManager & m_BlockManager;
	uint32_t cSignature;

	virtual TNodeRef CreateNewNode();
	virtual void DeleteNode(TNodeRef Node);
	virtual void Read(TNodeRef Node, uint32_t Offset, uint32_t Size, TNode & Dest);
	virtual void Write(TNodeRef Node, uint32_t Offset, uint32_t Size, TNode & Source);
	
	virtual void DestroyTree();
public:
	CFileBTree(CBlockManager & BlockManager, TNodeRef RootNode, uint16_t Signature);
	virtual ~CFileBTree();
};




template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
CFileBTree<TKey, TData, SizeParam, UniqueKeys>::CFileBTree(CBlockManager & BlockManager, TNodeRef RootNode, uint16_t Signature)
:	CBTree(RootNode),
	m_BlockManager(BlockManager)
{
	cSignature = Signature << 16;
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
CFileBTree<TKey, TData, SizeParam, UniqueKeys>::~CFileBTree()
{

}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
typename CFileBTree<TKey, TData, SizeParam, UniqueKeys>::TNodeRef CFileBTree<TKey, TData, SizeParam, UniqueKeys>::CreateNewNode()
{
	return m_BlockManager.CreateBlock(sizeof(TNode) - 2, cSignature);
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CFileBTree<TKey, TData, SizeParam, UniqueKeys>::DeleteNode(TNodeRef Node)
{
	m_BlockManager.DeleteBlock(Node);
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CFileBTree<TKey, TData, SizeParam, UniqueKeys>::Read(TNodeRef Node, uint32_t Offset, uint32_t Size, TNode & Dest)
{
	uint32_t sig = 0;
	if (Offset == 0)
	{
		m_BlockManager.ReadPart(Node, (uint16_t*)&Dest + 1, 0, Size - 2, sig);
	} else {
		m_BlockManager.ReadPart(Node, (uint8_t*)&Dest + Offset, Offset - 2, Size, sig);		
	}
	Dest.Info = sig & 0xffff;
	sig = sig & 0xffff0000;

	if (sig != cSignature)
			throw "Signature check failed";

}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CFileBTree<TKey, TData, SizeParam, UniqueKeys>::Write(TNodeRef Node, uint32_t Offset, uint32_t Size, TNode & Source)
{
	if (Offset == 0)
	{
		m_BlockManager.WritePart(Node, (uint16_t*)&Source + 1, 0, Size - 2, cSignature | Source.Info);
	} else {
		m_BlockManager.WritePart(Node, (uint8_t*)&Source + Offset, Offset - 2, Size, 0);		
	}
}

template <typename TKey, typename TData, int SizeParam, bool UniqueKeys>
void CFileBTree<TKey, TData, SizeParam, UniqueKeys>::DestroyTree()
{
	//nothing to do
}
