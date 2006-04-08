//	$Id$
/*!	@file
	@brief �A�E�g���C�����

	@author genta
	@date	2004.08.08 �쐬
	$Revision$
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
#include <string.h>
#include "global.h"

#include "CEditDoc.h"
#include "CFuncInfoArr.h"
#include "CDocLine.h"
#include "charcode.h"

//	Mar. 15, 2000 genta
//	From Here
/*!
	�X�y�[�X�̔���
*/
inline bool C_IsSpace( char c ){
	return ('\t' == c ||
			 ' ' == c ||
			  CR == c ||
			  LF == c
	);
}

/*!
	�֐��ɗp���邱�Ƃ��ł��镶�����ǂ����̔���
*/
inline bool C_IsWordChar( char c ){
	return ( '_' == c ||
			 ':' == c ||
			 '~' == c ||
			('a' <= c && c <= 'z' )||
			('A' <= c && c <= 'Z' )||
			('0' <= c && c <= '9' )
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
static bool C_IsOperator( char* szStr, int nLen	)
{
	if( nLen >= 8 && szStr[ nLen - 1 ] == 'r' ){
		if( nLen > 8 ?
				strcmp( szStr + nLen - 9, ":operator" ) == 0 :	// �����o�[�֐��ɂ���`
				strcmp( szStr, "operator" ) == 0	// friend�֐��ɂ���`
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
static bool C_IsLineEsc(const char *s, int len)
{
	if ( len > 0 && s[len-1] == '\n' ) len--;
	if ( len > 0 && s[len-1] == '\r' ) len--;
	if ( len > 0 && s[len-1] == '\n' ) len--;

	if ( len > 0 && s[len-1] == '\\' ) {
		if ( len == 1 ) {
			return(true);
		} else if ( len == 2 ) {
			if ( CMemory::GetSizeOfChar( s, 2 , 0 ) == 1 )
				return(true);
		} else {				/* �c��R�o�C�g�ȏ�	*/
			if ( CMemory::GetSizeOfChar( s, len , len-2 ) == 1 )
				return(true);
			if ( CMemory::GetSizeOfChar( s, len , len-3 ) == 2 )
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

	int ScanLine(const char*, int);

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
int CCppPreprocessMng::ScanLine( const char* str, int length )
{
	const char* lastptr = str + length;	//	���������񖖔�
	const char* p;	//	�������̈ʒu

	//	skip whitespace
	for( p = str; C_IsSpace( *p ) && p < lastptr ; ++p )
		;
	if( lastptr <= p )
		return length;	//	��s�̂��ߏ����s�v

	if(m_ismultiline){ // �����s�̃f�B���N�e�B�u�͖���
		m_ismultiline = C_IsLineEsc(str, length); // �s���� \ �ŏI����Ă��Ȃ���
		return length;
	}

	if( *p != '#' ){	//	�v���v���Z�b�T�ȊO�̏����̓��C�����ɔC����
		if( m_enablebuf ){
			return length;	//	1�r�b�g�ł�1�ƂȂ��Ă����疳��
		}
		return p - str;
	}

	++p; // #���X�L�b�v
	
	//	skip whitespace
	for( ; C_IsSpace( *p ) && p < lastptr ; ++p )
		;

	//	��������Preprocessor directive���
	if( p + 2 + 2 < lastptr && strncmp( p, "if", 2 ) == 0 ){	//	sizeof( "if" ) == 2
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
			if( *p == '0' ){
				enable = 1;
			}
			else {
				enable = 2;
			}
		}
		else if(
			( p + 3 < lastptr && strncmp( p, "def", 3 ) == 0 ) ||
			( p + 4 < lastptr && strncmp( p, "ndef", 4 ) == 0 )){
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
	else if( p + 4 < lastptr && strncmp( p, "else", 4 ) == 0 ){	//	sizeof( "else" ) == 4
		if( m_stackptr < m_maxnestlevel ){
			m_enablebuf ^= m_bitpattern;
		}
	}
	else if( p + 5 < lastptr && strncmp( p, "endif", 5 ) == 0 ){	//	sizeof( "endif" ) == 5
		if( m_stackptr > 0 ){
			--m_stackptr;
			m_enablebuf &= ~m_bitpattern;
			m_bitpattern = ( 1 << ( m_stackptr - 1 ));
		}
	}
	else{
		m_ismultiline = C_IsLineEsc(str, length); // �s���� \ �ŏI����Ă��Ȃ���
	}

	return length;	//	��{�I�Ƀv���v���Z�b�T�w�߂͖���
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
void CEditDoc::MakeFuncList_C( CFuncInfoArr* pcFuncInfoArr ,bool bVisibleMemberFunc )
{
	const char*	pLine;
	int			nLineLen;
	int			nLineCount;
	int			i;

	// 2002/10/27 frozen�@��������
	// nNestLevel�� nNestLevel_global �� nNestLevel_func �ɕ��������B
	int			nNestLevel_global = 0;	// nNestLevel_global �֐��O�� {}�̃��x��  
	int			nNestLevel_func   = 0;	//	nNestLevel_func �֐��̒�`�A����ъ֐�����	{}�̃��x��
//	int			nNestLevel2;			//	nNestLevel2	()�ɑ΂���ʒu // 2002/10/27 frozen nNastLevel_fparam��nMode2��M2_FUNC_NAME_END�ő�p
	int			nNestLevel_fparam = 0;	// ()�̃��x��
	int			nNestPoint_class = 0;	// �O�����牽�Ԗڂ�{���N���X�̒�`���͂�{���H (��ԊO���Ȃ�1�A0�Ȃ疳���BbVisibleMemberFunc��false�̎��̂ݗL���Btrue�ł͏��0)
	// 2002/10/27 frozen�@�����܂�

	int			nCharChars;			//	���o�C�g������ǂݔ�΂����߂̂���
	char		szWordPrev[256];	//	1�O��word
	char		szWord[256];		//	���݉�ǒ���word������Ƃ���
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
//	char		szFuncName[256];	//	�֐���

	const int	nNamespaceNestMax	= 32;			//!< �l�X�g�\�ȃl�[���X�y�[�X�A�N���X���̍ő吔
	int			nNamespaceLen[nNamespaceNestMax+1];	//!< �l�[���X�y�[�X�S�̂̒���
	const int	nNamespaceLenMax 	= 512;			//!< �ő�̃l�[���X�y�[�X�S�̂̒���
	char		szNamespace[nNamespaceLenMax];		//!< ���݂̃l�[���X�y�[�X(�I�[��\0�ɂȂ��Ă���Ƃ͌���Ȃ��̂Œ���)
	const int 	nItemNameLenMax	 	= 256;
	char		szItemName[nItemNameLenMax];		//!< �����O�� �֐��� or �N���X�� or �\���̖� or ���p�̖� or �񋓑̖� or �l�[���X�y�[�X��
	// �Ⴆ�Ή��̃R�[�h�́��̕����ł�
	// szNamespace��"Namespace\ClassName\"
	// nMamespaceLen��{10,20}
	// nNestLevel_global��2�ƂȂ�B
	//
	//�@namespace Namespace{
	//�@class ClassName{
	//�@��
	//�@}}

	int			nItemLine;			//!< �����O�� �֐� or �N���X or �\���� or ���p�� or �񋓑� or �l�[���X�y�[�X�̂���s
	int			nItemFuncId;

//	int			nFuncLine;
//	int			nFuncId;
	
//	int			nFuncNum;			// �g���Ă��Ȃ��悤�Ȃ̂ō폜
	// 2002/10/27 frozen�@�����܂�

	//	Mar. 4, 2001 genta
//	bool		bCppInitSkip;		//	C++�̃����o�[�ϐ��A�e�N���X�̏������q��SKIP // 2002/10/27 frozen nMode2�̋@�\�ő�p
	szWordPrev[0] = '\0';
	szWord[nWordIdx] = '\0';
	szNamespace[0] = '\0';	// 2002/10/27 frozen
	nNamespaceLen[0] = 0;	// 2002/10/27 frozen
	nMode = 0;
	
	//	Aug. 10, 2004 genta �v���v���Z�X�����N���X
	CCppPreprocessMng cCppPMng;
	
//	FuncNum = 0;
//	bCppInitSkip = false;
//	for( nLineCount = 0; nLineCount <  m_cLayoutMgr.GetLineCount(); ++nLineCount ){
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
//		pLine = m_cLayoutMgr.GetLineStr( nLineCount, &nLineLen );
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );

		//	From Here Aug. 10, 2004 genta
		//	�v���v���Z�X����
		//	�R�����g���łȂ���΃v���v���Z�b�T�w�߂��ɔ��肳����
		if( 8 != nMode && 10 != nMode ){	/* chg 2005/12/6 ���イ�� ���̍s���󔒂ł��悢	*/
			i = cCppPMng.ScanLine( pLine, nLineLen );
		}
		else {
			i = 0;
		}
		//	C/C++�Ƃ��Ă̏������s�v�ȃP�[�X�ł� i == nLineLen�ƂȂ��Ă���̂�
		//	�ȉ��̉�͏�����SKIP�����D
		//	To Here Aug. 10, 2004 genta
		
		for( ; i < nLineLen; ++i ){
			/* 1�o�C�g������������������ */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, i );
			if( 1 < nCharChars ){
				i += (nCharChars - 1);
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
			}else
			/* ���C���R�����g�ǂݍ��ݒ� */
			// 2003/06/24 zenryaku
			if( 10 == nMode)
			{
				if(!C_IsLineEsc(pLine, nLineLen)){
					nMode = 0;
				}
				i = nLineLen;
				continue;
			}else
/* add start 2005/12/6 ���イ��	*/
			/* �G�X�P�[�v�V�[�P���X�͏�Ɏ�菜�� */
			if( '\\' == pLine[i] ){
				++i;
			}else
			/* �V���O���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( 20 == nMode ){
				if( '\'' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* �_�u���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( 21 == nMode ){
				if( '"' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
/* add end 2005/12/6 ���イ��	*/
			/* �P��ǂݍ��ݒ� */
			if( 1 == nMode ){
				if( C_IsWordChar( pLine[i] ) ){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						if( pLine[i] == ':')
						{
							if(nMode2 == M2_NAMESPACE_SAVE)
							{
								if(szWord[0]!='\0')
									strcpy( szItemName, szWord );
								nMode2 = M2_NAMESPACE_END;
							}
							else if( nMode2 == M2_TEMPLATE_SAVE)
							{
								strncat( szItemName, szWord, nItemNameLenMax - strlen(szItemName) );
								szItemName[ nItemNameLenMax - 1 ] = '\0';
								nMode2 = M2_NAMESPACE_END;
							}
						}
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
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
									pLine[ i + 1 ] == '=' )
									oplen = 2;
							}
							break;
						case '-':
							oplen = 1;
							if( i + 1 < nLineLen ){
								if( pLine[ i + 1 ] == '-' ||
									pLine[ i + 1 ] == '=' )
									oplen = 2;
								else if( pLine[ i + 1 ] == '>' ){
									oplen = 2;
									if( i + 2 < nLineLen ){
										if( pLine[ i + 2 ] == '*' )
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
								if( pLine[ i + 1 ] == '=' )
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
										if( pLine[ i + 2 ] == '=' )
											oplen = 3;
									}
								}
								else if( pLine[ i + 1 ] == '=' )
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
						szWord[nWordIdx + 1] = '\0';
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
						strcpy( szItemName, szWord );
					else if( nMode2 == M2_TEMPLATE_SAVE)
					{
						strncat( szItemName, szWord, nItemNameLenMax - strlen(szItemName) );
						szItemName[ nItemNameLenMax - 1 ] = '\0';
					}
					else if( nNestLevel_func == 0 && nMode2 == M2_NORMAL )
					{
						nItemFuncId = 0;
						if( strcmp(szWord,"class")==0 )
							nItemFuncId = 3;
						if( strcmp(szWord,"struct")==0 )
							nItemFuncId = 4;
						else if( strcmp(szWord,"namespace")==0 )
							nItemFuncId = 7;
						else if( strcmp(szWord,"enum")==0 )
							nItemFuncId = 5;
						else if( strcmp(szWord,"union")==0 )
							nItemFuncId = 6;
						if( nItemFuncId != 0 )
						{
							nMode2 = M2_NAMESPACE_SAVE;
							nItemLine = nLineCount + 1;
							strcpy(szItemName,"����");
						}
					}
					else if( nMode2 == M2_FUNC_NAME_END )
					{
						nMode2 = M2_KR_FUNC;
					}
					// 2002/10/27 frozen�@�����܂�

					//	To Here Mar. 31, 2001 genta
					// 2004/03/12 zenryaku �L�[���[�h�� _ �� PARAMS ���g�킹�Ȃ� (GNU�̃R�[�h�����ɂ����Ȃ邩��)
					if( !( strcmp("PARAMS",szWord) == 0 || strcmp("_",szWord) == 0 ) )
						strcpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = '\0';
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
					strcpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = '\0';
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
						szWord[nWordIdx + 1] = '\0';
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
							(nNamespaceLen[nNestLevel_global] +  (nItemNameLen = strlen(szItemName)) + 10 + 1) < nNamespaceLenMax)
					// �R�Ԗڂ�(&&�̌��)����
					// �o�b�t�@������Ȃ��ꍇ�͍��ڂ̒ǉ����s��Ȃ��B
					// +10�͒ǉ����镶����̍ő咷(�ǉ����镶�����"::��`�ʒu"���Œ�)
					// +1�͏I�[NUL����
					{
						strcpy( &szNamespace[nNamespaceLen[nNestLevel_global]] , szItemName);
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
								strcpy(&szNamespace[nNamespaceLen[nNestLevel_global]],"::��`�ʒu");
							else
							{
								szNamespace[nNamespaceLen[nNestLevel_global]] = '\0';
								szNamespace[nNamespaceLen[nNestLevel_global]+1] = ':';
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
						int		nPosX;
						int		nPosY;
						m_cLayoutMgr.CaretPos_Phys2Log(
							0,
							nItemLine - 1,
							&nPosX,
							&nPosY
						);
						pcFuncInfoArr->AppendData( nItemLine, nPosY + 1 , szNamespace, nItemFuncId);
						//	Jan. 30, 2005 genta M2_KR_FUNC �ǉ�
						if( nMode2 != M2_FUNC_NAME_END && nMode2 != M2_KR_FUNC )
						{
							szNamespace[nNamespaceLen[nNestLevel_global]] = ':';
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
//						strcpy( szFuncName, szWordPrev );
//						nFuncLine = nLineCount + 1;
//						nNestLevel2 = 1;
//					}
//					nMode = 0;
					if( nNestLevel_func == 0 && (nMode2 == M2_NORMAL || nMode2 == M2_NAMESPACE_SAVE || nMode2 == M2_NAMESPACE_END) )
					{
						if( strcmp(szWordPrev, "__declspec") == 0 ) {continue;}
						if(nNestLevel_fparam==0)
						{
							strcpy( szItemName, szWordPrev);
							nItemLine = nLineCount + 1;
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
//					if( 2 == nNestLevel2 ){
//						//	������')'�̌��';' ���Ȃ킿�֐��錾
//						if( 0 != strcmp( "sizeof", szFuncName ) ){
//							nFuncId = 1;
//							++nFuncNum;
//							/*
//							  �J�[�\���ʒu�ϊ�
//							  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
//							  ��
//							  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
//							*/
//							int		nPosX;
//							int		nPosY;
//							m_cLayoutMgr.CaretPos_Phys2Log(
//								0,
//								nFuncLine - 1,
//								&nPosX,
//								&nPosY
//							);
//							pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1, szFuncName, nFuncId);
////						pcFuncInfoArr->AppendData( nFuncLine, szFuncName, nFuncId );
//						}
//					}
//					nNestLevel2 = 0;
					if( nMode2 == M2_KR_FUNC )
					{
						//	Jan. 30, 2005 genta �֐���� const, throw�̌���
						//	';'��K&R�`���錾�̏I���łȂ��֐��錾�̏I���
						if( strcmp( szWordPrev, "const" ) == 0 ||
							strcmp( szWordPrev, "throw" ) == 0 ){
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
						(nNamespaceLen[nNestLevel_global] + strlen(szItemName)) < nNamespaceLenMax &&
						nNestPoint_class == 0)
					// �R�Ԗڂ�(&&�̌��)����
					// �o�b�t�@������Ȃ��ꍇ�͍��ڂ̒ǉ����s��Ȃ��B
					{
						strcpy( &szNamespace[nNamespaceLen[ nNestLevel_global]] , szItemName);

						nItemFuncId = 1;
						/*
						  �J�[�\���ʒu�ϊ�
						  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
						  ��
						  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						*/
						int		nPosX;
						int		nPosY;
						m_cLayoutMgr.CaretPos_Phys2Log(
							0,
							nItemLine - 1,
							&nPosX,
							&nPosY
						);
						pcFuncInfoArr->AppendData( nItemLine, nPosY + 1, szNamespace, nItemFuncId);
					}
					nMode2 = M2_NORMAL;
					//  2002/10/27 frozen �����܂�
					nMode = 0;
					continue;
				}else if( nNestLevel_fparam == 0 ){
					if( C_IsWordChar( pLine[i] ) ){
						//  2002/10/27 frozen ��������폜
//						if( 2 == nNestLevel2 ){
//							//	�����ʂ��������ǂƂ肠�����o�^�����Ⴄ
//							if( 0 != strcmp( "sizeof", szFuncName ) ){
//								nFuncId = 2;
//								++nFuncNum;
//								/*
//								  �J�[�\���ʒu�ϊ�
//								  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
//								  ��
//								  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
//								*/
//								int		nPosX;
//								int		nPosY;
//								m_cLayoutMgr.CaretPos_Phys2Log(
//									0,
//									nFuncLine - 1,
//									&nPosX,
//									&nPosY
//								);
//								pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1 , szFuncName, nFuncId );
//							}
//							nNestLevel2 = 0;
//							//	Mar 4, 2001 genta	�������q�������Ƃ��͂���ȍ~�̓o�^�𐧌�����
//							if( pLine[i] == ':' )
//								bCppInitSkip = true;
//						}
						//  2002/10/27 frozen �����܂ō폜

						//	//	Mar. 15, 2000 genta
						//	From Here
						//	���O��word�̍Ōオ::���C���邢�͒����word�̐擪��::�Ȃ�
						//	�N���X����q�ƍl���ė��҂�ڑ�����D

						{
							int pos = strlen( szWordPrev ) - 2;
							if( //	�O�̕�����̖����`�F�b�N
								( pos > 0 &&	szWordPrev[pos] == ':' &&
								szWordPrev[pos + 1] == ':' ) ||
								//	���̕�����̐擪�`�F�b�N
								( i < nLineLen - 1 && pLine[i] == ':' &&
									pLine[i+1] == ':' )
							){
								//	�O�̕�����ɑ�����
								strcpy( szWord, szWordPrev );
								nWordIdx = strlen( szWord );
							}
							//	From Here Apr. 1, 2001 genta
							//	operator new/delete ���Z�q�̑Ή�
							else if( C_IsOperator( szWordPrev, pos + 2 ) ){
								//	�X�y�[�X�����āA�O�̕�����ɑ�����
								szWordPrev[pos + 2] = ' ';
								szWordPrev[pos + 3] = '\0';
								strcpy( szWord, szWordPrev );
								nWordIdx = strlen( szWord );
							}
							//	To Here Apr. 1, 2001 genta
							else{
								nWordIdx = 0;
							}
						}
						//	strcpy( szWordPrev, szWord );	�s�v�H
						//	To Here
						
						if( pLine[i] == ':')
						{
							if(nMode2 == M2_NAMESPACE_SAVE)
							{
								if(szWord[0]!='\0')
									strcpy( szItemName, szWord );
								nMode2 = M2_NAMESPACE_END;
							}
							else if( nMode2 == M2_TEMPLATE_SAVE)
							{
								strncat( szItemName, szWord, nItemNameLenMax - strlen(szItemName) );
								szItemName[ nItemNameLenMax - 1 ] = '\0';
								nMode2 = M2_NAMESPACE_END;
							}
						}

						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
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
						if( pLine[i] == '=' && nNestLevel_func == 0 && nNestLevel_fparam==0
							&& nMode2 == M2_NORMAL && ! C_IsOperator(szWordPrev,strlen(szWordPrev)) ){
							nMode2 = M2_AFTER_EQUAL;
						}
						strcpy( szWordPrev, szWord );
						nWordIdx = 0;
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
						nMode = 2;

						// 2002/10/27 frozen ��������
						if( nMode2 == M2_NAMESPACE_SAVE )
						{
							if( pLine[i] == '>' || pLine[i] == ',' || pLine[i] == '=')
								// '<' �̑O�� '>' , ',' , '=' ���������̂ŁA�����炭
								// �O�ɂ�����"class"�̓e���v���[�g�p�����[�^�̌^��\���Ă����ƍl������B
								// ����āA�N���X���̒����͏I���B
								// '>' �̓e���v���[�g�p�����[�^�̏I��
								// ',' �̓e���v���[�g�p�����[�^�̋�؂�
								// '=' �̓f�t�H���g�e���v���[�g�p�����[�^�̎w��
								nMode2 = M2_NORMAL; 
							else if( pLine[i] == '<' )
								nMode2 = M2_TEMPLATE_SAVE;
						}

						if( nMode2 == M2_TEMPLATE_SAVE)
						{
							int nItemNameLen = strlen(szItemName);
							if(nItemNameLen + 1 < nItemNameLenMax )
							{
								szItemName[nItemNameLen] = pLine[i];
								szItemName[nItemNameLen + 1 ] = '\0';
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
