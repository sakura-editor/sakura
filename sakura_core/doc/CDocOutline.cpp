/*!	@file
	@brief �A�E�g���C�����

	@author genta
	@date	2004.08.08 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, genta
	Copyright (C) 2002, frozen
	Copyright (C) 2003, zenryaku
	Copyright (C) 2005, genta, D.S.Koba, ���イ��

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include "CDocOutline.h"

#include <string.h>
#include "global.h"

#include "doc/CEditDoc.h"
#include "CFuncInfoArr.h"
#include "doc/CDocLine.h"
#include "charcode.h"
#include "util/other_util.h"

//	Mar. 15, 2000 genta
//	From Here
/*!
	�X�y�[�X�̔���
*/
bool C_IsSpace( char c );
inline bool C_IsSpace( wchar_t c )
{
	return (
		L'\t' == c ||
		L' ' == c ||
		WCODE::CR == c ||
		WCODE::LF == c
	);
}

/*!
	�֐��ɗp���邱�Ƃ��ł��镶�����ǂ����̔���
*/
bool C_IsWordChar( char c );
inline bool C_IsWordChar( wchar_t c )
{
	return (
		L'_' == c ||
		L':' == c ||
		L'~' == c ||
		(L'a' <= c && c <= L'z' )||
		(L'A' <= c && c <= L'Z' )||
		(L'0' <= c && c <= L'9' )
	);
}
//	To Here

//	From Here Apr. 1, 2001 genta
/*!
	����Ȋ֐��� "operator" ���ǂ����𔻒肷��B

	������"operator"���ꎩ�g���A���邢��::�̌���operator�Ƒ�����
	�I����Ă���Ƃ���operator�Ɣ���B

	���Z�q�̕]��������ۏ؂��邽��2��if���ɕ����Ă���

	@param szStr ����Ώۂ̕�����
	@param nLen ������̒����B
	�{���I�ɂ͕s�v�ł��邪�A�������̂��߂Ɋ��ɂ���l�𗘗p����B
*/
static bool C_IsOperator( wchar_t* szStr, int nLen	)
{
	if( nLen >= 8 && szStr[ nLen - 1 ] == L'r' ){
		if( nLen > 8 ?
				wcscmp( szStr + nLen - 9, L":operator" ) == 0 :	// �����o�[�֐��ɂ���`
				wcscmp( szStr, L"operator" ) == 0	// friend�֐��ɂ���`
		 ){
		 	return true;
		}
	}
	return false;
}
//	To Here Apr. 1, 2001 genta

/*!
	���s���O�� \ �ŃG�X�P�[�v���Ă��邩�ǂ�������

	@date 2005.12.06 ���イ�� �Ō��1�����������Ȃ���2�o�C�g�R�[�h�̌㔼���o�b�N�X���b�V���̏ꍇ�Ɍ�F����
*/
static bool C_IsLineEsc(const char *s, int len);
static bool C_IsLineEsc(const wchar_t *s, int len)
{
	if ( len > 0 && s[len-1] == L'\n' ) len--;
	if ( len > 0 && s[len-1] == L'\r' ) len--;
	if ( len > 0 && s[len-1] == L'\n' ) len--;

	if ( len > 0 && s[len-1] == L'\\' ) {
		if ( len == 1 ) {
			return(true);
		} else if ( len == 2 ) {
			if ( CNativeW::GetSizeOfChar( s, 2 , 0 ) == 1 )
				return(true);
		} else { //�c��R�o�C�g�ȏ�
			if ( CNativeW::GetSizeOfChar( s, len , len-2 ) == 1 )
				return(true);
			if ( CNativeW::GetSizeOfChar( s, len , len-3 ) == 2 )
				return(true);
		}
	}
	return(false);
}


/*!
	C�v���v���Z�b�T�� #if/ifdef/ifndef - #else - #endif��ԊǗ��N���X

	�l�X�g���x����32���x��=(sizeof(int) * 8)�܂�
*/

class CCppPreprocessMng {
public:
	CCppPreprocessMng(void) :
		m_stackptr( 0 ), m_bitpattern( 1 ), m_enablebuf( 0 ), m_maxnestlevel( 32 ), m_ismultiline( false )
	{}

	CLogicInt ScanLine(const wchar_t*, CLogicInt);

private:
	bool m_ismultiline; //!< �����s�̃f�B���N�e�B�u
	int m_maxnestlevel;	//!< �l�X�g���x���̍ő�l

	int m_stackptr;	//!< �l�X�g���x��
	/*!
		�l�X�g���x���ɑΉ�����r�b�g�p�^�[��
		
		m_stackptr = n �̎��C������(n-1)bit�ڂ�1�������Ă���
	*/
	unsigned int m_bitpattern;
	unsigned int m_enablebuf;	//!< �����̗L����ۑ�����o�b�t�@
};

/*!
	C�v���v���Z�b�T�� #if/ifdef/ifndef - #else - #endif������
	�����̕K�v���𔻒肷��D

	�^����ꂽ1�s�̕������擪���瑖�����CC/C++�ł̑������K�v�ȏꍇ��
	�擪�̋󔒂��������J�n�ʒu���C�s�v�ȏꍇ��length��Ԃ��D
	�Ăяo�����ł͖߂�l�����͂��n�߂�̂ŁC
	length��Ԃ����Ƃ͂��ׂċ󔒂ƌ��Ȃ����ƂɂȂ�D

	�l�X�g�̍ő�𒴂����ꍇ�ɂ͋L���悪�Ȃ����߂ɔ���͕s�\�ƂȂ邪�C
	�l�X�g���x�������͊Ǘ�����D

	@param str		[in] 1�s�̕�����
	@param length	[in] ������

	@return C��͊J�n�ʒu�D�����s�v�̏ꍇ��length(�s���܂ŃX�L�b�v)�D
	
	@par elif�̈���
	if (A) elif (B) elif (C) else (D) endif�̂悤�ȏꍇ�ɂ�(A)-(D)�̂ǂꂩ1��
	���������s�����D�������C�����Ȃ��1�r�b�g�ł͊Ǘ��ł��Ȃ����l�X�g��
	�͂ނ悤�ȃP�[�X��elif���g�����Ƃ͂��܂薳���Ə���Ɍ��߂Č��Ȃ��������Ƃɂ���D

	@author genta
	@date 2004.08.10 �V�K�쐬
	@date 2004.08.13 zenryaku �����s�̃f�B���N�e�B�u�ɑΉ�

*/
CLogicInt CCppPreprocessMng::ScanLine( const wchar_t* str, CLogicInt _length )
{
	int length=_length;

	const wchar_t* lastptr = str + length;	//	���������񖖔�
	const wchar_t* p;	//	�������̈ʒu

	//	skip whitespace
	for( p = str; C_IsSpace( *p ) && p < lastptr ; ++p )
		;
	if( lastptr <= p )
		return CLogicInt(length);	//	��s�̂��ߏ����s�v

	if(m_ismultiline){ // �����s�̃f�B���N�e�B�u�͖���
		m_ismultiline = C_IsLineEsc(str, length); // �s���� \ �ŏI����Ă��Ȃ���
		return CLogicInt(length);
	}

	if( *p != L'#' ){	//	�v���v���Z�b�T�ȊO�̏����̓��C�����ɔC����
		if( m_enablebuf ){
			return CLogicInt(length);	//	1�r�b�g�ł�1�ƂȂ��Ă����疳��
		}
		return CLogicInt(p - str);
	}

	++p; // #���X�L�b�v
	
	//	skip whitespace
	for( ; C_IsSpace( *p ) && p < lastptr ; ++p )
		;

	//	��������Preprocessor directive���
	if( p + 2 + 2 < lastptr && wcsncmp_literal( p, L"if" ) == 0 ){
		// if
		p += 2;
		
		int enable = 0;	//	0: �������Ȃ�, 1: else�ȍ~���L��, 2: �ŏ����L��, 
		
		//	if 0�͍ŏ������������Ƃ݂Ȃ��D
		//	����ȊO��if/ifdef/ifndef�͍ŏ����L�������ƌ��Ȃ�
		//	�ŏ��̏����ɂ���Ă��̎��_�ł�p < lastptr�Ȃ̂Ŕ���ȗ�
		if( C_IsSpace( *p ) ){
			//	if 0 �`�F�b�N
			//	skip whitespace
			for( ; C_IsSpace( *p ) && p < lastptr ; ++p )
				;
			if( *p == L'0' ){
				enable = 1;
			}
			else {
				enable = 2;
			}
		}
		else if(
			( p + 3 < lastptr && wcsncmp_literal( p, L"def" ) == 0 ) ||
			( p + 4 < lastptr && wcsncmp_literal( p, L"ndef" ) == 0 )){
			enable = 2;
		}
		
		//	�ۑ��̈�̊m�ۂƃr�b�g�p�^�[���̐ݒ�
		if( enable > 0 ){
			m_bitpattern = 1 << m_stackptr;
			++m_stackptr;
			if( enable == 1 ){
				m_enablebuf |= m_bitpattern;
			}
		}
	}
	else if( p + 4 < lastptr && wcsncmp_literal( p, L"else" ) == 0 ){
		if( m_stackptr < m_maxnestlevel ){
			m_enablebuf ^= m_bitpattern;
		}
	}
	else if( p + 5 < lastptr && wcsncmp_literal( p, L"endif" ) == 0 ){
		if( m_stackptr > 0 ){
			--m_stackptr;
			m_enablebuf &= ~m_bitpattern;
			m_bitpattern = ( 1 << ( m_stackptr - 1 ));
		}
	}
	else{
		m_ismultiline = C_IsLineEsc(str, length); // �s���� \ �ŏI����Ă��Ȃ���
	}

	return CLogicInt(length);	//	��{�I�Ƀv���v���Z�b�T�w�߂͖���
}

