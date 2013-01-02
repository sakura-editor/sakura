/*!	@file
	@brief �L�[���[�h�⊮

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, asa-o
	Copyright (C) 2003, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CHokanMgr;

#ifndef _CHOKANMGR_H_
#define _CHOKANMGR_H_

#include "dlg/CDialog.h"
#include <windows.h>
#include "mem/CMemory.h"
#include "util/container.h"

class CEditView;


/*! @brief �L�[���[�h�⊮

	@date 2003.06.25 Moca �t�@�C��������̕⊮�@�\��ǉ�
*/
class SAKURA_CORE_API CHokanMgr : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CHokanMgr();
	~CHokanMgr();

	HWND DoModeless( HINSTANCE, HWND, LPARAM );/* ���[�h���X�_�C�A���O�̕\�� */
	void Hide( void );
	/* ������ */
	int Search(
		POINT*			ppoWin,
		int				nWinHeight,
		int				nColmWidth,
		const wchar_t*	pszCurWord,
		const TCHAR*	pszHokanFile,
		BOOL			bHokanLoHiCase,			// ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� 2001/06/19 asa-o
		BOOL			bHokanByFile,			// �ҏW���f�[�^�������T���B 2003.06.23 Moca
		int				nHokanType,
		bool			bHokanByKeyword,
		CNativeW*		pcmemHokanWord = NULL	// �⊮��₪�P�̂Ƃ�����Ɋi�[ 2001/06/19 asa-o
	);
	void HokanSearchByKeyword(
		const wchar_t*	pszCurWord,
		BOOL 			bHokanLoHiCase,
		vector_ex<std::wstring>& 	vKouho
	);
//	void SetCurKouhoStr( void );
	BOOL DoHokan( int );
	void ChangeView( LPARAM );/* ���[�h���X���F�ΏۂƂȂ�r���[�̕ύX */


	BOOL OnInitDialog( HWND, WPARAM wParam, LPARAM lParam );
	BOOL OnDestroy( void );
	BOOL OnSize( WPARAM wParam, LPARAM lParam );
	BOOL OnBnClicked( int wID );
	BOOL OnKeyDown( WPARAM wParam, LPARAM lParam );
	BOOL OnLbnSelChange( HWND hwndCtl, int wID );
	BOOL OnLbnDblclk( int wID );
	BOOL OnKillFocus( WPARAM wParam, LPARAM lParam );
//	int OnVKeyToItem( WPARAM wParam, LPARAM lParam );
//	int OnCharToItem( WPARAM wParam, LPARAM lParam );

	int KeyProc( WPARAM, LPARAM );

//	2001/06/18 asa-o
	void ShowTip();	// �⊮�E�B���h�E�őI�𒆂̒P��ɃL�[���[�h�w���v�̕\��

	static bool AddKouhoUnique(vector_ex<std::wstring>&, const std::wstring&);

	CNativeW		m_cmemCurWord;
	vector_ex<std::wstring>	m_vKouho;
	int				m_nKouhoNum;

	int				m_nCurKouhoIdx;

	POINT			m_poWin;
	int				m_nWinHeight;
	int				m_nColmWidth;
	int				m_bTimerFlag;

protected:
	/*
	||  �����w���p�֐�
	*/
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

};



///////////////////////////////////////////////////////////////////////
#endif /* _CHOKANMGR_H_ */



