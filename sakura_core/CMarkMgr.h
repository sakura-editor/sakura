// $Id$
//
//	CMarkMgr.h	���ݍs�̃}�[�N���Ǘ�����
//
//	Author: genta
//	Copyright (C) 1998-2000, genta
//
//
//	CMarkMgr : �������z�֐����܂ރN���X�D���ۂɂ̓T�u�N���X���g���D
//	

/*
Specification:

[�ʏ푀��i���ʁj]
	Add()�Œǉ��D�ꏊ�Ɩ��O��o�^�ł���D���삻�̂��̂̓J�X�^�}�C�Y�\�D
	[�ԍ�]�ŊY���ԍ��̗v�f���擾�ł���D

[��������]
	�ő�l�𒴂����ꍇ��protected�Ȋ֐��ŏ�������D�i�J�X�^�}�C�Y�\�j
	Add()�̏����̓T�u�N���X�ɔC����D

[���݈ʒu�̊Ǘ�]
	���݈ʒu��Manager���ŊǗ�����D

�폜����̓T�u�N���X�ɂ܂�����

*/

#ifndef __CMARKMGR_H_
#define __CMARKMGR_H_

#include <vector>
#include <string>
//#include "CStr.h"
//#include "CVectorBase.h"

using namespace std;

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
		string m_name;	//	�v�f��
		int	m_line;		//	�Y���s�ԍ�: �s�ԍ��͘_���s�Ő�����
		int m_pos;
		int m_extra;	//	���̑��̃f�[�^
	};

	// GENERATE_FACTORY(CMark,CMarkFactory);	//	CMark�pFactory class
	
	//	�^�錾
	typedef std::vector<CMark> CMarkChain;
	typedef std::vector<CMark>::iterator	CMarkIterator;

	//	Interface
	//	constructor
	CMarkMgr() : curpos(0), maxitem(10){}
	// CMarkMgr(const CDocLineMgr *p) : doc(p) {}
	
	int Count(void) const { return dat.size(); }	//	���ڐ���Ԃ�
	int GetMax(void) const { return maxitem; }	//	�ő區�ڐ���Ԃ�
	void SetMax(int max);	//	�ő區�ڐ���ݒ�

	virtual void Add(const CMark& m) = 0;	//	�v�f�̒ǉ�

	//	�v�f�̎擾
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

class CAutoMarkMgr : public CMarkMgr{
public:
	virtual void Add(const CMark& m);	//	�v�f�̒ǉ�
	virtual void Expire(void);	//	�v�f���̒���
};

#endif