/*!
	@brief C/C++�֐����X�g�쐬

	@param bVisibleMemberFunc �N���X�A�\���̒�`���̃����o�֐��̐錾���A�E�g���C����͌��ʂɓo�^����ꍇ��true

	@par MODE�ꗗ
	- 0	�ʏ�
	- 20	Single quotation������ǂݍ��ݒ�
	- 21	Double quotation������ǂݍ��ݒ�
	- 8	�R�����g�ǂݍ��ݒ�
	- 1	�P��ǂݍ��ݒ�
	- 2	�L����ǂݍ��ݒ�
	- 999	���߂���P�ꖳ����


	@par FuncId�̒l�̈Ӗ�
	10�̈ʂŖړI�ʂɎg�������Ă���DC/C++�p��10�ʂ�0
	- 1: �錾
	- 2: �ʏ�̊֐� (�ǉ������񖳂�)
	- 3: �N���X("�N���X")
	- 4: �\���� ("�\����")
	- 5: �񋓑�("�񋓑�")
	- 6: ���p��("���p��")
	- 7: ���O���("���O���")

	@param pcFuncInfoArr [out] �֐��ꗗ��Ԃ����߂̃N���X�B
	�����Ɋ֐��̃��X�g��o�^����B
*/
void CDocOutline::MakeFuncList_C( CFuncInfoArr* pcFuncInfoArr ,bool bVisibleMemberFunc )
{
	const wchar_t*	pLine;
	CLogicInt	nLineLen;
	CLogicInt	i;

	// 2002/10/27 frozen�@��������
	// nNestLevel�� nNestLevel_global �� nNestLevel_func �ɕ��������B
	int			nNestLevel_global = 0;	// nNestLevel_global �֐��O�� {}�̃��x��  
	int			nNestLevel_func   = 0;	//	nNestLevel_func �֐��̒�`�A����ъ֐�����	{}�̃��x��
//	int			nNestLevel2;			//	nNestLevel2	()�ɑ΂���ʒu // 2002/10/27 frozen nNastLevel_fparam��nMode2��M2_FUNC_NAME_END�ő�p
	int			nNestLevel_fparam = 0;	// ()�̃��x��
	int			nNestPoint_class = 0;	// �O�����牽�Ԗڂ�{���N���X�̒�`���͂�{���H (��ԊO���Ȃ�1�A0�Ȃ疳���BbVisibleMemberFunc��false�̎��̂ݗL���Btrue�ł͏��0)
	// 2002/10/27 frozen�@�����܂�

	int			nCharChars;			//	���o�C�g������ǂݔ�΂����߂̂���
	wchar_t		szWordPrev[256];	//	1�O��word
	wchar_t		szWord[256];		//	���݉�ǒ���word������Ƃ���
	int			nWordIdx = 0;
	int			nMaxWordLeng = 100;	//	���e�����word�̍ő咷��
	int			nMode;				//	���݂�state

	// 2002/10/27 frozen�@��������
	//! ���2
	enum MODE2
	{
		M2_NORMAL			= 0x00,	//!< �ʏ�
		M2_NAMESPACE_SAVE	= 0x11,	//!< �l�[���X�y�[�X��������
			// �u�ʏ�v��ԂŒP�� "class" "struct" "union" "enum" "namespace"��ǂݍ��ނƁA���̏�ԂɂȂ�A';' '{' ',' '>' '='��ǂݍ��ނƁu�ʏ�v�ɂȂ�B
			//
			// ':' ��ǂݍ��ނƁu�l�[���X�y�[�X�����������v�ֈڍs����Ɠ�����
			// szWord��szTokenName�ɕۑ����A���Ƃ� ':' ���� '{' �̒��O�̒P�ꂪ���ׂ���悤�ɂ��Ă���B
			// ����� "__declspec( dllexport )"�̂悤��"class"�ƃN���X���̊ԂɃL�[���[�h�������Ă���ꍇ�ł��N���X�����擾�ł���悤�ɂ��邽�߁B
			//
			// '<' ��ǂݍ��ނƁu�e���v���[�g�N���X���������v�Ɉڍs����B
		M2_TEMPLATE_SAVE	= 0x12, //!< �e���v���[�g�N���X��������
			// ';' '{'��ǂݍ��ނƁu�ʏ�v�ɂȂ�B
			// �܂��A���̏�Ԃ̊Ԃ͒P�����؂���@���ꎞ�I�ɕύX���A
			// �utemplate_name <paramA,paramB>�v�̂悤�ȕ��������̒P����݂Ȃ��悤�ɂ���B
			// ����͓��ꉻ�����N���X�e���v���[�g����������ۂ̍\���ŗL���ɓ����B	
		M2_NAMESPACE_END	= 0x13,	//!< �l�[���X�y�[�X�����������B(';' '{' ��ǂݍ��񂾎��_�Łu�ʏ�v�ɂȂ�B )
		M2_FUNC_NAME_END	= 0x14, //!< �֐������������B(';' '{' ��ǂݍ��񂾎��_�Łu�ʏ�v�ɂȂ�B )
		M2_AFTER_EQUAL		= 0x05,	//!< '='�̌�B
			//�u�ʏ�v���� nNestLevel_fparam==0 ��'='��������Ƃ��̏�ԂɂȂ�B�i������ "opreator"�̒���͏����j
			// ';'��������Ɓu�ʏ�v�ɖ߂�B
			// int val=abs(-1);
			// �̂悤�ȕ����֐��Ƃ݂Ȃ���Ȃ��悤�ɂ��邽�߂Ɏg�p����B
		M2_KR_FUNC			= 0x16,	//!< K&R�X�^�C���̊֐���`�𒲍�����B
		M2_AFTER_ITEM		= 0x10,
	} nMode2 = M2_NORMAL;

	const int	nNamespaceNestMax	= 32;			//!< �l�X�g�\�ȃl�[���X�y�[�X�A�N���X���̍ő吔
	int			nNamespaceLen[nNamespaceNestMax+1];	//!< �l�[���X�y�[�X�S�̂̒���
	const int	nNamespaceLenMax 	= 512;			//!< �ő�̃l�[���X�y�[�X�S�̂̒���
	wchar_t		szNamespace[nNamespaceLenMax];		//!< ���݂̃l�[���X�y�[�X(�I�[��\0�ɂȂ��Ă���Ƃ͌���Ȃ��̂Œ���)
	const int 	nItemNameLenMax	 	= 256;
	wchar_t		szItemName[nItemNameLenMax];		//!< �����O�� �֐��� or �N���X�� or �\���̖� or ���p�̖� or �񋓑̖� or �l�[���X�y�[�X��
	// �Ⴆ�Ή��̃R�[�h�́��̕����ł�
	// szNamespace��"Namespace\ClassName\"
	// nMamespaceLen��{10,20}
	// nNestLevel_global��2�ƂȂ�B
	//
	//�@namespace Namespace{
	//�@class ClassName{
	//�@��
	//�@}}

	CLogicInt	nItemLine;			//!< �����O�� �֐� or �N���X or �\���� or ���p�� or �񋓑� or �l�[���X�y�[�X�̂���s
	int			nItemFuncId;

	szWordPrev[0] = L'\0';
	szWord[nWordIdx] = L'\0';
	szNamespace[0] = L'\0';	// 2002/10/27 frozen
	nNamespaceLen[0] = 0;	// 2002/10/27 frozen
	nMode = 0;
	
	//	Aug. 10, 2004 genta �v���v���Z�X�����N���X
	CCppPreprocessMng cCppPMng;
	
	CLogicInt		nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);

		//	From Here Aug. 10, 2004 genta
		//	�v���v���Z�X����
		//	�R�����g���łȂ���΃v���v���Z�b�T�w�߂��ɔ��肳����
		if( 8 != nMode && 10 != nMode ){	/* chg 2005/12/6 ���イ�� ���̍s���󔒂ł��悢	*/
			i = cCppPMng.ScanLine( pLine, nLineLen );
		}
		else {
			i = CLogicInt(0);
		}
		//	C/C++�Ƃ��Ă̏������s�v�ȃP�[�X�ł� i == nLineLen�ƂȂ��Ă���̂�
		//	�ȉ��̉�͏�����SKIP�����D
		//	To Here Aug. 10, 2004 genta
		
		for( ; i < nLineLen; ++i ){
			/* 1�o�C�g������������������ */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
			if( 1 < nCharChars ){
				i += CLogicInt(nCharChars - 1);
				continue;
			}
/* del start 2005/12/6 ���イ��	*/
			/* �G�X�P�[�v�V�[�P���X�͏�Ɏ�菜�� */
			/* �V���O���N�H�[�e�[�V����������ǂݍ��ݒ� */
			/* �_�u���N�H�[�e�[�V����������ǂݍ��ݒ� */
			// ��������R�����g�����̌�ֈړ�
/* del end 2005/12/6 ���イ��	*/
			/* �R�����g�ǂݍ��ݒ� */
			if( 8 == nMode ){
				if( i < nLineLen - 1 && '*' == pLine[i] &&  '/' == pLine[i + 1] ){
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}
			/* ���C���R�����g�ǂݍ��ݒ� */
			// 2003/06/24 zenryaku
			else if( 10 == nMode)
			{
				if(!C_IsLineEsc(pLine, nLineLen)){
					nMode = 0;
				}
				i = nLineLen;
				continue;
			}
			/* add start 2005/12/6 ���イ��	*/
			/* �G�X�P�[�v�V�[�P���X�͏�Ɏ�菜�� */
			else if( '\\' == pLine[i] ){
				++i;
			}
			/* �V���O���N�H�[�e�[�V����������ǂݍ��ݒ� */
			else if( 20 == nMode ){
				if( '\'' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}
			/* �_�u���N�H�[�e�[�V����������ǂݍ��ݒ� */
			else if( 21 == nMode ){
				if( '"' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}
			/* add end 2005/12/6 ���イ��	*/
			/* �P��ǂݍ��ݒ� */
			else if( 1 == nMode ){
				if( C_IsWordChar( pLine[i] ) ){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						if( pLine[i] == L':')
						{
							if(nMode2 == M2_NAMESPACE_SAVE)
							{
								if(szWord[0]!='\0')
									wcscpy( szItemName, szWord );
								nMode2 = M2_NAMESPACE_END;
							}
							else if( nMode2 == M2_TEMPLATE_SAVE)
							{
								wcsncat( szItemName, szWord, nItemNameLenMax - wcslen(szItemName) );
								szItemName[ nItemNameLenMax - 1 ] = L'\0';
								nMode2 = M2_NAMESPACE_END;
							}
						}
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = L'\0';
					}
				}else{
					//	From Here Mar. 31, 2001 genta
					//	operator�L�[���[�h(���Z�qoverload)�̑Ή�
					//	�������Aoperator�L�[���[�h�̌��ɃX�y�[�X�������Ă���Ƃ��܂������Ȃ��B
					if( C_IsOperator( szWord, nWordIdx + 1 ) ){
						//	operator���I
						/*  overload���鉉�Z�q�ꗗ
							& && &=
							| || |=
							+ ++ +=
							- -- -= -> ->*
							* *=
							/ /=
							% %=
							^ ^=
							! !=
							= ==
							< <= << <<=
							> >= >> >>=
							()
							[]
							~
							,
						*/
						int oplen = 0;	// ���Z�q�{�̕��̕�����
						switch( pLine[i] ){
						case '&': // no break
						case '|': // no break
						case '+':
							oplen = 1;
							if( i + 1 < nLineLen ){
								if( pLine[ i + 1 ] == pLine[ i ] ||
									pLine[ i + 1 ] == L'=' )
									oplen = 2;
							}
							break;
						case '-':
							oplen = 1;
							if( i + 1 < nLineLen ){
								if( pLine[ i + 1 ] == L'-' ||
									pLine[ i + 1 ] == L'=' )
									oplen = 2;
								else if( pLine[ i + 1 ] == L'>' ){
									oplen = 2;
									if( i + 2 < nLineLen ){
										if( pLine[ i + 2 ] == L'*' )
											oplen = 3;
									}
								}
							}
							break;
						case '*': // no break
						case '/': // no break
						case '%': // no break
						case '^': // no break
						case '!': // no break
						case '=':
							oplen = 1;
							if( i + 1 < nLineLen ){
								if( pLine[ i + 1 ] == L'=' )
									oplen = 2;
							}
							break;
						case '<': // no break
						case '>':
							oplen = 1;
							if( i + 1 < nLineLen ){
								if( pLine[ i + 1 ] == pLine[ i ] ){
									oplen = 2;
									if( i + 2 < nLineLen ){
										if( pLine[ i + 2 ] == L'=' )
											oplen = 3;
									}
								}
								else if( pLine[ i + 1 ] == L'=' )
									oplen = 2;
							}
							break;
						case '(':
							if( i + 1 < nLineLen )
								if( pLine[ i + 1 ] == /* ���ʑΉ��΍� ( */ ')' )
									oplen = 2;
							break;
						case '[':
							if( i + 1 < nLineLen )
								if( pLine[ i + 1 ] == /* ���ʑΉ��΍� [ */ ']' )
									oplen = 2;
							break;
						case '~': // no break
						case ',':
							oplen = 2;
							break;
						}

						//	oplen �̒��������L�[���[�h�ɒǉ�
						for( ; oplen > 0 ; oplen--, i++ ){
							++nWordIdx;
							szWord[nWordIdx] = pLine[i];
						}
						szWord[nWordIdx + 1] = L'\0';
							// �L����̏������s���O�͋L�����i�͋L����̐擪���w���Ă����B
							// ���̎��_��i�͋L�����1����w���Ă���

							// operator�̌��ɕs���ȕ���������ꍇ�̓���
							// ( �Ŏn�܂�ꍇ��operator�Ƃ����֐��ƔF�������
							// ����ȊO�̋L�����Ə]���ʂ�L����global�̂����Ɍ����B

							// ���Z�q�������Ă���ꍇ�̓���
							// ��������()�̏ꍇ�͂��ꂪ���Z�q�ƌ��Ȃ���邽�߁A���̍s�͊֐���`�ƔF������Ȃ�
							// ����ȊO�̏ꍇ��operator�Ƃ����֐��ƔF�������
					}
					// 2002/10/27 frozen�@��������
					if( nMode2 == M2_NAMESPACE_SAVE )
						wcscpy( szItemName, szWord );
					else if( nMode2 == M2_TEMPLATE_SAVE)
					{
						wcsncat( szItemName, szWord, nItemNameLenMax - wcslen(szItemName) );
						szItemName[ nItemNameLenMax - 1 ] = L'\0';
					}
					else if( nNestLevel_func == 0 && nMode2 == M2_NORMAL )
					{
						nItemFuncId = 0;
						if( wcscmp(szWord,L"class")==0 )
							nItemFuncId = 3;
						if( wcscmp(szWord,L"struct")==0 )
							nItemFuncId = 4;
						else if( wcscmp(szWord,L"namespace")==0 )
							nItemFuncId = 7;
						else if( wcscmp(szWord,L"enum")==0 )
							nItemFuncId = 5;
						else if( wcscmp(szWord,L"union")==0 )
							nItemFuncId = 6;
						if( nItemFuncId != 0 )
						{
							nMode2 = M2_NAMESPACE_SAVE;
							nItemLine = nLineCount + CLogicInt(1);
							wcscpy(szItemName,L"����");
						}
					}
					else if( nMode2 == M2_FUNC_NAME_END )
					{
						nMode2 = M2_KR_FUNC;
					}
					// 2002/10/27 frozen�@�����܂�

					//	To Here Mar. 31, 2001 genta
					// 2004/03/12 zenryaku �L�[���[�h�� _ �� PARAMS ���g�킹�Ȃ� (GNU�̃R�[�h�����ɂ����Ȃ邩��)
					if( !( wcscmp(L"PARAMS",szWord) == 0 || wcscmp(L"_",szWord) == 0 ) )
						wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = L'\0';
					nMode = 0;
					i--;
					continue;
				}
			}else
			/* �L����ǂݍ��ݒ� */
			if( 2 == nMode ){
				if( C_IsWordChar( pLine[i] ) ||
					C_IsSpace( pLine[i] ) ||
					 '{' == pLine[i] ||
					 '}' == pLine[i] ||
					 '(' == pLine[i] ||
					 ')' == pLine[i] ||
					 ';' == pLine[i] ||
					'\'' == pLine[i] ||
					 '"' == pLine[i] ||
					 '/' == pLine[i]
				){
					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = L'\0';
					nMode = 0;
					i--;
					continue;
				}else{
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = L'\0';
					}
				}
			}else
			/* ���߂���P�ꖳ���� */
			if( 999 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( C_IsSpace( pLine[i] ) ){
					nMode = 0;
					continue;
				}
			}else
			/* �m�[�}�����[�h */
			if( 0 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( C_IsSpace( pLine[i] ) )
					continue;

				if( i < nLineLen - 1 && '/' == pLine[i] &&  '/' == pLine[i + 1] ){
					++i;
					nMode = 10;
					continue;
				}else
				if( i < nLineLen - 1 && '/' == pLine[i] &&  '*' == pLine[i + 1] ){
					++i;
					nMode = 8;
					continue;
				}else
				if( '\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( '"' == pLine[i] ){
					nMode = 21;
					continue;
				}else
				
				// 2002/10/27 frozen ��������
				if( '{' == pLine[i] )
				{
					int nItemNameLen;
					if( nNestLevel_func !=0)
						++nNestLevel_func;
					else if(
							(nMode2 & M2_AFTER_ITEM) != 0  &&
							nNestLevel_global < nNamespaceNestMax &&
							(nNamespaceLen[nNestLevel_global] +  (nItemNameLen = wcslen(szItemName)) + 10 + 1) < nNamespaceLenMax)
					// �R�Ԗڂ�(&&�̌��)����
					// �o�b�t�@������Ȃ��ꍇ�͍��ڂ̒ǉ����s��Ȃ��B
					// +10�͒ǉ����镶����̍ő咷(�ǉ����镶�����"::��`�ʒu"���Œ�)
					// +1�͏I�[NUL����
					{
						wcscpy( &szNamespace[nNamespaceLen[nNestLevel_global]] , szItemName);
						//	Jan. 30, 2005 genta M2_KR_FUNC �ǉ�
						//	�֐��̌���const, throw �܂��͏������q�������
						//	M2_KR_FUNC�ɑJ�ڂ��āC';'��������Ȃ��Ƃ��̏�Ԃ̂܂�
						//	�����ʂɑ�������D
						if( nMode2 == M2_FUNC_NAME_END || nMode2 == M2_KR_FUNC )
							++ nNestLevel_func;
						else
						{
							++ nNestLevel_global;
							nNamespaceLen[nNestLevel_global] = nNamespaceLen[nNestLevel_global-1] + nItemNameLen;
							if( nItemFuncId == 7)
								wcscpy(&szNamespace[nNamespaceLen[nNestLevel_global]],L"::��`�ʒu");
							else
							{
								szNamespace[nNamespaceLen[nNestLevel_global]] = L'\0';
								szNamespace[nNamespaceLen[nNestLevel_global]+1] = L':';
								if(bVisibleMemberFunc == false && nNestPoint_class == 0)
									nNestPoint_class = nNestLevel_global;
							}
						}
						/*
						  �J�[�\���ʒu�ϊ�
						  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
						  ��
						  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						*/
						CLayoutPoint ptPosXY;
						m_pcDocRef->m_cLayoutMgr.LogicToLayout(
							CLogicPoint(0, nItemLine - 1),
							&ptPosXY
						);
						pcFuncInfoArr->AppendData( nItemLine, ptPosXY.GetY2() + CLayoutInt(1) , szNamespace, nItemFuncId);
						//	Jan. 30, 2005 genta M2_KR_FUNC �ǉ�
						if( nMode2 != M2_FUNC_NAME_END && nMode2 != M2_KR_FUNC )
						{
							szNamespace[nNamespaceLen[nNestLevel_global]] = L':';
							nNamespaceLen[nNestLevel_global] += 2;
						}
					}
					else
					{
						//	Jan. 30, 2005 genta M2_KR_FUNC �ǉ�
						if(nMode2 == M2_FUNC_NAME_END || nMode2 == M2_KR_FUNC)
							++ nNestLevel_func;
						else
						{
							++ nNestLevel_global;
							if ( nNestLevel_global <= nNamespaceNestMax )
								nNamespaceLen[nNestLevel_global]=nNamespaceLen[nNestLevel_global-1];
						}
					}
					// bCppInitSkip = false;	//	Mar. 4, 2001 genta
					nMode = 0;
					nMode2 = M2_NORMAL;
					// nNestLevel2 = 0;
					continue;
				}else
				// 2002/10/27 frozen �����܂�
				
				if( '}' == pLine[i] ){
					//  2002/10/27 frozen ��������
//					nNestLevel2 = 0;
					if(nNestLevel_func == 0)
					{
						if(nNestLevel_global!=0)
						{
							if(nNestLevel_global == nNestPoint_class)
								nNestPoint_class = 0;
							--nNestLevel_global;
						}
					}
					else
						--nNestLevel_func;
					//  2002/10/27 frozen �����܂�
					nMode = 0;
					nMode2 = M2_NORMAL;
					continue;
				}else
				if( '(' == pLine[i] ){
					//  2002/10/27 frozen ��������
//					if( nNestLevel == 0 && !bCppInitSkip ){
//						wcscpy( szFuncName, szWordPrev );
//						nFuncLine = nLineCount + 1;
//						nNestLevel2 = 1;
//					}
//					nMode = 0;
					if( nNestLevel_func == 0 && (nMode2 == M2_NORMAL || nMode2 == M2_NAMESPACE_SAVE || nMode2 == M2_NAMESPACE_END) )
					{
						if( wcscmp(szWordPrev, L"__declspec") == 0 ) {continue;}
						if(nNestLevel_fparam==0)
						{
							wcscpy( szItemName, szWordPrev);
							nItemLine = nLineCount + CLogicInt(1);
						}
						++ nNestLevel_fparam;
					}
					//  2002/10/27 frozen �����܂�
					
					//	From Here Jan. 30, 2005 genta 
					if( nNestLevel_func == 0 && nMode2 == M2_KR_FUNC ){
						//	throw�Ȃ� (��O�̌^,...) ��ǂݔ�΂�
						if(nNestLevel_fparam==0)
							++ nNestLevel_fparam;
					}
					//	To Here Jan. 30, 2005 genta 
					continue;
				}else
				if( ')' == pLine[i] ){
					//  2002/10/27 frozen ��������
//					if( 1 == nNestLevel2 ){
//						nNestLevel2 = 2;
//					}
//					nMode = 0;
					if( nNestLevel_fparam > 0)
					{
						--nNestLevel_fparam;
						if( nNestLevel_fparam == 0)
						{
							nMode2 = M2_FUNC_NAME_END;
							nItemFuncId = 2;
						}
					}
					//  2002/10/27 frozen �����܂�
					continue;
				}else
				if( ';' == pLine[i] ){
					//  2002/10/27 frozen ��������
					if( nMode2 == M2_KR_FUNC )
					{
						//	Jan. 30, 2005 genta �֐���� const, throw�̌���
						//	';'��K&R�`���錾�̏I���łȂ��֐��錾�̏I���
						if( wcscmp( szWordPrev, L"const" ) == 0 ||
							wcscmp( szWordPrev, L"throw" ) == 0 ){
								nMode2 = M2_FUNC_NAME_END;
								//	��������if���Ɉ���������܂��悤��
						}
						else {
							// zenryaku K&R�X�^�C���̊֐��錾�̏I���� M2_FUNC_NAME_END �ɂ��ǂ�
							nMode2 = M2_FUNC_NAME_END;
							continue;
						}
					} //	Jan. 30, 2005 genta K&R�����Ɉ��������Đ錾�������s���D
					if( nMode2 == M2_FUNC_NAME_END &&
						nNestLevel_global < nNamespaceNestMax &&
						(nNamespaceLen[nNestLevel_global] + wcslen(szItemName)) < nNamespaceLenMax &&
						nNestPoint_class == 0)
					// �R�Ԗڂ�(&&�̌��)����
					// �o�b�t�@������Ȃ��ꍇ�͍��ڂ̒ǉ����s��Ȃ��B
					{
						wcscpy( &szNamespace[nNamespaceLen[ nNestLevel_global]] , szItemName);

						nItemFuncId = 1;
						/*
						  �J�[�\���ʒu�ϊ�
						  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
						  ��
						  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						*/
						CLayoutPoint ptPosXY;
						m_pcDocRef->m_cLayoutMgr.LogicToLayout(
							CLogicPoint(0, nItemLine - 1),
							&ptPosXY
						);
						pcFuncInfoArr->AppendData( nItemLine, ptPosXY.GetY2() + CLayoutInt(1), szNamespace, nItemFuncId);
					}
					nMode2 = M2_NORMAL;
					//  2002/10/27 frozen �����܂�
					nMode = 0;
					continue;
				}else if( nNestLevel_fparam == 0 ){
					if( C_IsWordChar( pLine[i] ) ){
						//	//	Mar. 15, 2000 genta
						//	From Here
						//	���O��word�̍Ōオ::���C���邢�͒����word�̐擪��::�Ȃ�
						//	�N���X����q�ƍl���ė��҂�ڑ�����D

						{
							int pos = wcslen( szWordPrev ) - 2;
							if( //	�O�̕�����̖����`�F�b�N
								( pos > 0 &&	szWordPrev[pos] == L':' &&
								szWordPrev[pos + 1] == L':' ) ||
								//	���̕�����̐擪�`�F�b�N
								( i < nLineLen - 1 && pLine[i] == L':' &&
									pLine[i+1] == L':' )
							){
								//	�O�̕�����ɑ�����
								wcscpy( szWord, szWordPrev );
								nWordIdx = wcslen( szWord );
							}
							//	From Here Apr. 1, 2001 genta
							//	operator new/delete ���Z�q�̑Ή�
							else if( C_IsOperator( szWordPrev, pos + 2 ) ){
								//	�X�y�[�X�����āA�O�̕�����ɑ�����
								szWordPrev[pos + 2] = L' ';
								szWordPrev[pos + 3] = L'\0';
								wcscpy( szWord, szWordPrev );
								nWordIdx = wcslen( szWord );
							}
							//	To Here Apr. 1, 2001 genta
							else{
								nWordIdx = 0;
							}
						}
						//	wcscpy( szWordPrev, szWord );	�s�v�H
						//	To Here
						
						if( pLine[i] == L':')
						{
							if(nMode2 == M2_NAMESPACE_SAVE)
							{
								if(szWord[0]!='\0')
									wcscpy( szItemName, szWord );
								nMode2 = M2_NAMESPACE_END;
							}
							else if( nMode2 == M2_TEMPLATE_SAVE)
							{
								wcsncat( szItemName, szWord, nItemNameLenMax - wcslen(szItemName) );
								szItemName[ nItemNameLenMax - 1 ] = L'\0';
								nMode2 = M2_NAMESPACE_END;
							}
						}

						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = L'\0';
						//	//	Mar. 15, 2000 genta
						//	From Here
						//	�����`�F�b�N�͕K�{
						if( nWordIdx < nMaxWordLeng ){
							nMode = 1;
						}
						else{
							nMode = 999;
						}
						//	To Here
					}else{
						//	Aug. 13, 2004 genta
						//	szWordPrev�������Ȃ�������operator�̔�����s��
						//	operator�̔���͑O�ɃN���X�����t���Ă���\��������̂�
						//	��p�̔���֐����g���ׂ��D
						//	operator�Ŗ������=�͑���Ȃ̂ł����͐錾���ł͂Ȃ��D
						if( pLine[i] == L'=' && nNestLevel_func == 0 && nNestLevel_fparam==0
							&& nMode2 == M2_NORMAL && ! C_IsOperator(szWordPrev,wcslen(szWordPrev)) ){
							nMode2 = M2_AFTER_EQUAL;
						}
						wcscpy( szWordPrev, szWord );
						nWordIdx = 0;
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = L'\0';
						nMode = 2;

						// 2002/10/27 frozen ��������
						if( nMode2 == M2_NAMESPACE_SAVE )
						{
							if( pLine[i] == L'>' || pLine[i] == L',' || pLine[i] == L'=')
								// '<' �̑O�� '>' , ',' , '=' ���������̂ŁA�����炭
								// �O�ɂ�����"class"�̓e���v���[�g�p�����[�^�̌^��\���Ă����ƍl������B
								// ����āA�N���X���̒����͏I���B
								// '>' �̓e���v���[�g�p�����[�^�̏I��
								// ',' �̓e���v���[�g�p�����[�^�̋�؂�
								// '=' �̓f�t�H���g�e���v���[�g�p�����[�^�̎w��
								nMode2 = M2_NORMAL; 
							else if( pLine[i] == L'<' )
								nMode2 = M2_TEMPLATE_SAVE;
						}

						if( nMode2 == M2_TEMPLATE_SAVE)
						{
							int nItemNameLen = wcslen(szItemName);
							if(nItemNameLen + 1 < nItemNameLenMax )
							{
								szItemName[nItemNameLen] = pLine[i];
								szItemName[nItemNameLen + 1 ] = L'\0';
							}
						}
						// 2002/10/27 frozen �����܂�
					}
				}
			}
		}
	}
	return;
}




