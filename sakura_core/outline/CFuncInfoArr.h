/*!	@file
	@brief �A�E�g���C����� �f�[�^�z��

	@author Norio Nakatani
	@date	1998/06/23 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CFUNCINFOARR_H_
#define _CFUNCINFOARR_H_

class CFuncInfo;
#include <string>
#include <map>

// �W���I�ȕt�����萔
#define FL_OBJ_DEFINITION	0	// �e�N���X�̒�`�ʒu
#define FL_OBJ_DECLARE		1	// �֐��v���g�^�C�v�錾
#define FL_OBJ_FUNCTION		2	// �֐�
#define FL_OBJ_CLASS		3	// �N���X
#define FL_OBJ_STRUCT		4	// �\����
#define FL_OBJ_ENUM			5	// �񋓑�
#define FL_OBJ_UNION		6	// ���p��
#define FL_OBJ_NAMESPACE	7	// ���O���
#define FL_OBJ_INTERFACE	8	// �C���^�t�F�[�X
#define FL_OBJ_GLOBAL		9	// �O���[�o���i�g�ݍ��݉�͂ł͎g�p���Ȃ��j
#define FL_OBJ_ELEMENT_MAX	30	// �v���O�C���Œǉ��\�Ȓ萔�̏��

//! �A�E�g���C����� �f�[�^�z��
class CFuncInfoArr {
public:
	CFuncInfoArr();	/* CFuncInfoArr�N���X�\�z */
	~CFuncInfoArr();	/* CFuncInfoArr�N���X���� */
	CFuncInfo* GetAt( int );	/* 0<=�̎w��ԍ��̃f�[�^��Ԃ� */
	void AppendData( CFuncInfo* );	/* �z��̍Ō�Ƀf�[�^��ǉ����� */
	void AppendData( CLogicInt, CLayoutInt, const TCHAR*, int, int nDepth = 0 );	/* �z��̍Ō�Ƀf�[�^��ǉ����� 2002.04.01 YAZAKI �[������*/
	void AppendData( CLogicInt, CLayoutInt, const NOT_TCHAR*, int, int nDepth = 0 );	/* �z��̍Ō�Ƀf�[�^��ǉ����� 2002.04.01 YAZAKI �[������*/
	void AppendData( CLogicInt nLogicLine, CLogicInt nLogicCol, CLayoutInt nLayoutLine, CLayoutInt nLayoutCol, const TCHAR*, int, int nDepth = 0 );	/* �z��̍Ō�Ƀf�[�^��ǉ����� 2010.03.01 syat ������*/
	void AppendData( CLogicInt nLogicLine, CLogicInt nLogicCol, CLayoutInt nLayoutLine, CLayoutInt nLayoutCol, const NOT_TCHAR*, int, int nDepth = 0 );	/* �z��̍Ō�Ƀf�[�^��ǉ����� 2010.03.01 syat ������*/
	int	GetNum( void ){	return m_nFuncInfoArrNum; }	/* �z��v�f����Ԃ� */
	void Empty( void );
	void DUMP( void );
	void SetAppendText( int info, std::wstring s, bool overwrite );
	std::wstring GetAppendText( int info );
	int AppendTextLenMax(){ return m_nAppendTextLenMax; }



public:
	SFilePath	m_szFilePath;	/*!< ��͑Ώۃt�@�C���� */
private:
	int			m_nFuncInfoArrNum;	/*!< �z��v�f�� */
	CFuncInfo**	m_ppcFuncInfoArr;	/*!< �z�� */
	std::map<int, std::wstring>	m_AppendTextArr;	// �ǉ�������̃��X�g
	int			m_nAppendTextLenMax;
};



///////////////////////////////////////////////////////////////////////
#endif /* _CFUNCINFOARR_H_ */



