/*!	@file
	@brief ファイルオープンダイアログボックス

	@author Norio Nakatani
	@date	1998/08/10 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, jepro, Stonee, genta
	Copyright (C) 2002, MIK, YAZAKI, genta
	Copyright (C) 2003, MIK, KEITA, Moca, ryoji
	Copyright (C) 2004, genta
	Copyright (C) 2005, novice, ryoji
	Copyright (C) 2006, ryoji, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <array>
#include <wrl.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include "charset/CCodePage.h"
#include "dlg/CDlgOpenFile.h"
#include "env/CShareData.h"
#include "env/CDocTypeManager.h"
#include "doc/CDocListener.h"
#include "util/shell.h"
#include "util/file.h"
#include "util/os.h"
#include "util/module.h"
#include "CFileExt.h"

struct CDlgOpenFile_CommonItemDialog final
	:
	public IDlgOpenFile,
	private IFileDialogEvents,
	private IFileDialogControlEvents
{
	CDlgOpenFile_CommonItemDialog();

	void Create(
		HINSTANCE					hInstance,
		HWND						hwndParent,
		const TCHAR*				pszUserWildCard,
		const TCHAR*				pszDefaultPath,
		const std::vector<LPCTSTR>& vMRU,
		const std::vector<LPCTSTR>& vOPENFOLDER
	) override;

	bool DoModal_GetOpenFileName( TCHAR* pszPath, EFilter eAddFileter ) override;
	bool DoModal_GetSaveFileName( TCHAR* pszPath ) override;
	bool DoModalOpenDlg( SLoadInfo* pLoadInfo,
						 std::vector<std::tstring>* pFileNames,
						 bool bOptions ) override;
	bool DoModalSaveDlg( SSaveInfo*	pSaveInfo,
						 bool bSimpleMode ) override;

	bool DoModalOpenDlgImpl0( bool bAllowMultiSelect,
							  std::vector<std::tstring>* pFileNames,
							  LPCWSTR fileName,
							  const std::vector<COMDLG_FILTERSPEC>& specs );
	HRESULT DoModalOpenDlgImpl1( IFileOpenDialog* pFileOpenDialog,
								 bool bAllowMultiSelect,
								 std::vector<std::tstring>* pFileNames,
								 LPCWSTR fileName,
								 const std::vector<COMDLG_FILTERSPEC>& specs );
	bool DoModalSaveDlgImpl0( const TCHAR* pszPath );
	HRESULT DoModalSaveDlgImpl1( IFileSaveDialog* pFileSaveDialog,
								 const TCHAR* pszPath );

	HINSTANCE		m_hInstance;	/* アプリケーションインスタンスのハンドル */
	HWND			m_hwndParent;	/* オーナーウィンドウのハンドル */

	DLLSHAREDATA*	m_pShareData;

	SFilePath		m_szDefaultWildCard;	/* 「開く」での最初のワイルドカード（保存時の拡張子補完でも使用される） */
	SFilePath		m_szInitialDir;			/* 「開く」での初期ディレクトリ */

	std::vector<LPCTSTR>	m_vMRU;
	std::vector<LPCTSTR>	m_vOPENFOLDER;

	struct CustomizeSetting {
		bool bCustomize;
		bool bSkipAutoDetect;	// 文字コードコンボボックスのアイテムの自動選択を飛ばす
		bool bShowReadOnly;		// 読み取り専用チェックボックスを表示する
		bool bUseEol;
		bool bUseBom;			// BOMの有無を選択する機能を利用するかどうか
		bool bUseCharCode;
	} m_customizeSetting;
	bool			m_bViewMode;		// ビューモードか
	ECodeType		m_nCharCode;		// 文字コード
	CEol			m_cEol;
	bool			m_bBom;		//!< BOMを付けるかどうか	//	Jul. 26, 2003 ryoji BOM

	int AddComboCodePages(int nSelCode);

	HRESULT Customize();

	IFileDialog* m_pFileDialog = nullptr;
	IFileDialogCustomize* m_pFileDialogCustomize = nullptr;

	// IUnknown
	int m_RefCount = 0;

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject) override {
#pragma warning(push)
#pragma warning(disable: 4838) // conversion from 'DWORD' to 'int' requires a narrowing conversion
		static const QITAB qit[] = {
			QITABENT(CDlgOpenFile_CommonItemDialog, IFileDialogEvents),
			QITABENT(CDlgOpenFile_CommonItemDialog, IFileDialogControlEvents),
			{ 0 },
		};
		return QISearch(this, qit, iid, ppvObject);
#pragma warning(pop)
	}

	ULONG STDMETHODCALLTYPE AddRef() override {
		++m_RefCount;
		return m_RefCount;
	}

	ULONG STDMETHODCALLTYPE Release() override {
		--m_RefCount;
		int R = m_RefCount;
		//if(m_RefCount == 0)
		//	delete this;
		return R;
	}

	// IFileDialogCustomize

	HRESULT STDMETHODCALLTYPE EnableOpenDropDown(
		/* [in] */ DWORD dwIDCtl) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->EnableOpenDropDown(dwIDCtl);
	}
	
	HRESULT STDMETHODCALLTYPE AddMenu(
		/* [in] */ DWORD dwIDCtl,
		/* [string][in] */ __RPC__in_string LPCWSTR pszLabel) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->AddMenu(dwIDCtl, pszLabel);
	}
	
	HRESULT STDMETHODCALLTYPE AddPushButton(
		/* [in] */ DWORD dwIDCtl,
		/* [string][in] */ __RPC__in_string LPCWSTR pszLabel) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->AddPushButton(dwIDCtl, pszLabel);
	}
	
	HRESULT STDMETHODCALLTYPE AddComboBox(
		/* [in] */ DWORD dwIDCtl) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->AddComboBox(dwIDCtl);
	}
	
	HRESULT STDMETHODCALLTYPE AddRadioButtonList(
		/* [in] */ DWORD dwIDCtl) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->AddRadioButtonList(dwIDCtl);
	}
	
	HRESULT STDMETHODCALLTYPE AddCheckButton(
		/* [in] */ DWORD dwIDCtl,
		/* [string][in] */ __RPC__in_string LPCWSTR pszLabel,
		/* [in] */ BOOL bChecked) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->AddCheckButton(dwIDCtl, pszLabel, bChecked);
	}
	
	HRESULT STDMETHODCALLTYPE AddEditBox(
		/* [in] */ DWORD dwIDCtl,
		/* [string][in] */ __RPC__in_string LPCWSTR pszText) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->AddEditBox(dwIDCtl, pszText);
	}
	
	HRESULT STDMETHODCALLTYPE AddSeparator(
		/* [in] */ DWORD dwIDCtl) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->AddSeparator(dwIDCtl);
	}
	
	HRESULT STDMETHODCALLTYPE AddText(
		/* [in] */ DWORD dwIDCtl,
		/* [string][in] */ __RPC__in_string LPCWSTR pszText) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->AddText(dwIDCtl, pszText);
	}
	
	HRESULT STDMETHODCALLTYPE SetControlLabel(
		/* [in] */ DWORD dwIDCtl,
		/* [string][in] */ __RPC__in_string LPCWSTR pszLabel) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->SetControlLabel(dwIDCtl, pszLabel);
	}
	
	HRESULT STDMETHODCALLTYPE GetControlState(
		/* [in] */ DWORD dwIDCtl,
		/* [out] */ __RPC__out CDCONTROLSTATEF *pdwState) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->GetControlState(dwIDCtl, pdwState);
	}
	
	HRESULT STDMETHODCALLTYPE SetControlState(
		/* [in] */ DWORD dwIDCtl,
		/* [in] */ CDCONTROLSTATEF dwState) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->SetControlState(dwIDCtl, dwState);
	}
	
	HRESULT STDMETHODCALLTYPE GetEditBoxText(
		/* [in] */ DWORD dwIDCtl,
		/* [string][out] */ __RPC__deref_out_opt_string WCHAR **ppszText) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->GetEditBoxText(dwIDCtl, ppszText);
	}
	
	HRESULT STDMETHODCALLTYPE SetEditBoxText(
		/* [in] */ DWORD dwIDCtl,
		/* [string][in] */ __RPC__in_string LPCWSTR pszText) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->SetEditBoxText(dwIDCtl, pszText);
	}
	
	HRESULT STDMETHODCALLTYPE GetCheckButtonState(
		/* [in] */ DWORD dwIDCtl,
		/* [out] */ __RPC__out BOOL *pbChecked) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->GetCheckButtonState(dwIDCtl, pbChecked);
	}
	
	HRESULT STDMETHODCALLTYPE SetCheckButtonState(
		/* [in] */ DWORD dwIDCtl,
		/* [in] */ BOOL bChecked) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->SetCheckButtonState(dwIDCtl, bChecked);
	}
	
	HRESULT STDMETHODCALLTYPE AddControlItem(
		/* [in] */ DWORD dwIDCtl,
		/* [in] */ DWORD dwIDItem,
		/* [in] */ __RPC__in LPCWSTR pszLabel) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->AddControlItem(dwIDCtl, dwIDItem, pszLabel);
	}
	
	HRESULT STDMETHODCALLTYPE RemoveControlItem(
		/* [in] */ DWORD dwIDCtl,
		/* [in] */ DWORD dwIDItem) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->RemoveControlItem(dwIDCtl, dwIDItem);
	}
	
	HRESULT STDMETHODCALLTYPE RemoveAllControlItems(
		/* [in] */ DWORD dwIDCtl) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->RemoveAllControlItems(dwIDCtl);
	}
	
	HRESULT STDMETHODCALLTYPE GetControlItemState(
		/* [in] */ DWORD dwIDCtl,
		/* [in] */ DWORD dwIDItem,
		/* [out] */ __RPC__out CDCONTROLSTATEF *pdwState) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->GetControlItemState(dwIDCtl, dwIDItem, pdwState);
	}
	
	HRESULT STDMETHODCALLTYPE SetControlItemState(
		/* [in] */ DWORD dwIDCtl,
		/* [in] */ DWORD dwIDItem,
		/* [in] */ CDCONTROLSTATEF dwState) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->SetControlItemState(dwIDCtl, dwIDItem, dwState);
	}
	
	HRESULT STDMETHODCALLTYPE GetSelectedControlItem(
		/* [in] */ DWORD dwIDCtl,
		/* [out] */ __RPC__out DWORD *pdwIDItem) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->GetSelectedControlItem(dwIDCtl, pdwIDItem);
	}
	
	HRESULT STDMETHODCALLTYPE SetSelectedControlItem(
		/* [in] */ DWORD dwIDCtl,
		/* [in] */ DWORD dwIDItem) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->SetSelectedControlItem(dwIDCtl, dwIDItem);
	}
	
	HRESULT STDMETHODCALLTYPE StartVisualGroup(
		/* [in] */ DWORD dwIDCtl,
		/* [string][in] */ __RPC__in_string LPCWSTR pszLabel) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->StartVisualGroup(dwIDCtl, pszLabel);
	}
	
	HRESULT STDMETHODCALLTYPE EndVisualGroup(void) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->EndVisualGroup();
	}
	
	HRESULT STDMETHODCALLTYPE MakeProminent(
		/* [in] */ DWORD dwIDCtl) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->MakeProminent(dwIDCtl);
	}
	
	HRESULT STDMETHODCALLTYPE SetControlItemText(
		/* [in] */ DWORD dwIDCtl,
		/* [in] */ DWORD dwIDItem,
		/* [string][in] */ __RPC__in_string LPCWSTR pszLabel) {
		assert(m_pFileDialogCustomize);
		return m_pFileDialogCustomize->SetControlItemText(dwIDCtl, dwIDItem, pszLabel);
	}

	// IFileDialogEvents

	HRESULT STDMETHODCALLTYPE OnFileOk(
		/* [in] */ __RPC__in_opt IFileDialog *pfd) override {
		return E_NOTIMPL;
	}
		
	HRESULT STDMETHODCALLTYPE OnFolderChanging(
		/* [in] */ __RPC__in_opt IFileDialog *pfd,
		/* [in] */ __RPC__in_opt IShellItem *psiFolder) override {
		return E_NOTIMPL;
	}
		
	HRESULT STDMETHODCALLTYPE OnFolderChange(
		/* [in] */ __RPC__in_opt IFileDialog *pfd) override {
		return E_NOTIMPL;
	}
		
	HRESULT STDMETHODCALLTYPE OnSelectionChange(
		/* [in] */ __RPC__in_opt IFileDialog *pfd) override {
		return E_NOTIMPL;
	}
		
	HRESULT STDMETHODCALLTYPE OnShareViolation(
		/* [in] */ __RPC__in_opt IFileDialog *pfd,
		/* [in] */ __RPC__in_opt IShellItem *psi,
		/* [out] */ __RPC__out FDE_SHAREVIOLATION_RESPONSE *pResponse) override {
		return E_NOTIMPL;
	}
		
	HRESULT STDMETHODCALLTYPE OnTypeChange(
		/* [in] */ __RPC__in_opt IFileDialog *pfd) override {
		return E_NOTIMPL;
	}
		
	HRESULT STDMETHODCALLTYPE OnOverwrite(
		/* [in] */ __RPC__in_opt IFileDialog *pfd,
		/* [in] */ __RPC__in_opt IShellItem *psi,
		/* [out] */ __RPC__out FDE_OVERWRITE_RESPONSE *pResponse) override {
		return E_NOTIMPL;
	}

	// IFileDialogControlEvents

	HRESULT STDMETHODCALLTYPE OnItemSelected(
		/* [in] */ __RPC__in_opt IFileDialogCustomize *pfdc,
		/* [in] */ DWORD dwIDCtl,
		/* [in] */ DWORD dwIDItem) override;
		
	HRESULT STDMETHODCALLTYPE OnButtonClicked(
		/* [in] */ __RPC__in_opt IFileDialogCustomize *pfdc,
		/* [in] */ DWORD dwIDCtl) override {
		return E_NOTIMPL;
	}
	
	HRESULT STDMETHODCALLTYPE OnCheckButtonToggled(
		/* [in] */ __RPC__in_opt IFileDialogCustomize *pfdc,
		/* [in] */ DWORD dwIDCtl,
		/* [in] */ BOOL bChecked) override;
	
	HRESULT STDMETHODCALLTYPE OnControlActivating(
		/* [in] */ __RPC__in_opt IFileDialogCustomize *pfdc,
		/* [in] */ DWORD dwIDCtl) override {
		return E_NOTIMPL;
	}
};

