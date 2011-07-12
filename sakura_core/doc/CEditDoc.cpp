/*!	@file
	@brief �����֘A���̊Ǘ�

	@author Norio Nakatani
	@date	1998/03/13 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, genta, YAZAKI, jepro, novice, asa-o, MIK,
	Copyright (C) 2002, YAZAKI, hor, genta, aroka, frozen, Moca, MIK
	Copyright (C) 2003, MIK, genta, ryoji, Moca, zenryaku, naoh, wmlhq
	Copyright (C) 2004, genta, novice, Moca, MIK, zenryaku
	Copyright (C) 2005, genta, naoh, FILE, Moca, ryoji, D.S.Koba, aroka
	Copyright (C) 2006, genta, ryoji, aroka
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, nasukoji
	Copyright (C) 2011, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>	// Apr. 03, 2003 genta
#include <DLGS.H>
#include <OleCtl.h>
#include "global.h"
#include "window/CEditWnd.h"
#include "CAppMode.h"
#include "CClipboard.h"
#include "CCodeChecker.h"
#include "CControlTray.h"
#include "CEditApp.h"
#include "CGrepAgent.h"
#include "CNormalProcess.h"
#include "CPrintPreview.h"
#include "CVisualProgress.h"
#include "charset/CCodeMediator.h"
#include "charset/charcode.h"
#include "debug/CRunningTimer.h"
#include "debug/Debug.h"
#include "dlg/CDlgFileUpdateQuery.h"
#include "doc/CDocLine.h" /// 2002/2/3 aroka
#include "doc/CEditDoc.h"
#include "doc/CLayout.h"	// 2007.08.22 ryoji �ǉ�
#include "docplus/CModifyManager.h"
#include "env/CSakuraEnvironment.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "func/Funccode.h"
#include "mem/CMemoryIterator.h"	// 2007.08.22 ryoji �ǉ�
#include "outline/CFuncInfoArr.h" /// 2002/2/3 aroka
#include "macro/CSMacroMgr.h"
#include "util/file.h"
#include "util/format.h"
#include "util/module.h"
#include "util/other_util.h"
#include "util/string_ex2.h"
#include "util/window.h"
#include "sakura_rc.h"

#define IDT_ROLLMOUSE	1

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �����Ɣj��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*!
	@note
		m_pcEditWnd �̓R���X�g���N�^���ł͎g�p���Ȃ����ƁD

	@date 2000.05.12 genta ���������@�ύX
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
	@date 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	@date 2004.06.21 novice �^�O�W�����v�@�\�ǉ�
*/
CEditDoc::CEditDoc(CEditApp* pcApp)
: m_pcEditWnd(pcApp->m_pcEditWnd)
, m_nCommandExecNum( 0 )			/* �R�}���h���s�� */
, m_cDocFile(this)
, m_cDocOutline(this)
, m_cDocType(this)
, m_cDocEditor(this)
, m_cDocFileOperation(this)
, m_hBackImg(NULL)
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditDoc::CEditDoc" );

	// ���C�A�E�g�Ǘ����̏�����
	m_cLayoutMgr.Create( this, &m_cDocLineMgr );

	// ���C�A�E�g���̕ύX
	// 2008.06.07 nasukoji	�܂�Ԃ����@�̒ǉ��ɑΉ�
	// �u�w�茅�Ő܂�Ԃ��v�ȊO�̎��͐܂�Ԃ�����MAXLINEKETAS�ŏ���������
	// �u�E�[�Ő܂�Ԃ��v�́A���̌��OnSize()�ōĐݒ肳���
	STypeConfig ref = m_cDocType.GetDocumentAttribute();
	if( ref.m_nTextWrapMethod != WRAP_SETTING_WIDTH ){
		ref.m_nMaxLineKetas = MAXLINEKETAS;
	}
	m_cLayoutMgr.SetLayoutInfo(
		TRUE,
		ref
	);

	//	�����ۑ��̐ݒ�	//	Aug, 21, 2000 genta
	m_cAutoSaveAgent.ReloadAutoSaveParam();

	//$$ CModifyManager �C���X�^���X�𐶐�
	CModifyManager::Instance();

	//$$ CCodeChecker �C���X�^���X�𐶐�
	CCodeChecker::Instance();

	// 2008.06.07 nasukoji	�e�L�X�g�̐܂�Ԃ����@��������
	m_nTextWrapMethodCur = m_cDocType.GetDocumentAttribute().m_nTextWrapMethod;	// �܂�Ԃ����@
	m_bTextWrapMethodCurTemp = false;									// �ꎞ�ݒ�K�p��������

	// �����R�[�h��ʂ�������
	m_cDocFile.m_sFileInfo.eCharCode = static_cast<ECodeType>( CShareData::getInstance()->GetShareData()->m_Types[0].m_eDefaultCodetype );
	m_cDocFile.m_sFileInfo.bBomExist = ( CShareData::getInstance()->GetShareData()->m_Types[0].m_bDefaultBom != FALSE );
	m_cDocEditor.m_cNewLineCode = static_cast<EEolType>( CShareData::getInstance()->GetShareData()->m_Types[0].m_eDefaultEoltype );

	// �r������I�v�V������������
	m_cDocFile.SetShareMode( GetDllShareData().m_Common.m_sFile.m_nFileShareMode );
}


