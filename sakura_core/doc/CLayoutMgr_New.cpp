/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���Ǘ�

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK
	Copyright (C) 2002, MIK, aroka, genta, YAZAKI, novice
	Copyright (C) 2003, genta
	Copyright (C) 2004, Moca, genta
	Copyright (C) 2005, D.S.Koba, Moca
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include <mbstring.h>
#include "CLayoutMgr.h"
#include "charset/charcode.h"
#include "debug/Debug.h"
#include <commctrl.h>
#include "debug/CRunningTimer.h"
#include "doc/CLayout.h"/// 2002/2/10 aroka
#include "doc/CDocLine.h"/// 2002/2/10 aroka
#include "doc/CDocLineMgr.h"// 2002/2/10 aroka
#include "mem/CMemory.h"/// 2002/2/10 aroka
#include "mem/CMemoryIterator.h"
#include "doc/CEditDoc.h" /// 2003/07/20 genta
#include "util/window.h"
#include "view/colors/CColorStrategy.h"




/*!
	�s���֑������ɊY�����邩�𒲂ׂ�D

	@param[in] pLine ���ׂ镶���ւ̃|�C���^
	@param[in] length ���Y�ӏ��̕����T�C�Y
	@retval true �֑������ɊY��
	@retval false �֑������ɊY�����Ȃ�
*/
bool CLayoutMgr::IsKinsokuHead( wchar_t wc )
{
	return m_pszKinsokuHead_1.exist(wc);
}

/*!
	�s���֑������ɊY�����邩�𒲂ׂ�D

	@param[in] pLine ���ׂ镶���ւ̃|�C���^
	@param[in] length ���Y�ӏ��̕����T�C�Y
	@retval true �֑������ɊY��
	@retval false �֑������ɊY�����Ȃ�
*/
bool CLayoutMgr::IsKinsokuTail( wchar_t wc )
{
	return m_pszKinsokuTail_1.exist(wc);
}


/*!
	�֑��Ώۋ�Ǔ_�ɊY�����邩�𒲂ׂ�D

	@param [in] pLine  ���ׂ镶���ւ̃|�C���^
	@param [in] length ���Y�ӏ��̕����T�C�Y
	@retval true �֑������ɊY��
	@retval false �֑������ɊY�����Ȃ�
*/
bool CLayoutMgr::IsKinsokuKuto( wchar_t wc )
{
	return m_pszKinsokuKuto_1.exist(wc);
}

/*!
	@date 2005-08-20 D.S.Koba _DoLayout()��DoLayout_Range()���番��
*/
bool CLayoutMgr::IsKinsokuPosHead(
	CLayoutInt nRest,		//!< [in] �s�̎c�蕶����
	CLayoutInt nCharKetas,	//!< [in] ���݈ʒu�̕����T�C�Y
	CLayoutInt nCharKetas2	//!< [in] ���݈ʒu�̎��̕����T�C�Y
)
{
	switch( (Int)nRest )
	{
	//    321012  ���}�W�b�N�i���o�[
	// 3 "��j" : 22 "�j"��2�o�C�g�ڂŐ܂�Ԃ��̂Ƃ�
	// 2  "Z�j" : 12 "�j"��2�o�C�g�ڂŐ܂�Ԃ��̂Ƃ�
	// 2  "��j": 22 "�j"�Ő܂�Ԃ��̂Ƃ�
	// 2  "��)" : 21 ")"�Ő܂�Ԃ��̂Ƃ�
	// 1   "Z�j": 12 "�j"�Ő܂�Ԃ��̂Ƃ�
	// 1   "Z)" : 11 ")"�Ő܂�Ԃ��̂Ƃ�
	//���������O���H
	// ���������A"��Z"�������֑��Ȃ珈�����Ȃ��B
	case 3:	// 3�����O
		if( nCharKetas == 2 && nCharKetas2 == 2 ){
			return true;
		}
		break;
	case 2:	// 2�����O
		if( nCharKetas == 2 ){
			return true;
		}
		else if( nCharKetas == 1 && nCharKetas2 == 2 ){
			return true;
		}
		break;
	case 1:	// 1�����O
		if( nCharKetas == 1 ){
			return true;
		}
		break;
	}
	return false;
}

