/*!	@file
	@brief Drag & Drop

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani, Yebisuya Sugoroku
	Copyright (C) 2002, aroka
	Copyright (C) 2006, Moca
	Copyright (C) 2008, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CEditWnd.h"	// 2008.06.20 ryoji
#include "CEditView.h"// 2002/2/3 aroka
#include "CDropTarget.h"
#include "global.h"
#include "Debug.h"// 2002/2/3 aroka

/*
	@date 2006.01.16 Moca 他のTYMEDが利用可能でも、取得できるように変更。
	@note IDataObject::GetData() で tymed = TYMED_HGLOBAL を指定すること。
*/
BOOL IsDataAvailable( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat )
{
	FORMATETC	fe;

	// 2006.01.16 Moca 他のTYMEDが利用可能でも、IDataObject::GetData()で
	//  tymed = TYMED_HGLOBALを指定すれば問題ない
	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = TYMED_HGLOBAL;
	// 2006.03.16 Moca S_FALSEでも受け入れてしまうバグを修正(ファイルのドロップ等)
	return S_OK == pDataObject->QueryGetData( &fe );
}
HGLOBAL GetGlobalData( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat )
{
	FORMATETC fe;
	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	// 2006.01.16 Moca fe.tymed = -1からTYMED_HGLOBALに変更。
	fe.tymed = TYMED_HGLOBAL;

	HGLOBAL hDest = NULL;
	STGMEDIUM stgMedium;
	// 2006.03.16 Moca SUCCEEDEDマクロではS_FALSEのとき困るので、S_OKに変更
	if( S_OK == pDataObject->GetData( &fe, &stgMedium ) ){
		if( stgMedium.pUnkForRelease == NULL ){
			if( stgMedium.tymed == TYMED_HGLOBAL )
				hDest = stgMedium.hGlobal;
		}else{
			if( stgMedium.tymed == TYMED_HGLOBAL ){
				DWORD nSize = ::GlobalSize( stgMedium.hGlobal );
				hDest = ::GlobalAlloc( GMEM_SHARE|GMEM_MOVEABLE, nSize );
				if( hDest != NULL ){
					// copy the bits
					LPVOID lpSource = ::GlobalLock( stgMedium.hGlobal );
					LPVOID lpDest = ::GlobalLock( hDest );
					memcpy( lpDest, lpSource, nSize );
					::GlobalUnlock( hDest );
					::GlobalUnlock( stgMedium.hGlobal );
				}
			}
			::ReleaseStgMedium( &stgMedium );
		}
	}
	return hDest;
}


COleLibrary CYbInterfaceBase::m_olelib;

CYbInterfaceBase::CYbInterfaceBase()
{
	m_olelib.Initialize();
	return;
}
CYbInterfaceBase::~CYbInterfaceBase()
{
	m_olelib.UnInitialize();
	return;
}

HRESULT CYbInterfaceBase::QueryInterfaceImpl(
	IUnknown*	pThis,
	REFIID		owniid,
	REFIID		riid,
	void**		ppvObj
)
{
	if( riid == IID_IUnknown || riid == owniid ){
		pThis->AddRef();
		*ppvObj = pThis;
		return S_OK;
	}
	*ppvObj = NULL;
	return E_NOINTERFACE;
}

/////////////////////////////////////////



COleLibrary::COleLibrary()//:m_dwCount(0)	// 2009.01.08 ryoji m_dwCount削除
{
	return;
}
COleLibrary::~COleLibrary()
{
// 2009.01.08 ryoji OleUninitialize削除（WinMainにOleInitialize/OleUninitialize追加）
//	if( m_dwCount > 0 )
//		::OleUninitialize();
	return;
}


void COleLibrary::Initialize()
{
// 2009.01.08 ryoji OleInitialize削除（WinMainにOleInitialize/OleUninitialize追加）
//	if( m_dwCount++ == 0 )
//		::OleInitialize( NULL );
	return;
}


void COleLibrary::UnInitialize()
{
// 2009.01.08 ryoji OleUninitialize削除（WinMainにOleInitialize/OleUninitialize追加）
//	if( m_dwCount > 0 && --m_dwCount == 0 )
//		::OleUninitialize();
	return;
}


#define DECLARE_YB_INTERFACEIMPL( BASEINTERFACE ) \
template<> REFIID CYbInterfaceImpl<BASEINTERFACE>::m_owniid = IID_##BASEINTERFACE;