/*! @file
	@brief Python�A�E�g���C�����

	@author genta
	@date 2007.02.24 �V�K�쐬
*/
/*
	Copyright (C) 2007, genta

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

#include <string.h>
#include "global.h"
#include <assert.h>

#include "doc/CEditDoc.h"
#include "CFuncInfoArr.h"
#include "doc/CDocLine.h"
#include "charcode.h"



/*!
	�֐��ɗp���邱�Ƃ��ł��镶�����ǂ����̔���
	
	@date 2007.02.08 genta

	@note �����ɂ�1�����ڂɐ������g�����Ƃ͏o���Ȃ����C
		����͎��s���Ă݂�Ζ��炩�ɂ킩�邱�ƂȂ̂�
		�����܂Ō����Ƀ`�F�b�N���Ȃ�
*/
inline bool Python_IsWordChar( wchar_t c ){
	return ( L'_' == c ||
			(L'a' <= c && c <= L'z' )||
			(L'A' <= c && c <= L'Z' )||
			(L'0' <= c && c <= L'9' )
		);
}

/*! python�̃p�[�X��Ԃ��Ǘ�����\����

	��͒��ɉ�͊֐��̊Ԃ������n�����D
	���̃N���X�͌��݂̏�ԂƁC������̐�����ێ�����D
	��͈ʒu�͉�͊֐��ԂŃp�����[�^�Ƃ��ēn�����̂�
	���̒��ł͕ێ����Ȃ��D

	[��ԑJ��]
	�J�n : STATE_NORMAL

	STATE_NORMAL/STATE_CONTINUE��STATE_CONTINUE�̑J��
	- �p���s�}�[�N�L��

	STATE_NORMAL/STATE_CONTINUE��STATE_NORMAL�̑J��
	- �p���s�}�[�N���Ȃ��s���ɒB����
	- �R�����g�ɒB����

	STATE_NORMAL��STATE_STRING�̑J��
	- ���p������

	STATE_STRING��STATE_NORMAL�̑J��
	- �K��̕�����I���L��
	- short string�ŕ�����̏I�����������p�����p���s�}�[�N���Ȃ��s���ɒB����

	@date 2007.02.12 genta
*/
struct COutlinePython {
	enum {
		STATE_NORMAL,	//!< �ʏ�s : �s�����܂�
		STATE_STRING,	//!< ������
		STATE_CONTINUE,	//!< �p���s : �O�̍s����̑����Ȃ̂ōs���Ƃ݂͂Ȃ���Ȃ�
	} m_state;
	
	int m_quote_char;	//!<	���p���L��
	bool m_raw_string;	//!<	�G�X�P�[�v�L�������Ȃ�true
	bool m_long_string;	//!<	���������񒆂Ȃ�true

	COutlinePython();

	/*	�e��Ԃɂ����镶����X�L�������s��
		Scan*���Ăт������Ƃ��͊��ɂ��̏�ԂɂȂ��Ă��邱�Ƃ��O��D
		�����Ԃ���ʂ̏�ԂɈڂ�Ƃ���܂ł������D
		�ʂ̏�ԂɈڂ锻�肪��₱�����΂����́CEnter*�Ƃ��Ċ֐��ɂ���D
	*/	
	int ScanNormal( const wchar_t* data, int linelen, int start_offset );
	int ScanString( const wchar_t* data, int linelen, int start_offset );
	int EnterString( const wchar_t* data, int linelen, int start_offset );
	void DoScanLine( const wchar_t* data, int linelen, int start_offset );
	
	bool IsLogicalLineTop(void) const { return STATE_NORMAL == m_state; }
};

/*!�R���X�g���N�^: ������

	������Ԃ�STATE_NORMAL�ɐݒ肷��D
*/
COutlinePython::COutlinePython()
	: m_state( STATE_NORMAL ),
	m_raw_string( false ),
	m_long_string( false )
{
}

/*! @brief Python������̓�����ŕ������ʂ����肷��

	������̎�ނ�K�؂ɔ��ʂ��C������Ԃ�ݒ肷��D
	start_offset�͊J�n���p�����w���Ă��邱�ƁD

	- ���p��1��: short string
	- ���p��3��: long string
	- ���p���̑O��r��R���� : raw string

	@param[in] data �Ώە�����
	@param[in] linelen �f�[�^�̒���
	@param[in] start_offset �����J�n�ʒu
	
	@return ������̈ʒu

	@invariant
		m_state != STATE_STRING

	@note ���p���̈ʒu�ŌĂт����΁C��������͕K��STATE_STRING�ɂȂ��Ă���͂��D
		���p���ȊO�̈ʒu�ŌĂт������ꍇ�͉������Ȃ��Ŕ�����D
*/
int COutlinePython::EnterString( const wchar_t* data, int linelen, int start_offset )
{
	assert( m_state != STATE_STRING );

	int col = start_offset;
	//	������J�n�`�F�b�N
	if( data[ col ] == '\"' || data[ col ] == '\'' ){
		int quote_char = data[ col ];
		m_state = STATE_STRING;
		m_quote_char = quote_char;
		//	������̊J�n
		if( col >= 1 &&
			( data[ col - 1 ] == 'r' || data[ col - 1 ] == 'R' )){
			//	�����ɂ͒��O��SHIFT_JIS��2�o�C�g�ڂ��ƌ딻�肷��\�������邪
			//	�������������Ȃ��R�[�h�͑���ɂ��Ȃ�
			m_raw_string = true;
		}
		else {
			m_raw_string = false;
		}
		if( col + 2 < linelen &&
			data[ col + 1 ] == quote_char &&
			data[ col + 2 ] == quote_char ){
				m_long_string = true;
				col += 2;
		}
		else {
			m_long_string = false;
		}
		++col;
	}
	return col;
}

/*! @brief Python�v���O�����̏���

	�v���O�����{�̕����̏����D������̊J�n�C�p���s�C�R�����g�C�ʏ�s�����`�F�b�N����D
	�s�����肪�I�������ň����n�����̂ŁC�֐��E�N���X��`�͍l�����Ȃ��ėǂ��D
	
	�ȉ��̏ꍇ�ɏ������I������
	- �s��: STATE_NORMAL�Ƃ��ď����I��
	- �R�����g: STATE_NORMAL�Ƃ��ď����I��
	- ������̊J�n: EnterString() �ɂĕ������ʂ̔�����s������STATE_STRING�Ƃ��ď����I��
	- �p���s: STATE_CONTINUE�Ƃ��ď����I��

	@param[in] data �Ώە�����
	@param[in] linelen �f�[�^�̒���
	@param[in] start_offset �����J�n�ʒu
	
	@invaliant
		m_state == STATE_NORMAL || m_state == STATE_CONTINUE
	
	@return ������̈ʒu
*/
int COutlinePython::ScanNormal( const wchar_t* data, int linelen, int start_offset )
{
	assert( m_state == STATE_NORMAL || m_state == STATE_CONTINUE );

	for( int col = start_offset; col < linelen; ++col ){
		int nCharChars = CNativeW::GetSizeOfChar( data, linelen, col );
		if( 1 < nCharChars ){
			col += (nCharChars - 1);
			continue;
		}
		//	�R�����g
		if( data[col] == '#' ){
			//	�R�����g�͍s���Ɠ��������Ȃ̂�
			//	�킴�킴�Ɨ����Ĉ����K�v��������
			//	�����ŕЂ�t���Ă��܂���
			m_state = STATE_NORMAL;
			break;
		}
		//	������
		else if( data[col] == '\"' || data[col] == '\'' ){
			return EnterString( data, linelen, col );
		}
		else if( data[col] == '\\' ){	//	�p���s��������Ȃ�
			//	CR��CRLF��LF�ōs��
			//	�ŏI�s�ɂ͉��s�R�[�h���Ȃ����Ƃ����邪�C����ȍ~�ɂ͉����Ȃ��̂ŉe�����Ȃ�
			if(
				( linelen - 2 == col && 
				( data[ col + 1 ] == WCODE::CR && data[ col + 2 ] == WCODE::LF )) ||
				( linelen - 1 == col && 
				( data[ col + 1 ] == WCODE::CR || data[ col + 1 ] == WCODE::LF ))
			){
				m_state = STATE_CONTINUE;
				break;
			}
		}
	}
	return linelen;
}