enum CtrlId {
	CHECK_READONLY = 2000,
	LABEL_CODE,
	COMBO_CODE,
	CHECK_BOM,
	CHECK_CP,
	LABEL_EOL,
	COMBO_EOL,
	LABEL_MRU,
	COMBO_MRU,
	LABEL_OPENFOLDER,
	COMBO_OPENFOLDER,
};

CDlgOpenFile_CommonItemDialog::CDlgOpenFile_CommonItemDialog()
{
	m_hInstance = NULL;		/* アプリケーションインスタンスのハンドル */
	m_hwndParent = NULL;	/* オーナーウィンドウのハンドル */

	/* 共有データ構造体のアドレスを返す */
	m_pShareData = &GetDllShareData();

	TCHAR	szFile[_MAX_PATH + 1];
	TCHAR	szDrive[_MAX_DRIVE];
	TCHAR	szDir[_MAX_DIR];
	::GetModuleFileName(
		NULL,
		szFile, _countof( szFile )
	);
	_tsplitpath( szFile, szDrive, szDir, NULL, NULL );
	_tcscpy( m_szInitialDir, szDrive );
	_tcscat( m_szInitialDir, szDir );

	_tcscpy( m_szDefaultWildCard, _T("*.*") );	/*「開く」での最初のワイルドカード（保存時の拡張子補完でも使用される） */

	return;
}

