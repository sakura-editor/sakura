/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CTYPE_BF915633_AE38_4C73_8E5B_0411063A1AD89_H_
#define SAKURA_CTYPE_BF915633_AE38_4C73_8E5B_0411063A1AD89_H_

#include "CEol.h"
#include "env/CommonSetting.h"
#include "doc/CDocTypeSetting.h"
#include "doc/CLineComment.h"
#include "doc/CBlockComment.h"
#include "charset/charset.h"  // ECodeType
#include "CRegexKeyword.h"	// RegexKeywordInfo


/* �A�E�g���C����͂̎�� */
enum EOutlineType{
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
	OUTLINE_ERLANG,		//	2009.08.10 genta Erlang�A�E�g���C�����
	//	�V�����A�E�g���C����͕͂K�����̒��O�֑}��
	OUTLINE_CODEMAX,
	OUTLINE_BOOKMARK,	//	2001.12.03 hor
	OUTLINE_PLUGIN,		//	2009.10.29 syat �v���O�C���ɂ��A�E�g���C�����
	OUTLINE_DEFAULT =-1,//	2001.12.03 hor
	OUTLINE_UNKNOWN	= 99,
	OUTLINE_TREE = 100,		// �ėp�c���[ 2010.03.28 syat
	OUTLINE_CLSTREE = 200,	// �ėp�c���[(�N���X) 2010.03.28 syat
	OUTLINE_LIST = 300,		// �ėp���X�g 2010.03.28 syat
};

/* �X�}�[�g�C���f���g��� */
enum ESmartIndentType {
	SMARTINDENT_NONE,
	SMARTINDENT_CPP
};

enum EBackgroundImagePos {
	BGIMAGE_TOP_LEFT,
	BGIMAGE_TOP_RIGHT,
	BGIMAGE_BOTTOM_LEFT,
	BGIMAGE_BOTTOM_RIGHT,
	BGIMAGE_CENTER,
	BGIMAGE_TOP_CENTER,
	BGIMAGE_BOTTOM_CENTER,
	BGIMAGE_CENTER_LEFT,
	BGIMAGE_CENTER_RIGHT
};

struct SEncodingConfig{
	bool				m_bPriorCesu8;					// �������ʎ��� CESU-8 ��D�悷�邩�ǂ���
	ECodeType			m_eDefaultCodetype;				/* �f�t�H���g�����R�[�h */
	EEolType			m_eDefaultEoltype;				/* �f�t�H���g���s�R�[�h */	// 2011.01.24 ryoji
	bool				m_bDefaultBom;					// �f�t�H���gBOM			// 2011.01.24 ryoji
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �^�C�v�ʐݒ�                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! �^�C�v�ʐݒ�
struct STypeConfig{
	//2007.09.07 �ϐ����ύX: m_nMaxLineSize��m_nMaxLineKetas
	int					m_nIdx;
	TCHAR				m_szTypeName[64];				/*!< �^�C�v�����F���� */
	TCHAR				m_szTypeExts[MAX_TYPES_EXTS];	/*!< �^�C�v�����F�g���q���X�g */
	int					m_nTextWrapMethod;				/*!< �e�L�X�g�̐܂�Ԃ����@ */		// 2008.05.30 nasukoji
	CLayoutInt			m_nMaxLineKetas;				/*!< �܂�Ԃ����� */
	int					m_nColumnSpace;					/*!< �����ƕ����̌��� */
	int					m_nLineSpace;					/*!< �s�Ԃ̂����� */
	CLayoutInt			m_nTabSpace;					/*!< TAB�̕����� */
	bool				m_bTabArrow;					//!< �^�u���\��		//@@@ 2003.03.26 MIK
	EDIT_CHAR			m_szTabViewString[8+1];			/*!< TAB�\�������� */	// 2003.1.26 aroka �T�C�Y�g��	// 2009.02.11 ryoji �T�C�Y�߂�(17->8+1)
	int					m_bInsSpace;					/* �X�y�[�X�̑}�� */	// 2001.12.03 hor
	// 2005.01.13 MIK �z��
	int					m_nKeyWordSetIdx[MAX_KEYWORDSET_PER_TYPE];	/*!< �L�[���[�h�Z�b�g */

	CLineComment		m_cLineComment;					/*!< �s�R�����g�f���~�^ */			//@@@ 2002.09.22 YAZAKI
	CBlockComment		m_cBlockComments[2];			/*!< �u���b�N�R�����g�f���~�^ */	//@@@ 2002.09.22 YAZAKI

	int					m_nStringType;					/*!< �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	wchar_t				m_szIndentChars[64];			/*!< ���̑��̃C���f���g�Ώە��� */

	int					m_nColorInfoArrNum;				/*!< �F�ݒ�z��̗L���� */
	ColorInfo			m_ColorInfoArr[64];				/*!< �F�ݒ�z�� */

	SFilePath			m_szBackImgPath;				//!< �w�i�摜
	EBackgroundImagePos m_backImgPos;					
	bool				m_backImgRepeatX;				
	bool				m_backImgRepeatY;				
	bool				m_backImgScrollX;				
	bool				m_backImgScrollY;				
	POINT				m_backImgPosOffset;				

	bool				m_bLineNumIsCRLF;				/*!< �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
	int					m_nLineTermType;				/*!< �s�ԍ���؂�  0=�Ȃ� 1=�c�� 2=�C�� */
	wchar_t				m_cLineTermChar;				/*!< �s�ԍ���؂蕶�� */
	CLayoutInt			m_nVertLineIdx[MAX_VERTLINES];	/*!< �w�茅�c�� */