/*! @brief python������(1�s)�𒲍�����

	�^����ꂽ��Ԃ���Python������̏�ԕω���ǂ��C
	�ŏI�I�ȏ�Ԃ����肷��D
	
	������̊J�n�����EnterString()�֐��ŏ����ς݂ł���C���̌��ʂ�
	m_state, m_raw_string, m_long_string, m_quote_char�ɗ^�����Ă���D
	
	m_raw_string��true�Ȃ�backslash�ɂ��G�X�P�[�v�������s��Ȃ�
	m_long_string�Ȃ�m_quote_char��3�����܂ŕ�����ƂȂ�D

	@param[in] data �Ώە�����
	@param[in] linelen �f�[�^�̒���
	@param[in] start_offset �����J�n�ʒu
	
	@return ������̈ʒu
	
	@invariant
		m_state==STATE_STRING

	@author genta
	@date 2007.02.12 �V�K�쐬
	@date 2007.03.23 genta ������̌p���s�̏�����ǉ�

*/
int COutlinePython::ScanString( const wchar_t* data, int linelen, int start_offset )
{
	assert( m_state == STATE_STRING );

	int quote_char = m_quote_char;
	for( int col = start_offset; col < linelen; ++col ){
		int nCharChars = CNativeW::GetSizeOfChar( data, linelen, col );
		if( 1 < nCharChars ){
			col += (nCharChars - 1);
			continue;
		}
		//	raw���[�h�ȊO�ł̓G�X�P�[�v���`�F�b�N
		//	raw���[�h�ł��p���s�̓`�F�b�N
		if( data[ col ] == '\\' && col + 1 < linelen ){
			wchar_t key = data[ col + 1 ];
			if( ! m_raw_string ){
				if( key == L'\\' ||
					key == L'\"' ||
					key == L'\'' ){
					++col;
					//	ignore
					continue;
				}
			}
			if( key == WCODE::CR || key == WCODE::LF ){
				// \r\n���܂Ƃ߂�\n�Ƃ��Ĉ����K�v������
				if( col + 1 >= linelen ||
					data[ col + 2 ] == key ){
					// �{���ɍs��
					++col;
					continue;
				}
				else if( data[ col + 2 ] == WCODE::LF ){
					col += 2;	//	 CRLF
				}
			}
		}
		//	short string + ���s�̏ꍇ�̓G���[���狭�����A
		else if( data[ col ] == WCODE::CR || data[ col ] == WCODE::LF ){
			//���Ƃ�
			if( ! m_long_string ){
				//	������̖����𔭌�����
				m_state = STATE_NORMAL;
				return col + 1;
			}
		}
		//	���p��������������I���`�F�b�N
		else if( data[ col ] == quote_char ){
			if( ! m_long_string ){
				//	������̖����𔭌�����
				m_state = STATE_NORMAL;
				return col + 1;
			}
			//	long string�̏ꍇ
			if( col + 2 < linelen &&
				data[ col + 1 ] == quote_char &&
				data[ col + 2 ] == quote_char ){
				m_state = STATE_NORMAL;
				return col + 3;
			}
		}
	}
	return linelen;
}

/*!	Python��������s���܂ŃX�L�������Ď��̍s�̏�Ԃ����肷��

	m_state�ɐݒ肳�ꂽ���݂̏�Ԃ���J�n����data��start_offset����linelen�ɒB����܂�
	�������C�s���ɂ������Ԃ�m_state�Ɋi�[����D

	���݂̏�Ԃɉ����ăT�u���[�`���ɉ�͏������˗�����D
	�T�u���[�`��Scan**�ł͕�����data��start_offset�����ԑJ�ڂ���������܂ŏ�����
	�����C�ʂ̏�ԂɑJ�ڂ�������ɏ����ς݂̌��ʒu��Ԃ��ďI������D

	���̊֐��ɖ߂�����͍ēx���݂̏�Ԃɉ����ď����˗����s���D������s���ɒB����܂ŌJ��Ԃ��D

	@param[in] data �Ώە�����
	@param[in] linelen �f�[�^�̒���
	@param[in] start_offset �����J�n�ʒu

*/
void COutlinePython::DoScanLine( const wchar_t* data, int linelen, int start_offset )
{
	int col = start_offset;
	while( col < linelen ){
		if( STATE_NORMAL == m_state || STATE_CONTINUE == m_state){
			col = ScanNormal( data, linelen, col );
		}
		else if( STATE_STRING == m_state ){
			col = ScanString( data, linelen, col );
		}
		else {
			//	���肦�Ȃ��G���[
			return;
		}
	}
}


/*!	@brief python�֐����X�g�쐬

	class, def �Ŏn�܂�s���疼�O�𔲂��o���D
	
	class CLASS_NAME( superclass ):
	def FUNCTION_NAME( parameters ):

	������ƃR�����g�����O����K�v������D

	�ʏ�̍s���̏ꍇ�Ɋ֐��E�N���X����Ɠo�^�������s���D
	Python���L�̋󔒂̐��𐔂��ăl�X�g���x���𔻒肷��D
	indent_level��z��Ƃ��ėp���Ă���C�C���f���g���x�����Ƃ̃X�y�[�X�����i�[����D
	�Ȃ��CTAB��8����؂�ւ̈ړ��Ɖ��߂��邱�ƂɂȂ��Ă���D
	
	�ʏ�̍s���łȂ�(�����񒆂���ьp���s)�C���邢�͍s���̏����������
	��ԋ@�B python_analyze_state �ɔ��菈�����˗�����D

	@par ������
	'' "" ����OK
	���p��3�A���Ń����O������
	���O��r��R����������G�X�P�[�v�L���𖳎�(���������s�̃G�X�P�[�v�͗L��)
	
	@par �R�����g
	#�Ŏn�܂�C�s�̌p���͂Ȃ��D

	@date 2007.02.08 genta �V�K�쐬
*/
void CDocOutline::MakeFuncList_python( CFuncInfoArr* pcFuncInfoArr )
{
	CLogicInt	nLineLen;
	CLogicInt	nLineCount;

	COutlinePython python_analyze_state;

	const int MAX_DEPTH = 10;

	int indent_level[ MAX_DEPTH ]; // �e���x���̃C���f���g���ʒu()
	indent_level[0] = 0;	// do as python does.
	int depth_index = 0;

	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		const wchar_t*	pLine;
		int depth;	//	indent depth
		CLogicInt col = CLogicInt(0);	//	current working column position

		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		
		if( python_analyze_state.IsLogicalLineTop() ){
			//	indent check
			//	May 15, 2007 genta ���ʒucol�̓f�[�^�I�t�Z�b�gd�ƓƗ��ɂ��Ȃ���
			//	�������r�����������Ȃ�
			for( depth = 0, col = CLogicInt(0); col < nLineLen; ++col ){
				//	calculate indent level
				if( pLine[col] == L' ' ){
					++depth;
				}
				else if( pLine[col] == L'\t' ){
					depth = ( depth + 8 ) & ~7;
				}
				else {
					break;
				}
			}
			if( pLine[col] == L'\r' ||
				pLine[col] == L'\n' ||
				pLine[col] == L'\0' ||
				pLine[col] == L'#' ){
				//	blank line or comment line are ignored
				continue;
			}
			
			int nItemFuncId = 0;	// topic type
			if( nLineLen - col > CLogicInt(3 + 1) && wcsncmp_literal( pLine + col, L"def" ) == 0 ){
				//	"def"
				nItemFuncId = 1;
				col += CLogicInt(3); // strlen( def )
			}
			else if( nLineLen - col > CLogicInt(5 + 1) && wcsncmp_literal( pLine + col, L"class" ) == 0 ){
				// class
				nItemFuncId = 4;
				col += CLogicInt(5); // strlen( class )
			}
			else {
				python_analyze_state.DoScanLine( pLine, nLineLen, col );
				continue;
			}

			//	2006.02.28 genta ��؂�`�F�b�N
			//	define, classic�����ΏۂɂȂ�Ȃ��悤�ɁC���ɃX�y�[�X���^�u��
			//	�������Ƃ��m�F�D
			//	�{���͌p���s�Ƃ��Ď��̍s�Ɋ֐������������Ƃ����@��͉\����
			//	���G�ɂȂ�̂őΉ����Ȃ��D
			int c = pLine[col];
			if(  c != L' ' && c != L'\t' ){
				python_analyze_state.DoScanLine( pLine, nLineLen, col );
				continue;
			}

			//	adjust current depth level
			//	�֐������̎��s���̂���ɚ��Ɋ֐�������P�[�X���l��
			//	def/class�ȊO�̃C���f���g�͋L�^���Ȃ����������̂�
			//	���o���s�Ɗm�肵�Ă���C���f���g���x���̔�����s��
			int i;
			for( i = depth_index; i >= 0; --i ){
				if( depth == indent_level[ i ] ){
					depth_index = i;
					break;
				}
				else if( depth > indent_level[ i ] && i < MAX_DEPTH - 1 ){
					depth_index = i + 1;
					indent_level[ depth_index ] = depth;
					break;
				}
			}

			//	2007.02.08 genta �蔲���R�����g
			//	�����ɂ́C�����Ōp���s�����邱�Ƃ��\�����C
			//	����ȃ��A�ȃP�[�X�͍l�����Ȃ�
			
			//	skip whitespace
			while( col < nLineLen && C_IsSpace( pLine[col] ))
				++col;

			int w_end;
			for( w_end = col; w_end < nLineLen
					&& Python_IsWordChar( pLine[w_end] ); ++w_end )
				;
			
			//	2007.02.08 genta �蔲���R�����g
			//	�����ɂ́C���̌�Ɋ��ʂɈ͂܂ꂽ�������邢�͌p�����N���X��
			//	���ʂɈ͂܂�ē���C�����:�Ƒ�����
			//	�p���s�̉\��������̂ŁC�����܂Ń`�F�b�N���Ȃ�
			
			//	�����܂łœo�^�v��OK�Ƃ݂Ȃ�
			
			//	���̂�����͎b��

			wchar_t szWord[512];	// �K���ɑ傫�Ȑ�(python�ł͖��O�̒����̏��������̂��H)
			int len = w_end - col;
			
			if( len > 0 ){
				if( len > _countof( szWord ) - 1){
					len = _countof( szWord ) - 1;
				}
				wcsncpy( szWord, pLine + col, len );
				szWord[ len ] = L'\0';
			}
			else {
				wcscpy( szWord, L"���̖���" );
				len = 8;
			}
			if( nItemFuncId == 4  ){
				if( _countof( szWord ) - 8  < len ){
					//	��������ē����
					len = _countof( szWord ) - 8;
				}
				// class
				wcscpy( szWord + len, L" �N���X" );
			}
			
			/*
			  �J�[�\���ʒu�ϊ�
			  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
			  ��
			  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
			*/
			CLayoutPoint ptPosXY;
			m_pcDocRef->m_cLayoutMgr.LogicToLayout(
				CLogicPoint(CLogicInt(0), nLineCount),
				&ptPosXY
			);
			pcFuncInfoArr->AppendData(
				nLineCount + CLogicInt(1),
				ptPosXY.GetY2() + CLayoutInt(1),
				szWord,
				nItemFuncId,
				depth_index
			);
			col = CLogicInt(w_end); // �N���X�E�֐���`�̑����͂�������
		}
		python_analyze_state.DoScanLine( pLine, nLineLen, col );
	}
}


/*! ���[���t�@�C����ǂݍ��݁A���[���\���̂̔z����쐬����

	@date 2002.04.01 YAZAKI
	@date 2002.11.03 Moca ����nMaxCount��ǉ��B�o�b�t�@���`�F�b�N������悤�ɕύX
*/
int CDocOutline::ReadRuleFile( const TCHAR* pszFilename, SOneRule* pcOneRule, int nMaxCount )
{
	long	i;
	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	FILE*	pFile = _tfopen_absini( pszFilename, _T("r") );
	if( NULL == pFile ){
		return 0;
	}
	wchar_t	szLine[LINEREADBUFSIZE];
	const wchar_t*	pszDelimit = L" /// ";
	const wchar_t*	pszKeySeps = L",\0";
	wchar_t*	pszWork;
	int nDelimitLen = wcslen( pszDelimit );
	int nCount = 0;
	while( NULL != fgetws( szLine, _countof(szLine), pFile ) && nCount < nMaxCount ){
		pszWork = wcsstr( szLine, pszDelimit );
		if( NULL != pszWork && szLine[0] != L';' ){
			*pszWork = L'\0';
			pszWork += nDelimitLen;

			/* �ŏ��̃g�[�N�����擾���܂��B */
			wchar_t* pszToken = wcstok( szLine, pszKeySeps );
			while( NULL != pszToken ){
//				nRes = wcsicmp( pszKey, pszToken );
				for( i = 0; i < (int)wcslen(pszWork); ++i ){
					if( pszWork[i] == L'\r' ||
						pszWork[i] == L'\n' ){
						pszWork[i] = L'\0';
						break;
					}
				}
				wcsncpy( pcOneRule[nCount].szMatch, pszToken, 255 );
				wcsncpy( pcOneRule[nCount].szGroupName, pszWork, 255 );
				pcOneRule[nCount].szMatch[255] = L'\0';
				pcOneRule[nCount].szGroupName[255] = L'\0';
				pcOneRule[nCount].nLength = wcslen(pcOneRule[nCount].szMatch);
				nCount++;
				pszToken = wcstok( NULL, pszKeySeps );
			}
		}
	}
	fclose( pFile );
	return nCount;
}