CEditDoc::~CEditDoc()
{
	if( m_hBackImg ){
		::DeleteObject( m_hBackImg );
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �����Ɣj��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CEditDoc::Clear()
{
	// �t�@�C���̔r�����b�N����
	m_cDocFileOperation.DoFileUnlock();

	// �����݋֎~�̃N���A
	m_cDocLocker.Clear();

	// �A���h�D�E���h�D�o�b�t�@�̃N���A
	m_cDocEditor.m_cOpeBuf.ClearAll();

	// �e�L�X�g�f�[�^�̃N���A
	m_cDocLineMgr.DeleteAllLine();

	// �t�@�C���p�X�ƃA�C�R���̃N���A
	SetFilePathAndIcon( _T("") );

	// �t�@�C���̃^�C���X�^���v�̃N���A
	m_cDocFile.m_sFileInfo.cFileTime.ClearFILETIME();

	// �u��{�v�̃^�C�v�ʐݒ��K�p
	m_cDocType.SetDocumentType( CDocTypeManager().GetDocumentTypeOfPath( m_cDocFile.GetFilePath() ), true );
	// 2008.06.07 nasukoji	�܂�Ԃ����@�̒ǉ��ɑΉ�
	STypeConfig ref = m_cDocType.GetDocumentAttribute();
	if( ref.m_nTextWrapMethod != WRAP_SETTING_WIDTH ){
		ref.m_nMaxLineKetas = MAXLINEKETAS;
	}
	m_cLayoutMgr.SetLayoutInfo(
		TRUE,
		ref
	);
}

/* �����f�[�^�̃N���A */
void CEditDoc::InitDoc()
{
	CAppMode::Instance()->SetViewMode(false);	// �r���[���[�h $$ ����OnClearDoc��p�ӂ�����
	wcscpy( CAppMode::Instance()->m_szGrepKey, L"" );	//$$

	CEditApp::Instance()->m_pcGrepAgent->m_bGrepMode = false;	/* Grep���[�h */	//$$����
	m_cAutoReloadAgent.m_eWatchUpdate = WU_QUERY; // Dec. 4, 2002 genta �X�V�Ď����@ $$

	// 2005.06.24 Moca �o�O�C��
	//	�A�E�g�v�b�g�E�B���h�E�Łu����(����)�v���s���Ă��A�E�g�v�b�g�E�B���h�E�̂܂�
	if( CAppMode::Instance()->IsDebugMode() ){
		CAppMode::Instance()->SetDebugModeOFF();
	}

//	Sep. 10, 2002 genta
//	�A�C�R���ݒ�̓t�@�C�����ݒ�ƈ�̉��̂��߂�������͍폜

	Clear();

	/* �ύX�t���O */
	m_cDocEditor.SetModified(false,false);	//	Jan. 22, 2002 genta

	/* �����R�[�h��� */
	m_cDocFile.m_sFileInfo.eCharCode = static_cast<ECodeType>( CShareData::getInstance()->GetShareData()->m_Types[0].m_eDefaultCodetype );
	m_cDocFile.m_sFileInfo.bBomExist = ( CShareData::getInstance()->GetShareData()->m_Types[0].m_bDefaultBom != FALSE );
	m_cDocEditor.m_cNewLineCode = static_cast<EEolType>( CShareData::getInstance()->GetShareData()->m_Types[0].m_eDefaultEoltype );

	//	Oct. 2, 2005 genta �}�����[�h
	m_cDocEditor.SetInsMode( GetDllShareData().m_Common.m_sGeneral.m_bIsINSMode );
}

void CEditDoc::SetBackgroundImage()
{
	CFilePath path = m_cDocType.GetDocumentAttribute().m_szBackImgPath.c_str();
	if( m_hBackImg ){
		::DeleteObject( m_hBackImg );
		m_hBackImg = NULL;
	}
	if( 0 == path[0] ){
		return;
	}
	if( _IS_REL_PATH(path.c_str()) ){
		CFilePath fullPath;
		GetInidirOrExedir( &fullPath[0], &path[0] );
		path = fullPath;
	}
	const TCHAR* ext = path.GetExt();
	if( 0 != auto_stricmp(ext, _T(".bmp")) ){
		HANDLE hFile = ::CreateFile(path.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
		if( hFile == INVALID_HANDLE_VALUE ){
			return;
		}
		DWORD fileSize  = ::GetFileSize(hFile, NULL);
		HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, fileSize);
		if( hGlobal == NULL ){
			::CloseHandle(hFile);
			return;
		}
		DWORD nRead;
		BOOL bRead = ::ReadFile(hFile, GlobalLock(hGlobal), fileSize, &nRead, NULL);
		::CloseHandle(hFile);
		hFile = NULL;
		if( !bRead ){
			::GlobalFree(hGlobal);
			return;
		}
		::GlobalUnlock(hGlobal);
		{
			IPicture* iPicture = NULL;
			IStream*  iStream = NULL;
			//hGlobal�̊Ǘ����ڏ�
			if( S_OK != ::CreateStreamOnHGlobal(hGlobal, TRUE, &iStream) ){
				GlobalFree(hGlobal);
			}else{
				if( S_OK != ::OleLoadPicture(iStream, fileSize, FALSE, IID_IPicture, (void**)&iPicture) ){
				}else{
					HBITMAP hBitmap = NULL;
					short imgType = PICTYPE_NONE;
					if( S_OK == iPicture->get_Type(&imgType) && imgType == PICTYPE_BITMAP &&
					    S_OK == iPicture->get_Handle((OLE_HANDLE*)&hBitmap) ){
						m_nBackImgWidth = m_nBackImgHeight = 1;
						m_hBackImg = (HBITMAP)::CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, 0);
					}
				}
			}
			if( iStream )  iStream->Release();
			if( iPicture ) iPicture->Release();
		}
	}else{
		m_hBackImg = (HBITMAP)::LoadImage(NULL, path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	}
	if( m_hBackImg ){
		BITMAP bmp;
		GetObject(m_hBackImg, sizeof(BITMAP), &bmp);
		m_nBackImgWidth  = bmp.bmWidth;
		m_nBackImgHeight = bmp.bmHeight;
		if( 0 == m_nBackImgWidth || 0 == m_nBackImgHeight ){
			::DeleteObject(m_hBackImg);
			m_hBackImg = NULL;
		}
	}
}

/* �S�r���[�̏������F�t�@�C���I�[�v��/�N���[�Y�����ɁA�r���[������������ */
void CEditDoc::InitAllView( void )
{

	m_nCommandExecNum = 0;	/* �R�}���h���s�� */

	// 2008.05.30 nasukoji	�e�L�X�g�̐܂�Ԃ����@��������
	m_nTextWrapMethodCur = m_cDocType.GetDocumentAttribute().m_nTextWrapMethod;	// �܂�Ԃ����@
	m_bTextWrapMethodCurTemp = false;									// �ꎞ�ݒ�K�p��������

	// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�Ȃ�e�L�X�g�ő啝���Z�o�A����ȊO�͕ϐ����N���A
	if( m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP )
		m_cLayoutMgr.CalculateTextWidth();		// �e�L�X�g�ő啝���Z�o����
	else
		m_cLayoutMgr.ClearLayoutLineWidth();	// �e�s�̃��C�A�E�g�s���̋L�����N���A����

	// CEditWnd�Ɉ��z��
	m_pcEditWnd->InitAllViews();

	return;
}



/////////////////////////////////////////////////////////////////////////////
//
//	CEditDoc::Create
//	BOOL Create(HINSTANCE hInstance, HWND hwndParent)
//
//	����
//	  �E�B���h�E�̍쐬��
//
//	@date Sep. 29, 2001 genta �}�N���N���X��n���悤��
//	@date 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
/////////////////////////////////////////////////////////////////////////////
BOOL CEditDoc::Create(
	CImageListMgr* pcIcons
 )
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditDoc::Create" );

	//	Oct. 2, 2001 genta
	m_cFuncLookup.Init( GetDllShareData().m_Common.m_sMacro.m_MacroTable, &GetDllShareData().m_Common );

	SetBackgroundImage();

	MY_TRACETIME( cRunningTimer, "End: PropSheet" );

	return TRUE;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �ݒ�                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	�t�@�C�����̐ݒ�
	
	�t�@�C������ݒ肷��Ɠ����ɁC�E�B���h�E�A�C�R����K�؂ɐݒ肷��D
	
	@param szFile [in] �t�@�C���̃p�X��
	
	@author genta
	@date 2002.09.09
*/
void CEditDoc::SetFilePathAndIcon(const TCHAR* szFile)
{
	TCHAR szWork[MAX_PATH];
	if( ::GetLongFileName( szFile, szWork ) ){
		szFile = szWork;
	}
	m_cDocFile.SetFilePath(szFile);
	m_cDocType.SetDocumentIcon();
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! �h�L�������g�̕����R�[�h���擾
ECodeType CEditDoc::GetDocumentEncoding() const
{
	return m_cDocFile.m_sFileInfo.eCharCode;
}

//! �h�L�������g�̕����R�[�h��ݒ�
void CEditDoc::SetDocumentEncoding(ECodeType eCharCode)
{
	if(!IsValidCodeType(eCharCode))return; //�����Ȕ͈͂��󂯕t���Ȃ�

	m_cDocFile.m_sFileInfo.eCharCode = eCharCode;
}

//! �h�L�������g�̃f�t�H���g�����R�[�h���擾
ECodeType CEditDoc::GetDefaultDocumentEncoding() const
{
	int ntype = m_cDocType.GetDocumentType().GetIndex();
	return static_cast<ECodeType>( CShareData::getInstance()->GetShareData()->m_Types[ntype].m_eDefaultCodetype );
}



void CEditDoc::GetSaveInfo(SSaveInfo* pSaveInfo) const
{
	pSaveInfo->cFilePath = this->m_cDocFile.GetFilePath(),
	pSaveInfo->eCharCode = this->m_cDocFile.m_sFileInfo.eCharCode;
	pSaveInfo->cEol      = this->m_cDocEditor.m_cNewLineCode; //�ҏW�����s�R�[�h��ۑ������s�R�[�h�Ƃ��Đݒ�
	pSaveInfo->bBomExist = this->m_cDocFile.m_sFileInfo.bBomExist;
}


/* �ҏW�t�@�C�������i�[ */
void CEditDoc::GetEditInfo(
	EditInfo* pfi	//!< [out]
) const
{
	//�t�@�C���p�X
	_tcscpy(pfi->m_szPath, m_cDocFile.GetFilePath());

	//�\����
	pfi->m_nViewTopLine = m_pcEditWnd->GetActiveView().GetTextArea().GetViewTopLine();	/* �\����̈�ԏ�̍s(0�J�n) */
	pfi->m_nViewLeftCol = m_pcEditWnd->GetActiveView().GetTextArea().GetViewLeftCol();	/* �\����̈�ԍ��̌�(0�J�n) */

	//�L�����b�g�ʒu
	pfi->m_ptCursor.Set(m_pcEditWnd->GetActiveView().GetCaret().GetCaretLogicPos());

	//�e����
	pfi->m_bIsModified = m_cDocEditor.IsModified();			/* �ύX�t���O */
	pfi->m_nCharCode = m_cDocFile.m_sFileInfo.eCharCode;	/* �����R�[�h��� */
	pfi->m_nType = m_cDocType.GetDocumentType();

	//GREP���[�h
	pfi->m_bIsGrep = CEditApp::Instance()->m_pcGrepAgent->m_bGrepMode;
	wcscpy( pfi->m_szGrepKey, CAppMode::Instance()->m_szGrepKey );

	//�f�o�b�O���j�^ (�A�E�g�v�b�g�E�C���h�E) ���[�h
	pfi->m_bIsDebug = CAppMode::Instance()->IsDebugMode();
}


//	From Here Aug. 14, 2000 genta
//
//	�����������֎~����Ă��邩�ǂ���
//	�߂�l: true: �֎~ / false: ����
//
bool CEditDoc::IsModificationForbidden( EFunctionCode nCommand )
{
	//	�ҏW�\�̏ꍇ
	if( IsEditable() )
		return false; // ��ɏ�����������

	//	�ҏW�֎~�̏ꍇ
	//	�b��Case��: ���ۂɂ͂����ƌ����̗ǂ����@���g���ׂ�
	switch( nCommand ){
	//	�t�@�C��������������R�}���h�͎g�p�֎~
	case F_WCHAR:
	case F_IME_CHAR:
	case F_DELETE:
	case F_DELETE_BACK:
	case F_WordDeleteToEnd:
	case F_WordDeleteToStart:
	case F_WordDelete:
	case F_WordCut:
	case F_LineDeleteToStart:
	case F_LineDeleteToEnd:
	case F_LineCutToStart:
	case F_LineCutToEnd:
	case F_DELETE_LINE:
	case F_CUT_LINE:
	case F_DUPLICATELINE:
	case F_INDENT_TAB:
	case F_UNINDENT_TAB:
	case F_INDENT_SPACE:
	case F_UNINDENT_SPACE:
	case F_CUT:
	case F_PASTE:
	case F_INS_DATE:
	case F_INS_TIME:
	case F_CTRL_CODE_DIALOG:	//@@@ 2002.06.02 MIK
	case F_INSTEXT_W:
	case F_ADDTAIL_W:
	case F_PASTEBOX:
	case F_REPLACE_DIALOG:
	case F_REPLACE:
	case F_REPLACE_ALL:
	case F_CODECNV_EMAIL:
	case F_CODECNV_EUC2SJIS:
	case F_CODECNV_UNICODE2SJIS:
	case F_CODECNV_UNICODEBE2SJIS:
	case F_CODECNV_SJIS2JIS:
	case F_CODECNV_SJIS2EUC:
	case F_CODECNV_UTF82SJIS:
	case F_CODECNV_UTF72SJIS:
	case F_CODECNV_SJIS2UTF7:
	case F_CODECNV_SJIS2UTF8:
	case F_CODECNV_AUTO2SJIS:
	case F_TOLOWER:
	case F_TOUPPER:
	case F_TOHANKAKU:
	case F_TOHANKATA:				// 2002/08/29 ai
	case F_TOZENEI:					// 2001/07/30 Misaka
	case F_TOHANEI:
	case F_TOZENKAKUKATA:
	case F_TOZENKAKUHIRA:
	case F_HANKATATOZENKATA:
	case F_HANKATATOZENHIRA:
	case F_TABTOSPACE:
	case F_SPACETOTAB:  //---- Stonee, 2001/05/27
	case F_HOKAN:
	case F_CHGMOD_INS:
	case F_LTRIM:		// 2001.12.03 hor
	case F_RTRIM:		// 2001.12.03 hor
	case F_SORT_ASC:	// 2001.12.11 hor
	case F_SORT_DESC:	// 2001.12.11 hor
	case F_MERGE:		// 2001.12.11 hor
	case F_UNDO:		// 2007.10.12 genta
	case F_REDO:		// 2007.10.12 genta
		return true;
	}
	return false;	//	�f�t�H���g�ŏ�����������
}
//	To Here Aug. 14, 2000 genta


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! @brief ���̃E�B���h�E�ŐV�����t�@�C�����J���邩

	�V�����E�B���h�E���J�����Ɍ��݂̃E�B���h�E���ė��p�ł��邩�ǂ����̃e�X�g���s���D
	�ύX�ς݁C�t�@�C�����J���Ă���CGrep�E�B���h�E�C�A�E�g�v�b�g�E�B���h�E�̏ꍇ�ɂ�
	�ė��p�s�D

	@author Moca
	@date 2005.06.24 Moca
*/
bool CEditDoc::IsAcceptLoad() const
{
	if(m_cDocEditor.IsModified())return false;
	if(m_cDocFile.GetFilePathClass().IsValidPath())return false;
	if(CEditApp::Instance()->m_pcGrepAgent->m_bGrepMode)return false;
	if(CAppMode::Instance()->IsDebugMode())return false;
	return true;
}





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �C�x���g                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! �R�}���h�R�[�h�ɂ�鏈���U�蕪��

	@param[in] nCommand MAKELONG( �R�}���h�R�[�h�C���M�����ʎq )

	@date 2006.05.19 genta ���16bit�ɑ��M���̎��ʎq������悤�ɕύX
	@date 2007.06.20 ryoji �O���[�v���ŏ��񂷂�悤�ɕύX
*/
BOOL CEditDoc::HandleCommand( EFunctionCode nCommand )
{
	//	May. 19, 2006 genta ���16bit�ɑ��M���̎��ʎq������悤�ɕύX�����̂�
	//	����16�r�b�g�݂̂����o��
	switch( LOWORD( nCommand )){
	case F_PREVWINDOW:	//�O�̃E�B���h�E
		{
			int nPane = m_pcEditWnd->m_cSplitterWnd.GetPrevPane();
			if( -1 != nPane ){
				m_pcEditWnd->SetActivePane( nPane );
			}else{
				CControlTray::ActiveNextWindow();
			}
		}
		return TRUE;
	case F_NEXTWINDOW:	//���̃E�B���h�E
		{
			int nPane = m_pcEditWnd->m_cSplitterWnd.GetNextPane();
			if( -1 != nPane ){
				m_pcEditWnd->SetActivePane( nPane );
			}
			else{
				CControlTray::ActivePrevWindow();
			}
		}
		return TRUE;
	default:
		return m_pcEditWnd->GetActiveView().GetCommander().HandleCommand( nCommand, TRUE, 0, 0, 0, 0 );
	}
}

/*! �r���[�ɐݒ�ύX�𔽉f������

	@date 2004.06.09 Moca ���C�A�E�g�č\�z����Progress Bar��\������D
	@date 2008.05.30 nasukoji	�e�L�X�g�̐܂�Ԃ����@�̕ύX������ǉ�
*/
void CEditDoc::OnChangeSetting()
{
	int			i;
	HWND		hwndProgress = NULL;

	CEditWnd*	pCEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta

	//pCEditWnd->m_CFuncKeyWnd.Timer_ONOFF( FALSE ); // 20060126 aroka

	if( NULL != pCEditWnd ){
		hwndProgress = pCEditWnd->m_cStatusBar.GetProgressHwnd();
		//	Status Bar���\������Ă��Ȃ��Ƃ���m_hwndProgressBar == NULL
	}

	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_SHOW );
	}

	/* �t�@�C���̔r�����[�h�ύX */
	if( m_cDocFile.GetShareMode() != GetDllShareData().m_Common.m_sFile.m_nFileShareMode ){
		m_cDocFile.SetShareMode( GetDllShareData().m_Common.m_sFile.m_nFileShareMode );

		/* �t�@�C���̔r�����b�N���� */
		m_cDocFileOperation.DoFileUnlock();

		// �t�@�C�������\�̃`�F�b�N����
		bool bOld = m_cDocLocker.IsDocWritable();
		m_cDocLocker.CheckWritable(bOld);	// ��������s�ɑJ�ڂ����Ƃ��������b�Z�[�W���o���i�o�߂���ƟT��������ˁH�j
		if(bOld != m_cDocLocker.IsDocWritable()){
			pCEditWnd->UpdateCaption();
		}

		/* �t�@�C���̔r�����b�N */
		if( m_cDocLocker.IsDocWritable() ){
			m_cDocFileOperation.DoFileLock();
		}
	}

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	CFileNameManager::Instance()->TransformFileName_MakeCache();

	// �������
	m_cDocType.SetDocumentType( CDocTypeManager().GetDocumentTypeOfPath( m_cDocFile.GetFilePath() ), false );

	CLogicPoint* posSaveAry = m_pcEditWnd->SavePhysPosOfAllView();

	/* ���C�A�E�g���̍쐬 */
	STypeConfig ref = m_cDocType.GetDocumentAttribute();
	{
		// 2008.06.07 nasukoji	�܂�Ԃ����@�̒ǉ��ɑΉ�
		// �܂�Ԃ����@�̈ꎞ�ݒ�ƃ^�C�v�ʐݒ肪��v������ꎞ�ݒ�K�p���͉���
		if( m_nTextWrapMethodCur == ref.m_nTextWrapMethod )
			m_bTextWrapMethodCurTemp = false;		// �ꎞ�ݒ�K�p��������

		// �ꎞ�ݒ�K�p���łȂ���ΐ܂�Ԃ����@�ύX
		if( !m_bTextWrapMethodCurTemp )
			m_nTextWrapMethodCur = ref.m_nTextWrapMethod;	// �܂�Ԃ����@

		// �w�茅�Ő܂�Ԃ��F�^�C�v�ʐݒ���g�p
		// �E�[�Ő܂�Ԃ��F���Ɍ��݂̐܂�Ԃ������g�p
		// ��L�ȊO�FMAXLINEKETAS���g�p
		if( m_nTextWrapMethodCur != WRAP_SETTING_WIDTH ){
			if( m_nTextWrapMethodCur == WRAP_WINDOW_WIDTH )
				ref.m_nMaxLineKetas = m_cLayoutMgr.GetMaxLineKetas();	// ���݂̐܂�Ԃ���
			else
				ref.m_nMaxLineKetas = MAXLINEKETAS;
		}
	}
	CProgressSubject* pOld = CEditApp::Instance()->m_pcVisualProgress->CProgressListener::Listen(&m_cLayoutMgr);
	m_cLayoutMgr.SetLayoutInfo(true,ref);
	CEditApp::Instance()->m_pcVisualProgress->CProgressListener::Listen(pOld);

	// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�Ȃ�e�L�X�g�ő啝���Z�o�A����ȊO�͕ϐ����N���A
	if( m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP )
		m_cLayoutMgr.CalculateTextWidth();		// �e�L�X�g�ő啝���Z�o����
	else
		m_cLayoutMgr.ClearLayoutLineWidth();	// �e�s�̃��C�A�E�g�s���̋L�����N���A����

	/* �r���[�ɐݒ�ύX�𔽉f������ */
	int viewCount = m_pcEditWnd->GetAllViewCount();
	for( i = 0; i < viewCount; ++i ){
		m_pcEditWnd->GetView(i).OnChangeSetting();
	}
	m_pcEditWnd->RestorePhysPosOfAllView( posSaveAry );
	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
}

/*! �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F �� �ۑ����s

	@retval TRUE: �I�����ėǂ� / FALSE: �I�����Ȃ�
*/
BOOL CEditDoc::OnFileClose()
{
	int			nRet;
	int			nBool;

	//�N���[�Y���O����
	ECallbackResult eBeforeCloseResult = NotifyBeforeClose();
	if(eBeforeCloseResult==CALLBACK_INTERRUPT)return FALSE;


	// �f�o�b�O���j�^���[�h�̂Ƃ��͕ۑ��m�F���Ȃ�
	if(CAppMode::Instance()->IsDebugMode())return TRUE;

	//�e�L�X�g���ύX����Ă��Ȃ��ꍇ�͕ۑ��m�F���Ȃ�
	if(!m_cDocEditor.IsModified())return TRUE;

	//GREP���[�h�ŁA���A�uGREP���[�h�ŕۑ��m�F���邩�v��OFF��������A�ۑ��m�F���Ȃ�
	if( CEditApp::Instance()->m_pcGrepAgent->m_bGrepMode ){
		if( !GetDllShareData().m_Common.m_sSearch.m_bGrepExitConfirm ){
			return TRUE;
		}
	}

	// -- -- �ۑ��m�F -- -- //
	
	/* �E�B���h�E���A�N�e�B�u�ɂ��� */
	HWND	hwndMainFrame = CEditWnd::Instance()->GetHwnd();
	ActivateFrameWindow( hwndMainFrame );
	if( CAppMode::Instance()->IsViewMode() ){	/* �r���[���[�h */
		::MessageBeep( MB_ICONQUESTION );
		nRet = ::MYMESSAGEBOX(
			hwndMainFrame,
			MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
			GSTR_APPNAME,
			_T("%ts\n�͕ύX����Ă��܂��B ����O�ɕۑ����܂����H\n\n�r���[���[�h�ŊJ���Ă���̂ŁA���O��t���ĕۑ�����΂����Ǝv���܂��B\n"),
			m_cDocFile.GetFilePathClass().IsValidPath() ? m_cDocFile.GetFilePath() : _T("�i����j")
		);
		switch( nRet ){
		case IDYES:
			nBool = m_cDocFileOperation.FileSaveAs();	// 2006.12.30 ryoji
			return nBool;
		case IDNO:
			return TRUE;
		case IDCANCEL:
		default:
			return FALSE;
		}
	}
	else{
		::MessageBeep( MB_ICONQUESTION );
		nRet = ::MYMESSAGEBOX(
			hwndMainFrame,
			MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
			GSTR_APPNAME,
			_T("%ts\n�͕ύX����Ă��܂��B ����O�ɕۑ����܂����H"),
			m_cDocFile.GetFilePathClass().IsValidPath() ? m_cDocFile.GetFilePath() : _T("�i����j")
		);
		switch( nRet ){
		case IDYES:
			if( m_cDocFile.GetFilePathClass().IsValidPath() ){
				nBool = m_cDocFileOperation.FileSave();	// 2006.12.30 ryoji
			}
			else{
				nBool = m_cDocFileOperation.FileSaveAs();	// 2006.12.30 ryoji
			}
			return nBool;
		case IDNO:
			return TRUE;
		case IDCANCEL:
		default:
			return FALSE;
		}
	}
}

/*!	@brief �}�N���������s

	@param type [in] �������s�}�N���ԍ�
	@return

	@author ryoji
	@date 2006.09.01 ryoji �쐬
	@date 2007.07.20 genta HandleCommand�ɒǉ�����n���D
		�������s�}�N���Ŕ��s�����R�}���h�̓L�[�}�N���ɕۑ����Ȃ�
*/
void CEditDoc::RunAutoMacro( int idx, LPCTSTR pszSaveFilePath )
{
	// �J�t�@�C���^�^�C�v�ύX���̓A�E�g���C�����ĉ�͂���
	if( pszSaveFilePath == NULL ){
		m_pcEditWnd->m_cDlgFuncList.Refresh();
	}

	static bool bRunning = false;
	if( bRunning )
		return;	// �ē�����s�͂��Ȃ�

	bRunning = true;
	if( CEditApp::Instance()->m_pcSMacroMgr->IsEnabled(idx) ){
		if( !( ::GetAsyncKeyState(VK_SHIFT) & 0x8000 ) ){	// Shift �L�[��������Ă��Ȃ���Ύ��s
			if( NULL != pszSaveFilePath )
				m_cDocFile.SetSaveFilePath(pszSaveFilePath);
			//	2007.07.20 genta �������s�}�N���Ŕ��s�����R�}���h�̓L�[�}�N���ɕۑ����Ȃ�
			HandleCommand((EFunctionCode)(( F_USERMACRO_0 + idx ) | FA_NONRECORD) );
			m_cDocFile.SetSaveFilePath(_T(""));
		}
	}
	bRunning = false;
}

