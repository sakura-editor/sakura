/*!	@file
	@brief �s�f�[�^�̊Ǘ�

	@author Norio Nakatani
	@date 1998/3/5  �V�K�쐬
	@date 2001/06/23 N.Nakatani WhereCurrentWord_2()�ǉ� static�����o
	@date 2001/12/03 hor ������(bookmark)�@�\�ǉ��ɔ����֐��ǉ�
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, hor, genta
	Copyright (C) 2002, aroka, MIK, hor
	Copyright (C) 2003, Moca, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDocLineMgr;

#ifndef _CDOCLINEMGR_H_
#define _CDOCLINEMGR_H_

#include <windows.h>
#include "global.h" // 2002/2/10 aroka
#include "CEol.h" // 2002/2/10 aroka
class CDocLine; // 2002/2/10 aroka
class CMemory; // 2002/2/10 aroka
class CBregexp; // 2002/2/10 aroka
#include "basis/SakuraBasis.h"
#include "charset/CCodeBase.h"

struct DocLineReplaceArg {
	CLogicRange		sDelRange;			//!< �폜�͈́B���W�b�N�P�ʁB
	CNativeW*		pcmemDeleted;		//!< �폜���ꂽ�f�[�^��ۑ� */
	CLogicInt		nDeletedLineNum;	//!< �폜�����s�̑���
	const wchar_t*	pInsData;			//!< �}������f�[�^
	int				nInsDataLen;		//!< �}������f�[�^�̒���
	CLogicInt		nInsLineNum;		//!< �}���ɂ���đ������s�̐�
	CLogicPoint		ptNewPos;			//!< �}�����ꂽ�����̎��̈ʒu
};

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
//2007.09.30 kobake WhereCurrentWord_2 �� CWordParse �Ɉړ�
class SAKURA_CORE_API CDocLineMgr
{
public:
	/*
	||  Constructors
	*/
	CDocLineMgr();
	~CDocLineMgr();

	/*
	||  �Q�ƌn
	*/
	wchar_t* GetAllData( int* );	/* �S�s�f�[�^��Ԃ� */
	CLogicInt GetLineCount( void ) { return m_nLines; }	/* �S�s����Ԃ� */
	const wchar_t* GetLineStr( CLogicInt , CLogicInt* );
	const wchar_t* GetLineStrWithoutEOL( CLogicInt , int* ); // 2003.06.22 Moca
	const wchar_t* GetFirstLinrStr( int* );	/* ���A�N�Z�X���[�h�F�擪�s�𓾂� */
	const wchar_t* GetNextLinrStr( int* );	/* ���A�N�Z�X���[�h�F���̍s�𓾂� */

	bool WhereCurrentWord( CLogicInt , CLogicInt , CLogicInt* , CLogicInt*, CNativeW*, CNativeW* );	/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */

	bool PrevOrNextWord( CLogicInt , CLogicInt , CLogicInt* , BOOL bLEFT, BOOL bStopsBothEnds );	/* ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ� */
	//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
	int SearchWord( CLogicPoint ptSerachBegin, const wchar_t* , ESearchDirection eDirection, const SSearchOption& sSearchOption , CLogicRange* pMatchRange, CBregexp* ); /* �P�ꌟ�� */
//	static char* SearchString( const unsigned char*, int, int , const unsigned char* , int, int*, int*, int ); /* �����񌟍� */
	static const wchar_t* SearchString( const wchar_t*, int, int , const wchar_t* , int, int*, bool ); /* �����񌟍� */
	static void CreateCharCharsArr( const wchar_t*, int, int** );	/* ���������̏�� */
//	static void CreateCharUsedArr( const unsigned char*, int, const int*, int** ); /* ���������̏��(�L�[������̎g�p�����\)�쐬 */



	void DUMP( void );
	void ResetAllModifyFlag( void );	/* �s�ύX��Ԃ����ׂă��Z�b�g */


// From Here 2001.12.03 hor
	void ResetAllBookMark( void );			/* �u�b�N�}�[�N�̑S���� */
	int SearchBookMark( CLogicInt nLineNum, ESearchDirection , CLogicInt* pnLineNum ); /* �u�b�N�}�[�N���� */
// To Here 2001.12.03 hor