/*! ���[���t�@�C�������ɁA�g�s�b�N���X�g���쐬

	@date 2002.04.01 YAZAKI
	@date 2002.11.03 Moca �l�X�g�̐[�����ő�l�𒴂���ƃo�b�t�@�I�[�o�[��������̂��C��
		�ő�l�ȏ�͒ǉ������ɖ�������
	@date 2007.11.29 kobake SOneRule test[1024] �ŃX�^�b�N�����Ă����̂��C��
*/
void CDocOutline::MakeFuncList_RuleFile( CFuncInfoArr* pcFuncInfoArr )
{
	wchar_t*		pszText;

	/* ���[���t�@�C���̓��e���o�b�t�@�ɓǂݍ��� */
	auto_array_ptr<SOneRule> test = new SOneRule[1024];	// 1024���B 2007.11.29 kobake �X�^�b�N�g�������Ȃ̂ŁA�q�[�v�Ɋm�ۂ���悤�ɏC���B
	int nCount = ReadRuleFile(m_pcDocRef->m_cDocType.GetDocumentAttribute().m_szOutlineRuleFilename, test.get(), 1024 );
	if ( nCount < 1 ){
		return;
	}

	/*	�l�X�g�̐[���́A32���x���܂ŁA�ЂƂ̃w�b�_�́A�Œ�256�����܂ŋ��
		�i256�����܂œ����������瓯�����̂Ƃ��Ĉ����܂��j
	*/
	const int	nMaxStack = 32;	//	�l�X�g�̍Ő[
	int			nDepth = 0;				//	���܂̃A�C�e���̐[����\�����l�B
	wchar_t		pszStack[nMaxStack][256];
	wchar_t		szTitle[256];			//	�ꎞ�̈�
	for( CLogicInt nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount )
	{
		//�s�擾
		CLogicInt		nLineLen;
		const wchar_t*	pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine ){
			break;
		}

		//�s���̋󔒔�΂�
		int		i;
		for( i = 0; i < nLineLen; ++i ){
			if( pLine[i] == L' ' || pLine[i] == L'\t' || pLine[i] == L'�@'){
				continue;
			}
			break;
		}
		if( i >= nLineLen ){
			continue;
		}

		//�擪���������o���L���̂����ꂩ�ł���΁A���֐i��
		int		j;
		for( j = 0; j < nCount; j++ ){
			if ( 0 == wcsncmp( &pLine[i], test[j].szMatch, test[j].nLength ) ){
				wcscpy( szTitle, test[j].szGroupName );
				break;
			}
		}
		if( j >= nCount ){
			continue;
		}

		/*	���[���Ƀ}�b�`�����s�́A�A�E�g���C�����ʂɕ\������B
		*/

		//�s�����񂩂���s����菜�� pLine -> pszText
		pszText = new wchar_t[nLineLen + 1];
		wmemcpy( pszText, &pLine[i], nLineLen );
		pszText[nLineLen] = L'\0';
		int nTextLen = wcslen( pszText );
		for( i = 0; i < nTextLen; ++i ){
			if( pszText[i] == WCODE::CR || pszText[i] == WCODE::LF ){
				pszText[i] = L'\0';
				break;
			}
		}

		/*
		  �J�[�\���ʒu�ϊ�
		  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		  ��
		  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		*/
		CLayoutPoint ptPos;
		m_pcDocRef->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(0, nLineCount),
			&ptPos
		);

		/* nDepth���v�Z */
		int k;
		BOOL bAppend;
		bAppend = TRUE;
		for ( k = 0; k < nDepth; k++ ){
			int nResult = wcscmp( pszStack[k], szTitle );
			if ( nResult == 0 ){
				break;
			}
		}
		if ( k < nDepth ){
			//	���[�v�r����break;���Ă����B�����܂łɓ������o�������݂��Ă����B
			//	�̂ŁA�������x���ɍ��킹��AppendData.
			nDepth = k;
		}
		else if( nMaxStack> k ){
			//	���܂܂łɓ������o�������݂��Ȃ������B
			//	�̂ŁApszStack�ɃR�s�[����AppendData.
			wcscpy(pszStack[nDepth], szTitle);
		}else{
			// 2002.11.03 Moca �ő�l�𒴂���ƃo�b�t�@�I�[�o�[�������邩��K������
			// nDepth = nMaxStack;
			bAppend = FALSE;
		}
		
		if( FALSE != bAppend ){
			pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1) , pszText, 0, nDepth );
			nDepth++;
		}
		delete [] pszText;

	}
	return;
}



/*! COBOL �A�E�g���C����� */
void CDocOutline::MakeTopicList_cobol( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int				i;
	int				k;
	wchar_t			szDivision[1024];
	wchar_t			szLabel[1024];
	const wchar_t*	pszKeyWord;
	int				nKeyWordLen;
	BOOL			bDivision;

	szDivision[0] = L'\0';
	szLabel[0] =  L'\0';


	CLogicInt	nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine ){
			break;
		}
		/* �R�����g�s�� */
		if( 7 <= nLineLen && pLine[6] == L'*' ){
			continue;
		}
		/* ���x���s�� */
		if( 8 <= nLineLen && pLine[7] != L' ' ){
			k = 0;
			for( i = 7; i < nLineLen; ){
				if( pLine[i] == '.'
				 || pLine[i] == WCODE::CR
				 || pLine[i] == WCODE::LF
				){
					break;
				}
				szLabel[k] = pLine[i];
				++k;
				++i;
				if( pLine[i - 1] == L' ' ){
					for( ; i < nLineLen; ++i ){
						if( pLine[i] != L' ' ){
							break;
						}
					}
				}
			}
			szLabel[k] = L'\0';
//			MYTRACE_A( "szLabel=[%ls]\n", szLabel );



			pszKeyWord = L"division";
			nKeyWordLen = wcslen( pszKeyWord );
			bDivision = FALSE;
			for( i = 0; i <= (int)wcslen( szLabel ) - nKeyWordLen; ++i ){
				if( 0 == auto_memicmp( &szLabel[i], pszKeyWord, nKeyWordLen ) ){
					szLabel[i + nKeyWordLen] = L'\0';
					wcscpy( szDivision, szLabel );
					bDivision = TRUE;
					break;
				}
			}
			if( bDivision ){
				continue;
			}
			/*
			  �J�[�\���ʒu�ϊ�
			  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
			  ��
			  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
			*/

			CLayoutPoint ptPos;
			wchar_t	szWork[1024];
			m_pcDocRef->m_cLayoutMgr.LogicToLayout(
				CLogicPoint(0, nLineCount),
				&ptPos
			);
			auto_sprintf( szWork, L"%ls::%ls", szDivision, szLabel );
			pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1) , szWork, 0 );
		}
	}
	return;
}


/*! �A�Z���u�� �A�E�g���C�����

	@author MIK
	@date 2004.04.12 ��蒼��
*/
void CDocOutline::MakeTopicList_asm( CFuncInfoArr* pcFuncInfoArr )
{
	CLogicInt nTotalLine;

	nTotalLine = m_pcDocRef->m_cDocLineMgr.GetLineCount();

	for( CLogicInt nLineCount = CLogicInt(0); nLineCount < nTotalLine; nLineCount++ ){
		const WCHAR* pLine;
		CLogicInt nLineLen;
		WCHAR* pTmpLine;
		int length;
		int offset;
#define MAX_ASM_TOKEN 2
		WCHAR* token[MAX_ASM_TOKEN];
		int j;
		WCHAR* p;

		//1�s�擾����B
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( pLine == NULL ) break;

		//��Ɨp�ɃR�s�[���쐬����B�o�C�i�����������炻�̌��͒m��Ȃ��B
		pTmpLine = wcsdup( pLine );
		if( pTmpLine == NULL ) break;
		if( wcslen( pTmpLine ) >= (unsigned int)nLineLen ){	//�o�C�i�����܂�ł�����Z���Ȃ�̂�...
			pTmpLine[ nLineLen ] = L'\0';	//�w�蒷�Ő؂�l��
		}

		//�s�R�����g�폜
		p = wcsstr( pTmpLine, L";" );
		if( p ) *p = L'\0';

		length = wcslen( pTmpLine );
		offset = 0;

		//�g�[�N���ɕ���
		for( j = 0; j < MAX_ASM_TOKEN; j++ ) token[ j ] = NULL;
		for( j = 0; j < MAX_ASM_TOKEN; j++ ){
			token[ j ] = my_strtok<WCHAR>( pTmpLine, length, &offset, L" \t\r\n" );
			if( token[ j ] == NULL ) break;
			//�g�[�N���Ɋ܂܂��ׂ������łȂ����H
			if( wcsstr( token[ j ], L"\"") != NULL
			 || wcsstr( token[ j ], L"\\") != NULL
			 || wcsstr( token[ j ], L"'" ) != NULL ){
				token[ j ] = NULL;
				break;
			}
		}

		if( token[ 0 ] != NULL ){	//�g�[�N����1�ȏ゠��
			int nFuncId = -1;
			WCHAR* entry_token = NULL;

			length = wcslen( token[ 0 ] );
			if( length >= 2
			 && token[ 0 ][ length - 1 ] == L':' ){	//���x��
				token[ 0 ][ length - 1 ] = L'\0';
				nFuncId = 51;
				entry_token = token[ 0 ];
			}
			else if( token[ 1 ] != NULL ){	//�g�[�N����2�ȏ゠��
				if( wcsicmp( token[ 1 ], L"proc" ) == 0 ){	//�֐�
					nFuncId = 50;
					entry_token = token[ 0 ];
				}else
				if( wcsicmp( token[ 1 ], L"endp" ) == 0 ){	//�֐��I��
					nFuncId = 52;
					entry_token = token[ 0 ];
				//}else
				//if( my_stricmp( token[ 1 ], _T("macro") ) == 0 ){	//�}�N��
				//	nFuncId = -1;
				//	entry_token = token[ 0 ];
				//}else
				//if( my_stricmp( token[ 1 ], _T("struc") ) == 0 ){	//�\����
				//	nFuncId = -1;
				//	entry_token = token[ 0 ];
				}
			}

			if( nFuncId >= 0 ){
				/*
				  �J�[�\���ʒu�ϊ�
				  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
				  ��
				  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
				*/
				CLayoutPoint ptPos;
				m_pcDocRef->m_cLayoutMgr.LogicToLayout(
					CLogicPoint(0, nLineCount),
					&ptPos
				);
				pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1), entry_token, nFuncId );
			}
		}

		free( pTmpLine );
	}

	return;
}



/*! �K�w�t���e�L�X�g �A�E�g���C�����

	@author zenryaku
	@date 2003.05.20 zenryaku �V�K�쐬
	@date 2003.05.25 genta �������@�ꕔ�C��
	@date 2003.06.21 Moca �K�w��2�i�ȏ�[���Ȃ�ꍇ���l��
*/
void CDocOutline::MakeTopicList_wztxt(CFuncInfoArr* pcFuncInfoArr)
{
	int levelPrev = 0;

	for(CLogicInt nLineCount=CLogicInt(0);nLineCount<m_pcDocRef->m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		const wchar_t*	pLine;
		CLogicInt		nLineLen;

		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if(!pLine)
		{
			break;
		}
		//	May 25, 2003 genta ���菇���ύX
		if( *pLine == L'.' )
		{
			const wchar_t* pPos;	//	May 25, 2003 genta
			int			nLength;
			wchar_t		szTitle[1024];

			//	�s���I�h�̐����K�w�̐[���𐔂���
			for( pPos = pLine + 1 ; *pPos == L'.' ; ++pPos )
				;

			CLayoutPoint ptPos;
			m_pcDocRef->m_cLayoutMgr.LogicToLayout(
				CLogicPoint(0, nLineCount),
				&ptPos
			);
			
			int level = pPos - pLine;

			// 2003.06.27 Moca �K�w��2�i�ʏ�[���Ȃ�Ƃ��́A����̗v�f��ǉ�
			if( levelPrev < level && level != levelPrev + 1  ){
				int dummyLevel;
				// (����)��}��
				//	�������CTAG�ꗗ�ɂ͏o�͂���Ȃ��悤��
				for( dummyLevel = levelPrev + 1; dummyLevel < level; dummyLevel++ ){
					pcFuncInfoArr->AppendData(
						nLineCount+CLogicInt(1),
						ptPos.GetY2()+CLayoutInt(1),
						L"(����)",
						FUNCINFO_NOCLIPTEXT,
						dummyLevel - 1
					);
				}
			}
			levelPrev = level;

			nLength = auto_sprintf(szTitle,L"%d - ", level );
			
			wchar_t *pDest = szTitle + nLength; // �������ݐ�
			wchar_t *pDestEnd = szTitle + _countof(szTitle) - 2;
			
			while( pDest < pDestEnd )
			{
				if( *pPos ==L'\r' || *pPos ==L'\n' || *pPos == L'\0')
				{
					break;
				}
				else {
					*pDest++ = *pPos++;
				}
			}
			*pDest = L'\0';
			pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1),szTitle, 0, level - 1);
		}
	}
}

/*! HTML �A�E�g���C�����

	@author zenryaku
	@date 2003.05.20 zenryaku �V�K�쐬
	@date 2004.04.19 zenryaku ��v�f�𔻒�
	@date 2004.04.20 Moca �R�����g�����ƁA�s���ȏI���^�O�𖳎����鏈����ǉ�
*/
void CDocOutline::MakeTopicList_html(CFuncInfoArr* pcFuncInfoArr)
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int				i;
	int				j;
	int				k;
	BOOL			bEndTag;
	BOOL			bCommentTag = FALSE;

	/*	�l�X�g�̐[���́AnMaxStack���x���܂ŁA�ЂƂ̃w�b�_�́A�Œ�32�����܂ŋ��
		�i32�����܂œ����������瓯�����̂Ƃ��Ĉ����܂��j
	*/
	const int nMaxStack = 32;	//	�l�X�g�̍Ő[
	int nDepth = 0;				//	���܂̃A�C�e���̐[����\�����l�B
	wchar_t pszStack[nMaxStack][32];
	wchar_t szTitle[32];			//	�ꎞ�̈�
	CLogicInt			nLineCount;
	for(nLineCount=CLogicInt(0);nLineCount<m_pcDocRef->m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		pLine	=	m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if(!pLine)
		{
			break;
		}
		for(i=0;i<nLineLen-1;i++)
		{
			// 2004.04.20 Moca �R�����g����������
			if( bCommentTag )
			{
				if( i < nLineLen - 3 && 0 == wmemcmp( L"-->", pLine + i , 3 ) )
				{
					bCommentTag = FALSE;
					i += 2;
				}
				continue;
			}
			// 2004.04.20 Moca To Here
			if(pLine[i]!=L'<' || nDepth>=nMaxStack)
			{
				continue;
			}
			bEndTag	=	FALSE;
			if(pLine[++i]==L'/')
			{
				i++;
				bEndTag	=	TRUE;
			}
			for(j=0;i+j<nLineLen && j<_countof(szTitle)-1;j++)
			{
				if((pLine[i+j]<L'a' || pLine[i+j]>L'z') &&
					(pLine[i+j]<L'A' || pLine[i+j]>L'Z') &&
					!(j!=0 && pLine[i+j]>=L'0' && pLine[i+j]<=L'9'))
				{
					break;
				}
				szTitle[j]	=	pLine[i+j];
			}
			if(j==0)
			{
				// 2004.04.20 Moca From Here �R�����g����������
				if( i < nLineLen - 3 && 0 == wmemcmp( L"!--", pLine + i, 3 ) )
				{
					bCommentTag = TRUE;
					i += 3;
				}
				// 2004.04.20 Moca To Here
				continue;
			}
			szTitle[j]	=	'\0';
			if(bEndTag)
			{
				int nDepthOrg = nDepth; // 2004.04.20 Moca �ǉ�
				// �I���^�O
				while(nDepth>0)
				{
					nDepth--;
					if(!wcsicmp(pszStack[nDepth],szTitle))
					{
						break;
					}
				}
				// 2004.04.20 Moca �c���[���ƈ�v���Ȃ��Ƃ��́A���̏I���^�O�͖���
				if( nDepth == 0 )
				{
					if(wcsicmp(pszStack[nDepth],szTitle))
					{
						nDepth = nDepthOrg;
					}
				}
			}
			else
			{
				if(wcsicmp(szTitle,L"br") && wcsicmp(szTitle,L"area") &&
					wcsicmp(szTitle,L"base") && wcsicmp(szTitle,L"frame") && wcsicmp(szTitle,L"param"))
				{
					CLayoutPoint ptPos;

					m_pcDocRef->m_cLayoutMgr.LogicToLayout(
						CLogicPoint(i, nLineCount),
						&ptPos
					);

					if(wcsicmp(szTitle,L"hr") && wcsicmp(szTitle,L"meta") && wcsicmp(szTitle,L"link") &&
						wcsicmp(szTitle,L"input") && wcsicmp(szTitle,L"img") && wcsicmp(szTitle,L"area") &&
						wcsicmp(szTitle,L"base") && wcsicmp(szTitle,L"frame") && wcsicmp(szTitle,L"param"))
					{
						// �I���^�O�Ȃ��������S�Ẵ^�O�炵�����̂𔻒�
						wcscpy(pszStack[nDepth],szTitle);
						k	=	j;
						if(j<_countof(szTitle)-3)
						{
							for(;i+j<nLineLen;j++)
							{
								if(pLine[i+j]==L'/' && pLine[i+j+1]==L'>')
								{
									bEndTag	=	TRUE;
									break;
								}
								else if(pLine[i+j]==L'>')
								{
									break;
								}
							}
							if(!bEndTag)
							{
								szTitle[k++]	=	L' ';
								for(j-=k-1;i+j+k<nLineLen && k<_countof(szTitle)-1;k++)
								{
									if(pLine[i+j+k]==L'<' || pLine[i+j+k]==L'\r' || pLine[i+j+k]==L'\n')
									{
										break;
									}
									szTitle[k]	=	pLine[i+j+k];
								}
							j += k-1;
							}
						}
						szTitle[k]	=	L'\0';
						pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1),szTitle,0,(bEndTag ? nDepth : nDepth++));
					}
					else
					{
						for(;i+j<nLineLen && j<_countof(szTitle)-1;j++)
						{
							if(pLine[i+j]=='>')
							{
								break;
							}
							szTitle[j]	=	pLine[i+j];
						}
						szTitle[j]	=	L'\0';
						pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1),szTitle,0,nDepth);
					}
				}
			}
			i	+=	j;
		}
	}
}