/* 初期化 */
void CDlgOpenFile_CommonItemDialog::Create(
	HINSTANCE					hInstance,
	HWND						hwndParent,
	const TCHAR*				pszUserWildCard,
	const TCHAR*				pszDefaultPath,
	const std::vector<LPCTSTR>& vMRU,
	const std::vector<LPCTSTR>& vOPENFOLDER)
{
	m_hInstance = hInstance;
	m_hwndParent = hwndParent;

	/* ユーザー定義ワイルドカード（保存時の拡張子補完でも使用される） */
	if( NULL != pszUserWildCard ){
		_tcscpy( m_szDefaultWildCard, pszUserWildCard );
	}

	/* 「開く」での初期フォルダ */
	if( pszDefaultPath && pszDefaultPath[0] != _T('\0') ){	//現在編集中のファイルのパス	//@@@ 2002.04.18
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		//	Jun. 23, 2002 genta
		my_splitpath_t( pszDefaultPath, szDrive, szDir, NULL, NULL );
		// 2010.08.28 相対パス解決
		TCHAR szRelPath[_MAX_PATH];
		auto_sprintf( szRelPath, _T("%ts%ts"), szDrive, szDir );
		const TCHAR* p = szRelPath;
		if( ! ::GetLongFileName( p, m_szInitialDir ) ){
			auto_strcpy(m_szInitialDir, p );
		}
	}
	m_vMRU = vMRU;
	m_vOPENFOLDER = vOPENFOLDER;
	return;
}

