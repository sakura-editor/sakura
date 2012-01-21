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
#ifndef SAKURA_CDOCOUTLINE_F40D43DF_FDBE_4758_B015_E9C369A17606_H_
#define SAKURA_CDOCOUTLINE_F40D43DF_FDBE_4758_B015_E9C369A17606_H_

class CEditDoc;
class CFuncInfoArr;
struct SOneRule;

/*! ���[���t�@�C����1�s���Ǘ�����\����

	@date 2002.04.01 YAZAKI
	@date 2007.11.29 kobake ���O�ύX: oneRule��SOneRule
*/
struct SOneRule {
	wchar_t szMatch[256];
	int		nLength;
	wchar_t szGroupName[256];
};

class CDocOutline{
public:
	CDocOutline(CEditDoc* pcDoc) : m_pcDocRef(pcDoc) { }
	void	MakeFuncList_C( CFuncInfoArr*,bool bVisibleMemberFunc = true );					//!< C/C++�֐����X�g�쐬
	void	MakeFuncList_PLSQL( CFuncInfoArr* );											//!< PL/SQL�֐����X�g�쐬
	void	MakeTopicList_txt( CFuncInfoArr* );												//!< �e�L�X�g�E�g�s�b�N���X�g�쐬
	void	MakeFuncList_Java( CFuncInfoArr* );												//!< Java�֐����X�g�쐬
	void	MakeTopicList_cobol( CFuncInfoArr* );											//!< COBOL �A�E�g���C�����
	void	MakeTopicList_asm( CFuncInfoArr* );												//!< �A�Z���u�� �A�E�g���C�����
	void	MakeFuncList_Perl( CFuncInfoArr* );												//!< Perl�֐����X�g�쐬	//	Sep. 8, 2000 genta
	void	MakeFuncList_VisualBasic( CFuncInfoArr* );										//!< Visual Basic�֐����X�g�쐬 //June 23, 2001 N.Nakatani
	void	MakeFuncList_python( CFuncInfoArr* pcFuncInfoArr );								//!< Python �A�E�g���C����� // 2007.02.08 genta
	void	MakeFuncList_Erlang( CFuncInfoArr* pcFuncInfoArr );								//!< Erlang �A�E�g���C����� // 2009.08.10 genta
	void	MakeTopicList_wztxt(CFuncInfoArr*);												//!< �K�w�t���e�L�X�g �A�E�g���C����� // 2003.05.20 zenryaku
	void	MakeTopicList_html(CFuncInfoArr*);												//!< HTML �A�E�g���C����� // 2003.05.20 zenryaku
	void	MakeTopicList_tex(CFuncInfoArr*);												//!< TeX �A�E�g���C����� // 2003.07.20 naoh
	void	MakeFuncList_RuleFile( CFuncInfoArr* );											//!< ���[���t�@�C�����g���ă��X�g�쐬 2002.04.01 YAZAKI
	int		ReadRuleFile( const TCHAR* pszFilename, SOneRule* pcOneRule, int nMaxCount );	//!< ���[���t�@�C���Ǎ� 2002.04.01 YAZAKI
	void	MakeFuncList_BookMark( CFuncInfoArr* );											//!< �u�b�N�}�[�N���X�g�쐬 //2001.12.03 hor
private:
	CEditDoc* m_pcDocRef;
};

#endif /* SAKURA_CDOCOUTLINE_F40D43DF_FDBE_4758_B015_E9C369A17606_H_ */
/*[EOF]*/