/*! TeX �A�E�g���C�����

	@author naoh
	@date 2003.07.21 naoh �V�K�쐬
	@date 2005.01.03 naoh �u�}�v�Ȃǂ�"}"���܂ޕ����ɑ΂���C���Aprosper��slide�ɑΉ�
*/
void CDocOutline::MakeTopicList_tex(CFuncInfoArr* pcFuncInfoArr)
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int				i;
	int				j;
	int				k;

	const int nMaxStack = 8;	//	�l�X�g�̍Ő[
	int nDepth = 0;				//	���܂̃A�C�e���̐[����\�����l�B
	wchar_t szTag[32], szTitle[256];			//	�ꎞ�̈�
	int thisSection=0, lastSection = 0;	// ���݂̃Z�N�V������ނƈ�O�̃Z�N�V�������
	int stackSection[nMaxStack];		// �e�[���ł̃Z�N�V�����̔ԍ�
	int nStartTitlePos;					// \section{dddd} �� dddd �̕����̎n�܂�ԍ�
	int bNoNumber;						// * �t�̏ꍇ�̓Z�N�V�����ԍ���t���Ȃ�

	// ��s����
	CLogicInt	nLineCount;
	for(nLineCount=CLogicInt(0);nLineCount<m_pcDocRef->m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		pLine	=	m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if(!pLine) break;
		// �ꕶ������
		for(i=0;i<nLineLen-1;i++)
		{
			if(pLine[i] == L'%') break;	// �R�����g�Ȃ�ȍ~�͂���Ȃ�
			if(nDepth>=nMaxStack)continue;
			if(pLine[i] != L'\\')continue;	// �u\�v���Ȃ��Ȃ玟�̕�����
			++i;
			// ���������u\�v�ȍ~�̕�����`�F�b�N
			for(j=0;i+j<nLineLen && j<_countof(szTag)-1;j++)
			{
				if(pLine[i+j] == L'{' && !(i+j>0 && _IS_SJIS_1((unsigned char)pLine[i+j-1])) ) {	// SJIS1�`�F�b�N
					bNoNumber = (pLine[i+j-1] == '*');
					nStartTitlePos = j+i+1;
					break;
				}
				szTag[j] = pLine[i+j];
			}
			if(j==0) continue;
			if(bNoNumber){
				szTag[j-1] = L'\0';
			}else{
				szTag[j]   = L'\0';
			}
//			MessageBoxA(NULL, szTitle, L"", MB_OK);

			thisSection = 0;
			if(!wcscmp(szTag,L"subsubsection")) thisSection = 4;
			else if(!wcscmp(szTag,L"subsection")) thisSection = 3;
			else if(!wcscmp(szTag,L"section")) thisSection = 2;
			else if(!wcscmp(szTag,L"chapter")) thisSection = 1;
			else if(!wcscmp(szTag,L"begin")) {		// begin�Ȃ� prosper��slide�̉\�����l��
				// �����{slide}{}�܂œǂ݂Ƃ��Ă���
				if(wcsstr(pLine, L"{slide}")){
					k=0;
					for(j=nStartTitlePos+1;i+j<nLineLen && j<_countof(szTag)-1;j++)
					{
						if(pLine[i+j] == '{' && !(i+j>0 && _IS_SJIS_1((unsigned char)pLine[i+j-1])) ) {	// SJIS1�`�F�b�N
							nStartTitlePos = j+i+1;
							break;
						}
						szTag[k++]	=	pLine[i+j];
					}
					szTag[k] = '\0';
					thisSection = 1;
				}
			}

			if( thisSection > 0)
			{
				// section�̒��g�擾
				for(k=0;nStartTitlePos+k<nLineLen && k<_countof(szTitle)-1;k++)
				{
					if(_IS_SJIS_1((unsigned char)pLine[k+nStartTitlePos])) {
						szTitle[k] = pLine[k+nStartTitlePos];
						k++;	// ���̓`�F�b�N�s�v
					} else if(pLine[k+nStartTitlePos] == '}') {
						break;
					}
					szTitle[k] = pLine[k+nStartTitlePos];
				}
				szTitle[k] = '\0';

				CLayoutPoint ptPos;

				WCHAR tmpstr[256];
				WCHAR secstr[4];

				m_pcDocRef->m_cLayoutMgr.LogicToLayout(
					CLogicPoint(i, nLineCount),
					&ptPos
				);

				int sabunSection = thisSection - lastSection;
				if(lastSection == 0){
					nDepth = 0;
					stackSection[0] = 1;
				}else{
					nDepth += sabunSection;
					if(sabunSection > 0){
						if(nDepth >= nMaxStack) nDepth=nMaxStack-1;
						stackSection[nDepth] = 1;
					}else{
						if(nDepth < 0) nDepth=0;
						++stackSection[nDepth];
					}
				}
				tmpstr[0] = L'\0';
				if(!bNoNumber){
					for(k=0; k<=nDepth; k++){
						auto_sprintf(secstr, L"%d.", stackSection[k]);
						wcscat(tmpstr, secstr);
					}
					wcscat(tmpstr, L" ");
				}
				wcscat(tmpstr, szTitle);
				pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1), tmpstr, 0, nDepth);
				if(!bNoNumber) lastSection = thisSection;
			}
			i	+=	j;
		}
	}
}






/*! PL/SQL�֐����X�g�쐬 */
void CDocOutline::MakeFuncList_PLSQL( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int			i;
	int			nCharChars;
	wchar_t		szWordPrev[100];
	wchar_t		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;
	wchar_t		szFuncName[80];
	CLogicInt	nFuncLine;
	int			nFuncId;
	int			nFuncNum;
	int			nFuncOrProc = 0;
	int			nParseCnt = 0;

	szWordPrev[0] = L'\0';
	szWord[nWordIdx] = L'\0';
	nMode = 0;
	nFuncNum = 0;
	CLogicInt	nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL( &nLineLen );
		for( i = 0; i < nLineLen; ++i ){
			/* 1�o�C�g������������������ */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
			if( 0 == nCharChars ){
				nCharChars = 1;
			}
//			if( 1 < nCharChars ){
//				i += (nCharChars - 1);
//				continue;
//			}
			/* �V���O���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( 20 == nMode ){
				if( L'\'' == pLine[i] ){
					if( i + 1 < nLineLen && L'\'' == pLine[i + 1] ){
						++i;
					}else{
						nMode = 0;
						continue;
					}
				}else{
				}
			}else
			/* �R�����g�ǂݍ��ݒ� */
			if( 8 == nMode ){
				if( i + 1 < nLineLen && L'*' == pLine[i] &&  L'/' == pLine[i + 1] ){
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* �P��ǂݍ��ݒ� */
			if( 1 == nMode ){
				if( (1 == nCharChars && (
					L'_' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )
					) )
				 || 2 == nCharChars
				){
//					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);
					}
				}
				else{
					if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"FUNCTION" ) ){
						nFuncOrProc = 1;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"PROCEDURE" ) ){
						nFuncOrProc = 2;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"PACKAGE" ) ){
						nFuncOrProc = 3;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 1 == nParseCnt && 3 == nFuncOrProc && 0 == wcsicmp( szWord, L"BODY" ) ){
						nFuncOrProc = 4;
						nParseCnt = 1;
					}
					else if( 1 == nParseCnt ){
						if( 1 == nFuncOrProc ||
							2 == nFuncOrProc ||
							3 == nFuncOrProc ||
							4 == nFuncOrProc ){
							++nParseCnt;
							wcscpy( szFuncName, szWord );
						}else
						if( 3 == nFuncOrProc ){

						}
					}else
					if( 2 == nParseCnt ){
						if( 0 == wcsicmp( szWord, L"IS" ) ){
							if( 1 == nFuncOrProc ){
								nFuncId = 11;	/* �t�@���N�V�����{�� */
							}else
							if( 2 == nFuncOrProc ){
								nFuncId = 21;	/* �v���V�[�W���{�� */
							}else
							if( 3 == nFuncOrProc ){
								nFuncId = 31;	/* �p�b�P�[�W�d�l�� */
							}else
							if( 4 == nFuncOrProc ){
								nFuncId = 41;	/* �p�b�P�[�W�{�� */
							}
							++nFuncNum;
							/*
							  �J�[�\���ʒu�ϊ�
							  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
							  ��
							  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
							*/
							CLayoutPoint ptPos;
							m_pcDocRef->m_cLayoutMgr.LogicToLayout(
								CLogicPoint(0, nFuncLine - 1),
								&ptPos
							);
							pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1), szFuncName, nFuncId );
							nParseCnt = 0;
						}
						if( 0 == wcsicmp( szWord, L"AS" ) ){
							if( 3 == nFuncOrProc ){
								nFuncId = 31;	/* �p�b�P�[�W�d�l�� */
								++nFuncNum;
								/*
								  �J�[�\���ʒu�ϊ�
								  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
								  ��
								  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
								*/
								CLayoutPoint ptPos;
								m_pcDocRef->m_cLayoutMgr.LogicToLayout(
									CLogicPoint(0, nFuncLine - 1),
									&ptPos
								);
								pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1) , szFuncName, nFuncId );
								nParseCnt = 0;
							}
							else if( 4 == nFuncOrProc ){
								nFuncId = 41;	/* �p�b�P�[�W�{�� */
								++nFuncNum;
								/*
								  �J�[�\���ʒu�ϊ�
								  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
								  ��
								  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
								*/
								CLayoutPoint ptPos;
								m_pcDocRef->m_cLayoutMgr.LogicToLayout(
									CLogicPoint(0, nFuncLine - 1),
									&ptPos
								);
								pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1) , szFuncName, nFuncId );
								nParseCnt = 0;
							}
						}
					}
					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = L'\0';
					nMode = 0;
					i--;
					continue;
				}
			}else
			/* �L����ǂݍ��ݒ� */
			if( 2 == nMode ){
				if( L'_' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					L'\t' == pLine[i] ||
					 L' ' == pLine[i] ||
					 WCODE::CR == pLine[i] ||
					 WCODE::LF == pLine[i] ||
					 L'{' == pLine[i] ||
					 L'}' == pLine[i] ||
					 L'(' == pLine[i] ||
					 L')' == pLine[i] ||
					 L';' == pLine[i] ||
					L'\'' == pLine[i] ||
					 L'/' == pLine[i] ||
					 L'-' == pLine[i]
				){
					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = L'\0';
					nMode = 0;
					i--;
					continue;
				}else{
//					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);
					}
				}
			}else
			/* ���߂���P�ꖳ���� */
			if( 999 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					nMode = 0;
					continue;
				}
			}else
			/* �m�[�}�����[�h */
			if( 0 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen - 1 && L'-' == pLine[i] &&  L'-' == pLine[i + 1] ){
					break;
				}else
				if( i < nLineLen - 1 && L'/' == pLine[i] &&  L'*' == pLine[i + 1] ){
					++i;
					nMode = 8;
					continue;
				}else
				if( L'\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( L';' == pLine[i] ){
					if( 2 == nParseCnt ){
						if( 1 == nFuncOrProc ){
							nFuncId = 10;	/* �t�@���N�V�����錾 */
						}else{
							nFuncId = 20;	/* �v���V�[�W���錾 */
						}
						++nFuncNum;
						/*
						  �J�[�\���ʒu�ϊ�
						  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
						  ��
						  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						*/
						CLayoutPoint ptPos;
						m_pcDocRef->m_cLayoutMgr.LogicToLayout(
							CLogicPoint(0, nFuncLine - 1),
							&ptPos
						);
						pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1) , szFuncName, nFuncId );
						nParseCnt = 0;
					}
					nMode = 0;
					continue;
				}else{
					if( (1 == nCharChars && (
						L'_' == pLine[i] ||
						L'~' == pLine[i] ||
						(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
						(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
						(L'0' <= pLine[i] &&	pLine[i] <= L'9' )
						) )
					 || 2 == nCharChars
					){
						wcscpy( szWordPrev, szWord );
						nWordIdx = 0;

//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);

						nMode = 1;
					}else{
						wcscpy( szWordPrev, szWord );
						nWordIdx = 0;
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';

						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);

						nMode = 2;
					}
				}
			}
			i += (nCharChars - 1);
		}
	}
	return;
}





/*!	�e�L�X�g�E�g�s�b�N���X�g�쐬
	
	@date 2002.04.01 YAZAKI CDlgFuncList::SetText()���g�p����悤�ɉ����B
	@date 2002.11.03 Moca	�K�w���ő�l�𒴂���ƃo�b�t�@�I�[�o�[��������̂��C��
							�ő�l�ȏ�͒ǉ������ɖ�������
	@date 2007.8��   kobake �@�B�I��UNICODE��
	@date 2007.11.29 kobake UNICODE�Ή��ł��ĂȂ������̂ŏC��
*/
void CDocOutline::MakeTopicList_txt( CFuncInfoArr* pcFuncInfoArr )
{
	using namespace WCODE;

	//���o���L��
	const wchar_t*	pszStarts = GetDllShareData().m_Common.m_sFormat.m_szMidashiKigou;
	int				nStartsLen = wcslen( pszStarts );

	/*	�l�X�g�̐[���́AnMaxStack���x���܂ŁA�ЂƂ̃w�b�_�́A�Œ�32�����܂ŋ��
		�i32�����܂œ����������瓯�����̂Ƃ��Ĉ����܂��j
	*/
	const int nMaxStack = 32;	//	�l�X�g�̍Ő[
	int nDepth = 0;				//	���܂̃A�C�e���̐[����\�����l�B
	wchar_t pszStack[nMaxStack][32];
	wchar_t szTitle[32];			//	�ꎞ�̈�
	CLogicInt				nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount )
	{
		//�s�擾
		CLogicInt		nLineLen;
		const wchar_t*	pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine )break;

		//�s���̋󔒔�΂�
		int i;
		for( i = 0; i < nLineLen; ++i ){
			if( WCODE::isBlank(pLine[i]) ){
				continue;
			}
			break;
		}
		if( i >= nLineLen ){
			continue;
		}

		//�擪���������o���L���̂����ꂩ�ł���΁A���֐i��
		if(NULL==wcschr(pszStarts,pLine[0]))continue;

		//���o����ނ̔��� -> szTitle
		if( pLine[i] == L'(' ){
			if(0){}
			else if ( IsInRange(pLine[i + 1], L'0', L'9') ) wcscpy( szTitle, L"(0)" ); //����
			else if ( IsInRange(pLine[i + 1], L'A', L'Z') ) wcscpy( szTitle, L"(A)" ); //�p�啶��
			else if ( IsInRange(pLine[i + 1], L'a', L'z') ) wcscpy( szTitle, L"(a)" ); //�p������
			else continue; //���u(�v�̎����p�����Ŗ����ꍇ�A���o���Ƃ݂Ȃ��Ȃ�
		}
		else if( IsInRange(pLine[i], L'�O', L'�X') ) wcscpy( szTitle, L"�O" ); // �S�p����
		else if( IsInRange(pLine[i], L'�@', L'�S') ) wcscpy( szTitle, L"�@" ); // �@�`�S
		else if( IsInRange(pLine[i], L'�T', L'�]') ) wcscpy( szTitle, L"�T" ); // �T�`�]
		else if( wcschr(L"�Z���O�l�ܘZ������\�S����Q��", pLine[i]) ) wcscpy( szTitle, L"��" ); //������
		else{
			szTitle[0]=pLine[i];
			szTitle[1]=L'\0';
		}

		/*	�u���o���L���v�Ɋ܂܂�镶���Ŏn�܂邩�A
			(0�A(1�A...(9�A(A�A(B�A...(Z�A(a�A(b�A...(z
			�Ŏn�܂�s�́A�A�E�g���C�����ʂɕ\������B
		*/

		//�s�����񂩂���s����菜�� pLine -> pszText
		wchar_t*	pszText = new wchar_t[nLineLen + 1];
		wmemcpy( pszText, &pLine[i], nLineLen );
		pszText[nLineLen] = L'\0';
		for( i = 0; i < (int)wcslen(pszText); ++i ){
			if( pszText[i] == CR || pszText[i] == LF )pszText[i] = L'\0';
		}

		/*
		  �J�[�\���ʒu�ϊ�
		  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		  ��
		  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		*/
		CLayoutPoint ptPos;
		m_pcDocRef->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(0, nLineCount),
			&ptPos
		);

		/* nDepth���v�Z */
		int k;
		bool bAppend = true;
		for ( k = 0; k < nDepth; k++ ){
			int nResult = wcscmp( pszStack[k], szTitle );
			if ( nResult == 0 ){
				break;
			}
		}
		if ( k < nDepth ){
			//	���[�v�r����break;���Ă����B�����܂łɓ������o�������݂��Ă����B
			//	�̂ŁA�������x���ɍ��킹��AppendData.
			nDepth = k;
		}
		else if( nMaxStack > k ){
			//	���܂܂łɓ������o�������݂��Ȃ������B
			//	�̂ŁApszStack�ɃR�s�[����AppendData.
			wcscpy(pszStack[nDepth], szTitle);
		}
		else{
			// 2002.11.03 Moca �ő�l�𒴂���ƃo�b�t�@�I�[�o�[����
			// nDepth = nMaxStack;
			bAppend = false;
		}
		
		if( bAppend ){
			pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1) , pszText, 0, nDepth );
			nDepth++;
		}
		delete [] pszText;

	}
	return;
}