bool CDlgOpenFile_CommonItemDialog::DoModal_GetOpenFileName( TCHAR* pszPath, EFilter eAddFilter )
{
	//	2003.05.12 MIK
	std::vector<COMDLG_FILTERSPEC> specs;
	std::vector<std::wstring> strs;
	strs.reserve(8);

	strs.push_back(LS(STR_DLGOPNFL_EXTNAME1));
	specs.push_back(COMDLG_FILTERSPEC{strs.back().c_str(), m_szDefaultWildCard});

	switch( eAddFilter ){
	case EFITER_TEXT:
		strs.push_back(LS(STR_DLGOPNFL_EXTNAME2));
		specs.push_back(COMDLG_FILTERSPEC{strs.back().c_str(), L"*.txt"});
		break;
	case EFITER_MACRO:
		specs.push_back(COMDLG_FILTERSPEC{L"Macros", L"*.js;*.vbs;*.ppa;*.mac"});
		specs.push_back(COMDLG_FILTERSPEC{L"JScript", L"*.js"});
		specs.push_back(COMDLG_FILTERSPEC{L"VBScript", L"*.vbs"});
		specs.push_back(COMDLG_FILTERSPEC{L"Pascal", L"*.ppa"});
		specs.push_back(COMDLG_FILTERSPEC{L"Key Macro", L"*.mac"});
		break;
	case EFITER_NONE:
	default:
		break;
	}

	if( 0 != auto_strcmp(m_szDefaultWildCard, _T("*.*")) ){
		strs.push_back(LS(STR_DLGOPNFL_EXTNAME3));
		specs.push_back(COMDLG_FILTERSPEC{strs.back().c_str(), L"*.*"});
	}

	m_customizeSetting.bCustomize = false;
	std::vector<std::wstring> fileNames;
	bool ret = DoModalOpenDlgImpl0(false, &fileNames, L"", specs);
	if (ret) {
		_tcscpy(pszPath, fileNames[0].c_str());
	}
	return ret;
}

/*! 保存ダイアログ モーダルダイアログの表示
	@param pszPath [i/o] 初期ファイル名．選択されたファイル名の格納場所
*/
bool CDlgOpenFile_CommonItemDialog::DoModal_GetSaveFileName( TCHAR* pszPath )
{
	// 2010.08.28 カレントディレクトリを移動するのでパス解決する
	if( pszPath[0] ){
		TCHAR szFullPath[_MAX_PATH];
		const TCHAR* pOrg = pszPath;
		if( ::GetLongFileName( pOrg, szFullPath ) ){
			// 成功。書き戻す
			auto_strcpy( pszPath , szFullPath );
		}
	}

	m_customizeSetting.bCustomize = false;
	return DoModalSaveDlgImpl0(pszPath);
}

