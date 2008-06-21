#pragma once

#include "doc/CDocTypeSetting.h"
#include "config/maxdata.h" //MAX_REGEX_KEYWORD
#include "doc/CLineComment.h"
#include "doc/CBlockComment.h"

/* �A�E�g���C����͂̎�� */
SAKURA_CORE_API enum EOutlineType{
	OUTLINE_C,
	OUTLINE_CPP,
	OUTLINE_PLSQL,
	OUTLINE_TEXT,
	OUTLINE_JAVA,
	OUTLINE_COBOL,
	OUTLINE_ASM,
	OUTLINE_PERL,		//	Sep. 8, 2000 genta
	OUTLINE_VB,			//	June 23, 2001 N.Nakatani
	OUTLINE_WZTXT,		// 2003.05.20 zenryaku �K�w�t�e�L�X�g�A�E�g���C�����
	OUTLINE_HTML,		// 2003.05.20 zenryaku HTML�A�E�g���C�����
	OUTLINE_TEX,		// 2003.07.20 naoh TeX�A�E�g���C�����
	OUTLINE_FILE,		//	2002.04.01 YAZAKI ���[���t�@�C���p
	OUTLINE_PYTHON,		//	2007.02.08 genta Python�A�E�g���C�����
	OUTLINE_CODEMAX,
	OUTLINE_BOOKMARK,	//	2001.12.03 hor
	OUTLINE_DEFAULT =-1,//	2001.12.03 hor
	OUTLINE_UNKNOWN	= 99
};