	//@@@ 2002.05.25 MIK
	void ResetAllDiffMark( void );			/* �����\���̑S���� */
	int SearchDiffMark( CLogicInt , ESearchDirection , CLogicInt* ); /* �������� */
	void SetDiffMarkRange( int nMode, CLogicInt nStartLine, CLogicInt nEndLine );	/* �����͈͂̓o�^ */
	bool IsDiffUse( void ) const { return m_bIsDiffUse; }	/* DIFF�g�p�� */

// From Here 2002.01.16 hor
	void MarkSearchWord( const wchar_t* , const SSearchOption& , CBregexp* ); /* ���������ɊY������s�Ƀu�b�N�}�[�N���Z�b�g���� */
	void SetBookMarks( wchar_t* ); /* �����s�ԍ��̃��X�g����܂Ƃ߂čs�}�[�N */
	wchar_t* GetBookMarks( void ); /* �s�}�[�N����Ă镨���s�ԍ��̃��X�g����� */
// To Here 2001.01.16 hor

	/*
	|| �X�V�n
	*/
	void Init();
	void Empty();

	//	May 15, 2000 genta
	void AddLineStrX( const wchar_t*, int, CEOL );	/* �����ɍs��ǉ� Ver1.5 */

	void DeleteData_CDocLineMgr(
		CLogicInt	nLine,
		CLogicInt	nDelPos,
		CLogicInt	nDelLen,
		CLogicInt*	pnModLineOldFrom,	/* �e���̂������ύX�O�̍s(from) */
		CLogicInt*	pnModLineOldTo,		/* �e���̂������ύX�O�̍s(to) */
		CLogicInt*	pnDelLineOldFrom,	/* �폜���ꂽ�ύX�O�_���s(from) */
		CLogicInt*	pnDelLineOldNum,	/* �폜���ꂽ�s�� */
		CNativeW*	cmemDeleted			/* �폜���ꂽ�f�[�^ */
	);

	/* �w��͈͂̃f�[�^��u��(�폜 & �f�[�^��}��)
	  From���܂ވʒu����To�̒��O���܂ރf�[�^���폜����
	  From�̈ʒu�փe�L�X�g��}������
	*/
	void CDocLineMgr::ReplaceData( DocLineReplaceArg* );
	void DeleteNode( CDocLine* );/* �s�I�u�W�F�N�g�̍폜�A���X�g�ύX�A�s��-- */
	void InsertNode( CDocLine*, CDocLine* );	/* �s�I�u�W�F�N�g�̑}���A���X�g�ύX�A�s��++ */

	/* �f�[�^�̑}�� */
	void InsertData_CDocLineMgr(
		CLogicInt		nLine,
		CLogicInt		nInsPos,
		const wchar_t*	pInsData,
		CLogicInt		nInsDataLen,
		CLogicInt*		pnInsLineNum,	// �}���ɂ���đ������s�̐�
		CLogicPoint*	pptNewPos		// �}�����ꂽ�����̎��̈ʒu
	);

	//	Nov. 12, 2000 genta �����ǉ�
	//	Jul. 26, 2003 ryoji BOM�����ǉ�
	int ReadFile( const TCHAR* pszPath, HWND, HWND, ECodeType, FILETIME*, int extraflag, BOOL* pbBomExist = NULL );

	//	Feb. 6, 2001 genta �����ǉ�(���s�R�[�h�ݒ�)
	//	Jul. 26, 2003 ryoji BOM�����ǉ�
	EConvertResult WriteFile( const TCHAR*, HWND, HWND, ECodeType, FILETIME*, CEOL, BOOL bBomExist = FALSE );

	CDocLine* GetLineInfo( CLogicInt nLine );
	// 2002/2/10 aroka �����o�� private �ɂ��ăA�N�Z�T�ǉ�
	CDocLine* GetDocLineTop() const { return m_pDocLineTop; }
	CDocLine* GetDocLineBottom() const { return m_pDocLineBot; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �����⏕                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	// -- -- �`�F�[���֐� -- -- // 2007.10.11 kobake �쐬
	void _PushBottom(CDocLine* pDocLineNew);             //!< �ŉ����ɑ}��
	void _Insert(CDocLine* pDocLineNew, CDocLine* pPos); //!< pPos�̒��O�ɑ}��

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �����o�ϐ�                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	CDocLine*	m_pDocLineTop; //�ŏ��̍s
	CDocLine*	m_pDocLineBot; //�Ō�̍s(��1�s�����Ȃ��ꍇ��m_pDocLineTop�Ɠ������Ȃ�)
	CDocLine*	m_pDocLineCurrent;	/* ���A�N�Z�X���̌��݈ʒu */
	CLogicInt	m_nLines;		/* �S�s�� */
	CLogicInt	m_nPrevReferLine;
	CDocLine*	m_pCodePrevRefer;
	bool		m_bIsDiffUse;	/* DIFF�����\�����{�� */	//@@@ 2002.05.25 MIK
};


///////////////////////////////////////////////////////////////////////
#endif /* _CDOCLINEMGR_H_ */


/*[EOF]*/