HRESULT CDlgOpenFile_CommonItemDialog::Customize()
{
	HRESULT hr;
#define RETURN_IF_FAILED if (FAILED(hr)) { /* __debugbreak(); */ return hr; }
	if (m_customizeSetting.bShowReadOnly) {
		hr = AddCheckButton(CtrlId::CHECK_READONLY, LS(STR_FILEDIALOG_READONLY), m_bViewMode ? TRUE : FALSE); RETURN_IF_FAILED
	}

	if (m_customizeSetting.bUseCharCode) {
		hr = StartVisualGroup(CtrlId::LABEL_CODE, LS(STR_FILEDIALOG_CODE)); RETURN_IF_FAILED
		hr = AddComboBox(CtrlId::COMBO_CODE); RETURN_IF_FAILED
		CCodeTypesForCombobox cCodeTypes;
		bool bCodeSel = false;
		ECodeType eCodeSel = CODE_NONE;
		for( int i = (m_customizeSetting.bSkipAutoDetect ? 1 : 0) /* 保存の場合は自動選択飛ばし */; i < cCodeTypes.GetCount(); ++i ){
			auto code = cCodeTypes.GetCode(i);
			hr = AddControlItem(CtrlId::COMBO_CODE, (DWORD)code, cCodeTypes.GetName(i)); RETURN_IF_FAILED
			if( code == m_nCharCode ){
				bCodeSel = true;
				eCodeSel = code;
			}
		}
		if (bCodeSel) {
			hr = SetSelectedControlItem(CtrlId::COMBO_CODE, (DWORD)eCodeSel); RETURN_IF_FAILED
			hr = AddCheckButton(CtrlId::CHECK_CP, L"C&P", FALSE); RETURN_IF_FAILED
		}
		else {
			if( -1 == AddComboCodePages( m_nCharCode ) ){
				hr = SetSelectedControlItem(CtrlId::COMBO_CODE, CODE_SJIS); RETURN_IF_FAILED
			}
			hr = AddCheckButton(CtrlId::CHECK_CP, L"C&P", TRUE); RETURN_IF_FAILED
			hr = SetControlState(CtrlId::CHECK_CP, CDCS_VISIBLE); RETURN_IF_FAILED
		}
		if (m_customizeSetting.bUseBom) {
			hr = AddCheckButton(CtrlId::CHECK_BOM, L"&BOM", FALSE); RETURN_IF_FAILED
			if (CCodeTypeName(m_nCharCode).UseBom()) {
				hr = SetCheckButtonState(CtrlId::CHECK_BOM, m_bBom ? TRUE : FALSE); RETURN_IF_FAILED
			}
			else {
				hr = SetControlState(CtrlId::CHECK_BOM, CDCS_VISIBLE); RETURN_IF_FAILED
			}
		}
		hr = EndVisualGroup(); RETURN_IF_FAILED
	}

	if (m_customizeSetting.bUseEol) {
		hr = StartVisualGroup(CtrlId::LABEL_EOL, LS(STR_FILEDIALOG_EOL)); RETURN_IF_FAILED
		hr = AddComboBox(CtrlId::COMBO_EOL); RETURN_IF_FAILED
		hr = AddControlItem(CtrlId::COMBO_EOL, 0, LS(STR_DLGOPNFL1)); RETURN_IF_FAILED
		hr = AddControlItem(CtrlId::COMBO_EOL, 1, L"CR+LF"); RETURN_IF_FAILED
		hr = AddControlItem(CtrlId::COMBO_EOL, 2, L"LF (UNIX)"); RETURN_IF_FAILED
		hr = AddControlItem(CtrlId::COMBO_EOL, 3, L"CR (Mac)"); RETURN_IF_FAILED
		hr = SetSelectedControlItem(CtrlId::COMBO_EOL, 0); RETURN_IF_FAILED
		hr = EndVisualGroup(); RETURN_IF_FAILED
	}

	hr = StartVisualGroup(CtrlId::LABEL_MRU, LS(STR_FILEDIALOG_MRU)); RETURN_IF_FAILED
	hr = AddComboBox(CtrlId::COMBO_MRU); RETURN_IF_FAILED
	for (size_t i = 0; i < m_vMRU.size(); ++i) {
		hr = AddControlItem(CtrlId::COMBO_MRU, (DWORD)1+i, m_vMRU[i]); RETURN_IF_FAILED
	}
	hr = EndVisualGroup(); RETURN_IF_FAILED
	hr = StartVisualGroup(CtrlId::LABEL_OPENFOLDER, LS(STR_FILEDIALOG_OPENFOLDER)); RETURN_IF_FAILED
	hr = AddComboBox(CtrlId::COMBO_OPENFOLDER); RETURN_IF_FAILED
	for (size_t i = 0; i < m_vOPENFOLDER.size(); ++i) {
		hr = AddControlItem(CtrlId::COMBO_OPENFOLDER, (DWORD)1+i, m_vOPENFOLDER[i]); RETURN_IF_FAILED
	}
	hr = EndVisualGroup(); RETURN_IF_FAILED
#undef RETURN_IF_FAILED
	return hr;
}

