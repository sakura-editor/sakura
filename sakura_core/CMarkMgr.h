// $Id$
//
/*!	@file
	@brief ���ݍs�̃}�[�N���Ǘ�����

	@author genta
	$Revision$

*/
/*
	Copyright (C) 2000-2001, genta
	
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __CMARKMGR_H_
#define __CMARKMGR_H_

#include <vector>
#include <string>
//#include "CStr.h"
//#include "CVectorBase.h"

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
		CMark( int x, int y ) : m_pos(x), m_line(y), m_extra(0) {}
		CMark( void ) : m_extra(-1) {}
		
		//	type converter
		const char *GetNameStr(void) const { return m_name.c_str(); }
		void SetNameStr(const char* newstr) { m_name = newstr; }
		
		int GetLine(void) const { return m_line; }
		int GetPos(void) const { return m_pos; }
		void SetPosition(int x, int y) { m_pos = x; m_line = y; }

		int GetExtra(void) const { return m_extra; }
		void SetExtra(int l) { m_extra = l; }
		
		bool IsValid(void) const { return m_extra != -1; }
		void Invalidate(void){ m_extra = -1; }
		
		bool operator==(CMark &r) const { return m_line == r.m_line; }
		bool operator!=(CMark &r) const { return m_line != r.m_line; }

	private:
		string m_name;	//!<	�v�f��
		int	m_line;		//!<	�Y���s�ԍ�: �s�ԍ��͘_���s�Ő�����
		int m_pos;		//!<	�Y�����ʒu
		int m_extra;	//!<	�T�u�N���X�Ŏg����\���̈�B
	};

	// GENERATE_FACTORY(CMark,CMarkFactory);	//	CMark�pFactory class
	
	//	�^�錾
	typedef std::vector<CMark> CMarkChain;
	typedef std::vector<CMark>::iterator	CMarkIterator;

	//	Interface
	//	constructor
	CMarkMgr() : curpos(0), maxitem(10){}
	// CMarkMgr(const CDocLineMgr *p) : doc(p) {}
	
	int Count(void) const { return dat.size(); }	//!<	���ڐ���Ԃ�
	int GetMax(void) const { return maxitem; }	//!<	�ő區�ڐ���Ԃ�
	void SetMax(int max);	//!<	�ő區�ڐ���ݒ�

	virtual void Add(const CMark& m) = 0;	//!<	�v�f�̒ǉ�
	
	//	Apr. 1, 2001 genta
	virtual void Flush(void);	//!<	�v�f�̑S����

	//!	�v�f�̎擾
	const CMark& GetCurrent(void) const { return dat[curpos]; }
	
	//	�L�����̊m�F
	bool  CheckCurrent(void) const;
	bool  CheckPrev(void) const;
	bool  CheckNext(void) const;
	
	//	���݈ʒu�̈ړ�
	bool NextValid(void);
	bool PrevValid(void);

	const CMark& operator[](int index) const { return dat[index]; }
	
	//	�A���擾�C���^�[�t�F�[�X
	CMarkIterator CurrentPos(void) const { return (CMarkIterator)dat.begin() + curpos; }
	CMarkIterator Begin(void) const { return (CMarkIterator)dat.begin(); }
	CMarkIterator End(void) const { return (CMarkIterator)dat.end(); }

protected:
	virtual void Expire(void) = 0;

	// CMarkFactory m_factory;	//	Factory Class (�}�N���Ő��������j
	CMarkChain dat;	//	�}�[�N�f�[�^�{��
	int curpos;	//	���݈ʒu�i�ԍ��j
	
	int maxitem;	//	�ۊǉ\�A�C�e���̍ő吔
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