	bool				m_bWordWrap;					//!< �p�����[�h���b�v������
	bool				m_bKinsokuHead;					//!< �s���֑�������		//@@@ 2002.04.08 MIK
	bool				m_bKinsokuTail;					//!< �s���֑�������		//@@@ 2002.04.08 MIK
	bool				m_bKinsokuRet;					//!< ���s�����̂Ԃ牺��	//@@@ 2002.04.13 MIK
	bool				m_bKinsokuKuto;					//!< ��Ǔ_�̂Ԃ炳��	//@@@ 2002.04.17 MIK
	bool				m_bKinsokuHide;					//!< �Ԃ牺�����B��		// 2011/11/30 Uchi
	wchar_t				m_szKinsokuHead[200];			/*!< �s���֑����� */	//@@@ 2002.04.08 MIK
	wchar_t				m_szKinsokuTail[200];			/*!< �s���֑����� */	//@@@ 2002.04.08 MIK
	wchar_t				m_szKinsokuKuto[200];			/*!< ��Ǔ_�Ԃ炳������ */	// 2009.08.07 ryoji

	int					m_nCurrentPrintSetting;			/*!< ���ݑI�����Ă������ݒ� */

	BOOL				m_bOutlineDockDisp;				/*!< �A�E�g���C����͕\���̗L�� */
	EDockSide			m_eOutlineDockSide;				/*!< �A�E�g���C����̓h�b�L���O�z�u */
	int					m_cxOutlineDockLeft;			/*!< �A�E�g���C���̍��h�b�L���O�� */
	int					m_cyOutlineDockTop;				/*!< �A�E�g���C���̏�h�b�L���O�� */
	int					m_cxOutlineDockRight;			/*!< �A�E�g���C���̉E�h�b�L���O�� */
	int					m_cyOutlineDockBottom;			/*!< �A�E�g���C���̉��h�b�L���O�� */
	EOutlineType		m_eDefaultOutline;				/*!< �A�E�g���C����͕��@ */
	SFilePath			m_szOutlineRuleFilename;		/*!< �A�E�g���C����̓��[���t�@�C�� */
	int					m_nOutlineSortCol;				/*!< �A�E�g���C����̓\�[�g��ԍ� */
	bool				m_bOutlineSortDesc;				/*!< �A�E�g���C����̓\�[�g�~�� */
	int					m_nOutlineSortType;				/*!< �A�E�g���C����̓\�[�g� */

	ESmartIndentType	m_eSmartIndent;					/*!< �X�}�[�g�C���f���g��� */
	int					m_nImeState;					//!< ����IME���	Nov. 20, 2000 genta

	//	2001/06/14 asa-o �⊮�̃^�C�v�ʐݒ�
	SFilePath			m_szHokanFile;					/*!< ���͕⊮ �P��t�@�C�� */
	int					m_nHokanType;					/*!< ���͕⊮ ���(�v���O�C��) */
	//	2003.06.23 Moca �t�@�C��������̓��͕⊮�@�\
	bool				m_bUseHokanByFile;				//!< ���͕⊮ �J���Ă���t�@�C�����������T��
	bool				m_bUseHokanByKeyword;			// �����L�[���[�h������͕⊮
	
	//	2001/06/19 asa-o
	bool				m_bHokanLoHiCase;				//!< ���͕⊮�@�\�F�p�啶���������𓯈ꎋ����

	SFilePath			m_szExtHelp;					/* �O���w���v�P */
	SFilePath			m_szExtHtmlHelp;				/* �O��HTML�w���v */
	bool				m_bHtmlHelpIsSingle;			// HtmlHelp�r���[�A�͂ЂƂ�

	bool				m_bChkEnterAtEnd;				//!< �I�����A���s�̈�v����������	2013/4/14 Uchi

	SEncodingConfig		m_encoding;						//!< �G���R�[�h�I�v�V����


//@@@ 2001.11.17 add start MIK
	bool				m_bUseRegexKeyword;						/* ���K�\���L�[���[�h���g����*/
	int					m_nRegexKeyMagicNumber;					/* ���K�\���L�[���[�h�X�V�}�W�b�N�i���o�[ */
	RegexKeywordInfo	m_RegexKeywordArr[MAX_REGEX_KEYWORD];	/* ���K�\���L�[���[�h */
	wchar_t				m_RegexKeywordList[MAX_REGEX_KEYWORDLISTLEN];	// ���K�\���L�[���[�h
//@@@ 2001.11.17 add end MIK

//@@@ 2006.04.10 fon ADD-start
	bool				m_bUseKeyWordHelp;				// �L�[���[�h�����Z���N�g�@�\���g����
	int					m_nKeyHelpNum;					/* �L�[���[�h�����̍��� */
	KeyHelpInfo			m_KeyHelpArr[MAX_KEYHELP_FILE];	/* �L�[���[�h�����t�@�C�� */
	bool				m_bUseKeyHelpAllSearch;			// �q�b�g�������̎���������(&A)
	bool				m_bUseKeyHelpKeyDisp;			// 1�s�ڂɃL�[���[�h���\������(&W)
	bool				m_bUseKeyHelpPrefix;			// �I��͈͂őO����v����(&P)
//@@@ 2006.04.10 fon ADD-end

	//	2002/04/30 YAZAKI Common����ړ��B
	bool				m_bAutoIndent;					/* �I�[�g�C���f���g */
	bool				m_bAutoIndent_ZENSPACE;			/* ���{��󔒂��C���f���g */
	bool				m_bRTrimPrevLine;				// 2005.10.11 ryoji ���s���ɖ����̋󔒂��폜
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
GEN_CTYPE(CType_Other)



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

#endif /* SAKURA_CTYPE_BF915633_AE38_4C73_8E5B_0411063A1AD89_H_ */
/*[EOF]*/