/*!
	@date 2005-08-20 D.S.Koba _DoLayout()��DoLayout_Range()���番��
*/
bool CLayoutMgr::IsKinsokuPosTail(
	CLayoutInt nRest,		//!< [in] �s�̎c�蕶����
	CLayoutInt nCharKetas,	//!< [in] ���݈ʒu�̕����T�C�Y
	CLayoutInt nCharKetas2	//!< [in] ���݈ʒu�̎��̕����T�C�Y
)
{
	switch( (Int)nRest )
	{
	case 3:	// 3�����O
		if( nCharKetas == 2 && nCharKetas2 == 2){
			// "�i��": "��"��2�o�C�g�ڂŐ܂�Ԃ��̂Ƃ�
			return true;
		}
		break;
	case 2:	// 2�����O
		if( nCharKetas == 2 ){
			// "�i��": "��"�Ő܂�Ԃ��̂Ƃ�
			return true;
		}
		else if( nCharKetas == 1 && nCharKetas2 == 2){
			// "(��": "��"��2�o�C�g�ڂŐ܂�Ԃ��̂Ƃ�
			return true;
		}
		break;
	case 1:	// 1�����O
		if( nCharKetas == 1 ){
			// "(��": "��"�Ő܂�Ԃ��̂Ƃ�
			return true;
		}
		break;
	}
	return false;
}

int CLayoutMgr::Match_Quote( wchar_t wcQuote, int nPos, const CStringRef& cLineStr/*int nLineLen, const wchar_t* pLine*/ ) const
{
	int nCharChars;
	int i;
	for( i = nPos; i < cLineStr.GetLength(); ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CNativeW::GetSizeOfChar( cLineStr.GetPtr(), cLineStr.GetLength(), i );
		if( 0 == nCharChars ){
			nCharChars = 1;
		}
		if(	m_sTypeConfig.m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
			if( 1 == nCharChars && cLineStr.At(i) == L'\\' ){
				++i;
			}else
			if( 1 == nCharChars && cLineStr.At(i) == wcQuote ){
				return i + 1;
			}
		}
		else if(	m_sTypeConfig.m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
			if( 1 == nCharChars && cLineStr.At(i) == wcQuote ){
				if( i + 1 < cLineStr.GetLength() && cLineStr.At(i + 1) == wcQuote ){
					++i;
				}else{
					return i + 1;
				}
			}
		}
		if( 2 == nCharChars ){
			++i;
		}
	}
	return cLineStr.GetLength();
}

/*!
	@brief �s�̒������v�Z���� (2�s�ڈȍ~�̎���������)
	
	���������s��Ȃ��̂ŁC���0��Ԃ��D
	�����͎g��Ȃ��D
	
	@return 1�s�̕\�������� (���0)
	
	@author genta
	@date 2002.10.01
*/
CLayoutInt CLayoutMgr::getIndentOffset_Normal( CLayout* )
{
	return CLayoutInt(0);
}

/*!
	@brief �C���f���g�����v�Z���� (Tx2x)
	
	�O�̍s�̍Ō��TAB�̈ʒu���C���f���g�ʒu�Ƃ��ĕԂ��D
	�������C�c�蕝��6���������̏ꍇ�̓C���f���g���s��Ȃ��D
	
	@author Yazaki
	@return �C���f���g���ׂ�������
	
	@date 2002.10.01 
	@date 2002.10.07 YAZAKI ���̕ύX, ����������
*/
CLayoutInt CLayoutMgr::getIndentOffset_Tx2x( CLayout* pLayoutPrev )
{
	//	�O�̍s�������Ƃ��́A�C���f���g�s�v�B
	if ( pLayoutPrev == NULL ) return CLayoutInt(0);

	CLayoutInt nIpos = pLayoutPrev->GetIndent();

	//	�O�̍s���܂�Ԃ��s�Ȃ�΂���ɍ��킹��
	if( pLayoutPrev->GetLogicOffset() > 0 )
		return nIpos;
	
	CMemoryIterator it( pLayoutPrev, m_sTypeConfig.m_nTabSpace );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndexDelta() == 1 && it.getCurrentChar() == WCODE::TAB ){
			nIpos = it.getColumn() + it.getColumnDelta();
		}
		it.addDelta();
	}
	if ( m_sTypeConfig.m_nMaxLineKetas - nIpos < 6 ){
		nIpos = pLayoutPrev->GetIndent();	//	������߂�
	}
	return nIpos;	//	�C���f���g
}