DECLARE_YB_INTERFACEIMPL( IDataObject )
DECLARE_YB_INTERFACEIMPL( IDropSource )
DECLARE_YB_INTERFACEIMPL( IDropTarget )
DECLARE_YB_INTERFACEIMPL( IEnumFORMATETC )



CDropTarget::CDropTarget( CEditWnd* pCEditWnd )
{
	m_pCEditWnd = pCEditWnd;	// 2008.06.20 ryoji
	m_pCEditView = NULL;
	m_hWnd_DropTarget = NULL;
	return;
}

CDropTarget::CDropTarget( CEditView* pCEditView )
{
	m_pCEditWnd = NULL;	// 2008.06.20 ryoji
	m_pCEditView = pCEditView;
	m_hWnd_DropTarget = NULL;
	return;
}


CDropTarget::~CDropTarget()
{
	Revoke_DropTarget();
	return;
}


BOOL CDropTarget::Register_DropTarget( HWND hWnd )
{
	if( FAILED( ::RegisterDragDrop( hWnd, this ) ) ){
		TopWarningMessage( hWnd, _T("::RegisterDragDrop()\n失敗") );
		return FALSE;
	}
	m_hWnd_DropTarget = hWnd;
	return TRUE;
}


BOOL CDropTarget::Revoke_DropTarget( void )
{
	BOOL bResult = TRUE;
	if( m_hWnd_DropTarget != NULL ){
		bResult = SUCCEEDED( ::RevokeDragDrop( m_hWnd_DropTarget ) );
		m_hWnd_DropTarget = NULL;
	}
	return bResult;
}
STDMETHODIMP CDropTarget::DragEnter( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
#ifdef _DEBUG
	MYTRACE_A( "CDropTarget::DragEnter()\n" );
#endif
	if( m_pCEditWnd ){	// 2008.06.20 ryoji
		return m_pCEditWnd->DragEnter( pDataObject, dwKeyState, pt, pdwEffect );
	}
	return m_pCEditView->DragEnter( pDataObject, dwKeyState, pt, pdwEffect );
}
STDMETHODIMP CDropTarget::DragOver( DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	if( m_pCEditWnd ){	// 2008.06.20 ryoji
		return m_pCEditWnd->DragOver( dwKeyState, pt, pdwEffect );
	}
	return m_pCEditView->DragOver( dwKeyState, pt, pdwEffect );
}
STDMETHODIMP CDropTarget::DragLeave( void )
{
	if( m_pCEditWnd ){	// 2008.06.20 ryoji
		return m_pCEditWnd->DragLeave();
	}
	return m_pCEditView->DragLeave();
}


STDMETHODIMP CDropTarget::Drop( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	if( m_pCEditWnd ){	// 2008.06.20 ryoji
		return m_pCEditWnd->Drop( pDataObject, dwKeyState, pt, pdwEffect );
	}
	return m_pCEditView->Drop( pDataObject, dwKeyState, pt, pdwEffect );
}


STDMETHODIMP CDropSource::QueryContinueDrag( BOOL bEscapePressed, DWORD dwKeyState )
{
	if( bEscapePressed || (dwKeyState & (m_bLeft ? MK_RBUTTON : MK_LBUTTON)) )
		return DRAGDROP_S_CANCEL;
	if( !(dwKeyState & (m_bLeft ? MK_LBUTTON : MK_RBUTTON)) )
		return DRAGDROP_S_DROP;
	return S_OK;
}


STDMETHODIMP CDropSource::GiveFeedback( DWORD dropEffect )
{
	return DRAGDROP_S_USEDEFAULTCURSORS;
}