/* �X�}�[�g�C���f���g��� */
SAKURA_CORE_API enum ESmartIndentType {
	SMARTINDENT_NONE,
	SMARTINDENT_CPP
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �^�C�v�ʐݒ�                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! �^�C�v�ʐݒ�
struct STypeConfig{
	//2007.09.07 �ϐ����ύX: m_nMaxLineSize��m_nMaxLineKetas
	int					m_nIdx;
	TCHAR				m_szTypeName[64];				/*!< �^�C�v�����F���� */
	TCHAR				m_szTypeExts[64];				/*!< �^�C�v�����F�g���q���X�g */
	int					m_nTextWrapMethod;				/*!< �e�L�X�g�̐܂�Ԃ����@ */		// 2008.05.30 nasukoji
	CLayoutInt			m_nMaxLineKetas;				/*!< �܂�Ԃ����� */
	int					m_nColmSpace;					/*!< �����ƕ����̌��� */
	int					m_nLineSpace;					/*!< �s�Ԃ̂����� */
	CLayoutInt			m_nTabSpace;					/*!< TAB�̕����� */
	bool				m_bTabArrow;					/*!< �^�u���\�� */	//@@@ 2003.03.26 MIK
	EDIT_CHAR			m_szTabViewString[17];			/*!< TAB�\�������� */	// 2003.1.26 aroka �T�C�Y�g��
	int					m_bInsSpace;					/* �X�y�[�X�̑}�� */	// 2001.12.03 hor
	// 2005.01.13 MIK �z��
	int					m_nKeyWordSetIdx[MAX_KEYWORDSET_PER_TYPE];	/*!< �L�[���[�h�Z�b�g */

	CLineComment		m_cLineComment;					/*!< �s�R�����g�f���~�^ */			//@@@ 2002.09.22 YAZAKI
	CBlockComment		m_cBlockComment;				/*!< �u���b�N�R�����g�f���~�^ */	//@@@ 2002.09.22 YAZAKI

	int					m_nStringType;					/*!< �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	wchar_t				m_szIndentChars[64];			/*!< ���̑��̃C���f���g�Ώە��� */

	int					m_nColorInfoArrNum;				/*!< �F�ݒ�z��̗L���� */
	ColorInfo			m_ColorInfoArr[64];				/*!< �F�ݒ�z�� */

	int					m_bLineNumIsCRLF;				/*!< �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
	int					m_nLineTermType;				/*!< �s�ԍ���؂�  0=�Ȃ� 1=�c�� 2=�C�� */
	wchar_t				m_cLineTermChar;				/*!< �s�ԍ���؂蕶�� */
	CLayoutInt			m_nVertLineIdx[MAX_VERTLINES];	/*!< �w�茅�c�� */

	BOOL				m_bWordWrap;					/*!< �p�����[�h���b�v������ */
	BOOL				m_bKinsokuHead;					/*!< �s���֑������� */	//@@@ 2002.04.08 MIK
	BOOL				m_bKinsokuTail;					/*!< �s���֑������� */	//@@@ 2002.04.08 MIK
	BOOL				m_bKinsokuRet;					/*!< ���s�����̂Ԃ牺�� */	//@@@ 2002.04.13 MIK
	BOOL				m_bKinsokuKuto;					/*!< ��Ǔ_�̂Ԃ炳�� */	//@@@ 2002.04.17 MIK
	wchar_t				m_szKinsokuHead[200];			/*!< �s���֑����� */	//@@@ 2002.04.08 MIK
	wchar_t				m_szKinsokuTail[200];			/*!< �s���֑����� */	//@@@ 2002.04.08 MIK

	int					m_nCurrentPrintSetting;			/*!< ���ݑI�����Ă������ݒ� */

	EOutlineType		m_eDefaultOutline;				/*!< �A�E�g���C����͕��@ */
	SFilePath			m_szOutlineRuleFilename;		/*!< �A�E�g���C����̓��[���t�@�C�� */

	ESmartIndentType	m_eSmartIndent;					/*!< �X�}�[�g�C���f���g��� */
	int					m_nImeState;	//	Nov. 20, 2000 genta ����IME���

	//	2001/06/14 asa-o �⊮�̃^�C�v�ʐݒ�
	SFilePath			m_szHokanFile;					/*!< ���͕⊮ �P��t�@�C�� */
	//	2003.06.23 Moca �t�@�C��������̓��͕⊮�@�\
	int					m_bUseHokanByFile;				/*!< ���͕⊮ �J���Ă���t�@�C�����������T�� */
	//	2001/06/19 asa-o
	int					m_bHokanLoHiCase;				/*!< ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� */

	SFilePath			m_szExtHelp;					/* �O���w���v�P */
	SFilePath			m_szExtHtmlHelp;				/* �O��HTML�w���v */
	BOOL				m_bHtmlHelpIsSingle;			/* HtmlHelp�r���[�A�͂ЂƂ� */
	
	
//@@@ 2001.11.17 add start MIK
	BOOL	m_bUseRegexKeyword;	/* ���K�\���L�[���[�h���g����*/
	int	m_nRegexKeyMagicNumber;	/* ���K�\���L�[���[�h�X�V�}�W�b�N�i���o�[ */
	struct RegexKeywordInfo	m_RegexKeywordArr[MAX_REGEX_KEYWORD];	/* ���K�\���L�[���[�h */
//@@@ 2001.11.17 add end MIK

//@@@ 2006.04.10 fon ADD-start
	BOOL				m_bUseKeyWordHelp;			/* �L�[���[�h�����Z���N�g�@�\���g���� */
	int					m_nKeyHelpNum;					/* �L�[���[�h�����̍��� */
	KeyHelpInfo			m_KeyHelpArr[MAX_KEYHELP_FILE];	/* �L�[���[�h�����t�@�C�� */
	BOOL				m_bUseKeyHelpAllSearch;			/* �q�b�g�������̎���������(&A) */
	BOOL				m_bUseKeyHelpKeyDisp;			/* 1�s�ڂɃL�[���[�h���\������(&W) */
	BOOL				m_bUseKeyHelpPrefix;			/* �I��͈͂őO����v����(&P) */
//@@@ 2006.04.10 fon ADD-end

	//	2002/04/30 YAZAKI Common����ړ��B
	bool				m_bAutoIndent;					/* �I�[�g�C���f���g */
	bool				m_bAutoIndent_ZENSPACE;			/* ���{��󔒂��C���f���g */
	BOOL				m_bRTrimPrevLine;				/* 2005.10.11 ryoji ���s���ɖ����̋󔒂��폜 */
	int					m_nIndentLayout;				/* �܂�Ԃ���2�s�ڈȍ~���������\�� */
	
	//	Sep. 10, 2002 genta
	int					m_bUseDocumentIcon;	/*!< �t�@�C���Ɋ֘A�Â���ꂽ�A�C�R�����g�� */


	STypeConfig()
	: m_nMaxLineKetas(10) //	��ʐ܂�Ԃ�����TAB���ȉ��ɂȂ�Ȃ����Ƃ������l�ł��ۏ؂���	//	2004.04.03 Moca
	{
	}

}; /* STypeConfig */



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   �^�C�v�ʐݒ�A�N�Z�T                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//!�h�L�������g��ށB���L�f�[�^�� STypeConfig �ւ̃A�N�Z�T�����˂�B
//2007.12.13 kobake �쐬
class CTypeConfig{
public:
	CTypeConfig()
	{
#ifdef _DEBUG
		//����int�������̂ŁA���������Ŏg���Ɩ�肪��������悤�ɁA�����āA�ςȒl�����Ă����B
		m_nType = 1234;
#else
		//�����[�X���́A���������ł���肪�N����ɂ����悤�ɁA�[���N���A���Ă���
		m_nType = 0;
#endif
	}
	explicit CTypeConfig(int n)
	{
		m_nType = n;
	}
	bool IsValid() const{ return m_nType>=0 && m_nType<MAX_TYPES; }
	int GetIndex() const{ /*assert(IsValid());*/ return m_nType; }

	//���L�f�[�^�ւ̊ȈՃA�N�Z�T
	STypeConfig* operator->(){ return GetTypeConfig(); }
	STypeConfig* GetTypeConfig();
private:
	int m_nType;
};



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �^�C�v�ݒ�                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CType{
public:
	virtual ~CType(){ }
	void InitTypeConfig(int nIdx);
protected:
	virtual void InitTypeConfigImp(STypeConfig* pType) = 0;
};

#define GEN_CTYPE(CLASS_NAME) \
class CLASS_NAME : public CType{ \
protected: \
	void InitTypeConfigImp(STypeConfig* pType); \
};

GEN_CTYPE(CType_Asm)
GEN_CTYPE(CType_Awk)
GEN_CTYPE(CType_Basis)
GEN_CTYPE(CType_Cobol)
GEN_CTYPE(CType_Cpp)
GEN_CTYPE(CType_Dos)
GEN_CTYPE(CType_Html)
GEN_CTYPE(CType_Ini)
GEN_CTYPE(CType_Java)
GEN_CTYPE(CType_Pascal)
GEN_CTYPE(CType_Perl)
GEN_CTYPE(CType_Rich)
GEN_CTYPE(CType_Sql)
GEN_CTYPE(CType_Tex)
GEN_CTYPE(CType_Text)
GEN_CTYPE(CType_Vb)
GEN_CTYPE(CType_Other1)
GEN_CTYPE(CType_Other2)
GEN_CTYPE(CType_Other3)
GEN_CTYPE(CType_Other4)
GEN_CTYPE(CType_Other5)
GEN_CTYPE(CType_Other6)
GEN_CTYPE(CType_Other7)
GEN_CTYPE(CType_Other8)
GEN_CTYPE(CType_Other9)
GEN_CTYPE(CType_Other10)
GEN_CTYPE(CType_Other11)
GEN_CTYPE(CType_Other12)
GEN_CTYPE(CType_Other13)
GEN_CTYPE(CType_Other14)



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	�X�y�[�X�̔���
*/
inline bool C_IsSpace( wchar_t c )
{
	return (
		L'\t' == c ||
		L' ' == c ||
		WCODE::CR == c ||
		WCODE::LF == c
	);
}
