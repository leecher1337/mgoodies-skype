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
	
	TType ** m_Heap;
	unsigned int m_HeapSize;
	unsigned int m_AllocSize;
	TIterationType m_Type;
	bool m_DeleteItems;

	void ShiftDown();
	
private:

};


#define COMPARE_FUNC(a,b) (((m_Type == ITForward) && (*(a) < *(b))) || ((m_Type == ITBackward) && (*(a) > *(b))))

template <class TType>
CIterationHeap<TType>::CIterationHeap(TType & InitialItem, TIterationType MinMax, bool DeleteItems)
{
	m_Heap = new TType *[1];
	m_HeapSize = 1;
	m_AllocSize = 1 << 1;
	m_Type = ForBack;
	m_Heap[0] = &InitialItem;
	m_DeleteItems = DeleteItems;
}

template <class TType>
CIterationHeap<TType>::~CIterationHeap()
{
	if (m_DeleteItems)
	{
		int i = 0; 
		while ((i < m_HeapSize) && (m_Heap[i]))
		{
			delete m_Heap[i];
			++i;
		}
	}
	delete []m_Heap;
}

template <class TType>
bool CIterationHeap<TType>::Insert(TType & Item)
{
	if (m_AllocSize == m_HeapSize + 1)
	{
		TType ** backup = m_Heap;
		m_AllocSize = m_AllocSize << 1;
		m_Heap = new TType *[m_AllocSize - 1];
		memset(m_Heap, 0, sizeof(TType*) * (m_AllocSize - 1));
		memcpy(m_Heap, backup, m_HeapSize * sizeof(TType *));
		delete []backup;
	}

	m_HeapSize++;

	unsigned int way = m_AllocSize >> 2;	
	unsigned int index = 0;	
	TType * ins = &Item;
	TType * next;

	while ((way > 0) && (index + 1 < m_HeapSize))
	{
		next = m_Heap[index];
		if ((!(*next)) || COMPARE_FUNC(ins, next))
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
	return *m_Head[0];
}

template <class TType>
void CIterationHeap<TType>::Pop()
{
	if (m_Type == ITForward)
		++(*m_Heap[0]);
	else
		--(*m_Heap[0]);

	unsigned int index = 0;
	TType* ins = m_Heap[0];
	unsigned int big = 1;

	while ((big > 0) && (index < (m_HeapSize >> 1)))
	{
		big = 0;

		if ((((index << 1) + 2) < m_HeapSize) && (*m_Heap[(index << 1) + 2])) 
		{
			if (*ins)
			{
				if (COMPARE_FUNC(m_Heap[(index << 1) + 2], m_Heap[(index << 1) + 1]))
					big = (index << 1) + 2;					
				else 
					big = (index << 1) + 1;

			} else {				
				m_Heap[index] = m_Heap[(index << 1) + 2];
				index = (index << 1) + 2;
				m_Heap[index] = ins;
			}
		} else if ((((index << 1) + 1) < m_HeapSize) && (*m_Heap[(index << 1) + 1]))
		{
			if (*ins) 
			{
				big = (index << 1) + 1;
			} else {
				m_Heap[index] = m_Heap[(index << 1) + 1];
				index = (index << 1) + 1;
				m_Heap[index] = ins;
			}
		}

		if ((big > 0) && (COMPARE_FUNC(m_Heap[big], ins)))
		{
			m_Heap[index] = m_Heap[big];
			index = big;
			m_Heap[big] = ins;
		} else {
			big = 0;
		}
	}
}

#undef COMPARE_FUNC