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

// T : �v�f�^
// BlockSize : �u���b�N�̑傫���̃o�C�g��
template <typename T, size_t BlockSize = 4096>
class CMemPool final
{
public:
	DISALLOW_COPY_AND_ASSIGN(CMemPool);

	CMemPool()
	{
		// �n�߂̃u���b�N���������m��
		AllocateBlock();
	}

	~CMemPool()
	{
		// �������m�ۂ����̈�̘A�����X�g��H���đS�Ẵu���b�N���̃��������
		Node* curr = m_currentBlock;
		while (curr) {
			Node* next = curr->next;
			operator delete(reinterpret_cast<void*>(curr));
			curr = next;
		}
	}

	// �v�f�\�z�A�����͗v�f�^�̃R���X�g���N�^����
	template <typename... Args>
	T* Construct(Args&&... args)
	{
		T* p = Allocate();
		new (p) T(std::forward<Args>(args)...);
		return p;
	}
	
	// �v�f�j���A������ Construct ���Ԃ����|�C���^
	void Destruct(T* p)
	{
		if (p) {
			p->~T();
			Free(p);
		}
	}

private:
	// ���p�̂��g�����ŗv�f�^�Ǝ��ȎQ�Ɨp�̃|�C���^�𓯂��̈�Ɋ��蓖�Ă�
	// ���p�̂̃T�C�Y�͊e�����o���i�[�ł���T�C�Y�ɂȂ鎖�𗘗p����
	union Node {
		~Node() {}
		T element;	// �v�f�^
		Node* next; // �u���b�N�̃w�b�_�̏ꍇ�́A���̃u���b�N�Ɍq����
					// �����̖������̈�̏ꍇ�͎��̖������̈�Ɍq����
	};
	
	// �u���b�N�̑傫���͍Œ� Node 2���ȏ�Ƃ���
	static_assert(BlockSize >= 2 * sizeof(Node), "BlockSize too small.");

	// �v�f�̃������m�ۏ����A�v�f�̗̈�̃|�C���^��Ԃ�
	T* Allocate()
	{
		// �������m�ێ��ɂ͖������̈悩��g�p���Ă���
		if (m_unassignedNode) {
			T* ret = reinterpret_cast<T*>(m_unassignedNode);
			m_unassignedNode = m_unassignedNode->next;
			return ret;
		}
		else {
			// �������̈悪�����ꍇ�́A�u���b�N�̒�����؂�o��
			// ���݂̃u���b�N��Node�T�C�Y���̗̈悪�����ꍇ�͐V�K�̃u���b�N���m��
			Node* border = reinterpret_cast<Node*>(reinterpret_cast<char*>(m_currentBlock) + BlockSize - sizeof(Node) + 1);
			if (m_currentNode >= border) {
				AllocateBlock();
			}
			// �v�f�̗̈�̃|�C���^��Ԃ��Ɠ����Ƀ|�C���^�����ɐi�߂Đ؂�o���ʒu���X�V����
			return reinterpret_cast<T*>(m_currentNode++);
		}
	}

	// ��������������A�v�f�̗̈�̃|�C���^���󂯎��
	// Allocate ���\�b�h�ŕԂ����|�C���^��n����
	void Free(T* p)
	{
		if (p) {
			// ��������������̈�𖢊����̈�Ƃ��Ď��ȎQ�Ƌ��p�̂̕Е����A�����X�g�Ōq����
			// ����̃������m�ێ��ɂ��̗̈���ė��p����
			Node* next = m_unassignedNode;
			m_unassignedNode = reinterpret_cast<Node*>(p);
			m_unassignedNode->next = next;
		}
	}

	// �Ăяo���̓x�Ƀ������̓��I�m�ۂ��ׂ����s�����������ׂɁA�ꊇ�Ńu���b�N�̈���m��
	// �u���b�N�̐擪(head)�ɂ̓u���b�N�̘A���p�̃|�C���^���z�u����A�c��̈�ibody�j�ɂ͗v�f���L�^�����
	void AllocateBlock()
	{
		char* buff = reinterpret_cast<char*>(operator new (BlockSize));
		Node* next = m_currentBlock;
		// �u���b�N�̈�̐擪�ihead�j��Node�̃|�C���^�Ƃ��Ĉ����A�ȑO�ɍ쐬�����u���b�N�ɘA������
		m_currentBlock = reinterpret_cast<Node*>(buff);
		m_currentBlock->next = next;

		// �u���b�N�̈�̎c�镔���͗v�f�̗̈�Ƃ��邪�A�A���C�����g�����
		void* body = buff + sizeof(Node*);
		size_t space = BlockSize - sizeof(Node*);
		body = std::align(alignof(Node), sizeof(Node), body, space);
		assert(body);
		m_currentNode = reinterpret_cast<Node*>(body);
	}

	Node* m_unassignedNode = nullptr; // �������̈�̐擪
	Node* m_currentBlock = nullptr; // ���݂̃u���b�N
	Node* m_currentNode = nullptr; // �v�f�m�ۏ������Ɍ��݂̃u���b�N�̒�����؂�o��Node���w���|�C���^�A�������m�ێ��ɖ������̈悪�����ꍇ�͂������g��
};

#endif /* SAKURA_CMEMPOOL_H_ */