HRESULT CDlgOpenFile_CommonItemDialog::DoModalOpenDlgImpl1(
	IFileOpenDialog* pFileOpenDialog,
	bool bAllowMultiSelect,
	std::vector<std::tstring>* pFileNames,
	LPCWSTR fileName,
	const std::vector<COMDLG_FILTERSPEC>& specs)
{
	//カレントディレクトリを保存。関数から抜けるときに自動でカレントディレクトリは復元される。
	CCurrentDirectoryBackupPoint cCurDirBackup;

	// 2010.08.28 Moca DLLが読み込まれるので移動
	ChangeCurrentDirectoryToExeDir();

	m_customizeSetting.bUseEol = false;	//	Feb. 9, 2001 genta
	m_customizeSetting.bUseBom = false;	//	Jul. 26, 2003 ryoji

	HRESULT hr;
#define RETURN_IF_FAILED if (FAILED(hr)) { /* __debugbreak(); */ return hr; }
	FILEOPENDIALOGOPTIONS options;
	hr = pFileOpenDialog->GetOptions(&options); RETURN_IF_FAILED
	options |= FOS_NOCHANGEDIR | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_CREATEPROMPT;
	if (bAllowMultiSelect) {
		options |= FOS_ALLOWMULTISELECT;
	}

	hr = pFileOpenDialog->SetOptions(options); RETURN_IF_FAILED
	hr = pFileOpenDialog->SetFileTypes(specs.size(), &specs[0]); RETURN_IF_FAILED
	hr = pFileOpenDialog->SetFileName(fileName); RETURN_IF_FAILED

	using namespace Microsoft::WRL;
	ComPtr<IShellItem> psiFolder;
	SHCreateItemFromParsingName(m_szInitialDir, NULL, IID_PPV_ARGS(&psiFolder));
	hr = pFileOpenDialog->SetFolder(psiFolder.Get()); RETURN_IF_FAILED

	m_pFileDialog = pFileOpenDialog;
	// カスタマイズ
	if (m_customizeSetting.bCustomize) {
		hr = Customize(); RETURN_IF_FAILED
	}
	hr = pFileOpenDialog->Show(m_hwndParent);
	m_pFileDialog = nullptr;
	RETURN_IF_FAILED

	ComPtr<IShellItemArray> pShellItems;
	hr = pFileOpenDialog->GetResults(&pShellItems); RETURN_IF_FAILED
	DWORD numItems;
	hr = pShellItems->GetCount(&numItems); RETURN_IF_FAILED
	pFileNames->resize(numItems);
	for (DWORD i = 0; i < numItems; ++i) {
		ComPtr<IShellItem> pShellItem;
		hr = pShellItems->GetItemAt(i, &pShellItem); RETURN_IF_FAILED
		PWSTR pszFilePath;
		hr = pShellItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath); RETURN_IF_FAILED
		(*pFileNames)[i] = pszFilePath;
		CoTaskMemFree(pszFilePath);
	}
#undef RETURN_IF_FAILED
	return S_OK;
}

bool CDlgOpenFile_CommonItemDialog::DoModalOpenDlgImpl0(
	bool bAllowMultiSelect,
	std::vector<std::tstring>* pFileNames,
	LPCWSTR fileName,
	const std::vector<COMDLG_FILTERSPEC>& specs)
{
	using namespace Microsoft::WRL;
	ComPtr<IFileOpenDialog> pFileDialog;
	HRESULT hr;
#define RETURN_IF_FAILED if (FAILED(hr)) { /* __debugbreak(); */ return false; }
	hr = CoCreateInstance(
		CLSID_FileOpenDialog,
		NULL,
		CLSCTX_ALL,
		IID_IFileOpenDialog,
		&pFileDialog); RETURN_IF_FAILED
	DWORD dwCookie;
	hr = pFileDialog->Advise(this, &dwCookie); RETURN_IF_FAILED
	ComPtr<IFileDialogCustomize> pFileDialogCustomize;
	if (m_customizeSetting.bCustomize) {
		hr = pFileDialog.Get()->QueryInterface(IID_PPV_ARGS(&pFileDialogCustomize)); RETURN_IF_FAILED
		m_pFileDialogCustomize = pFileDialogCustomize.Get();
	}
	m_pFileDialog = pFileDialog.Get();
	hr = DoModalOpenDlgImpl1(pFileDialog.Get(), bAllowMultiSelect, pFileNames, fileName, specs);
	pFileDialog->Unadvise(dwCookie);
	m_pFileDialog = nullptr;
	m_pFileDialogCustomize = nullptr;
	RETURN_IF_FAILED
#undef RETURN_IF_FAILED
	return true;
}