/* Java�֐����X�g�쐬 */
void CDocOutline::MakeFuncList_Java( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt	nLineLen;
	int			i;
	int			nNestLevel;
	int			nCharChars;
	wchar_t		szWordPrev[100];
	wchar_t		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;
	wchar_t		szFuncName[80];
	CLogicInt	nFuncLine = CLogicInt(0);
	int			nFuncId;
	int			nFuncNum;
	wchar_t		szClass[1024];

	int			nClassNestArr[16];
	int			nClassNestArrNum;
	int			nNestLevel2Arr[16];

	nNestLevel = 0;
	szWordPrev[0] = L'\0';
	szWord[nWordIdx] = L'\0';
	nMode = 0;
	nNestLevel2Arr[0] = 0;
	nFuncNum = 0;
	szClass[0] = L'\0';
	nClassNestArrNum = 0;
	CLogicInt		nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		for( i = 0; i < nLineLen; ++i ){
			/* 1�o�C�g������������������ */
			//nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
			if(	1 < nCharChars ){
				i += (nCharChars - 1);
				continue;
			}

			/* �G�X�P�[�v�V�[�P���X�͏�Ɏ�菜�� */
			if( L'\\' == pLine[i] ){
				++i;
			}else
			/* �V���O���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( 20 == nMode ){
				if( L'\'' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* �_�u���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( 21 == nMode ){
				if( L'"' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* �R�����g�ǂݍ��ݒ� */
			if( 8 == nMode ){
				if( i < nLineLen - 1 && L'*' == pLine[i] &&  L'/' == pLine[i + 1] ){
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}
			/* �P��ǂݍ��ݒ� */
			else if( 1 == nMode ){
				if( L'_' == pLine[i] ||
					L':' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					L'.' == pLine[i]
				){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
					}
				}
else{
					/* �N���X�錾������������ */
					//	Oct. 10, 2002 genta interface���Ώۂ�
					if( 0 == wcscmp( L"class", szWordPrev ) ||
						0 == wcscmp( L"interface", szWordPrev )
					 ){
						nClassNestArr[nClassNestArrNum] = nNestLevel;
						++nClassNestArrNum;
						if( 0 < nNestLevel	){
							wcscat( szClass, L"\\" );
						}
						wcscat( szClass, szWord );

						nFuncId = 0;
						++nFuncNum;
						/*
						  �J�[�\���ʒu�ϊ�
						  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
						  ��
						  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						*/
						CLogicPoint  ptPosXY_Logic = CLogicPoint(CLogicInt(0), nLineCount);
						CLayoutPoint ptPosXY_Layout;
						m_pcDocRef->m_cLayoutMgr.LogicToLayout(
							ptPosXY_Logic,
							&ptPosXY_Layout
						);
						wchar_t szWork[256];
						auto_sprintf( szWork, L"%ls::%ls", szClass, L"��`�ʒu" );
						pcFuncInfoArr->AppendData( ptPosXY_Logic.GetY2() + CLogicInt(1), ptPosXY_Layout.GetY2() + CLayoutInt(1), szWork, nFuncId ); //2007.10.09 kobake ���C�A�E�g�E���W�b�N�̍��݃o�O�C��
					}

					nMode = 0;
					i--;
					continue;
				}
			}else
			/* �L����ǂݍ��ݒ� */
			if( 2 == nMode ){
				if( L'_' == pLine[i] ||
					L':' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i] ||
					L'{' == pLine[i] ||
					L'}' == pLine[i] ||
					L'(' == pLine[i] ||
					L')' == pLine[i] ||
					L';' == pLine[i]	||
					L'\'' == pLine[i] ||
					L'"' == pLine[i] ||
					L'/' == pLine[i] ||
					L'.' == pLine[i]
				){
					nMode = 0;
					i--;
					continue;
				}else{
				}
			}else
			/* ���߂���P�ꖳ���� */
			if( 999 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					nMode = 0;
					continue;
				}
			}else
			/* �m�[�}�����[�h */
			if( 0 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen - 1 && L'/' == pLine[i] &&  L'/' == pLine[i + 1] ){
					break;
				}else
				if( i < nLineLen - 1 && L'/' == pLine[i] &&  L'*' == pLine[i + 1] ){
					++i;
					nMode = 8;
					continue;
				}else
				if( L'\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( L'"' == pLine[i] ){
					nMode = 21;
					continue;
				}else
				if( L'{' == pLine[i] ){
					if( 0 < nClassNestArrNum && 2 == nNestLevel2Arr[nClassNestArrNum - 1] ){
						//	Oct. 10, 2002 genta
						//	���\�b�h���ł���Ƀ��\�b�h���`���邱�Ƃ͂Ȃ��̂�
						//	�l�X�g���x������ǉ� class/interface�̒����̏ꍇ�̂ݔ��肷��
						if( nClassNestArr[nClassNestArrNum - 1] == nNestLevel - 1
						 && 0 != wcscmp( L"sizeof", szFuncName )
						 && 0 != wcscmp( L"if", szFuncName )
						 && 0 != wcscmp( L"for", szFuncName )
						 && 0 != wcscmp( L"do", szFuncName )
						 && 0 != wcscmp( L"while", szFuncName )
						 && 0 != wcscmp( L"catch", szFuncName )
						 && 0 != wcscmp( L"switch", szFuncName )
						 && 0 != wcscmp( L"return", szFuncName )
						){
							nFuncId = 2;
							++nFuncNum;
							/*
							  �J�[�\���ʒu�ϊ�
							  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
							  ��
							  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
							*/
							CLayoutPoint ptPosXY;
							m_pcDocRef->m_cLayoutMgr.LogicToLayout(
								CLogicPoint(CLogicInt(0), nFuncLine - CLogicInt(1)),
								&ptPosXY
							);
							wchar_t szWork[256];
							auto_sprintf( szWork, L"%ls::%ls", szClass, szFuncName );
							pcFuncInfoArr->AppendData( nFuncLine, ptPosXY.GetY2() + CLayoutInt(1), szWork, nFuncId );
						}
					}
					if( 0 < nClassNestArrNum ){
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}
					++nNestLevel;
					nMode = 0;
					continue;
				}else
				if( L'}' == pLine[i] ){
					if( 0 < nClassNestArrNum ){
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}

					nNestLevel--;
					if( 0 < nClassNestArrNum &&
						nClassNestArr[nClassNestArrNum - 1] == nNestLevel
					){
						nClassNestArrNum--;
						int k;
						for( k = wcslen( szClass ) - 1; k >= 0; k-- ){
							if( L'\\' == szClass[k] ){
								break;
							}
						}
						if( 0 > k ){
							k = 0;
						}
						szClass[k] = L'\0';
					}
					nMode = 0;
					continue;
				}else
				if( L'(' == pLine[i] ){
					if( 0 < nClassNestArrNum /*nNestLevel == 1*/ &&
						0 != wcscmp( L"new", szWordPrev )
					){
						wcscpy( szFuncName, szWord );
						nFuncLine = nLineCount + CLogicInt(1);
						if( 0 < nClassNestArrNum ){
							nNestLevel2Arr[nClassNestArrNum - 1] = 1;
						}
					}
					nMode = 0;
					continue;
				}else
				if( L')' == pLine[i] ){
					int			k;
					const wchar_t*	pLine2;
					CLogicInt		nLineLen2;
					CLogicInt	nLineCount2;
					nLineCount2 = nLineCount;
					pLine2 = pLine;
					nLineLen2 = nLineLen;
					k = i + 1;
					BOOL		bCommentLoop;
					bCommentLoop = FALSE;
				loop_is_func:;
					for( ; k < nLineLen2; ++k ){
						if( !bCommentLoop ){
							if( pLine2[k] != L' ' && pLine2[k] != WCODE::TAB && pLine2[k] != WCODE::CR && pLine2[k] != WCODE::LF ){
								if( k + 1 < nLineLen2 && pLine2[k] == L'/' && pLine2[k + 1] == L'*' ){
									bCommentLoop = TRUE;
									++k;
								}else
								if( k + 1 < nLineLen2 && pLine2[k] == L'/' && pLine2[k + 1] == L'/' ){
									k = nLineLen2 + 1;
									break;
								}else{
									break;
								}
							}
						}else{
							if( k + 1 < nLineLen2 && pLine2[k] == L'*' && pLine2[k + 1] == L'/' ){
								bCommentLoop = FALSE;
								++k;
							}
						}
					}
					if( k >= nLineLen2 ){
						k = 0;
						++nLineCount2;
						pLine2 = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount2)->GetDocLineStrWithEOL(&nLineLen2);
						if( NULL != pLine2 ){
							goto loop_is_func;
						}
						if( 0 < nClassNestArrNum ){
							nNestLevel2Arr[nClassNestArrNum - 1] = 0;
						}
					}else{
						//	Oct. 10, 2002 genta
						//	abscract �ɂ��Ή�
						if( pLine2[k] == L'{' || pLine2[k] == L';' ||
							__iscsym( pLine2[k] ) ){
							if( 0 < nClassNestArrNum ){
								if( 1 == nNestLevel2Arr[nClassNestArrNum - 1] ){
									nNestLevel2Arr[nClassNestArrNum - 1] = 2;
								}
							}
						}else{
							if( 0 < nClassNestArrNum ){
								nNestLevel2Arr[nClassNestArrNum - 1] = 0;
							}
						}
					}
					nMode = 0;
					continue;
				}else
				if( L';' == pLine[i] ){
					if( 0 < nClassNestArrNum && 2 == nNestLevel2Arr[nClassNestArrNum - 1] ){
						//	Oct. 10, 2002 genta
						// �֐��̒��ŕʂ̊֐��̐錾�����g�����Ƃ��āCJava�ł���́H
						if( nClassNestArr[nClassNestArrNum - 1] == nNestLevel - 1
						 && 0 != wcscmp( L"sizeof", szFuncName )
						 && 0 != wcscmp( L"if", szFuncName )
						 && 0 != wcscmp( L"for", szFuncName )
						 && 0 != wcscmp( L"do", szFuncName )
						 && 0 != wcscmp( L"while", szFuncName )
						 && 0 != wcscmp( L"catch", szFuncName )
						 && 0 != wcscmp( L"switch", szFuncName )
						 && 0 != wcscmp( L"return", szFuncName )
						){
							nFuncId = 1;
							++nFuncNum;
							/*
							  �J�[�\���ʒu�ϊ�
							  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
							  ��
							  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
							*/
							CLayoutPoint ptPosXY;
							m_pcDocRef->m_cLayoutMgr.LogicToLayout(
								CLogicPoint(CLogicInt(0), nFuncLine - CLogicInt(1)),
								&ptPosXY
							);
							wchar_t szWork[256];
							auto_sprintf( szWork, L"%ls::%ls", szClass, szFuncName );
							pcFuncInfoArr->AppendData( nFuncLine, ptPosXY.GetY2() + CLayoutInt(1), szWork, nFuncId );
						}
					}
					if( 0 < nClassNestArrNum ){
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}
					nMode = 0;
					continue;
				}else{
					if( L'_' == pLine[i] ||
						L':' == pLine[i] ||
						L'~' == pLine[i] ||
						(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
						(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
						(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
						L'.' == pLine[i]
					){
						wcscpy( szWordPrev, szWord );
						nWordIdx = 0;
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = L'\0';
						nMode = 1;
					}else{
						nMode = 0;
					}
				}
			}
		}
	}
#ifdef _DEBUG
	pcFuncInfoArr->DUMP();
#endif
	return;
}




//	From Here Sep 8, 2000 genta
//
//!	Perl�p�A�E�g���C����͋@�\�i�ȈՔŁj
/*!
	�P���� /^\\s*sub\\s+(\\w+)/ �Ɉ�v������ $1�����o��������s���B
	�l�X�g�Ƃ��͖ʓ|�������̂ōl���Ȃ��B
	package{ }���g��Ȃ���΂���ŏ\���D�������͂܂��B

	@par nMode�̈Ӗ�
	@li 0: �͂���
	@li 2: sub����������
	@li 1: �P��ǂݏo����

	@date 2005.06.18 genta �p�b�P�[�W��؂��\�� ::��'���l������悤��
*/
void CDocOutline::MakeFuncList_Perl( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt			nLineLen;
	int			i;
	int			nCharChars;
	wchar_t		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;

	CLogicInt	nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		nMode = 0;
		for( i = 0; i < nLineLen; ++i ){
			/* 1�o�C�g������������������ */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
			if(	1 < nCharChars ){
				break;
			}

			/* �P��ǂݍ��ݒ� */
			if( 0 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					continue;
				}
				if( 's' != pLine[i] )
					break;
				//	sub �̈ꕶ���ڂ�������Ȃ�
				if( nLineLen - i < 4 )
					break;
				if( wcsncmp_literal( pLine + i, L"sub" ) )
					break;
				int c = pLine[ i + 3 ];
				if( c == L' ' || c == L'\t' ){
					nMode = 2;	//	����
					i += 3;
				}
				else
					break;
			}
			else if( 2 == nMode ){
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					continue;
				}
				if( L'_' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )
				){
					//	�֐����̎n�܂�
					nWordIdx = 0;
					szWord[nWordIdx] = pLine[i];
					szWord[nWordIdx + 1] = L'\0';
					nMode = 1;
					continue;
				}
				else
					break;

			}
			else if( 1 == nMode ){
				if( L'_' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					//	Jun. 18, 2005 genta �p�b�P�[�W�C���q���l��
					//	�R������2�A�����Ȃ��Ƃ����Ȃ��̂����C�����͎蔲��
					L':' == pLine[i] || L'\'' == pLine[i]
				){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						break;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = L'\0';
					}
				}else{
					//	�֐����擾
					/*
					  �J�[�\���ʒu�ϊ�
					  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
					  ��
					  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
					*/
					CLayoutPoint ptPosXY;
					m_pcDocRef->m_cLayoutMgr.LogicToLayout(
						CLogicPoint(CLogicInt(0), nLineCount),
						&ptPosXY
					);
					//	Mar. 9, 2001
					pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPosXY.GetY2() + CLayoutInt(1), szWord, 0 );

					break;
				}
			}
		}
	}