/** 転送対象の文字列を設定する
	@param lpszText [in] 文字列
	@param nTextLen [in] pszTextの長さ
	@param bColmnSelect [in] 矩形選択か

	@date 2008.03.26 ryoji 複数フォーマット対応
*/
void CDataObject::SetText( LPCSTR lpszText, int nTextLen, BOOL bColmnSelect )
{
	//Feb. 26, 2001, fixed by yebisuya sugoroku
	int i;
	if( m_pData != NULL )
	{
		for( i = 0; i < m_nFormat; i++ )
			delete [](m_pData[i].data);
		delete []m_pData;
		m_pData = NULL;
		m_nFormat = 0;
	}
	if( lpszText != NULL ){
		m_nFormat = bColmnSelect? 3: 2;	// 矩形を含めるか
		m_pData = new DATA[m_nFormat];

		i = 0;
		m_pData[0].cfFormat = CF_TEXT;
		m_pData[0].size = nTextLen + 1;
		m_pData[0].data = new BYTE[m_pData[0].size];
		memcpy( m_pData[0].data, lpszText, nTextLen );
		*((char*)m_pData[0].data + nTextLen) = '\0';

		//i++;
		//m_pData[i].cfFormat = CF_UNICODETEXT;
		//m_pData[i].size = ::MultiByteToWideChar(CP_ACP, 0, lpszText, -1, NULL, 0) * sizeof(WCHAR);
		//m_pData[i].data = new BYTE[m_pData[i].size];
		//::MultiByteToWideChar(CP_ACP, 0, lpszText, -1, (LPWSTR)m_pData[i].data, m_pData[i].size);

		i++;
		m_pData[i].cfFormat = ::RegisterClipboardFormat( _T("SAKURAClip") );
		m_pData[i].size = sizeof(int) + nTextLen;
		m_pData[i].data = new BYTE[m_pData[i].size];
		*(int*)m_pData[i].data = nTextLen;
		memcpy( m_pData[i].data + sizeof(int), lpszText, nTextLen );

		i++;
		if( bColmnSelect ){
			m_pData[i].cfFormat = ::RegisterClipboardFormat( _T("MSDEVColumnSelect") );
			m_pData[i].size = 1;
			m_pData[i].data = new BYTE[1];
			m_pData[i].data[0] = '\0';
		}
	}
}

DWORD CDataObject::DragDrop( BOOL bLeft, DWORD dwEffects )
{
	DWORD dwEffect;
	CDropSource drop( bLeft );
	if( SUCCEEDED( ::DoDragDrop( this, &drop, dwEffects, &dwEffect ) ) )
		return dwEffect;
	return DROPEFFECT_NONE;
}

/** IDataObject::GetData
	@date 2008.03.26 ryoji 複数フォーマット対応
*/
STDMETHODIMP CDataObject::GetData( LPFORMATETC lpfe, LPSTGMEDIUM lpsm )
{
	//Feb. 26, 2001, fixed by yebisuya sugoroku
	if( lpfe == NULL || lpsm == NULL )
		return E_INVALIDARG;
	if( m_pData == NULL )
		return OLE_E_NOTRUNNING;
	if( lpfe->lindex != -1 )
		return DV_E_LINDEX;
	if( (lpfe->tymed & TYMED_HGLOBAL) == 0 )
		return DV_E_TYMED;
	if( lpfe->dwAspect != DVASPECT_CONTENT )
		return DV_E_DVASPECT;
	if( !(lpfe->tymed & TYMED_HGLOBAL)
		|| lpfe->lindex != -1
		|| lpfe->dwAspect != DVASPECT_CONTENT )
		return DV_E_FORMATETC;

	int i;
	for( i = 0; i < m_nFormat; i++ ){
		if( lpfe->cfFormat == m_pData[i].cfFormat )
			break;
	}
	if( i == m_nFormat )
		return DV_E_FORMATETC;

	lpsm->tymed = TYMED_HGLOBAL;
	lpsm->hGlobal = ::GlobalAlloc( GHND | GMEM_DDESHARE, m_pData[i].size );
	memcpy( ::GlobalLock( lpsm->hGlobal ), m_pData[i].data, m_pData[i].size );
	::GlobalUnlock( lpsm->hGlobal );
	lpsm->pUnkForRelease = NULL;

	return S_OK;
}

/** IDataObject::GetDataHere
	@date 2008.03.26 ryoji 複数フォーマット対応
*/
STDMETHODIMP CDataObject::GetDataHere( LPFORMATETC lpfe, LPSTGMEDIUM lpsm )
{
	//Feb. 26, 2001, fixed by yebisuya sugoroku
	if( lpfe == NULL || lpsm == NULL || lpsm->hGlobal == NULL )
		return E_INVALIDARG;
	if( m_pData == NULL )
		return OLE_E_NOTRUNNING;

	if( lpfe->lindex != -1 )
		return DV_E_LINDEX;
	if( lpfe->tymed != TYMED_HGLOBAL
		|| lpsm->tymed != TYMED_HGLOBAL )
		return DV_E_TYMED;
	if( lpfe->dwAspect != DVASPECT_CONTENT )
		return DV_E_DVASPECT;

	int i;
	for( i = 0; i < m_nFormat; i++ ){
		if( lpfe->cfFormat == m_pData[i].cfFormat )
			break;
	}
	if( i == m_nFormat )
		return DV_E_FORMATETC;
	if( m_pData[i].size > ::GlobalSize( lpsm->hGlobal ) )
		return STG_E_MEDIUMFULL;

	memcpy( ::GlobalLock( lpsm->hGlobal ), m_pData[i].data, m_pData[i].size );
	::GlobalUnlock( lpsm->hGlobal );

	return S_OK;
}