/*! 「開く」ダイアログ モーダルダイアログの表示
*/
bool CDlgOpenFile_CommonItemDialog::DoModalOpenDlg(
	SLoadInfo* pLoadInfo,
	std::vector<std::tstring>* pFileNames,
	bool bOptions )
{
	// ファイルの種類	2003.05.12 MIK
	const int nTypesCount = m_pShareData->m_nTypesCount;
	std::vector<COMDLG_FILTERSPEC> specs(2 + nTypesCount);
	std::vector<std::wstring> strs(2 + nTypesCount);
	strs[0] = LS(STR_DLGOPNFL_EXTNAME3);
	strs[1] = LS(STR_DLGOPNFL_EXTNAME2);
	specs[0].pszName = strs[0].c_str();
	specs[0].pszSpec = _T("*.*");
	specs[1].pszName = strs[1].c_str();
	specs[1].pszSpec = _T("*.txt");
	CDocTypeManager docTypeMgr;
	TCHAR szWork[_countof(STypeConfigMini::m_szTypeExts) * 3];
	for( int i = 0; i < nTypesCount; i++ ){
		const STypeConfigMini* type;
		docTypeMgr.GetTypeConfigMini(CTypeConfig(i), &type);
		specs[2 + i].pszName = type->m_szTypeName;
		if (CDocTypeManager::ConvertTypesExtToDlgExt(type->m_szTypeExts, NULL, szWork)) {
			strs[2 + i] = szWork;
			specs[2 + i].pszSpec = strs[2 + i].c_str();
		}
		else {
			specs[2 + i].pszSpec = L"";
		}
	}
	m_bViewMode = pLoadInfo->bViewMode;
	m_nCharCode = pLoadInfo->eCharCode;	/* 文字コード自動判別 */
	m_customizeSetting.bShowReadOnly = true;
	m_customizeSetting.bSkipAutoDetect = false;
	m_customizeSetting.bUseCharCode = bOptions;
	bool ret = DoModalOpenDlgImpl0(true, pFileNames, pLoadInfo->cFilePath, specs);
	if (ret) {
		pLoadInfo->eCharCode = m_nCharCode;
		pLoadInfo->bViewMode = m_bViewMode;
	}
	return ret;
}

HRESULT CDlgOpenFile_CommonItemDialog::DoModalSaveDlgImpl1(
	IFileSaveDialog* pFileSaveDialog,
	const TCHAR* pszPath)
{
	//カレントディレクトリを保存。関数から抜けるときに自動でカレントディレクトリは復元される。
	CCurrentDirectoryBackupPoint cCurDirBackup;

	// 2010.08.28 Moca DLLが読み込まれるので移動
	ChangeCurrentDirectoryToExeDir();

	using namespace Microsoft::WRL;
	HRESULT hr;
	std::array<COMDLG_FILTERSPEC, 3> specs;
	std::array<std::wstring, 3> strs;
	strs[0] = LS(STR_DLGOPNFL_EXTNAME1);
	strs[1] = LS(STR_DLGOPNFL_EXTNAME2);
	strs[2] = LS(STR_DLGOPNFL_EXTNAME3);
	specs[0].pszName = strs[0].c_str();
	specs[0].pszSpec = m_szDefaultWildCard;
	specs[1].pszName = strs[1].c_str();
	specs[1].pszSpec = _T("*.txt");
	specs[2].pszName = strs[2].c_str();
	specs[2].pszSpec = _T("*.*");
#define RETURN_IF_FAILED if (FAILED(hr)) { /* __debugbreak(); */ return hr; }
	hr = pFileSaveDialog->SetFileTypes(specs.size(), &specs[0]); RETURN_IF_FAILED
	ComPtr<IShellItem> psiFolder;
	SHCreateItemFromParsingName(m_szInitialDir, NULL, IID_PPV_ARGS(&psiFolder));
	hr = pFileSaveDialog->SetFolder(psiFolder.Get()); RETURN_IF_FAILED
	TCHAR szFileName[_MAX_FNAME];
	SplitPath_FolderAndFile(pszPath, NULL, szFileName);
	hr = pFileSaveDialog->SetFileName(szFileName); RETURN_IF_FAILED

	if (m_customizeSetting.bCustomize) {
		hr = Customize(); RETURN_IF_FAILED
	}
	hr = pFileSaveDialog->Show(m_hwndParent); RETURN_IF_FAILED
#undef RETURN_IF_FAILED

	return S_OK;
}

bool CDlgOpenFile_CommonItemDialog::DoModalSaveDlgImpl0( const TCHAR* pszPath )
{
	using namespace Microsoft::WRL;
	ComPtr<IFileSaveDialog> pFileDialog;
	HRESULT hr;
#define RETURN_IF_FAILED if (FAILED(hr)) { /* __debugbreak(); */ return false; }
	hr = CoCreateInstance(
		CLSID_FileSaveDialog,
		NULL,
		CLSCTX_ALL,
		IID_IFileSaveDialog,
		&pFileDialog); RETURN_IF_FAILED
	m_pFileDialog = pFileDialog.Get();
	DWORD dwCookie;
	ComPtr<IFileDialogCustomize> pFileDialogCustomize;
	if (m_customizeSetting.bCustomize) {
		hr = pFileDialog.Get()->QueryInterface(IID_PPV_ARGS(&pFileDialogCustomize)); RETURN_IF_FAILED
		m_pFileDialogCustomize = pFileDialogCustomize.Get();
		hr = pFileDialog->Advise(this, &dwCookie); RETURN_IF_FAILED
		hr = DoModalSaveDlgImpl1(pFileDialog.Get(), pszPath);
		pFileDialog->Unadvise(dwCookie);
		m_pFileDialogCustomize = nullptr;
	}
	else {
		hr = DoModalSaveDlgImpl1(pFileDialog.Get(), pszPath);
	}
	m_pFileDialog = nullptr;
	RETURN_IF_FAILED
#undef RETURN_IF_FAILED
	return true;
}

