#include "StdAfx.h"
#include "CType.h"
#include "doc/CDocOutline.h"
#include "doc/CEditDoc.h"
#include "outline/CFuncInfoArr.h"
#include "COpeBlk.h"
#include "cmd/CViewCommander_inline.h"
#include "view/CEditView.h"
#include "view/colors/EColorIndexType.h"

//!CPP�L�[���[�h�Ŏn�܂��Ă���� true
inline bool IsHeadCppKeyword(const wchar_t* pData)
{
	#define HEAD_EQ(DATA,LITERAL) (wcsncmp(DATA,LITERAL,_countof(LITERAL)-1)==0)
	if( HEAD_EQ(pData, L"case"      ) )return true;
	if( HEAD_EQ(pData, L"default:"  ) )return true;
	if( HEAD_EQ(pData, L"public:"   ) )return true;
	if( HEAD_EQ(pData, L"private:"  ) )return true;
	if( HEAD_EQ(pData, L"protected:") )return true;
	return false;
}


/* C/C++ */
// Oct. 31, 2000 JEPRO VC++�̐�������e�L�X�g�t�@�C�����ǂ߂�悤�ɂ���
// Jan. 24, 2004 genta �֘A�Â���D�܂����Ȃ��̂�dsw,dsp,dep,mak�͂͂���
//	2003.06.23 Moca �t�@�C��������̓��͕⊮�@�\
void CType_Cpp::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("C/C++") );
	_tcscpy( pType->m_szTypeExts, _T("c,cpp,cxx,cc,cp,c++,h,hpp,hxx,hh,hp,h++,rc,hm") );

	//�ݒ�
	pType->m_cLineComment.CopyTo( 0, L"//", -1 );							/* �s�R�����g�f���~�^ */
	pType->m_cBlockComments[0].SetBlockCommentRule( L"/*", L"*/" );			/* �u���b�N�R�����g�f���~�^ */
	pType->m_cBlockComments[1].SetBlockCommentRule( L"#if 0", L"#endif" );	/* �u���b�N�R�����g�f���~�^2 */	//Jul. 11, 2001 JEPRO
	pType->m_nKeyWordSetIdx[0] = 0;											/* �L�[���[�h�Z�b�g */
	pType->m_eDefaultOutline = OUTLINE_CPP;									/* �A�E�g���C����͕��@ */
	pType->m_eSmartIndent = SMARTINDENT_CPP;								/* �X�}�[�g�C���f���g��� */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;					//���p���l��F�����\��	//Mar. 10, 2001 JEPRO
	pType->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp = true;			//	Sep. 21, 2002 genta �Ί��ʂ̋������f�t�H���gON��
	pType->m_bUseHokanByFile = true;										/*! ���͕⊮ �J���Ă���t�@�C�����������T�� */
}


//	Mar. 15, 2000 genta
//	From Here


