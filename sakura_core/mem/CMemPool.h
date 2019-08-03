/*
	Copyright (C) 2018-2019 Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#ifndef SAKURA_CMEMPOOL_H_
#define SAKURA_CMEMPOOL_H_

#include <memory>
#include "util/design_template.h"

template <typename T, size_t BlockSize = 4096>
class CMemPool final
{
public:
	DISALLOW_COPY_AND_ASSIGN(CMemPool);

	CMemPool()
	{
		AllocateBlock();
	}

	~CMemPool()
	{
		Node* curr = m_currentBlock;
		while (curr) {
			Node* next = curr->next;
			operator delete(reinterpret_cast<void*>(curr));
			curr = next;
		}
	}

	template <typename... Args>
	T* Construct(Args&&... args)
	{
		T* p = Allocate();
		new (p) T(std::forward<Args>(args)...);
		return p;
	}

	void Destruct(T* p)
	{
		if (p) {
			p->~T();
			Free(p);
		}
	}

private:
	union Node {
		~Node() {}
		T element;
		Node* next;
	};

	T* Allocate()
	{
		if (m_unassignedNode) {
			T* ret = reinterpret_cast<T*>(m_unassignedNode);
			m_unassignedNode = m_unassignedNode->next;
			return ret;
		}
		else {
			Node* border = reinterpret_cast<Node*>(reinterpret_cast<char*>(m_currentBlock) + BlockSize - sizeof(Node) + 1);
			if (m_currentNode >= border) {
				AllocateBlock();
			}
			return reinterpret_cast<T*>(m_currentNode++);
		}
	}

	void Free(T* p)
	{
		if (p) {
			Node* next = m_unassignedNode;
			m_unassignedNode = reinterpret_cast<Node*>(p);
			m_unassignedNode->next = next;
		}
	}

	void AllocateBlock()
	{
		char* buff = reinterpret_cast<char*>(operator new (BlockSize));
		Node* next = m_currentBlock;
		m_currentBlock = reinterpret_cast<Node*>(buff);
		m_currentBlock->next = next;

		void* body = buff + sizeof(Node*);
		size_t space = BlockSize - sizeof(Node*);
		body = std::align(alignof(Node), sizeof(Node), body, space);
		assert(body);
		m_currentNode = reinterpret_cast<Node*>(body);
	}

	Node* m_unassignedNode = nullptr;
	Node* m_currentBlock = nullptr;
	Node* m_currentNode = nullptr;
};

#endif /* SAKURA_CMEMPOOL_H_ */