/** IDataObject::QueryGetData
	@date 2008.03.26 ryoji 複数フォーマット対応
*/
STDMETHODIMP CDataObject::QueryGetData( LPFORMATETC lpfe )
{
	if( lpfe == NULL )
		return E_INVALIDARG;
	//Feb. 26, 2001, fixed by yebisuya sugoroku
	if( m_pData == NULL )
		return OLE_E_NOTRUNNING;

	if( lpfe->ptd != NULL
		|| lpfe->dwAspect != DVASPECT_CONTENT
		|| lpfe->lindex != -1
		|| !(lpfe->tymed & TYMED_HGLOBAL) )
		return DATA_E_FORMATETC;

	int i;
	for( i = 0; i < m_nFormat; i++ ){
		if( lpfe->cfFormat == m_pData[i].cfFormat )
			break;
	}
	if( i == m_nFormat )
		return DATA_E_FORMATETC;
	return S_OK;
}

STDMETHODIMP CDataObject::GetCanonicalFormatEtc( LPFORMATETC, LPFORMATETC )
{
	return DATA_S_SAMEFORMATETC;
}

STDMETHODIMP CDataObject::SetData( LPFORMATETC, LPSTGMEDIUM, BOOL )
{
	return E_NOTIMPL;
}

/** IDataObject::EnumFormatEtc
	@date 2008.03.26 ryoji IEnumFORMATETCをサポート
*/
STDMETHODIMP CDataObject::EnumFormatEtc( DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc )
{
	if( dwDirection != DATADIR_GET )
		return S_FALSE;
	*ppenumFormatetc = new CEnumFORMATETC(this);
	return *ppenumFormatetc? S_OK: S_FALSE;
}

STDMETHODIMP CDataObject::DAdvise( LPFORMATETC, DWORD, LPADVISESINK, LPDWORD )
{
	return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP CDataObject::DUnadvise( DWORD )
{
	return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP CDataObject::EnumDAdvise( LPENUMSTATDATA* )
{
	return OLE_E_ADVISENOTSUPPORTED;
}


/** IEnumFORMATETC::Next
	@date 2008.03.26 ryoji 新規作成
*/
STDMETHODIMP CEnumFORMATETC::Next(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched)
{
	if( celt <= 0 || rgelt == NULL || m_nIndex >= m_pcDataObject->m_nFormat )
		return S_FALSE;
	if( celt != 1 && pceltFetched == NULL )
		return S_FALSE;

	ULONG i = celt;
	while( m_nIndex < m_pcDataObject->m_nFormat && i > 0 ){
		(*rgelt).cfFormat = m_pcDataObject->m_pData[m_nIndex].cfFormat;
		(*rgelt).ptd = NULL;
		(*rgelt).dwAspect = DVASPECT_CONTENT;
		(*rgelt).lindex = -1;
		(*rgelt).tymed = TYMED_HGLOBAL;
		rgelt++;
		m_nIndex++;
		i--;
	}
	if( pceltFetched != NULL )
		*pceltFetched = celt - i;

	return (i == 0)? S_OK : S_FALSE;
}

/** IEnumFORMATETC::Skip
	@date 2008.03.26 ryoji 新規作成
*/
STDMETHODIMP CEnumFORMATETC::Skip(ULONG celt)
{
	while( m_nIndex < m_pcDataObject->m_nFormat && celt > 0 ){
		++m_nIndex;
		--celt;
	}

	return (celt == 0)? S_OK : S_FALSE;
}

/** IEnumFORMATETC::Reset
	@date 2008.03.26 ryoji 新規作成
*/
STDMETHODIMP CEnumFORMATETC::Reset(void)
{
	m_nIndex = 0;
	return S_OK;
}

/** IEnumFORMATETC::Clone
	@date 2008.03.26 ryoji 新規作成
*/
STDMETHODIMP CEnumFORMATETC::Clone(IEnumFORMATETC** ppenum)
{
	return E_NOTIMPL;
}


/*[EOF]*/