/*!
	�֐��ɗp���邱�Ƃ��ł��镶�����ǂ����̔���
*/
inline bool C_IsWordChar( wchar_t c )
{
	return (
		L'_' == c ||
		L':' == c ||
		L'~' == c ||
		(L'a' <= c && c <= L'z' )||
		(L'A' <= c && c <= L'Z' )||
		(L'0' <= c && c <= L'9' )||
		(0xa1 <= c && !iswcntrl(c) && !iswspace(c))
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
static bool C_IsLineEsc(const wchar_t *s, int len)
{
	if ( len > 0 && WCODE::IsLineDelimiter(s[len-1]) ) len--;
	if ( len > 0 && s[len-1] == L'\r' ) len--;

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

static bool CPP_IsFunctionAfterKeyword( const wchar_t* s )
{
	return
		wcscmp( s, L"const" ) == 0 ||
		wcscmp( s, L"volatile" ) == 0 ||
		wcscmp( s, L"throw" ) == 0 ||
		wcscmp( s, L"default" ) == 0 ||
		wcscmp( s, L"delete" ) == 0 ||
		wcscmp( s, L"override" ) == 0 ||
		wcscmp( s, L"final" ) == 0 ||
		wcscmp( s, L"noexcept" ) == 0
		;
}


/*!
	C�v���v���Z�b�T�� #if/ifdef/ifndef - #else - #endif��ԊǗ��N���X

	�l�X�g���x����32���x��=(sizeof(int) * 8)�܂�
	
	@date 2007.12.15 genta : m_enablebuf�̏����l�����������邱�Ƃ�����̂�0��
*/

class CCppPreprocessMng {
public:
	CCppPreprocessMng(void) :
		// 2007.12.15 genta : m_bitpattern��0�ɂ��Ȃ��ƁC
		// �����Ȃ�#else�����ꂽ�Ƃ��Ƀp�^�[�������������Ȃ�
		m_stackptr( 0 ), m_bitpattern( 0 ), m_enablebuf( 0 ), m_maxnestlevel( 32 ), m_ismultiline( false )
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
	@date 2007.12.13 ���イ�� : if�̒���ɃX�y�[�X���Ȃ��ꍇ�̑Ή�

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
		// 2007/12/13 ���イ�� : #if(0)�ƃX�y�[�X���󂯂Ȃ��ꍇ�̑Ή�
		if( C_IsSpace( *p ) || *p == L'(' ){
			//	if 0 �`�F�b�N
			//	skip whitespace
			//	2007.12.15 genta
			for( ; ( C_IsSpace( *p ) || *p == L'(' ) && p < lastptr ; ++p )
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
		//	2007.12.14 genta : #if������#else���o���Ƃ��̃K�[�h�ǉ�
		if( 0 < m_stackptr && m_stackptr < m_maxnestlevel ){
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
#ifdef _DEBUG
// #define TRACE_OUTLINE
#endif
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

	bool bInInitList = false;	// 2010.07.08 ryoji �֐��������̍ہA���݈ʒu�����������X�g�i':'�Ȍ�j�ɓ��B�������ǂ���������
	int			nNestLevel_template = 0; // template<> func<vector<int>> �Ȃǂ�<>�̐�

	wchar_t		szWordPrev[256];	//	1�O��word
	wchar_t		szWord[256];		//	���݉�ǒ���word������Ƃ���
	int			nWordIdx = 0;
	int			nMaxWordLeng = 100;	//	���e�����word�̍ő咷��
	int			nMode;				//	���݂�state
	/*
		nMode
		  0 : �����l
		  1 : �P��ǂݍ��ݒ�
		      szWord�ɒP����i�[
		  2 : �L����
		  8 : �u���b�N�R�����g��
		 10 : // �R�����g��
		 20 : �����萔 ''
		 21 : ������ ""
		999 : �����P�ꖳ��
	*/

	// 2002/10/27 frozen�@��������
	//! ���2
	enum MODE2
	{
		M2_NORMAL			= 0x00,	//!< �ʏ�
		M2_ATTRIBUTE		= 0x02,	//!< C++/CLI attribute : 2007.05.26 genta
		M2_TEMPLATE			= 0x03, //!< "template<" ��M2_TEMPLATE�ɂȂ� '>' ��M2_NORMAL�ɖ߂�
		M2_NAMESPACE_SAVE	= 0x11,	//!< �l�[���X�y�[�X��������
			// �u�ʏ�v��ԂŒP�� "class" "struct" "union" "enum" "namespace", "__interface" ��ǂݍ��ނƁA���̏�ԂɂȂ�A';' '{' ',' '>' '='��ǂݍ��ނƁu�ʏ�v�ɂȂ�B
			//	2007.05.26 genta �L�[���[�h��__interface�ǉ�
			//
			// ':' ��ǂݍ��ނƁu�l�[���X�y�[�X�����������v�ֈڍs����Ɠ�����
			// szWord��szItemName�ɕۑ����A���Ƃ� ':' ���� '{' �̒��O�̒P�ꂪ���ׂ���悤�ɂ��Ă���B
			// ����� "__declspec( dllexport )"�̂悤��"class"�ƃN���X���̊ԂɃL�[���[�h�������Ă���ꍇ�ł��N���X�����擾�ł���悤�ɂ��邽�߁B
			//
			// '<' ��ǂݍ��ނƁu�e���v���[�g�N���X���������v�Ɉڍs����B
		M2_TEMPLATE_SAVE	= 0x12, //!< �e���v���[�g�N���X��������
			// ';' '{'��ǂݍ��ނƁu�ʏ�v�ɂȂ�B
			// �܂��A���̏�Ԃ̊Ԃ͒P�����؂���@���ꎞ�I�ɕύX���A
			// �utemplate_name <paramA,paramB>�v�̂悤�ȕ��������̒P����݂Ȃ��悤�ɂ���B
			// ����͓��ꉻ�����N���X�e���v���[�g����������ۂ̍\���ŗL���ɓ����B	
		M2_NAMESPACE_END	= 0x13,	//!< �l�[���X�y�[�X�����������B(';' '{' ��ǂݍ��񂾎��_�Łu�ʏ�v�ɂȂ�B )
		M2_OPERATOR_WORD	= 0x14, //!< operator���������Boperator�� '('�Ŏ��ɑJ�ځ@template<> names::operator<T>(x)
		M2_TEMPLATE_WORD	= 0x15, //!< �e���v���[�g���ꉻ�𒲍��� func<int>()�� '�P�� <'��M2_TEMPLATE_WORD�ɂȂ�A'>'(�l�X�g�F��)��M2_NORMAL/M2_OPERATOR_WORD�ɖ߂�
		M2_FUNC_NAME_END	= 0x16, //!< �֐������������B(';' '{' ��ǂݍ��񂾎��_�Łu�ʏ�v�ɂȂ�B )
		M2_AFTER_EQUAL		= 0x05,	//!< '='�̌�B
			//�u�ʏ�v���� nNestLevel_fparam==0 ��'='��������Ƃ��̏�ԂɂȂ�B�i������ "opreator"�̒���͏����j
			// ';'��������Ɓu�ʏ�v�ɖ߂�B
			// int val=abs(-1);
			// �̂悤�ȕ����֐��Ƃ݂Ȃ���Ȃ��悤�ɂ��邽�߂Ɏg�p����B
		M2_KR_FUNC	= 0x18,	//!< K&R�X�^�C��/C++�̊֐���`�𒲍�����Bfunc() word ��word������ƑJ�ڂ���
		M2_AFTER_ITEM		= 0x10,
	} nMode2 = M2_NORMAL;
	MODE2 nMode2Old = M2_NORMAL; // M2_TEMPLATE_WORD�ɂȂ钼�O��nMode2
	MODE2 nMode2AttOld = M2_NORMAL;
	bool  bDefinedTypedef = false;	// typedef ��������Ă���Btrue�̊Ԃ͊֐����Ƃ��ĔF�����Ȃ��B;�ŕ��A����
	bool  bNoFunction = true; // fparam�����Ǌ֐���`�łȂ��\���̏ꍇ

	const int	nNamespaceNestMax	= 32;			//!< �l�X�g�\�ȃl�[���X�y�[�X�A�N���X���̍ő吔
	int			nNamespaceLen[nNamespaceNestMax+1];	//!< �l�[���X�y�[�X�S�̂̒���
	const int	nNamespaceLenMax 	= 512;			//!< �ő�̃l�[���X�y�[�X�S�̂̒���
	wchar_t		szNamespace[nNamespaceLenMax];		//!< ���݂̃l�[���X�y�[�X(�I�[��\0�ɂȂ��Ă���Ƃ͌���Ȃ��̂Œ���)
	const int 	nItemNameLenMax	 	= 256;
	wchar_t		szItemName[nItemNameLenMax];		//!< �����O�� �֐��� or �N���X�� or �\���̖� or ���p�̖� or �񋓑̖� or �l�[���X�y�[�X��
	wchar_t		szTemplateName[nItemNameLenMax];		//!< �����O�� �֐��� or �N���X�� or �\���̖� or ���p�̖� or �񋓑̖� or �l�[���X�y�[�X��
	// �Ⴆ�Ή��̃R�[�h�́��̕����ł�
	// szNamespace��"Namespace\ClassName\"
	// nMamespaceLen��{10,20}
	// nNestLevel_global��2�ƂȂ�B
	//
	//�@namespace Namespace{
	//�@class ClassName{
	//�@��
	//�@}}
	wchar_t		szRawStringTag[32];	// C++11 raw string litteral
	int nRawStringTagLen = 0;
	int nRawStringTagCompLen = 0;

	CLogicInt	nItemLine(0);		//!< �����O�� �֐� or �N���X or �\���� or ���p�� or �񋓑� or �l�[���X�y�[�X�̂���s
	int			nItemFuncId = 0;

	szWordPrev[0] = L'\0';
	szWord[nWordIdx] = L'\0';
	szNamespace[0] = L'\0';	// 2002/10/27 frozen
	nNamespaceLen[0] = 0;	// 2002/10/27 frozen
	szItemName[0] = L'\0';
	szTemplateName[0] = L'\0';
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
#ifdef TRACE_OUTLINE
		DEBUG_TRACE(_T("line:%ls"), pLine);
#endif
		for( ; i < nLineLen; ++i ){
#ifdef TRACE_OUTLINE
			DEBUG_TRACE(_T("%2d [%lc] %d %x %d %d %d wd[%ls] pre[%ls] tmp[%ls] til[%ls] %d\n"), int((Int)i), pLine[i], nMode, nMode2,
				nNestLevel_global, nNestLevel_func, nNestLevel_fparam, szWord, szWordPrev, szTemplateName, szItemName, nWordIdx );
#endif
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
			else if( '\\' == pLine[i] && nRawStringTagLen == 0 ){
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
				// operator "" _userliteral
				if( nMode2 == M2_OPERATOR_WORD ){
					int nLen = wcslen(szWordPrev);
					if( nLen + 1 < _countof(szWordPrev) ){
						szWordPrev[nLen] = pLine[i];
						szWordPrev[nLen + 1] = L'\0';
					}
				}
				if( '"' == pLine[i] ){
					if( nRawStringTagLen ){
						// R"abc(test
						// )abc"
						if( nRawStringTagLen <= i && wcsncmp( szRawStringTag, &pLine[i - nRawStringTagLen], nRawStringTagCompLen ) == 0 ){
							nRawStringTagLen = 0;
							nMode = 0;
							continue;
						}
					}else{
						nMode = 0;
						continue;
					}
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
						if( pLine[i] == L':' ){
							if( pLine[i + 1] == L':' ||  0 < i && pLine[i-1] == L':' ){
								// name ::class or class :: member
							}else{
								// class Klass:base �̂悤��:�̑O�ɃX�y�[�X���Ȃ��ꍇ
								if(nMode2 == M2_NAMESPACE_SAVE)
								{
									if(szWord[0]!='\0')
										wcscpy( szItemName, szWord );
									nMode2 = M2_NAMESPACE_END;
								}
								else if( nMode2 == M2_TEMPLATE_SAVE)
								{
									wcsncat( szTemplateName, szWord, nItemNameLenMax - wcslen(szTemplateName) );
									szTemplateName[ nItemNameLenMax - 1 ] = L'\0';
									nMode2 = M2_NAMESPACE_END;
								}
							}
						}
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = L'\0';
					}
				}else{
					// 2002/10/27 frozen�@��������
					if( nMode2 == M2_NAMESPACE_SAVE ){
						if( wcscmp(L"final", szWord) == 0 && wcscmp(L"����", szItemName) != 0 ){
							// strcut name final ��final�̓N���X���̈ꕔ�ł͂Ȃ�
							// ������ struct final�͖��O
						}else{
							wcscpy( szTemplateName, szWord );
							wcscpy( szItemName, szWord );
						}
					}else if( nMode2 == M2_TEMPLATE_SAVE || nMode2 == M2_TEMPLATE_WORD ){
						// strcut name<X> final ��final�̓N���X���̈ꕔ�ł͂Ȃ�
						// struct name<final> ��final�͈ꕔ
						if( wcscmp(L"final", szWord) != 0 || nNestLevel_template != 0 ){
							int nLen = wcslen(szTemplateName);
							if( 0 < nLen && C_IsWordChar(szTemplateName[nLen - 1]) && szTemplateName[nLen - 1] != L':' && szWord[nWordIdx] != L':' ){
								// template func<const x>() �̂悤�ȏꍇ��const�̌��ɃX�y�[�X��}��
								if( nLen + 1 < nItemNameLenMax ){
									wcscat( szTemplateName, L" " );
								}
							}
							wcsncat( szTemplateName, szWord, nItemNameLenMax - nLen );
							szTemplateName[ nItemNameLenMax - 1 ] = L'\0';
						}
					}
					else if( nNestLevel_func == 0 && (nMode2 == M2_NORMAL || nMode2 == M2_FUNC_NAME_END) )	// 2010.07.08 ryoji �֐��^�}�N���ďo�����֐��ƌ�F���邱�Ƃ�������΍�Ƃ��� nMode2 == M2_FUNC_NAME_END ������ǉ����A�␳��������悤�ɂ����B
					{
						if( nMode2 == M2_NORMAL )
							nItemFuncId = 0;
						if( wcscmp(szWord,L"class")==0 )
							nItemFuncId = FL_OBJ_CLASS;
						else if( wcscmp(szWord,L"struct")==0 )
							nItemFuncId = FL_OBJ_STRUCT;
						else if( wcscmp(szWord,L"namespace")==0 )
							nItemFuncId = FL_OBJ_NAMESPACE;
						else if( wcscmp(szWord,L"enum")==0 )
							nItemFuncId = FL_OBJ_ENUM;
						else if( wcscmp(szWord,L"union")==0 )
							nItemFuncId = FL_OBJ_UNION;
						else if( wcscmp(szWord,L"__interface")==0 ) // 2007.05.26 genta "__interface" ���N���X�ɗނ��鈵���ɂ���
							nItemFuncId = FL_OBJ_INTERFACE;
						else if( wcscmp(szWord,L"typedef") == 0 )
							bDefinedTypedef = true;
						if( nItemFuncId != 0 && nItemFuncId != FL_OBJ_FUNCTION )	//  2010.07.08 ryoji nMode2 == M2_FUNC_NAME_END �̂Ƃ��� nItemFuncId == 2 �̂͂�
						{
							nMode2 = M2_NAMESPACE_SAVE;
							nItemLine = nLineCount + CLogicInt(1);
							wcscpy(szItemName,L"����");
						}
					}
					/*else*/ if( nMode2 == M2_FUNC_NAME_END )	// 2010.07.08 ryoji ��ŏ����ύX�����̂ōs���� else ������
					{
						nMode2 = M2_KR_FUNC;
					}
					// 2002/10/27 frozen�@�����܂�
					if( nMode2 == M2_NORMAL ){
						// template�͏I�����
						szTemplateName[0] = L'\0';
					}

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
					 (L'/' == pLine[i] && (L'*' == pLine[i+1] || L'/' == pLine[i+1]))
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
					if( nMode2 == M2_TEMPLATE_SAVE || nMode2 == M2_TEMPLATE_WORD ){
						int nItemNameLen = wcslen(szTemplateName);
						if(nItemNameLen + 1 < nItemNameLenMax ){
							szTemplateName[nItemNameLen] = pLine[i];
							szTemplateName[nItemNameLen + 1 ] = L'\0';
						}
					}
					if( nMode2 == M2_OPERATOR_WORD && L'<' == pLine[i] ){
						const wchar_t* p = &szWord[nWordIdx-8];
						if(  (8 <= nWordIdx && wcsncmp(L"operator<", p, 9) == 0)
						 || ((9 <= nWordIdx && wcsncmp(L"operator<<", p-1, 10) == 0) && 0 < i && L'<' == pLine[i-1]) ){
							// �Ⴄ�Foperator<<const() / operator<<()
						}else{
							// operator< <T>() / operator<<<T>() / operator+<T>()
							nMode2Old = nMode2;
							nMode2 = M2_TEMPLATE_WORD;
							wcsncpy( szTemplateName, szWord, nItemNameLenMax );
							szTemplateName[ nItemNameLenMax - 1 ] = L'\0';
						}
					}
					if( nMode2 == M2_TEMPLATE || nMode2 == M2_TEMPLATE_SAVE || nMode2 == M2_TEMPLATE_WORD ){
						if( pLine[i] == L'<' ){
							nNestLevel_template++;
						}else if( pLine[i] == L'>' ){
							wcscpy( szItemName, szTemplateName );
							nNestLevel_template--;
							if( nNestLevel_template == 0 ){
								if( nMode2 == M2_TEMPLATE ){
									nMode2 = M2_NORMAL;
								}else if( nMode2 == M2_TEMPLATE_WORD ){
									nMode2 = nMode2Old;
									if( nMode2 == M2_OPERATOR_WORD ){
										wcscpy(szWord, szTemplateName);
										nWordIdx = wcslen(szWord) - 1;
										szTemplateName[0] = L'\0';
									}
								}
							}
						}
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
					int nLen = (int)wcslen(szWordPrev);
					if( nMode2 == M2_NORMAL && C_IsOperator(szWordPrev, nLen) ){
						// ���Z�q�̃I�y���[�^������ operator ""i
						if( nLen + 1 < _countof(szWordPrev) ){
							szWordPrev[nLen] = pLine[i];
							szWordPrev[nLen + 1] = L'\0';
						}
						nMode2 = M2_OPERATOR_WORD;
					}else{
						if( wcscmp( szWordPrev, L"R" ) == 0 ||
							wcscmp( szWordPrev, L"LR" ) == 0 ||
							wcscmp( szWordPrev, L"uR" ) == 0 ||
							wcscmp( szWordPrev, L"UR" ) == 0 ||
							wcscmp( szWordPrev, L"u8R" ) == 0
						){
							// C++11 raw string
							// R"abc(string)abc"
							for( int k = i + 1; k < nLineLen; k++ ){
								if( pLine[k] == L'(' ){
									// i = 1, k = 5, len = 5-1-1=3
									CLogicInt tagLen = t_min(k - i - 1, CLogicInt(_countof(szRawStringTag) - 1));
									nRawStringTagLen = tagLen + 1;
									szRawStringTag[0] = L')';
									wcsncpy( szRawStringTag + 1, &pLine[i+1], tagLen );
									szRawStringTag[nRawStringTagLen] = L'\0';
									nRawStringTagCompLen = auto_strlen(szRawStringTag);
									break;
								}
							}
						}
					}
					nMode = 21;
					continue;
				}else
				
				// 2002/10/27 frozen ��������
				if( '{' == pLine[i] )
				{
					bool bAddFunction = false;
					if( nMode2 == M2_FUNC_NAME_END || nMode2 == M2_KR_FUNC ){
						bAddFunction = true;
					}
					int nItemNameLen = 0;
					if( nNestLevel_func !=0 || (szWordPrev[0] == L'=' && szWordPrev[1] == L'\0') || nMode2 == M2_AFTER_EQUAL )
						++nNestLevel_func;
					else if(
							(nMode2 & M2_AFTER_ITEM) != 0  &&
							nNestLevel_global < nNamespaceNestMax &&
							(nNamespaceLen[nNestLevel_global] +  (nItemNameLen = wcslen(szItemName)) + 6 + 1) < nNamespaceLenMax)
					// �R�Ԗڂ�(&&�̌��)����
					// �o�b�t�@������Ȃ��ꍇ�͍��ڂ̒ǉ����s��Ȃ��B
					// +6�͒ǉ����镶����̍ő咷(�ǉ����镶�����"::��`�ʒu"���Œ�)
					// +1�͏I�[NUL����
					{
						wcscpy( &szNamespace[nNamespaceLen[nNestLevel_global]] , szItemName);
						szItemName[0] = L'\0';
						//	Jan. 30, 2005 genta M2_KR_FUNC �ǉ�
						//	�֐��̌���const, throw �܂��͏������q�������
						//	M2_KR_FUNC�ɑJ�ڂ��āC';'��������Ȃ��Ƃ��̏�Ԃ̂܂�
						//	�����ʂɑ�������D
						if( bAddFunction ){
							++ nNestLevel_func;
						}
						else
						{
							++ nNestLevel_global;
							nNamespaceLen[nNestLevel_global] = nNamespaceLen[nNestLevel_global-1] + nItemNameLen;
							if( nItemFuncId == FL_OBJ_NAMESPACE )
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
#ifdef TRACE_OUTLINE
						DEBUG_TRACE( _T("AppendData %d %ls\n"), nItemLine, szNamespace );
#endif
						pcFuncInfoArr->AppendData( nItemLine, ptPosXY.GetY2() + CLayoutInt(1) , szNamespace, nItemFuncId);
						bDefinedTypedef = false;
						nItemLine = -1;
						//	Jan. 30, 2005 genta M2_KR_FUNC �ǉ�
						if( !bAddFunction )
						{
							szNamespace[nNamespaceLen[nNestLevel_global]] = L':';
							nNamespaceLen[nNestLevel_global] += 2;
						}
					}
					else
					{
						//	Jan. 30, 2005 genta M2_KR_FUNC �ǉ�
						if( bAddFunction )
							++ nNestLevel_func;
						else
						{
							++ nNestLevel_global;
							if ( nNestLevel_global <= nNamespaceNestMax )
								nNamespaceLen[nNestLevel_global]=nNamespaceLen[nNestLevel_global-1];
						}
					}
					// bCppInitSkip = false;	//	Mar. 4, 2001 genta
					nNestLevel_template = 0;
					nMode = 0;
					nMode2 = M2_NORMAL;
					nMode2Old = M2_NORMAL;
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
					nMode2Old = M2_NORMAL;
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
					int nLen = (int)wcslen(szWordPrev);
					bool bOperator = false;
					if( nMode2 == M2_NORMAL && nNestLevel_fparam == 0 && C_IsOperator(szWordPrev, nLen) ){
						int k;
						for( k = i + 1; k < nLineLen && C_IsSpace(pLine[k]); k++){}
						if( k < nLineLen && pLine[k] == L')' ){
							for( k++; k < nLineLen && C_IsSpace(pLine[k]); k++){}
							if( k < nLineLen && (pLine[k] == L'<' || pLine[k] == L'(' ) ){
								// �I�y���[�^������ operator()( / operator()<;
								if( nLen + 1 < _countof(szWordPrev) ){
									szWordPrev[nLen] = pLine[i];
									szWordPrev[nLen + 1] = L'\0';
								}
								nMode2 = M2_OPERATOR_WORD;
								bOperator = true;
							}else{
								// �I�y���[�^��operator�Ƃ���C�̊֐�
							}
						}
					}
					//	2007.05.26 genta C++/CLI nMode2 == M2_NAMESPACE_END�̏ꍇ��ΏۊO��
					//	NAMESPACE_END(class �N���X�� :�̌��)�ɂ����Ă�()���֐��Ƃ݂Ȃ��Ȃ��D
					//	TEMPLATE<sizeof(int)> �̂悤�ȃP�[�X��sizeof���֐��ƌ�F����D
					if( !bOperator && nNestLevel_func == 0
					  && (nMode2 == M2_NORMAL || nMode2 == M2_NAMESPACE_SAVE || nMode2 == M2_OPERATOR_WORD || nMode2 == M2_KR_FUNC) ){
						if(nNestLevel_fparam==0)
						{
							bool bAdd = true;
							if( wcscmp(szWordPrev, L"__declspec") == 0
									|| wcscmp(szWordPrev, L"alignas") == 0
									|| wcscmp(szWordPrev, L"decltype") == 0
									|| wcscmp(szWordPrev, L"_Alignas") == 0
									|| wcscmp(szWordPrev, L"__attribute__") == 0
							){
								bAdd = false;
							}else{
								if( !(nMode2 == M2_KR_FUNC && bInInitList) && !(nMode2 == M2_KR_FUNC
									&& CPP_IsFunctionAfterKeyword(szWordPrev)) )	// 2010.07.08 ryoji ���������X�g�ɓ���ȑO�܂ł͌㔭�̖��O��D��I�Ɋ֐������Ƃ���
								{
									bNoFunction = false;
								}else{
									bAdd = false;
								}
							}
							if( bAdd ){
								if( szTemplateName[0] ){
									wcscpy(szItemName, szTemplateName);
								}else{
									wcscpy(szItemName, szWordPrev);
								}
								nItemLine = nLineCount + CLogicInt(1);
							}
						}
						++ nNestLevel_fparam;
					}
					//  2002/10/27 frozen �����܂�
					if( nMode2 == M2_TEMPLATE_SAVE || nMode2 == M2_TEMPLATE_WORD ){
						int nItemNameLen = wcslen(szTemplateName);
						if( nItemNameLen + 1 < nItemNameLenMax ){
							szTemplateName[nItemNameLen] = pLine[i];
							szTemplateName[nItemNameLen + 1 ] = L'\0';
						}
					}
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
						//	2007.05.26 genta C++/CLI Attribute�����ł�nMode2�̕ύX�͍s��Ȃ�
						if( nNestLevel_fparam == 0 && nMode2 != M2_ATTRIBUTE && nMode2 != M2_TEMPLATE_WORD ){
							if( nMode2 == M2_NORMAL )
								bInInitList = false;
							if( !bNoFunction ){
								nMode2 = M2_FUNC_NAME_END;
								nItemFuncId = FL_OBJ_FUNCTION;
							}
						}
					}
					//  2002/10/27 frozen �����܂�
					if( nMode2 == M2_OPERATOR_WORD ){
						int nLen = wcslen(szWordPrev);
						if( nLen + 1 < _countof(szWordPrev) ){
							szWordPrev[nLen] = pLine[i];
							szWordPrev[nLen + 1] = L'\0';
						}
					}else if( nMode2 == M2_TEMPLATE_SAVE || nMode2 == M2_TEMPLATE_WORD ){
						int nItemNameLen = wcslen(szTemplateName);
						if( nItemNameLen + 1 < nItemNameLenMax ){
							szTemplateName[nItemNameLen] = pLine[i];
							szTemplateName[nItemNameLen + 1 ] = L'\0';
						}
					}
					continue;
				}else
				// From Here 2007.05.26 genta C++/CLI Attribute�̎�舵��
				if( '[' == pLine[i] ){
					int nLen = (int)wcslen(szWordPrev);
					if( nMode2 == M2_NORMAL && C_IsOperator(szWordPrev, nLen) ){
						// ���Z�q�̃I�y���[�^������ operator []
						if( nLen + 1 < _countof(szWordPrev) ){
							szWordPrev[nLen] = pLine[i];
							szWordPrev[nLen + 1] = L'\0';
						}
						nMode2 = M2_OPERATOR_WORD;
					}else
					if( nNestLevel_func == 0 && nNestLevel_fparam == 0 &&
						(nMode2 == M2_NORMAL || nMode2 == M2_NAMESPACE_SAVE || nMode2 ==  M2_TEMPLATE_SAVE
						 || nMode2 == M2_OPERATOR_WORD || nMode2 == M2_TEMPLATE_WORD )) {
						nMode2AttOld = nMode2;
						nMode2 = M2_ATTRIBUTE;
					}
					continue;
				} else
				if( ']' == pLine[i] ){
					//	Attribute�����ł�[]��z��Ƃ��Ďg����������Ȃ��̂ŁC
					//	���ʂ̃��x���͌��ɖ߂��Ă���K�v�L��
					if( nNestLevel_fparam == 0 && nMode2 == M2_ATTRIBUTE ) {
						nMode2 = nMode2AttOld;
						continue;
					}
					if( nMode2 == M2_OPERATOR_WORD ){
						int nLen = wcslen(szWordPrev);
						if( nLen + 1 < _countof(szWordPrev) ){
							szWordPrev[nLen] = pLine[i];
							szWordPrev[nLen + 1] = L'\0';
						}
					}
				} else
				// To Here 2007.05.26 genta C++/CLI Attribute�̎�舵��
				if( ';' == pLine[i] ){
					//  2002/10/27 frozen ��������
					if( nMode2 == M2_KR_FUNC )
					{
						// zenryaku K&R�X�^�C���̊֐��錾�̏I���� M2_FUNC_NAME_END �ɂ��ǂ�
						nMode2 = M2_FUNC_NAME_END;
					} //	Jan. 30, 2005 genta K&R�����Ɉ��������Đ錾�������s���D
					if( nMode2 == M2_FUNC_NAME_END &&
						nNestLevel_global < nNamespaceNestMax &&
						(nNamespaceLen[nNestLevel_global] + wcslen(szItemName)) < nNamespaceLenMax &&
						nNestPoint_class == 0 && !bDefinedTypedef && 0 < nItemLine)
					// �R�Ԗڂ�(&&�̌��)����
					// �o�b�t�@������Ȃ��ꍇ�͍��ڂ̒ǉ����s��Ȃ��B
					{
						wcscpy( &szNamespace[nNamespaceLen[ nNestLevel_global]] , szItemName);

						nItemFuncId = FL_OBJ_DECLARE;
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
#ifdef TRACE_OUTLINE
						DEBUG_TRACE( _T("AppendData %d %ls\n"), nItemLine, szNamespace );
#endif
						pcFuncInfoArr->AppendData( nItemLine, ptPosXY.GetY2() + CLayoutInt(1), szNamespace, nItemFuncId);
					}
					nItemLine = -1;
					nNestLevel_template = 0;
					nMode2 = M2_NORMAL;
					//  2002/10/27 frozen �����܂�
					bNoFunction = true;
					bDefinedTypedef = false;
					nMode2Old = M2_NORMAL;
					nMode = 0;
					continue;
				}else if( nNestLevel_fparam == 0 && nMode2 != M2_ATTRIBUTE ){
					// 2007.05.26 genta C++/CLI Attribute�����ł͊֐��������͈�؍s��Ȃ�
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
								if( nMode2 == M2_NORMAL || nMode2 == M2_OPERATOR_WORD ){
									if( szTemplateName[0] ){
										wcscpy( szWord, szTemplateName );
										szTemplateName[0] = '\0';
									}else{
										wcscpy( szWord, szWordPrev );
									}
								}
								nWordIdx = wcslen( szWord );
							}
							//	From Here Apr. 1, 2001 genta
							//	operator new/delete ���Z�q�̑Ή�
							else if( nMode2 == M2_NORMAL && C_IsOperator( szWordPrev, pos + 2 ) ){
								if( -1 < pos && C_IsWordChar( szWordPrev[pos + 1] ) ){
									//	�X�y�[�X�����āA�O�̕�����ɑ�����
									szWordPrev[pos + 2] = L' ';
									szWordPrev[pos + 3] = L'\0';
								}
								wcscpy( szWord, szWordPrev );
								nWordIdx = wcslen( szWord );
								nMode2 = M2_OPERATOR_WORD;
							}else if( nMode2 == M2_OPERATOR_WORD ){
								// operator �p����
								if( -1 < pos && C_IsWordChar( szWordPrev[pos + 1] ) ){
									//	�X�y�[�X�����āA�O�̕�����ɑ�����
									szWordPrev[pos + 2] = L' ';
									szWordPrev[pos + 3] = L'\0';
								}
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
						if( pLine[i] == L':' && pLine[i+1] != L':')
						{
							if(nMode2 == M2_NAMESPACE_SAVE)
							{
								if(szWord[0]!='\0')
									wcscpy( szItemName, szWord );
								nMode2 = M2_NAMESPACE_END;
							}
							else if( nMode2 == M2_TEMPLATE_SAVE)
							{
								wcsncat( szTemplateName, szWord, nItemNameLenMax - wcslen(szTemplateName) );
								szTemplateName[ nItemNameLenMax - 1 ] = L'\0';
								nMode2 = M2_NAMESPACE_END;
							}
							else if( nMode2 == M2_FUNC_NAME_END || nMode2 == M2_KR_FUNC )
							{
								bInInitList = true;
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
						// 2011.12.02 template�Ή�
						if( nMode2 == M2_NORMAL || nMode2 == M2_OPERATOR_WORD ){
							if( pLine[i] == L'<' ){
								int nLen = (int)wcslen(szWordPrev);
								if( wcscmp(szWordPrev, L"template") == 0 ){
									nMode2 = M2_TEMPLATE;
									szTemplateName[0] = L'\0';
								}else if( C_IsOperator(szWordPrev, nLen) ){
									// operator<
								}else{
									// int func <int>();
									nMode2Old = nMode2;
									nMode2 = M2_TEMPLATE_WORD;
									wcsncpy( szTemplateName, szWordPrev, nItemNameLenMax );
									szTemplateName[ nItemNameLenMax - 1 ] = L'\0';
								}
							}else{
								szTemplateName[0] = L'\0';
							}
						}
						//	Aug. 13, 2004 genta
						//	szWordPrev�������Ȃ�������operator�̔�����s��
						//	operator�̔���͑O�ɃN���X�����t���Ă���\��������̂�
						//	��p�̔���֐����g���ׂ��D
						//	operator�Ŗ������=�͑���Ȃ̂ł����͐錾���ł͂Ȃ��D
						int nLen = (int)wcslen(szWordPrev);
						if( pLine[i] == L'=' && nNestLevel_func == 0
							&& nMode2 == M2_NORMAL && ! C_IsOperator(szWordPrev,nLen) ){
							nMode2 = M2_AFTER_EQUAL;
						}else if( nMode2 == M2_NORMAL && C_IsOperator(szWordPrev, nLen) ){
							// ���Z�q�̃I�y���[�^������ operator +
							wcscpy(szWord, szWordPrev);
							nWordIdx = (int)nLen -1;
							nMode2 = M2_OPERATOR_WORD;
						}else if( nMode2 == M2_OPERATOR_WORD ){
							// operator �p����
							wcscpy(szWord, szWordPrev);
							nWordIdx = (int)nLen -1;
						}else{
							wcscpy(szWordPrev, szWord);
							nWordIdx = -1;
						}
						// 2002/10/27 frozen ��������
						if( nMode2 == M2_NAMESPACE_SAVE )
						{
#if 0
							if( pLine[i] == L'>' || pLine[i] == L',' || pLine[i] == L'=')
								// '<' �̑O�� '>' , ',' , '=' ���������̂ŁA�����炭
								// �O�ɂ�����"class"�̓e���v���[�g�p�����[�^�̌^��\���Ă����ƍl������B
								// ����āA�N���X���̒����͏I���B
								// '>' �̓e���v���[�g�p�����[�^�̏I��
								// ',' �̓e���v���[�g�p�����[�^�̋�؂�
								// '=' �̓f�t�H���g�e���v���[�g�p�����[�^�̎w��
								nMode2 = M2_NORMAL; 
							else
#endif
							if( pLine[i] == L'<' )
								nMode2 = M2_TEMPLATE_SAVE;
						}
						nMode = 2;
						i--;
					}
				}
			}
		}
	}
}



/* C/C++�X�}�[�g�C���f���g���� */
void CEditView::SmartIndent_CPP( wchar_t wcChar )
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int			k;
	int			m;
	const wchar_t*	pLine2;
	CLogicInt	nLineLen2;
	int			nLevel;
	CLogicInt j;

	/* �����ɂ���Ēu�������ӏ� */
	CLogicRange sRangeA;
	sRangeA.Clear(-1);

	wchar_t*	pszData = NULL;
	CLogicInt	nDataLen;

	int			nWork = 0;
	CDocLine*	pCDocLine = NULL;
	int			nCharChars;
	int			nSrcLen;
	wchar_t		pszSrc[1024];
	BOOL		bChange;

	int			nCaretPosX_PHY;

	CLogicPoint	ptCP;

	if(wcChar==WCODE::CR || wcschr(L":{}()",wcChar)!=NULL){
		//���֐i��
	}else return;

	switch( wcChar ){
	case WCODE::CR:
	case L':':
	case L'}':
	case L')':
	case L'{':
	case L'(':

		nCaretPosX_PHY = GetCaret().GetCaretLogicPos().x;

		pLine = m_pcEditDoc->m_cDocLineMgr.GetLine(GetCaret().GetCaretLogicPos().GetY2())->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine ){
			if( WCODE::CR != wcChar ){
				return;
			}
			/* �����ɂ���Ēu�������ӏ� */
			sRangeA.Set(CLogicPoint(0,GetCaret().GetCaretLogicPos().y));
		}else{
			pCDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() );


			//	nWork�ɏ����̊���ʒu��ݒ肷��
			if( WCODE::CR != wcChar ){
				nWork = nCaretPosX_PHY - 1;
			}else{
				/*
				|| CR�����͂��ꂽ���A�J�[�\������̎��ʎq���C���f���g����B
				|| �J�[�\������̎��ʎq��'}'��')'�Ȃ��
				|| '}'��')'�����͂��ꂽ���Ɠ�������������
				*/

				int i;
				for( i = nCaretPosX_PHY; i < nLineLen; i++ ){
					if( WCODE::TAB != pLine[i] && WCODE::SPACE != pLine[i] ){
						break;
					}
				}
				if( i < nLineLen ){
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
					if( 1 == nCharChars && ( pLine[i] == L')' || pLine[i] == L'}' ) ){
						wcChar = pLine[i];
					}
					nCaretPosX_PHY = i;
					nWork = nCaretPosX_PHY;
				}else{
					nWork = nCaretPosX_PHY;
				}
			}
			int i;
			for( i = 0; i < nWork; i++ ){
				if( WCODE::TAB != pLine[i] && WCODE::SPACE != pLine[i] ){
					break;
				}
			}
			if( i < nWork ){
				if( ( L':' == wcChar && IsHeadCppKeyword(&pLine[i]) )
					//	Sep. 18, 2002 �����
					|| ( L'{' == wcChar && L'#' != pLine[i] )
					|| ( L'(' == wcChar && L'#' != pLine[i] )
				){

				}else{
					return;
				}
			}else{
				if( L':' == wcChar ){
					return;
				}
			}
			/* �����ɂ���Ēu�������ӏ� */
			sRangeA.SetFrom(CLogicPoint(0, GetCaret().GetCaretLogicPos().GetY2()));
			sRangeA.SetTo(CLogicPoint(i, GetCaret().GetCaretLogicPos().GetY2()));
		}


		/* �Ή����銇�ʂ������� */
		nLevel = 0;	/* {}�̓���q���x�� */


		nDataLen = CLogicInt(0);
		for( j = GetCaret().GetCaretLogicPos().GetY2(); j >= CLogicInt(0); --j ){
			pLine2 = m_pcEditDoc->m_cDocLineMgr.GetLine(j)->GetDocLineStrWithEOL(&nLineLen2);
			if( j == GetCaret().GetCaretLogicPos().y ){
				// 2005.10.11 ryoji EOF �݂̂̍s���X�}�[�g�C���f���g�̑Ώۂɂ���
				if( NULL == pLine2 ){
					if( GetCaret().GetCaretLogicPos().y == m_pcEditDoc->m_cDocLineMgr.GetLineCount() )
						continue;	// EOF �݂̂̍s
					break;
				}
				nCharChars = CLogicInt(&pLine2[nWork] - CNativeW::GetCharPrev( pLine2, nLineLen2, &pLine2[nWork] ));
				k = nWork - nCharChars;
			}else{
				if( NULL == pLine2 )
					break;
				nCharChars = CLogicInt(&pLine2[nLineLen2] - CNativeW::GetCharPrev( pLine2, nLineLen2, &pLine2[nLineLen2] ));
				k = nLineLen2 - nCharChars;
			}

			for( ; k >= 0; /*k--*/ ){
				if( 1 == nCharChars && ( L'}' == pLine2[k] || L')' == pLine2[k] )
				){
					if( 0 < k && L'\'' == pLine2[k - 1]
					 && nLineLen2 - 1 > k && L'\'' == pLine2[k + 1]
					){
//						MYTRACE( _T("��[%ls]\n"), pLine2 );
					}else{
						//�����s�̏ꍇ
						if( j == GetCaret().GetCaretLogicPos().y ){
							if( L'{' == wcChar && L'}' == pLine2[k] ){
								wcChar = L'}';
								nLevel--;	/* {}�̓���q���x�� */
							}
							if( L'(' == wcChar && L')' == pLine2[k] ){
								wcChar = L')';
								nLevel--;	/* {}�̓���q���x�� */
							}
						}

						nLevel++;	/* {}�̓���q���x�� */
					}
				}
				if( 1 == nCharChars && ( L'{' == pLine2[k] || L'(' == pLine2[k] )
				){
					if( 0 < k && L'\'' == pLine2[k - 1]
					 && nLineLen2 - 1 > k && L'\'' == pLine2[k + 1]
					){
//						MYTRACE( _T("��[%ls]\n"), pLine2 );
					}else{
						//�����s�̏ꍇ
						if( j == GetCaret().GetCaretLogicPos().y ){
							if( L'{' == wcChar && L'{' == pLine2[k] ){
								return;
							}
							if( L'(' == wcChar && L'(' == pLine2[k] ){
								return;
							}
						}
						if( 0 == nLevel ){
							break;
						}else{
							nLevel--;	/* {}�̓���q���x�� */
						}

					}
				}
				nCharChars = CLogicInt(&pLine2[k] - CNativeW::GetCharPrev( pLine2, nLineLen2, &pLine2[k] ));
				if( 0 == nCharChars ){
					nCharChars = CLogicInt(1);
				}
				k -= nCharChars;
			}
			if( k < 0 ){
				/* ���̍s�ɂ͂Ȃ� */
				continue;
			}

			for( m = 0; m < nLineLen2; m++ ){
				if( WCODE::TAB != pLine2[m] && WCODE::SPACE != pLine2[m] ){
					break;
				}
			}


			nDataLen = CLogicInt(m);
			nCharChars = (m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bInsSpace)? (Int)m_pcEditDoc->m_cLayoutMgr.GetTabSpace(): 1;
			pszData = new wchar_t[nDataLen + nCharChars + 1];
			wmemcpy( pszData, pLine2, nDataLen );
			if( WCODE::CR  == wcChar || L'{' == wcChar || L'(' == wcChar ){
				// 2005.10.11 ryoji TAB�L�[��SPACE�}���̐ݒ�Ȃ�ǉ��C���f���g��SPACE�ɂ���
				//	����������̉E�[�̕\���ʒu�����߂���ő}������X�y�[�X�̐������肷��
				if( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bInsSpace ){	// SPACE�}���ݒ�
					int i;
					i = m = 0;
					while( i < nDataLen ){
						nCharChars = CNativeW::GetSizeOfChar( pszData, nDataLen, i );
						if( nCharChars == 1 && WCODE::TAB == pszData[i] )
							m += (Int)m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace(CLayoutInt(m));
						else
							m += nCharChars;
						i += nCharChars;
					}
					nCharChars = (Int)m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace(CLayoutInt(m));
					for( int i = 0; i < nCharChars; i++ )
						pszData[nDataLen + i] = WCODE::SPACE;
					pszData[nDataLen + nCharChars] = L'\0';
					nDataLen += CLogicInt(nCharChars);
				}else{
					pszData[nDataLen] = WCODE::TAB;
					pszData[nDataLen + 1] = L'\0';
					++nDataLen;
				}
			}else{
				pszData[nDataLen] = L'\0';

			}
			break;
		}
		if( j < 0 ){
			/* �Ή����銇�ʂ�������Ȃ����� */
			if( WCODE::CR == wcChar ){
				return;
			}else{
				nDataLen = CLogicInt(0);
				pszData = new wchar_t[nDataLen + 1];
				pszData[nDataLen] = L'\0';
			}
		}

		/* ������̃J�[�\���ʒu���v�Z���Ă��� */
		ptCP.x = nCaretPosX_PHY - sRangeA.GetTo().x + nDataLen;
		ptCP.y = GetCaret().GetCaretLogicPos().y;

		nSrcLen = sRangeA.GetTo().x - sRangeA.GetFrom().x;
		if( nSrcLen >= _countof( pszSrc ) - 1 ){
			//	Sep. 18, 2002 genta ���������[�N�΍�
			delete [] pszData;
			return;
		}
		if( NULL == pLine ){
			pszSrc[0] = L'\0';
		}else{
			wmemcpy( pszSrc, &pLine[sRangeA.GetFrom().x], nSrcLen );
			pszSrc[nSrcLen] = L'\0';
		}


		/* �����ɂ���Ēu�������ӏ� */
		CLayoutRange sRangeLayout;
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout( sRangeA, &sRangeLayout );

		if( ( 0 == nDataLen && sRangeLayout.IsOne() )
		 || ( nDataLen == nSrcLen && 0 == wmemcmp( pszSrc, pszData, nDataLen ) )
		 ){
			bChange = FALSE;
		}else{
			bChange = TRUE;

			/* �f�[�^�u�� �폜&�}���ɂ��g���� */
			ReplaceData_CEditView(
				sRangeLayout,
				NULL,		/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
				pszData,	/* �}������f�[�^ */
				nDataLen,	/* �}������f�[�^�̒��� */
				true,
				m_bDoing_UndoRedo?NULL:m_cCommander.GetOpeBlk()
			);
		}


		/* �J�[�\���ʒu���� */
		CLayoutPoint ptCP_Layout;
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptCP, &ptCP_Layout );

		/* �I���G���A�̐擪�փJ�[�\�����ړ� */
		GetCaret().MoveCursor( ptCP_Layout, true );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();


		if( bChange && !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			/* ����̒ǉ� */
			m_cCommander.GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
					GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
				)
			);
		}
		break;
	}
	if( NULL != pszData ){
		delete [] pszData;
		pszData = NULL;
	}
}