/*!
	@brief �C���f���g�����v�Z���� (�X�y�[�X��������)
	
	�_���s�s���̃z���C�g�X�y�[�X�̏I���C���f���g�ʒu�Ƃ��ĕԂ��D
	�������C�c�蕝��6���������̏ꍇ�̓C���f���g���s��Ȃ��D
	
	@author genta
	@return �C���f���g���ׂ�������
	
	@date 2002.10.01 
*/
CLayoutInt CLayoutMgr::getIndentOffset_LeftSpace( CLayout* pLayoutPrev )
{
	//	�O�̍s�������Ƃ��́A�C���f���g�s�v�B
	if ( pLayoutPrev == NULL ) return CLayoutInt(0);

	//	�C���f���g�̌v�Z
	CLayoutInt nIpos = pLayoutPrev->GetIndent();
	
	//	Oct. 5, 2002 genta
	//	�܂�Ԃ���3�s�ڈȍ~��1�O�̍s�̃C���f���g�ɍ��킹��D
	if( pLayoutPrev->GetLogicOffset() > 0 )
		return nIpos;
	
	//	2002.10.07 YAZAKI �C���f���g�̌v�Z
	CMemoryIterator it( pLayoutPrev, m_sTypeConfig.m_nTabSpace );

	//	Jul. 20, 2003 genta �����C���f���g�ɏ���������ɂ���
	bool bZenSpace = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bAutoIndent_ZENSPACE;
	const wchar_t* szSpecialIndentChar = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_szIndentChars;
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndexDelta() == 1 && WCODE::IsIndentChar(it.getCurrentChar(),bZenSpace) )
		{
			//	�C���f���g�̃J�E���g���p������
		}
		//	Jul. 20, 2003 genta �C���f���g�Ώە���
		else if( szSpecialIndentChar[0] != L'\0' ){
			wchar_t buf[3]; // �����̒�����1 or 2
			wmemcpy( buf, it.getCurrentPos(), it.getIndexDelta() );
			buf[ it.getIndexDelta() ] = L'\0';
			if( NULL != wcsstr( szSpecialIndentChar, buf )){
				//	�C���f���g�̃J�E���g���p������
			}
			else {
				nIpos = it.getColumn();	//	�I��
				break;
			}
		}
		else {
			nIpos = it.getColumn();	//	�I��
			break;
		}
		it.addDelta();
	}
	if ( m_sTypeConfig.m_nMaxLineKetas - nIpos < 6 ){
		nIpos = pLayoutPrev->GetIndent();	//	������߂�
	}
	return nIpos;	//	�C���f���g
}