#ifdef _DEBUG
	pcFuncInfoArr->DUMP();
#endif
	return;
}
//	To HERE Sep. 8, 2000 genta








//	From Here June 23, 2001 N.Nakatani
//!	Visual Basic�֐����X�g�쐬�i�ȈՔŁj
/*!
	Visual Basic�̃R�[�h����P���Ƀ��[�U�[��`�̊֐���X�e�[�g�����g�����o��������s���B

    Jul 10, 2003 little YOSHI  �ׂ�����͂���悤�ɕύX
                               ���ׂẴL�[���[�h�͎����I�ɐ��`�����̂ŁA�啶���������͊��S�Ɉ�v����B
                               �t�H�[���⃂�W���[�������ł͂Ȃ��A�N���X�ɂ��Ή��B
							   �������AConst�́u,�v�ŘA���錾�ɂ͖��Ή�
	Jul. 21, 2003 genta �L�[���[�h�̑啶���E�������𓯈ꎋ����悤�ɂ���
	Aug  7, 2003 little YOSHI  �_�u���N�H�[�e�[�V�����ň͂܂ꂽ�e�L�X�g�𖳎�����悤�ɂ���
	                           �֐����Ȃǂ�VB�̖��O�t���K�����255�����Ɋg��
*/
void CDocOutline::MakeFuncList_VisualBasic( CFuncInfoArr* pcFuncInfoArr )
{
	const int	nMaxWordLeng = 255;	// Aug 7, 2003 little YOSHI  VB�̖��O�t���K�����255�����Ɋg��
	const wchar_t*	pLine;
	CLogicInt		nLineLen = CLogicInt(0);//: 2002/2/3 aroka �x���΍�F������
	int			i;
	int			nCharChars;
	wchar_t		szWordPrev[256];	// Aug 7, 2003 little YOSHI  VB�̖��O�t���K�����255�����Ɋg��
	wchar_t		szWord[256];		// Aug 7, 2003 little YOSHI  VB�̖��O�t���K�����255�����Ɋg��
	int			nWordIdx = 0;
	int			nMode;
	wchar_t		szFuncName[256];	// Aug 7, 2003 little YOSHI  VB�̖��O�t���K�����255�����Ɋg��
	CLogicInt	nFuncLine;
	int			nFuncId;
	int			nParseCnt = 0;
	bool		bClass;			// �N���X���W���[���t���O
	bool		bProcedure;		// �v���V�[�W���t���O�i�v���V�[�W�����ł�True�j
	bool		bDQuote;		// �_�u���N�H�[�e�[�V�����t���O�i�_�u���N�H�[�e�[�V������������True�j

	// ���ׂ�t�@�C�����N���X���W���[���̂Ƃ���Type�AConst�̋������قȂ�̂Ńt���O�𗧂Ă�
	bClass	= false;
	int filelen = _tcslen(m_pcDocRef->m_cDocFile.GetFilePath());
	if ( 4 < filelen ) {
		if ( 0 == _tcsicmp((m_pcDocRef->m_cDocFile.GetFilePath() + filelen - 4), _FT(".cls")) ) {
			bClass	= true;
		}
	}

	szWordPrev[0] = L'\0';
	szWord[nWordIdx] = L'\0';
	nMode = 0;
	pLine = NULL;
	bProcedure	= false;
	CLogicInt		nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		if( NULL != pLine ){
			if( L'_' != pLine[nLineLen-1]){
				nParseCnt = 0;
			}
		}
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		nFuncId = 0;
		bDQuote	= false;
		for( i = 0; i < nLineLen; ++i ){
			/* 1�o�C�g������������������ */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
			if(	0 == nCharChars ){
				nCharChars = 1;
			}
			/* �P��ǂݍ��ݒ� */
			if( 1 == nMode ){
				if( (1 == nCharChars && (
					L'_' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )
					) )
				 || 2 == nCharChars
				){
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						auto_memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);
					}
				} else if (1 == nCharChars && '"' == pLine[i]) {
					// Aug 7, 2003 little YOSHI  �ǉ�
					// �e�L�X�g�̒��͖������܂��B
					nMode	= 3;
				}else{
					if ( 0 == nParseCnt && 0 == wcsicmp(szWord, L"Public") ) {
						// �p�u���b�N�錾���������I
						nFuncId |= 0x10;
					}else
					if ( 0 == nParseCnt && 0 == wcsicmp(szWord, L"Private") ) {
						// �v���C�x�[�g�錾���������I
						nFuncId |= 0x20;
					}else
					if ( 0 == nParseCnt && 0 == wcsicmp(szWord, L"Friend") ) {
						// �t�����h�錾���������I
						nFuncId |= 0x30;
					}else
					if ( 0 == nParseCnt && 0 == wcsicmp(szWord, L"Static") ) {
						// �X�^�e�B�b�N�錾���������I
						nFuncId |= 0x100;
					}else
					if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Function" ) ){
						if ( 0 == wcsicmp( szWordPrev, L"End" ) ){
							// �v���V�[�W���t���O���N���A
							bProcedure	= false;
						}else
						if( 0 != wcsicmp( szWordPrev, L"Exit" ) ){
							if( 0 == wcsicmp( szWordPrev, L"Declare" ) ){
								nFuncId |= 0x200;	// DLL�Q�Ɛ錾
							}else{
								bProcedure	= true;	// �v���V�[�W���t���O���Z�b�g
							}
							nFuncId |= 0x01;		// �֐�
							nParseCnt = 1;
							nFuncLine = nLineCount + CLogicInt(1);
						}
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Sub" ) ){
						if ( 0 == wcsicmp( szWordPrev, L"End" ) ){
							// �v���V�[�W���t���O���N���A
							bProcedure	= false;
						}else
						if( 0 != wcsicmp( szWordPrev, L"Exit" ) ){
							if( 0 == wcsicmp( szWordPrev, L"Declare" ) ){
								nFuncId |= 0x200;	// DLL�Q�Ɛ錾
							}else{
								bProcedure	= true;	// �v���V�[�W���t���O���Z�b�g
							}
							nFuncId |= 0x02;		// �֐�
							nParseCnt = 1;
							nFuncLine = nLineCount + CLogicInt(1);
						}
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Get" )
					 && 0 == wcsicmp( szWordPrev, L"Property" )
					){
						bProcedure	= true;	// �v���V�[�W���t���O���Z�b�g
						nFuncId	|= 0x03;		// �v���p�e�B�擾
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Let" )
					 && 0 == wcsicmp( szWordPrev, L"Property" )
					){
						bProcedure	= true;	// �v���V�[�W���t���O���Z�b�g
						nFuncId |= 0x04;		// �v���p�e�B�ݒ�
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Set" )
					 && 0 == wcsicmp( szWordPrev, L"Property" )
					){
						bProcedure	= true;	// �v���V�[�W���t���O���Z�b�g
						nFuncId |= 0x05;		// �v���p�e�B�Q��
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Const" )
					 && 0 != wcsicmp( szWordPrev, L"#" )
					){
						if ( bClass || bProcedure || 0 == ((nFuncId >> 4) & 0x0f) ) {
							// �N���X���W���[���ł͋����I��Private
							// �v���V�[�W�����ł͋����I��Private
							// Public�̎w�肪�Ȃ��Ƃ��A�f�t�H���g��Private�ɂȂ�
							nFuncId &= 0x0f2f;
							nFuncId	|= 0x20;
						}
						nFuncId	|= 0x06;		// �萔
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Enum" )
					){
						nFuncId	|= 0x207;		// �񋓌^�錾
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Type" )
					){
						if ( bClass ) {
							// �N���X���W���[���ł͋����I��Private
							nFuncId &= 0x0f2f;
							nFuncId	|= 0x20;
						}
						nFuncId	|= 0x208;		// ���[�U��`�^�錾
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Event" )
					){
						nFuncId	|= 0x209;		// �C�x���g�錾
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Property" )
					 && 0 == wcsicmp( szWordPrev, L"End")
					){
						bProcedure	= false;	// �v���V�[�W���t���O���N���A
					}
					else if( 1 == nParseCnt ){
						wcscpy( szFuncName, szWord );
						/*
						  �J�[�\���ʒu�ϊ�
						  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
						  �� ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						*/
						CLayoutPoint ptPosXY;
						m_pcDocRef->m_cLayoutMgr.LogicToLayout(	CLogicPoint(CLogicInt(0), nFuncLine - CLogicInt(1)), &ptPosXY );
						pcFuncInfoArr->AppendData( nFuncLine, ptPosXY.GetY2() + CLayoutInt(1) , szFuncName, nFuncId );
						nParseCnt = 0;
						nFuncId	= 0;	// Jul 10, 2003  little YOSHI  �_���a���g�p���邽�߁A�K��������
					}

					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = L'\0';
					nMode = 0;
					i--;
					continue;
				}
			}
			/* �L����ǂݍ��ݒ� */
			else if( 2 == nMode ){
				// Jul 10, 2003  little YOSHI
				// �u#Const�v�ƁuConst�v����ʂ��邽�߂ɁA�u#�v�����ʂ���悤�ɕύX
				if( L'_' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i] ||
					L'{' == pLine[i] ||
					L'}' == pLine[i] ||
					L'(' == pLine[i] ||
					L')' == pLine[i] ||
					L';' == pLine[i]	||
					L'\'' == pLine[i] ||
					L'/' == pLine[i]	||
					L'-' == pLine[i] ||
					L'#' == pLine[i]
				){
					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = L'\0';
					nMode = 0;
					i--;
					continue;
				} else if (1 == nCharChars && L'"' == pLine[i]) {
					// Aug 7, 2003 little YOSHI  �ǉ�
					// �e�L�X�g�̒��͖������܂��B
					nMode	= 3;
				}else{
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);
					}
				}
			}else
			/* ���߂���P�ꖳ���� */
			if( 999 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					nMode = 0;
					continue;
				}
			}else
			/* �m�[�}�����[�h */
			if( 0 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen && L'\'' == pLine[i] ){
					break;
				} else if (1 == nCharChars && L'"' == pLine[i]) {
					// Aug 7, 2003 little YOSHI  �ǉ�
					// �e�L�X�g�̒��͖������܂��B
					nMode	= 3;
				}else{
					if( (1 == nCharChars && (
						L'_' == pLine[i] ||
						L'~' == pLine[i] ||
						(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
						(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
						(L'0' <= pLine[i] &&	pLine[i] <= L'9' )
						) )
					 || 2 == nCharChars
					){
						nWordIdx = 0;

						auto_memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);

						nMode = 1;
					}else{
						nWordIdx = 0;
						auto_memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);

						nMode = 2;
					}
				}
			} else
			/* �e�L�X�g������܂œǂݔ�΂� */	// Aug 7, 2003 little YOSHI  �ǉ�
			if (nMode == 3) {
				// �A������_�u���N�H�[�e�[�V�����͖�������
				if (1 == nCharChars && L'"' == pLine[i]) {
					// �_�u���N�H�[�e�[�V���������ꂽ��t���O�𔽓]����
					bDQuote	= !bDQuote;
				} else if (bDQuote) {
					// �_�u���N�H�[�e�[�V�����̎���
					// �_�u���N�H�[�e�[�V�����ȊO�̕��������ꂽ��m�[�}�����[�h�Ɉڍs
					--i;
					nMode	= 0;
					bDQuote	= false;
					continue;
				}
			}
			i += (nCharChars - 1);
		}
	}
	return;
}
//	To Here June 23, 2001 N.Nakatani


// From Here 2001.12.03 hor
/*! �u�b�N�}�[�N���X�g�쐬�i������I�j

	@date 2002.01.19 aroka ��s���}�[�N�Ώۂɂ���t���O bMarkUpBlankLineEnable �𓱓����܂����B
	@date 2005.10.11 ryoji "��@" �̉E�Q�o�C�g���S�p�󔒂Ɣ��肳�����̑Ώ�
	@date 2005.11.03 genta �����񒷏C���D�E�[�̃S�~������
*/
void CDocOutline::MakeFuncList_BookMark( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	CLogicInt		nLineCount;
	int		leftspace, pos_wo_space, k;
	wchar_t*	pszText;
	BOOL	bMarkUpBlankLineEnable = GetDllShareData().m_Common.m_sOutline.m_bMarkUpBlankLineEnable;	//! ��s���}�[�N�Ώۂɂ���t���O 20020119 aroka
	int		nNewLineLen	= m_pcDocRef->m_cDocEditor.m_cNewLineCode.GetLen();
	CLogicInt	nLineLast	= m_pcDocRef->m_cDocLineMgr.GetLineCount();
	int		nCharChars;

	for( nLineCount = CLogicInt(0); nLineCount <  nLineLast; ++nLineCount ){
		if(!CBookmarkGetter(m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)).IsBookmarked())continue;
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine ){
			break;
		}
		// Jan, 16, 2002 hor
		if( bMarkUpBlankLineEnable ){// 20020119 aroka
			if( nLineLen<=nNewLineLen && nLineCount< nLineLast ){
			  continue;
			}
		}// LTrim
		for( leftspace = 0; leftspace < nLineLen; ++leftspace ){
			if( WCODE::isBlank(pLine[leftspace]) ){
				continue;
			}
			break;
		}
		
		if( bMarkUpBlankLineEnable ){// 20020119 aroka
			if(( leftspace >= nLineLen-nNewLineLen && nLineCount< nLineLast )||
				( leftspace >= nLineLen )) {
				continue;
			}
		}// RTrim
		// 2005.10.11 ryoji �E����k��̂ł͂Ȃ�������T���悤�ɏC���i"��@" �̉E�Q�o�C�g���S�p�󔒂Ɣ��肳�����̑Ώ��j
		k = pos_wo_space = leftspace;
		while( k < nLineLen ){
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, k );
			if( 1 == nCharChars ){
				if( !(pLine[k] == WCODE::CR ||
						pLine[k] == WCODE::LF ||
						pLine[k] == WCODE::SPACE ||
						pLine[k] == WCODE::TAB ||
						WCODE::isZenkakuSpace(pLine[k]) ||
						pLine[k] == L'\0') )
					pos_wo_space = k + nCharChars;
			}
			k += nCharChars;
		}
		//	Nov. 3, 2005 genta �����񒷌v�Z���̏C��
		{
			int nLen = pos_wo_space - leftspace;
			pszText = new wchar_t[nLen + 1];
			wmemcpy( pszText, &pLine[leftspace], nLen );
			pszText[nLen] = L'\0';
		}
		CLayoutPoint ptXY;
		//int nX,nY
		m_pcDocRef->m_cLayoutMgr.LogicToLayout(	CLogicPoint(CLogicInt(0), nLineCount), &ptXY );
		pcFuncInfoArr->AppendData( nLineCount+CLogicInt(1), ptXY.GetY2()+CLayoutInt(1) , pszText, 0 );
		delete [] pszText;
	}
	return;
}
// To Here 2001.12.03 hor