const wchar_t* g_ppszKeywordsCPP[] = {
	L"#define",
	L"#elif",
	L"#else",
	L"#endif",
	L"#error",
	L"#if",
	L"#ifdef",
	L"#ifndef",
	L"#include",
	L"#line",
	L"#pragma",
	L"#undef",
	L"_Alignas",
	L"_Alignof",
	L"_Atomic",
	L"_Bool",
	L"_Complex",
	L"_Generic",
	L"_Imaginary",
	L"_Noreturn",
	L"_Pragma",
	L"_Static_assert",
	L"_Thread_local",
	L"__DATE__",
	L"__FILE__",
	L"__LINE__",
	L"__TIME__",
	L"__VA_ARGS__",
	L"__cplusplus",
	L"__declspec",
	L"__func__",
	L"alignas",
	L"alignof",
	L"and",
	L"and_eq",
	L"asm",
	L"auto",
	L"bitand",
	L"bitor",
	L"bool",
	L"break",
	L"case",
	L"catch",
	L"char",
	L"char16_t",
	L"char32_t",
	L"class",
	L"compl",
	L"const",
	L"const_cast",
	L"constexpr",
	L"continue",
	L"decltype"
	L"default",
	L"define",
	L"defined",
	L"delete",
	L"do",
	L"double",
	L"dynamic_cast",
	L"elif",
	L"else",
	L"endif",
	L"enum",
	L"error",
	L"explicit",
	L"export",
	L"extern",
	L"false",
	L"final"
	L"float",
	L"for",
	L"friend",
	L"goto",
	L"if",
	L"ifdef",
	L"ifndef",
	L"include",
	L"inline",
	L"int",
	L"line",
	L"long",
	L"mutable",
	L"namespace",
	L"new",
	L"noexcept"
	L"not",
	L"not_eq",
	L"nullptr"
	L"operator",
	L"or",
	L"or_eq",
	L"override",
	L"pragma",
	L"private",
	L"protected",
	L"public",
	L"register",
	L"reinterpret_cast",
	L"restrict",
	L"return",
	L"short",
	L"signed",
	L"sizeof",
	L"static",
	L"static_assert"
	L"static_cast",
	L"struct",
	L"switch",
	L"template",
	L"this",
	L"thread_local"
	L"throw",
	L"true",
	L"try",
	L"typedef",
	L"typeid",
	L"typename",
	L"undef",
	L"union",
	L"unsigned",
	L"using",
	L"virtual",
	L"void",
	L"volatile",
	L"wchar_t",
	L"while"
	L"xor",
	L"xor_eq",
};
int g_nKeywordsCPP = _countof(g_ppszKeywordsCPP);
