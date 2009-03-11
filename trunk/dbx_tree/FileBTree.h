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
#include "BTree.h"
#include "BlockManager.h"

template <typename TKey, int SizeParam = 4>
class CFileBTree :	public CBTree<TKey, SizeParam>
{
private:

protected:
	CBlockManager & m_BlockManager;
	uint32_t cSignature;

	virtual typename CFileBTree<TKey, SizeParam>::TNodeRef CreateNewNode();
	virtual void DeleteNode(typename CBTree<TKey, SizeParam>::TNodeRef Node);
	virtual void Read(typename CBTree<TKey, SizeParam>::TNodeRef Node, uint32_t Offset, uint32_t Size, typename CBTree<TKey, SizeParam>::TNode & Dest);
	virtual void Write(typename CBTree<TKey, SizeParam>::TNodeRef Node, uint32_t Offset, uint32_t Size, typename CBTree<TKey, SizeParam>::TNode & Source);

public:
	CFileBTree(CBlockManager & BlockManager, typename CBTree<TKey, SizeParam>::TNodeRef RootNode, uint16_t Signature);
	virtual ~CFileBTree();
};




template <typename TKey, int SizeParam>
CFileBTree<TKey, SizeParam>::CFileBTree(CBlockManager & BlockManager, typename CBTree<TKey, SizeParam>::TNodeRef RootNode, uint16_t Signature)
:	CBTree<TKey, SizeParam>::CBTree(RootNode),
	m_BlockManager(BlockManager)
{
	cSignature = Signature << 16;
	CBTree<TKey, SizeParam>::m_DestroyTree = false;
}

template <typename TKey, int SizeParam>
CFileBTree<TKey, SizeParam>::~CFileBTree()
{

}

template <typename TKey, int SizeParam>
typename CFileBTree<TKey, SizeParam>::TNodeRef CFileBTree<TKey, SizeParam>::CreateNewNode()
{
	return m_BlockManager.CreateBlock(sizeof(typename CBTree<TKey, SizeParam>::TNode) - 2, cSignature);
}

template <typename TKey, int SizeParam>
void CFileBTree<TKey, SizeParam>::DeleteNode(typename CBTree<TKey, SizeParam>::TNodeRef Node)
{
	m_BlockManager.DeleteBlock(Node);
}

template <typename TKey, int SizeParam>
void CFileBTree<TKey, SizeParam>::Read(typename CBTree<TKey, SizeParam>::TNodeRef Node, uint32_t Offset, uint32_t Size, typename CBTree<TKey, SizeParam>::TNode & Dest)
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
		throwException("Signature check failed");

}

template <typename TKey, int SizeParam>
void CFileBTree<TKey, SizeParam>::Write(typename CBTree<TKey, SizeParam>::TNodeRef Node, uint32_t Offset, uint32_t Size, typename CBTree<TKey, SizeParam>::TNode & Source)
{
	if (Offset == 0)
	{
		m_BlockManager.WritePart(Node, (uint16_t*)&Source + 1, 0, Size - 2, cSignature | Source.Info);
	} else {
		m_BlockManager.WritePart(Node, (uint8_t*)&Source + Offset, Offset - 2, Size);
	}
}
