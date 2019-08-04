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
		Node* curr = m_currentBlock;
		while (curr) {
			Node* next = curr->next;
			operator delete(reinterpret_cast<void*>(curr));
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
	// 共用体を使う事で要素型と自己参照用のポインタを同じ領域に割り当てる
	// 共用体のサイズは各メンバを格納できるサイズになる事を利用する
	union Node {
		~Node() {}
		T element;	// 要素型
		Node* next; // ブロックのヘッダの場合は、次のブロックに繋がる
					// 解放後の未割当領域の場合は次の未割当領域に繋がる
	};
	
	// ブロックの大きさをNode2個分以上とする事で、最低限ブロック連結用のポインタとNode1つを記録出来る事を保証
	static_assert(BlockSize >= 2 * sizeof(Node), "BlockSize too small.");

	// 要素のメモリ確保処理、要素の領域のポインタを返す
	T* Allocate()
	{
		// メモリ確保時には未割当領域から使用していく
		if (m_unassignedNode) {
			T* ret = reinterpret_cast<T*>(m_unassignedNode);
			m_unassignedNode = m_unassignedNode->next;
			return ret;
		}
		else {
			// 未割当領域が無い場合は、ブロックの中から切り出す
			// 現在のブロックにNodeサイズ分の領域が無い場合は新規のブロックを確保
			Node* border = reinterpret_cast<Node*>(reinterpret_cast<char*>(m_currentBlock) + BlockSize - sizeof(Node) + 1);
			if (m_currentNode >= border) {
				AllocateBlock();
			}
			// 要素の領域のポインタを返すと同時にポインタを次に進めて切り出す位置を更新する
			return reinterpret_cast<T*>(m_currentNode++);
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
	// ブロックの先頭(head)にはブロックの連結用のポインタが配置され、残る領域（body）には要素が記録される
	void AllocateBlock()
	{
		char* buff = reinterpret_cast<char*>(operator new (BlockSize));
		Node* next = m_currentBlock;
		// ブロック領域の先頭（head）はNodeのポインタとして扱い、以前に作成したブロックに連結する
		m_currentBlock = reinterpret_cast<Node*>(buff);
		m_currentBlock->next = next;

		// ブロック領域の残る部分は要素の領域とするが、アライメントを取る
		void* body = buff + sizeof(Node*);
		size_t space = BlockSize - sizeof(Node*);
		body = std::align(alignof(Node), sizeof(Node), body, space);
		assert(body);
		m_currentNode = reinterpret_cast<Node*>(body);
	}

	Node* m_unassignedNode = nullptr; // 未割当領域の先頭
	Node* m_currentBlock = nullptr; // 現在のブロック
	Node* m_currentNode = nullptr; // 要素確保処理時に現在のブロックの中から切り出すNodeを指すポインタ、メモリ確保時に未割当領域が無い場合はここを使う
};

#endif /* SAKURA_CMEMPOOL_H_ */
