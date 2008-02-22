#pragma once 

template <class TType>
class CIterationHeap
{
public:
	enum TIterationType {ITForward, ITBackward};

	CIterationHeap(TType & InitialItem, TIterationType ForBack, bool DeleteItems = true);
	~CIterationHeap();

	bool Insert(TType & Item);
	TType & Top();
	void Pop();
	
protected:
	typedef struct THeapElement {
		TType * Elem;
		unsigned int Index;
	} THeapElement, * PHeapElement;

	PHeapElement * m_Heap;
	unsigned int m_HeapSize;
	unsigned int m_AllocSize;
	TIterationType m_Type;
	bool m_DeleteItems;	

	bool A_b4_B(PHeapElement a, PHeapElement b);
private:

};


template <class TType>
bool CIterationHeap<TType>::A_b4_B(PHeapElement a, PHeapElement b)
{
	if (m_Type == ITForward)
	{
		if ((*a->Elem) == (*b->Elem)) return a->Index < b->Index;
		return (*a->Elem) < (*b->Elem);
	} else {
		if ((*a->Elem) == (*b->Elem)) return a->Index > b->Index;
		return (*a->Elem) > (*b->Elem);
	}
}

template <class TType>
CIterationHeap<TType>::CIterationHeap(TType & InitialItem, TIterationType MinMax, bool DeleteItems)
{
	m_Heap = (PHeapElement*)malloc(sizeof(PHeapElement));
	m_HeapSize = 1;
	m_AllocSize = 1 << 1;
	m_Type = MinMax;
	
	m_Heap[0] = (THeapElement*)malloc(sizeof(THeapElement));
	m_Heap[0]->Elem = &InitialItem;
	m_Heap[0]->Index = m_HeapSize;
	m_DeleteItems = DeleteItems;
}

template <class TType>
CIterationHeap<TType>::~CIterationHeap()
{
	unsigned int i = 0; 
	while ((i < m_HeapSize) && (m_Heap[i]))
	{
		if (m_DeleteItems && m_Heap[i]->Elem)
			free(m_Heap[i]->Elem);

		free(m_Heap[i]);
		++i;
	}
	free(m_Heap);
}

template <class TType>
bool CIterationHeap<TType>::Insert(TType & Item)
{
	if (m_AllocSize == m_HeapSize + 1)
	{		
		m_AllocSize = m_AllocSize << 1;
		m_Heap = (PHeapElement*)realloc(m_Heap, (m_AllocSize - 1) * sizeof(PHeapElement));		
	}

	m_HeapSize++;

	unsigned int way = m_AllocSize >> 2;	
	unsigned int index = 0;	
	PHeapElement ins = (THeapElement*)malloc(sizeof(THeapElement));
	ins->Elem = &Item;
	ins->Index = m_HeapSize;

	PHeapElement next;

	while ((way > 0) && (index + 1 < m_HeapSize))
	{
		next = m_Heap[index];
		if ((!(*next->Elem)) || A_b4_B(ins, next))
		{
			m_Heap[index] = ins;
			ins = next;
		}

		if (way & m_HeapSize)	 //right
		{
			index = (index << 1) + 2;
		} else {	// left
			index = (index << 1) + 1;
		}
		way = way >> 1;						
	}

	m_Heap[index] = ins;

	return true;
}

template <class TType>
TType & CIterationHeap<TType>::Top()
{
	return *m_Heap[0]->Elem;
}

template <class TType>
void CIterationHeap<TType>::Pop()
{
	if (m_Type == ITForward)
		++(*m_Heap[0]->Elem);
	else
		--(*m_Heap[0]->Elem);

	unsigned int index = 0;
	PHeapElement ins = m_Heap[0];
	unsigned int big = 1;

	while ((big > 0) && (index < (m_HeapSize >> 1)))
	{
		big = 0;

		if ((((index << 1) + 2) < m_HeapSize) && (*m_Heap[(index << 1) + 2]->Elem)) 
		{
			if (*ins->Elem)
			{
				if (A_b4_B(m_Heap[(index << 1) + 2], m_Heap[(index << 1) + 1]))
					big = (index << 1) + 2;					
				else 
					big = (index << 1) + 1;

			} else {				
				m_Heap[index] = m_Heap[(index << 1) + 2];
				index = (index << 1) + 2;
				m_Heap[index] = ins;
			}
		} else if ((((index << 1) + 1) < m_HeapSize) && (*m_Heap[(index << 1) + 1]->Elem))
		{
			if (*ins->Elem) 
			{
				big = (index << 1) + 1;
			} else {
				m_Heap[index] = m_Heap[(index << 1) + 1];
				index = (index << 1) + 1;
				m_Heap[index] = ins;
			}
		}

		if ((big > 0) && A_b4_B(m_Heap[big], ins))
		{
			m_Heap[index] = m_Heap[big];
			index = big;
			m_Heap[big] = ins;
		} else {
			big = 0;
		}
	}
}
