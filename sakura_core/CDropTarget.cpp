/*!	@file
	@brief Drag & Drop

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani, Yebisuya Sugoroku
	Copyright (C) 2002, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include "view/CEditView.h"// 2002/2/3 aroka
#include "CDropTarget.h"
#include "global.h"
#include "debug.h"// 2002/2/3 aroka


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



COleLibrary::COleLibrary():m_dwCount(0)
{
	return;
}
COleLibrary::~COleLibrary()
{
	if( m_dwCount > 0 )
		::OleUninitialize();
	return;
}


void COleLibrary::Initialize()
{
	if( m_dwCount++ == 0 )
		::OleInitialize( NULL );
	return;
}


void COleLibrary::UnInitialize()
{
	if( m_dwCount > 0 && --m_dwCount == 0 )
		::OleUninitialize();
	return;
}


#define DECLARE_YB_INTERFACEIMPL( BASEINTERFACE ) \
template<> REFIID CYbInterfaceImpl<BASEINTERFACE>::m_owniid = IID_##BASEINTERFACE;

DECLARE_YB_INTERFACEIMPL( IDataObject )
DECLARE_YB_INTERFACEIMPL( IDropSource )
DECLARE_YB_INTERFACEIMPL( IDropTarget )




CDropTarget::CDropTarget( CEditView* pCEditView )
{
	m_pCEditView = pCEditView;
	m_hWnd_DropTarget = NULL;
	m_pDataObject = NULL;
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
		TopWarningMessage( hWnd, _T("::RegisterDragDrop()\nŽ¸”s") );
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
	return m_pCEditView->DragEnter( pDataObject, dwKeyState, pt, pdwEffect );
}
STDMETHODIMP CDropTarget::DragOver( DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	return m_pCEditView->DragOver( dwKeyState, pt, pdwEffect );
}
STDMETHODIMP CDropTarget::DragLeave( void )
{
	return m_pCEditView->DragLeave();
}


STDMETHODIMP CDropTarget::Drop( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
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




void CDataObject::SetText( LPCWSTR lpszText )
{
	//Feb. 26, 2001, fixed by yebisuya sugoroku
	if( m_pData != NULL )
	{
		int i;
		for( i = 0; i < m_nFormat; i++ )
			delete [](m_pData[i].data);
		delete []m_pData;
		m_pData = NULL;
		m_nFormat = 0;
	}
	if( lpszText != NULL ){
		m_nFormat = 2;
		m_pData = new DATA[m_nFormat];

		m_pData[0].cfFormat = CF_UNICODETEXT;
		m_pData[0].size = ( wcslen(lpszText) + 1) * sizeof( wchar_t );
		m_pData[0].data = new BYTE[m_pData[0].size];
		memcpy_raw( m_pData[0].data, lpszText, m_pData[0].size );

		m_pData[1].cfFormat = CF_TEXT;
		m_pData[1].size = ::WideCharToMultiByte( CP_ACP, 0, lpszText, m_pData[0].size/sizeof(wchar_t), NULL, 0, NULL, NULL );
		m_pData[1].data = new BYTE[m_pData[1].size];
		::WideCharToMultiByte( CP_ACP, 0, lpszText, m_pData[0].size/sizeof(wchar_t), (LPSTR)m_pData[1].data, m_pData[1].size, NULL, NULL );
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
	memcpy_raw( ::GlobalLock( lpsm->hGlobal ), m_pData[i].data, m_pData[i].size );
	::GlobalUnlock( lpsm->hGlobal );
	lpsm->pUnkForRelease = NULL;

	return S_OK;
}

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

	memcpy_raw( ::GlobalLock( lpsm->hGlobal ), m_pData[i].data, m_pData[i].size );
	::GlobalUnlock( lpsm->hGlobal );

	return S_OK;
}

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

STDMETHODIMP CDataObject::EnumFormatEtc( DWORD, LPENUMFORMATETC* )
{
	return E_NOTIMPL;
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



