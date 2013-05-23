/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���Ǘ�

	@author Norio Nakatani
	@date 1998/03/06 �V�K�쐬
	@date 1998/04/14 �f�[�^�̍폜������
	@date 1999/12/20 �f�[�^�̒u��������
	@date 2009/08/28 nasukoji	CalTextWidthArg��`�ǉ��ADoLayout_Range()�̈����ύX
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, jepro
	Copyright (C) 2002, MIK, aroka, genta, YAZAKI
	Copyright (C) 2003, genta
	Copyright (C) 2005, Moca, genta, D.S.Koba
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CLAYOUTMGR_H_
#define _CLAYOUTMGR_H_

#include <Windows.h>// 2002/2/10 aroka
#include <vector>
#include "doc/CDocListener.h"
#include "_main/global.h"// 2002/2/10 aroka
#include "basis/SakuraBasis.h"
#include "types/CType.h"
#include "view/colors/EColorIndexType.h"
#include "util/container.h"

class CBregexp;// 2002/2/10 aroka
class CLayout;// 2002/2/10 aroka
class CDocLineMgr;// 2002/2/10 aroka
class CDocLine;// 2002/2/10 aroka
class CMemory;// 2002/2/10 aroka
class CEditDoc;// 2003/07/20 genta
class CColorStrategy;

struct LayoutReplaceArg {
	CLayoutRange	sDelRange;		//!< �폜�͈́B���C�A�E�g�P�ʁB
	CNativeW*		pcmemDeleted;	//!< �폜���ꂽ�f�[�^
	const wchar_t*	pInsData;		//!< �}������f�[�^
	CLogicInt		nInsDataLen;	//!< �}������f�[�^�̒���
	CLayoutInt		nAddLineNum;	//!< �ĕ`��q���g ���C�A�E�g�s�̑���
	CLayoutInt		nModLineFrom;	//!< �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��)
	CLayoutInt		nModLineTo;		//!< �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��)
	CLayoutPoint	ptLayoutNew;	//!< �}�����ꂽ�����̎��̈ʒu�̈ʒu(���C�A�E�g���ʒu, ���C�A�E�g�s)
};

// �ҏW���̃e�L�X�g�ő啝�Z�o�p		// 2009.08.28 nasukoji
struct CalTextWidthArg {
	CLayoutInt		nLineFrom;		//!< �ҏW�J�n�s
	CLayoutInt		nColumnFrom;	//!< �ҏW�J�n��
	CLayoutInt		nDelLines;		//!< �폜�Ɋ֌W����s�� - 1�i�����̎��폜�Ȃ��j
	CLayoutInt		nAllLinesOld;	//!< �ҏW�O�̃e�L�X�g�s��
	BOOL			bInsData;		//!< �ǉ������񂠂�
};

class CLogicPointEx: public CLogicPoint{
public:
	CLayoutInt ext;
};

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!	@brief �e�L�X�g�̃��C�A�E�g���Ǘ�

	@date 2005.11.21 Moca �F�������������o�[�ֈړ��D�s�v�ƂȂ��������������o�֐�����폜�D
*/
//2007.10.15 XYLogicalToLayout��p�~�BLogicToLayout�ɓ����B
class CLayoutMgr : public CProgressSubject
{
private:
	typedef CLayoutInt (CLayoutMgr::*CalcIndentProc)( CLayout* );

public:
	//�����Ɣj��
	CLayoutMgr();
	~CLayoutMgr();
	void Create( CEditDoc*, CDocLineMgr* );
	void Init();
	void _Empty();


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �R���t�B�O                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//! �^�u���̎擾
	CLayoutInt GetTabSpace() const { return m_sTypeConfig.m_nTabSpace; }


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                          �Q�ƌn                             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//2007.10.09 kobake �֐����ύX: Search �� SearchLineByLayoutY
	CLayoutInt		GetLineCount() const{ return m_nLines; }	/* �S�����s����Ԃ� */
	const wchar_t*	GetLineStr( CLayoutInt nLine, CLogicInt* pnLineLen ) const;	/* �w�肳�ꂽ�����s�̃f�[�^�ւ̃|�C���^�Ƃ��̒�����Ԃ� */
	const wchar_t*	GetLineStr( CLayoutInt nLine, CLogicInt* pnLineLen, const CLayout** ppcLayoutDes ) const;	/* �w�肳�ꂽ�����s�̃f�[�^�ւ̃|�C���^�Ƃ��̒�����Ԃ� */

	//�擪�Ɩ���
	CLayout*		GetTopLayout()		{ return m_pLayoutTop; }
	CLayout*		GetBottomLayout()	{ return m_pLayoutBot; }
	const CLayout*	GetTopLayout() const { return m_pLayoutTop; }
	const CLayout*	GetBottomLayout() const { return m_pLayoutBot; }

	//���C�A�E�g��T��
	const CLayout*	SearchLineByLayoutY( CLayoutInt nLineLayout ) const;	/* �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(CLayout)�ւ̃|�C���^��Ԃ� */
	CLayout*		SearchLineByLayoutY( CLayoutInt nLineLayout ){ return const_cast<CLayout*>(static_cast<const CLayoutMgr*>(this)->SearchLineByLayoutY(nLineLayout)); }

	//���[�h��T��
	bool			WhereCurrentWord( CLayoutInt , CLogicInt , CLayoutRange* pSelect, CNativeW*, CNativeW* );	/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */

	//����
	bool			IsEndOfLine( const CLayoutPoint& ptLinePos );	/* �w��ʒu���s��(���s�����̒��O)�����ׂ� */	//@@@ 2002.04.18 MIK

	/*! ����TAB�ʒu�܂ł̕�
		@param pos [in] ���݂̈ʒu
		@return ����TAB�ʒu�܂ł̕������D1�`TAB��
	 */
	CLayoutInt GetActualTabSpace(CLayoutInt pos) const { return m_sTypeConfig.m_nTabSpace - pos % m_sTypeConfig.m_nTabSpace; }

	//	Aug. 14, 2005 genta
	// Sep. 07, 2007 kobake �֐����ύX GetMaxLineSize��GetMaxLineKetas
	CLayoutInt GetMaxLineKetas(void) const { return m_sTypeConfig.m_nMaxLineKetas; }

	// 2005.11.21 Moca ���p���̐F���������������珜��
	bool ChangeLayoutParam( CLayoutInt nTabSize, CLayoutInt nMaxLineKetas );

	// Jul. 29, 2006 genta
	void GetEndLayoutPos(CLayoutPoint* ptLayoutEnd);

	CLayoutInt GetMaxTextWidth(void) const { return m_nTextWidth; }		// 2009.08.28 nasukoji	�e�L�X�g�ő啝��Ԃ�


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ����                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	int PrevOrNextWord( CLayoutInt, CLogicInt, CLayoutPoint* pptLayoutNew, BOOL, BOOL bStopsBothEnds );	/* ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ� */
public:
	int PrevWord( CLayoutInt nLineNum, CLogicInt nIdx, CLayoutPoint* pptLayoutNew, BOOL bStopsBothEnds ){ return PrevOrNextWord(nLineNum, nIdx, pptLayoutNew, TRUE, bStopsBothEnds); }	/* ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ� */
	int NextWord( CLayoutInt nLineNum, CLogicInt nIdx, CLayoutPoint* pptLayoutNew, BOOL bStopsBothEnds ){ return PrevOrNextWord(nLineNum, nIdx, pptLayoutNew, FALSE, bStopsBothEnds); }	/* ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ� */

	int SearchWord( CLayoutInt nLine, CLogicInt nIdx, const wchar_t* , ESearchDirection eSearchDirection, const SSearchOption& sSearchOption, CLayoutRange* pMatchRange, CBregexp* );	/* �P�ꌟ�� */


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �P�ʂ̕ϊ�                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//!���W�b�N�����C�A�E�g
	void LogicToLayoutEx( const CLogicPointEx& ptLogicEx, CLayoutPoint* pptLayout, CLayoutInt nLineHint = CLayoutInt(0) )
	{
		LogicToLayout( ptLogicEx, pptLayout, nLineHint );
		pptLayout->x += ptLogicEx.ext;
	}
	void LogicToLayout( const CLogicPoint& ptLogic, CLayoutPoint* pptLayout, CLayoutInt nLineHint = CLayoutInt(0) );
	void LogicToLayout( const CLogicRange& rangeLogic, CLayoutRange* prangeLayout )
	{
		LogicToLayout(rangeLogic.GetFrom(), prangeLayout->GetFromPointer());
		LogicToLayout(rangeLogic.GetTo(), prangeLayout->GetToPointer());
	}

	//!���C�A�E�g�����W�b�N�ϊ�
	void LayoutToLogicEx( const CLayoutPoint& ptLayout, CLogicPointEx* pptLogicEx ) const;
	void LayoutToLogic( const CLayoutPoint& ptLayout, CLogicPoint* pptLogic ) const;
	void LayoutToLogic( const CLayoutRange& rangeLayout, CLogicRange* prangeLogic ) const
	{
		LayoutToLogic(rangeLayout.GetFrom(), prangeLogic->GetFromPointer());
		LayoutToLogic(rangeLayout.GetTo(), prangeLogic->GetToPointer());
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �f�o�b�O                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void DUMP();	/* �e�X�g�p�Ƀ��C�A�E�g�����_���v */


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �ҏW�Ƃ�                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	/*
	|| �X�V�n
	*/
	/* ���C�A�E�g���̕ύX
		@date Jun. 01, 2001 JEPRO char* (�s�R�����g�f���~�^3�p)��1�ǉ�
		@date 2002.04.13 MIK �֑�,���s�������Ԃ牺����,��Ǔ_�Ԃ炳����ǉ�
		@date 2002/04/27 YAZAKI STypeConfig��n���悤�ɕύX�B
	*/
	void SetLayoutInfo(
		bool			bDoRayout,
		const STypeConfig&	refType
	);
	
	/* �s�������폜 */
	// 2005.11.21 Moca ���p���̐F���������������珜��
	void DeleteData_CLayoutMgr(
		CLayoutInt	nLineNum,
		CLogicInt	nDelPos,
		CLogicInt	nDelLen,
		CLayoutInt*	pnModifyLayoutLinesOld,
		CLayoutInt*	pnModifyLayoutLinesNew,
		CLayoutInt*	pnDeleteLayoutLines,
		CNativeW*	cmemDeleted			/* �폜���ꂽ�f�[�^ */
	);

	/* ������}�� */
	// 2005.11.21 Moca ���p���̐F���������������珜��
	void InsertData_CLayoutMgr(
		CLayoutInt		nLineNum,
		CLogicInt		nInsPos,
		const wchar_t*	pInsData,
		CLogicInt		nInsDataLen,
		CLayoutInt*		pnModifyLayoutLinesOld,
		CLayoutInt*		pnInsLineNum,		/* �}���ɂ���đ��������C�A�E�g�s�̐� */
		CLayoutPoint*	pptNewLayout		// �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu
	);

	/* ������u�� */
	void ReplaceData_CLayoutMgr(
		LayoutReplaceArg*	pArg
	);

	BOOL CalculateTextWidth( BOOL bCalLineLen = TRUE, CLayoutInt nStart = CLayoutInt(-1), CLayoutInt nEnd = CLayoutInt(-1) );	/* �e�L�X�g�ő啝���Z�o���� */		// 2009.08.28 nasukoji
	void ClearLayoutLineWidth( void );				/* �e�s�̃��C�A�E�g�s���̋L�����N���A���� */		// 2009.08.28 nasukoji



protected:
	/*
	||  �Q�ƌn
	*/
	const char* GetFirstLinrStr( int* );	/* ���A�N�Z�X���[�h�F�擪�s�𓾂� */
	const char* GetNextLinrStr( int* );	/* ���A�N�Z�X���[�h�F���̍s�𓾂� */


	/*
	|| �X�V�n
	*/
	// 2005.11.21 Moca ���p���̐F���������������珜��
	void _DoLayout();	/* ���݂̐܂�Ԃ��������ɍ��킹�đS�f�[�^�̃��C�A�E�g�����Đ������܂� */
	// 2005.11.21 Moca ���p���̐F���������������珜��
	// 2009.08.28 nasukoji	�e�L�X�g�ő啝�Z�o�p�����ǉ�
	CLayoutInt DoLayout_Range( CLayout* , CLogicInt, CLogicPoint, EColorIndexType, const CalTextWidthArg*, CLayoutInt* );	/* �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g���� */
	void CalculateTextWidth_Range( const CalTextWidthArg* pctwArg );	/* �e�L�X�g���ҏW���ꂽ��ő啝���Z�o���� */	// 2009.08.28 nasukoji
	CLayout* DeleteLayoutAsLogical( CLayout*, CLayoutInt, CLogicInt , CLogicInt, CLogicPoint, CLayoutInt* );	/* �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜 */
	void ShiftLogicalLineNum( CLayout* , CLogicInt );	/* �w��s����̍s�̃��C�A�E�g���ɂ��āA�_���s�ԍ����w��s�������V�t�g���� */

	//���i
	struct SLayoutWork{
		//�����[�v������
		int				nKinsokuType;
		CLogicInt		nPos;
		CLogicInt		nBgn;
		CStringRef		cLineStr;
		CLogicInt		nWordBgn;
		CLogicInt		nWordLen;
		CLayoutInt		nPosX;
		CLayoutInt		nIndent;
		CLayout*		pLayoutCalculated;

		//���[�v�O
		CDocLine*		pcDocLine;
		CLayout*		pLayout;
		CColorStrategy*	pcColorStrategy;
		CColorStrategy*	pcColorStrategy_Prev;
		CLogicInt		nCurLine;

		//���[�v�O (DoLayout�̂�)
//		CLogicInt		nLineNum;

		//���[�v�O (DoLayout_Range�̂�)
		bool			bNeedChangeCOMMENTMODE;
		CLayoutInt		nModifyLayoutLinesNew;
		
		//���[�v�O (DoLayout_Range����)
		CLayoutInt*		pnExtInsLineNum;
		CLogicPoint		ptDelLogicalFrom;

		//�֐�
		CLayout* _CreateLayout(CLayoutMgr* mgr);
	};
	//�֐��|�C���^
	typedef void (CLayoutMgr::*PF_OnLine)(SLayoutWork*);
	//DoLayout�p
	bool _DoKinsokuSkip(SLayoutWork* pWork, PF_OnLine pfOnLine);
	void _DoWordWrap(SLayoutWork* pWork, PF_OnLine pfOnLine);
	void _DoKutoBurasage(SLayoutWork* pWork);
	void _DoGyotoKinsoku(SLayoutWork* pWork, PF_OnLine pfOnLine);
	void _DoGyomatsuKinsoku(SLayoutWork* pWork, PF_OnLine pfOnLine);
	bool _DoTab(SLayoutWork* pWork, PF_OnLine pfOnLine);
	void _MakeOneLine(SLayoutWork* pWork, PF_OnLine pfOnLine);
	//DoLayout�p�R�A
	void _OnLine1(SLayoutWork* pWork);
	//DoLayout_Range�p�R�A
	void _OnLine2(SLayoutWork* pWork);


private:
	bool _ExistKinsokuKuto(wchar_t wc) const{ return m_pszKinsokuKuto_1.exist(wc); }
	bool _ExistKinsokuHead(wchar_t wc) const{ return m_pszKinsokuHead_1.exist(wc); }
	bool IsKinsokuHead( wchar_t wc );	/*!< �s���֑��������`�F�b�N���� */	//@@@ 2002.04.08 MIK
	bool IsKinsokuTail( wchar_t wc );	/*!< �s���֑��������`�F�b�N���� */	//@@@ 2002.04.08 MIK
	bool IsKinsokuKuto( wchar_t wc );	/*!< ��Ǔ_�������`�F�b�N���� */	//@@@ 2002.04.17 MIK
	//	2005-08-20 D.S.Koba �֑��֘A�����̊֐���
	/*! ��Ǔ_�Ԃ牺���̏����ʒu��
		@date 2005-08-20 D.S.Koba
		@date Sep. 3, 2005 genta �œK��
	*/
	bool IsKinsokuPosKuto(CLayoutInt nRest, CLayoutInt nCharChars ) const {
		return nRest < nCharChars;
	}
	bool IsKinsokuPosHead(CLayoutInt, CLayoutInt, CLayoutInt);	//!< �s���֑��̏����ʒu��
	bool IsKinsokuPosTail(CLayoutInt, CLayoutInt, CLayoutInt);	//!< �s���֑��̏����ʒu��
public:
	int Match_Quote( wchar_t wcQuote, int nPos, const CStringRef& cLineStr ) const;
private:
	//	Oct. 1, 2002 genta �C���f���g���v�Z�֐��Q
	CLayoutInt getIndentOffset_Normal( CLayout* pLayoutPrev );
	CLayoutInt getIndentOffset_Tx2x( CLayout* pLayoutPrev );
	CLayoutInt getIndentOffset_LeftSpace( CLayout* pLayoutPrev );

protected:
	/*
	|| �����w���p�n
	*/
	//@@@ 2002.09.23 YAZAKI
	// 2009.08.28 nasukoji	nPosX�����ǉ�
	CLayout* CreateLayout( CDocLine* pCDocLine, CLogicPoint ptLogicPos, CLogicInt nLength, EColorIndexType nTypePrev, CLayoutInt nIndent, CLayoutInt nPosX );
	CLayout* InsertLineNext( CLayout*, CLayout* );
	void AddLineBottom( CLayout* );

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �����o�ϐ�                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	CDocLineMgr*			m_pcDocLineMgr;	/* �s�o�b�t�@�Ǘ��}�l�[�W�� */

protected:
	// 2002.10.07 YAZAKI add m_nLineTypeBot
	// 2007.09.07 kobake �ϐ����ύX: m_nMaxLineSize��m_nMaxLineKetas
	// 2007.10.08 kobake �ϐ����ύX: getIndentOffset��m_getIndentOffset

	//�Q��
	CEditDoc*		m_pcEditDoc;

	//���f�[�^
	CLayout*				m_pLayoutTop;
	CLayout*				m_pLayoutBot;

	//�^�C�v�ʐݒ�
	STypeConfig				m_sTypeConfig;
	vector_ex<wchar_t>		m_pszKinsokuHead_1;			// �s���֑�����	//@@@ 2002.04.08 MIK
	vector_ex<wchar_t>		m_pszKinsokuTail_1;			// �s���֑�����	//@@@ 2002.04.08 MIK
	vector_ex<wchar_t>		m_pszKinsokuKuto_1;			// ��Ǔ_�Ԃ炳������	//@@@ 2002.04.17 MIK
	CalcIndentProc			m_getIndentOffset;			//	Oct. 1, 2002 genta �C���f���g���v�Z�֐���ێ�

	//�t���O��
	EColorIndexType			m_nLineTypeBot;				//!< �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V����������
	CLayoutInt				m_nLines;					// �S���C�A�E�g�s��

	mutable CLayoutInt		m_nPrevReferLine;
	mutable CLayout*		m_pLayoutPrevRefer;
	
	// EOF�J�[�\���ʒu���L������(_DoLayout/DoLayout_Range�Ŗ����ɂ���)	//2006.10.01 Moca
	CLayoutInt				m_nEOFLine; //!< EOF�s��
	CLayoutInt				m_nEOFColumn; //!< EOF���ʒu

	// �e�L�X�g�ő啝���L���i�܂�Ԃ��ʒu�Z�o�Ɏg�p�j	// 2009.08.28 nasukoji
	CLayoutInt				m_nTextWidth;				// �e�L�X�g�ő啝�̋L��
	CLayoutInt				m_nTextWidthMaxLine;		// �ő啝�̃��C�A�E�g�s
};


///////////////////////////////////////////////////////////////////////
#endif /* _CLAYOUTMGR_H_ */