/*!
	@brief  �e�L�X�g�ő啝���Z�o����

	�w�肳�ꂽ���C���𑖍����ăe�L�X�g�̍ő啝���쐬����B
	�S�č폜���ꂽ���͖��Z�o��Ԃɖ߂��B

	@param bCalLineLen	[in] �e���C�A�E�g�s�̒����̎Z�o���s��
	@param nStart		[in] �Z�o�J�n���C�A�E�g�s
	@param nEnd			[in] �Z�o�I�����C�A�E�g�s

	@retval TRUE �ő啝���ω�����
	@retval FALSE �ő啝���ω����Ȃ�����

	@note nStart, nEnd�������Ƃ�-1�̎��A�S���C���𑖍�����
		  �͈͂��w�肳��Ă���ꍇ�͍ő啝�̊g��̂݃`�F�b�N����

	@date 2009.08.28 nasukoji	�V�K�쐬
*/
BOOL CLayoutMgr::CalculateTextWidth( BOOL bCalLineLen, CLayoutInt nStart, CLayoutInt nEnd )
{
	BOOL bRet = FALSE;
	BOOL bOnlyExpansion = TRUE;		// �ő啝�̊g��݂̂��`�F�b�N����
	CLayoutInt nMaxLen = CLayoutInt(0);
	CLayoutInt nMaxLineNum = CLayoutInt(0);

	CLayoutInt nLines = CLayoutInt( GetLineCount() );		// �e�L�X�g�̃��C�A�E�g�s��

	// �J�n�E�I���ʒu���ǂ�����w�肳��Ă��Ȃ�
	if( nStart < 0 && nEnd < 0 )
		bOnlyExpansion = FALSE;		// �ő啝�̊g��E�k�����`�F�b�N����

	if( nStart < 0 )			// �Z�o�J�n�s�̎w��Ȃ�
		nStart = 0;
	else if( nStart > nLines )	// �͈̓I�[�o�[
		nStart = nLines;
	
	if( nEnd < 0 || nEnd >= nLines )	// �Z�o�I���s�̎w��Ȃ� �܂��� �����s���ȏ�
		nEnd = nLines;
	else
		nEnd++;					// �Z�o�I���s�̎��s

	CLayout* pLayout;

	// �Z�o�J�n���C�A�E�g�s��T��
	if( nStart * 2 < nLines ){
		// �O������T�[�`
		CLayoutInt nCount = CLayoutInt(0);
		pLayout = m_pLayoutTop;
		while( NULL != pLayout ){
			if( nStart == nCount ){
				break;
			}
			pLayout = pLayout->GetNextLayout();
			nCount++;
		}
	}else{
		// �������T�[�`
		CLayoutInt nCount = CLayoutInt( m_nLines - 1 );
		pLayout = m_pLayoutBot;
		while( NULL != pLayout ){
			if( nStart == nCount ){
				break;
			}
			pLayout = pLayout->GetPrevLayout();
			nCount--;
		}
	}

	// ���C�A�E�g�s�̍ő啝�����o��
	for( CLayoutInt i = nStart; i < nEnd; i++ ){
		if( !pLayout )
			break;

		// ���C�A�E�g�s�̒������Z�o����
		if( bCalLineLen ){
			CLayoutInt nWidth = pLayout->GetIndent() + pLayout->CalcLayoutWidth(*this) + CLayoutInt(pLayout->GetLayoutEol().GetLen()>0?1:0);
			pLayout->SetLayoutWidth( nWidth );
		}

		// �ő啝���X�V
		if( nMaxLen < pLayout->GetLayoutWidth() ){
			nMaxLen = pLayout->GetLayoutWidth();
			nMaxLineNum = i;		// �ő啝�̃��C�A�E�g�s

			// �A�v���P�[�V�����̍ő啝�ƂȂ�����Z�o�͒�~
			if( nMaxLen >= MAXLINEKETAS && !bCalLineLen )
				break;
		}

		// ���̃��C�A�E�g�s�̃f�[�^
		pLayout = pLayout->GetNextLayout();
	}

	// �e�L�X�g�̕��̕ω����`�F�b�N
	if( Int(nMaxLen) ){
		// �ő啝���g�債�� �܂��� �ő啝�̊g��̂݃`�F�b�N�łȂ�
		if( m_nTextWidth < nMaxLen || !bOnlyExpansion ){
			m_nTextWidthMaxLine = nMaxLineNum;
			if( m_nTextWidth != nMaxLen ){	// �ő啝�ω�����
				m_nTextWidth = nMaxLen;
				bRet = TRUE;
			}
		}
	}else if( Int(m_nTextWidth) && !Int(nLines) ){
		// �S�폜���ꂽ�畝�̋L�����N���A
		m_nTextWidthMaxLine = 0;
		m_nTextWidth = 0;
		bRet = TRUE;
	}
	
	return bRet;
}

/*!
	@brief  �e�s�̃��C�A�E�g�s���̋L�����N���A����
	
	@note �܂�Ԃ����@���u�܂�Ԃ��Ȃ��v�ȊO�̎��́A�p�t�H�[�}���X�̒ቺ��
		  �h�~����ړI�Ŋe�s�̃��C�A�E�g�s��(m_nLayoutWidth)���v�Z���Ă��Ȃ��B
		  ��Ő݌v����l������Ďg�p���Ă��܂���������Ȃ��̂Łu�܂�Ԃ��Ȃ��v
		  �ȊO�̎��̓N���A���Ă����B
		  �p�t�H�[�}���X�̒ቺ���C�ɂȂ�Ȃ����Ȃ�A�S�Ă̐܂�Ԃ����@�Ōv�Z
		  ����悤�ɂ��Ă��ǂ��Ǝv���B

	@date 2009.08.28 nasukoji	�V�K�쐬
*/
void CLayoutMgr::ClearLayoutLineWidth( void )
{
	CLayout* pLayout = m_pLayoutTop;

	while( pLayout ){
		pLayout->m_nLayoutWidth = 0;			// ���C�A�E�g�s�����N���A
		pLayout = pLayout->GetNextLayout();		// ���̃��C�A�E�g�s�̃f�[�^
		
	}
}


