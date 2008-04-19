//
/*!	@file
	@brief ���ݍs�̃}�[�N���Ǘ�����

	@author genta
*/
/*
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka

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

#ifndef __CMARKMGR_H_
#define __CMARKMGR_H_

#pragma warning(disable: 4786)
#include <vector>
#include <string>

using namespace std;

/*!
	�s�}�[�N���Ǘ�����N���X�B
	�������z�֐����܂ނ̂ŁA���ۂɂ̓T�u�N���X������Ďg���B

	@par �ʏ푀��i���ʁj
	Add()�Œǉ��D�ꏊ�Ɩ��O��o�^�ł���D���삻�̂��̂̓J�X�^�}�C�Y�\�D
	[�ԍ�]�ŊY���ԍ��̗v�f���擾�ł���D

	@par ��������
	�ő�l�𒴂����ꍇ��protected�Ȋ֐��ŏ�������D�i�J�X�^�}�C�Y�\�j
	Add()�̏����̓T�u�N���X�ɔC����D

	@par ���݈ʒu�̊Ǘ�
	���݈ʒu��Manager���ŊǗ�����D

	�폜����̓T�u�N���X�ɂ܂�����

*/
class CMarkMgr {
public:

	//	���ڂ̃N���X
	class CMark {
	public:
		//	constructor
		CMark( const CLogicPoint& pt ) : m_ptLogic(pt) { }

		int GetLineNo(void) const { return m_ptLogic.y; }
		int GetPos(void) const { return m_ptLogic.x; }
		void SetPosition(const CLogicPoint& pt) { m_ptLogic=pt; }

		bool IsValid(void) const { return true; }

		bool operator==(CMark &r) const { return m_ptLogic.y == r.m_ptLogic.y; }
		bool operator!=(CMark &r) const { return m_ptLogic.y != r.m_ptLogic.y; }

	private:
		CLogicPoint m_ptLogic;
		/*
		int	m_line;		//!<	�Y���s�ԍ�: �s�ԍ��͘_���s�Ő�����
		int m_pos;		//!<	�Y�����ʒu
		*/
	};

	// GENERATE_FACTORY(CMark,CMarkFactory);	//	CMark�pFactory class

	//	�^�錾
	typedef std::vector<CMark> CMarkChain;
	typedef std::vector<CMark>::const_iterator	CMarkIterator;

	//	Interface
	//	constructor
	CMarkMgr() : m_nCurpos(0), m_nMaxitem(10){}
	// CMarkMgr(const CDocLineMgr *p) : doc(p) {}

	int Count(void) const { return (int)m_cMarkChain.size(); }	//!<	���ڐ���Ԃ�
	int GetMax(void) const { return m_nMaxitem; }	//!<	�ő區�ڐ���Ԃ�
	void SetMax(int max);	//!<	�ő區�ڐ���ݒ�

	virtual void Add(const CMark& m) = 0;	//!<	�v�f�̒ǉ�

	//	Apr. 1, 2001 genta
	virtual void Flush(void);	//!<	�v�f�̑S����

	//!	�v�f�̎擾
	const CMark& GetCurrent(void) const { return m_cMarkChain[m_nCurpos]; }

	//	�L�����̊m�F
	bool  CheckCurrent(void) const;
	bool  CheckPrev(void) const;
	bool  CheckNext(void) const;

	//	���݈ʒu�̈ړ�
	bool NextValid(void);
	bool PrevValid(void);

	const CMark& operator[](int index) const { return m_cMarkChain[index]; }

	//	�A���擾�C���^�[�t�F�[�X
//	CMarkIterator CurrentPos(void) const { return (CMarkIterator)m_cMarkChain.begin() + m_nCurpos; }
//	CMarkIterator Begin(void) const { return (CMarkIterator)m_cMarkChain.begin(); }
//	CMarkIterator End(void) const { return (CMarkIterator)m_cMarkChain.end(); }

protected:
	virtual void Expire(void) = 0;

	// CMarkFactory m_factory;	//	Factory Class (�}�N���Ő��������j
	CMarkChain m_cMarkChain;	//	�}�[�N�f�[�^�{��
	int m_nCurpos;	//	���݈ʒu�i�ԍ��j

	int m_nMaxitem;	//	�ۊǉ\�A�C�e���̍ő吔
private:
	//CMarkMgr( const CMarkMgr& );	//	Copy�֎~

};

// ----------------------------------------------------
/*!
	@brief �ړ������̊Ǘ��N���X

	CMarkMgr ���p�����A���삪�K�肳��Ă��Ȃ���������������B
*/
class CAutoMarkMgr : public CMarkMgr{
public:
	virtual void Add(const CMark& m);	//!<	�v�f�̒ǉ�
	virtual void Expire(void);	//!<	�v�f���̒���
};

#endif