/*! 保存ダイアログ モーダルダイアログの表示
*/
bool CDlgOpenFile_CommonItemDialog::DoModalSaveDlg( SSaveInfo* pSaveInfo, bool bSimpleMode )
{
	if (bSimpleMode) {
		m_customizeSetting.bCustomize = false;
	}
	else {
		m_nCharCode = pSaveInfo->eCharCode;
		m_bBom = pSaveInfo->bBomExist;
		m_customizeSetting.bCustomize = true;
		m_customizeSetting.bUseCharCode = true;
		m_customizeSetting.bUseEol = true;
		m_customizeSetting.bUseBom = true;
		m_customizeSetting.bSkipAutoDetect = true;
		m_customizeSetting.bShowReadOnly = false;
	}

	bool ret = DoModalSaveDlgImpl0(pSaveInfo->cFilePath);

	if (ret && !bSimpleMode) {
		pSaveInfo->eCharCode = m_nCharCode;
		//	Feb. 9, 2001 genta
		if( m_customizeSetting.bUseEol ){
			pSaveInfo->cEol = m_cEol;
		}
		//	Jul. 26, 2003 ryoji BOM設定
		if( m_customizeSetting.bUseBom ){
			pSaveInfo->bBomExist = m_bBom;
		}
	}
	return ret;
}

HRESULT CDlgOpenFile_CommonItemDialog::OnItemSelected(
	/* [in] */ __RPC__in_opt IFileDialogCustomize *pfdc,
	/* [in] */ DWORD dwIDCtl,
	/* [in] */ DWORD dwIDItem)
{
	switch (dwIDCtl) {
	case CtrlId::COMBO_CODE:
		{
			CCodeTypeName cCodeTypeName( (int)dwIDItem );
			CDCONTROLSTATEF state;
			bool bChecked;
			if (cCodeTypeName.UseBom()) {
				state = CDCS_ENABLEDVISIBLE;
				bChecked = (dwIDItem == m_nCharCode) ? m_bBom : cCodeTypeName.IsBomDefOn();
			}
			else {
				state = CDCS_VISIBLE;
				bChecked = false;
			}
			SetControlState(CtrlId::CHECK_BOM, state);
			SetCheckButtonState(CtrlId::CHECK_BOM, bChecked ? TRUE : FALSE);
		}
		break;
	case CtrlId::COMBO_MRU:
		if (dwIDItem != 0) {
			m_pFileDialog->SetFileName(m_vMRU[dwIDItem - 1]);
		}
		break;
	case CtrlId::COMBO_OPENFOLDER:
		if (dwIDItem != 0) {
			using namespace Microsoft::WRL;
			ComPtr<IShellItem> psiFolder;
			SHCreateItemFromParsingName(m_vOPENFOLDER[dwIDItem - 1], NULL, IID_PPV_ARGS(&psiFolder));
			m_pFileDialog->SetFolder(psiFolder.Get());
		}
		break;
	}
	return S_OK;
}

HRESULT CDlgOpenFile_CommonItemDialog::OnCheckButtonToggled(
	/* [in] */ __RPC__in_opt IFileDialogCustomize *pfdc,
	/* [in] */ DWORD dwIDCtl,
	/* [in] */ BOOL bChecked)
{
	switch (dwIDCtl) {
	case CtrlId::CHECK_READONLY:
		m_bViewMode = bChecked ? true : false;
		break;
	case CtrlId::CHECK_CP:
		SetControlState(CtrlId::CHECK_CP, CDCS_VISIBLE);
		AddComboCodePages(m_nCharCode);
		break;
	}
	return S_OK;
}

int CDlgOpenFile_CommonItemDialog::AddComboCodePages( int nSelCode )
{
	HRESULT hr;
	int nSel = -1;
	hr = AddControlItem(CtrlId::COMBO_CODE, (DWORD)CODE_CPACP, _T("CP_ACP"));
	if( nSelCode == CODE_CPACP ){
		SetSelectedControlItem(CtrlId::COMBO_CODE, (DWORD)CODE_CPACP);
		nSel = nSelCode;
	}
	hr = AddControlItem(CtrlId::COMBO_CODE, (DWORD)CODE_CPOEM, _T("CP_OEM"));
	if( nSelCode == CODE_CPOEM ){
		SetSelectedControlItem(CtrlId::COMBO_CODE, (DWORD)CODE_CPOEM);
		nSel = nSelCode;
	}
	CCodePage::CodePageList& cpList = CCodePage::GetCodePageList();
	for( auto it = cpList.begin(); it != cpList.end(); ++it ){
		hr = AddControlItem(CtrlId::COMBO_CODE, (DWORD)it->first, it->second.c_str());
		if( nSelCode == it->first ){
			SetSelectedControlItem(CtrlId::COMBO_CODE, (DWORD)it->first);
			nSel = nSelCode;
		}
	}
	return nSel;
}

std::shared_ptr<IDlgOpenFile> New_CDlgOpenFile_CommonItemDialog()
{
	std::shared_ptr<IDlgOpenFile> ret(new CDlgOpenFile_CommonItemDialog());
	return ret;
}
