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

// T : 要素型
// BlockSize : ブロックの大きさのバイト数
template <typename T, size_t BlockSize = 4096>
class CMemPool final
{
public:
	DISALLOW_COPY_AND_ASSIGN(CMemPool);

	CMemPool()
	{
		// 始めのブロックをメモリ確保
		AllocateBlock();
	}

	~CMemPool()
	{
		// メモリ確保した領域の連結リストを辿って全てのブロック分のメモリ解放
		Block* curr = m_currentBlock;
		while (curr) {
			Block* next = curr->next;
			operator delete(curr);
			curr = next;
		}
	}

	// 要素構築、引数は要素型のコンストラクタ引数
	template <typename... Args>
	T* Construct(Args&&... args)
	{
		T* p = Allocate();
		new (p) T(std::forward<Args>(args)...);
		return p;
	}
	
	// 要素破棄、引数は Construct が返したポインタ
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
		T element;	// 要素型
		Node* next; // 解放後の未割当領域の場合は次の未割当領域に繋がる
	};

	union Block {
		~Block() {}
		struct {
			Block* next;
			Node nodes[1];
		};
		uint8_t padding[BlockSize];
	};
	
	// 要素のメモリ確保処理、要素の領域のポインタを返す
	T* Allocate()
	{
		// メモリ確保時には未割当領域から使用していく
		if (m_unassignedNode) {
			T* ret = &m_unassignedNode->element;
			m_unassignedNode = m_unassignedNode->next;
			return ret;
		}
		else {
			// 未割当領域が無い場合は、ブロックの中のNode領域を使用する
			if (reinterpret_cast<void*>(m_currentNode + 1) >= reinterpret_cast<void*>(m_currentBlock + 1)) {
				// 現在のブロックに新規に割り当てるNode分の領域が残っていない場合は新規にブロックを確保
				AllocateBlock();
			}
			T* ret = &m_currentNode->element;
			++m_currentNode;
			return ret;
		}
	}

	// メモリ解放処理、要素の領域のポインタを受け取る
	// Allocate メソッドで返したポインタを渡す事
	void Free(T* p)
	{
		if (p) {
			// メモリ解放した領域を未割当領域として自己参照共用体の片方向連結リストで繋げる
			// 次回のメモリ確保時にその領域を再利用する
			Node* next = m_unassignedNode;
			m_unassignedNode = reinterpret_cast<Node*>(p);
			m_unassignedNode->next = next;
		}
	}

	// 呼び出しの度にメモリの動的確保を細かく行う事を避ける為に、一括でブロック領域を確保
	void AllocateBlock()
	{
		Block* next = m_currentBlock;
		// 以前に作成したブロックに連結する
		m_currentBlock = reinterpret_cast<Block*>(operator new(sizeof(Block)));
		m_currentBlock->next = next;

		// 新規に作成したブロックの先頭のNodeから使用する
		m_currentNode = &m_currentBlock->nodes[0];
	}

	Block* m_currentBlock = nullptr; // 現在のブロック
	Node* m_unassignedNode = nullptr; // 未割当領域の先頭
	Node* m_currentNode = nullptr; // 現在のブロックの中のNodeを指すポインタ、メモリ確保時に未割当領域が無い場合はここを使う
};

#endif /* SAKURA_CMEMPOOL_H_ */
