//	$Id$
/*!	@file
	@brief Drag & Drop

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani, Yebisuya Sugoroku
	Copyright (C) 2002, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "CEditView.h"// 2002/2/3 aroka
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
		::MYMESSAGEBOX( hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
			"::RegisterDragDrop()\nŽ¸”s"
		);
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
	MYTRACE( "CDropTarget::DragEnter()\n" );
#endif
//	if( pDataObject == NULL || pdwEffect == NULL )
//		return E_INVALIDARG;
//	if( IsDataAvailable( pDataObject, CF_TEXT ) ){
//		m_pDataObject = pDataObject;
//		*pdwEffect = DROPEFFECT_COPY;
//		::SetFocus( m_hWnd_DropTarget );
//	}else{
//		*pdwEffect = DROPEFFECT_NONE;
//	}
	return m_pCEditView->DragEnter( pDataObject, dwKeyState, pt, pdwEffect );
//	return S_OK;
}
STDMETHODIMP CDropTarget::DragOver( DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
//	MYTRACE( "CDropTarget::DragOver()\n" );
//	if( pdwEffect == NULL )
//		return E_INVALIDARG;
//	::ScreenToClient( m_hWnd_DropTarget, (LPPOINT)&pt );
//	DWORD dwIndex = LOWORD( ::SendMessage( m_hWnd_DropTarget, EM_CHARFROMPOS, 0, MAKELPARAM( pt.x, pt.y ) ) );
//	if( dwIndex != (WORD) -1 ){
//		::SendMessage( m_hWnd_DropTarget, EM_SETSEL, dwIndex, dwIndex );
//		::SendMessage( m_hWnd_DropTarget, EM_SCROLLCARET, 0, 0 );
//	}
//	*pdwEffect = (m_pDataObject != NULL) ? DROPEFFECT_COPY : DROPEFFECT_NONE;
	return m_pCEditView->DragOver( dwKeyState, pt, pdwEffect );
//	return S_OK;
}
STDMETHODIMP CDropTarget::DragLeave( void )
{
//	MYTRACE( "CDropTarget::DragLeave()\n" );
//	m_pDataObject = NULL;
//	::SetFocus(NULL);
	return m_pCEditView->DragLeave();
//	return S_OK;
}


STDMETHODIMP CDropTarget::Drop( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
//	MYTRACE( "CDropTarget::Drop()\n" );
//	if( pDataObject == NULL || pdwEffect == NULL )
//		return E_INVALIDARG;
//	*pdwEffect = DROPEFFECT_NONE;
//	if( IsDataAvailable( pDataObject, CF_TEXT ) ){
//		HGLOBAL hData = GetGlobalData(pDataObject, CF_TEXT);
//		if( hData != NULL ){
//			DWORD nSize = 0;
//			LPCTSTR lpszSource = (LPCTSTR) ::GlobalLock( hData );
//			LPCTSTR lpsz = lpszSource;
//			while( *lpsz != _T('\0') ){
//				if( *lpsz == _T('\n') && (lpsz == lpszSource || *(lpsz - 1) != _T('\r')) )
//					++nSize;
//				++nSize;
//				++lpsz;
//			}
//			lpsz = lpszSource;
//			LPTSTR lpszDest = (LPTSTR) alloca( (nSize + 1) * sizeof( TCHAR ) );
//			LPTSTR lpsz2 = lpszDest;
//			while( *lpsz != _T('\0') ){
//				if( *lpsz == _T('\n') && (lpsz == lpszSource || *(lpsz - 1) != _T('\r')) )
//					*lpsz2++ = _T('\r');
//				*lpsz2++ = *lpsz++;
//			}
//			*lpsz2++ = _T('\0');
//
//			::SendMessage( m_hWnd_DropTarget, EM_REPLACESEL, 0, (LPARAM) lpszDest );
//			::GlobalUnlock( hData );
//			*pdwEffect = DROPEFFECT_COPY;
//		}
//	}
//	m_pDataObject = NULL;
//	::SetFocus( NULL );
	return m_pCEditView->Drop( pDataObject, dwKeyState, pt, pdwEffect );
//	return S_OK;
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




void CDataObject::SetText( LPCTSTR lpszText )
{
#ifdef ENABLED_YEBISUYA_ADDITION		//Feb. 26, 2001, fixed by yebisuya sugoroku
	if( data != NULL ){
		delete[] data;
		data = NULL;
		size = 0;
		m_cfFormat = 0;
	}
	if( lpszText != NULL ){
		size = (strlen( lpszText ) + 1) * sizeof( TCHAR );
		data = new BYTE[size];
		memcpy( data, lpszText, size) ;
		m_cfFormat = CF_TEXT;
	}
#else
	if( m_hData != NULL ){
		::GlobalFree( m_hData );
		m_hData = NULL;
		m_cfFormat = 0;
	}
	if( lpszText != NULL ){
//?		m_hData = ::GlobalAlloc( GHND | GMEM_DDESHARE, (_tcslen(lpszText) + 1) * sizeof( TCHAR ) );
		m_hData = ::GlobalAlloc( GHND | GMEM_DDESHARE, (lstrlen(lpszText) + 1) * sizeof( TCHAR ) );
//?		_tcscpy( (LPTSTR) ::GlobalLock( m_hData ), lpszText );
		strcpy( (LPTSTR) ::GlobalLock( m_hData ), lpszText );
		::GlobalUnlock( m_hData );
		m_cfFormat = CF_TEXT;
	}
#endif
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
#ifdef ENABLED_YEBISUYA_ADDITION		//Feb. 26, 2001, fixed by yebisuya sugoroku
	if( lpfe == NULL || lpsm == NULL )
		return E_INVALIDARG;
	if( data == NULL )
		return OLE_E_NOTRUNNING;
	if( lpfe->lindex != -1 )
		return DV_E_LINDEX;
	if( (lpfe->tymed & TYMED_HGLOBAL) == 0 )
		return DV_E_TYMED;
	if( lpfe->dwAspect != DVASPECT_CONTENT )
		return DV_E_DVASPECT;
	if( lpfe->cfFormat != m_cfFormat )
		return DV_E_FORMATETC;
	if( lpfe->cfFormat != m_cfFormat
		|| !(lpfe->tymed & TYMED_HGLOBAL)
		|| lpfe->lindex != -1
		|| lpfe->dwAspect != DVASPECT_CONTENT )
		return DV_E_FORMATETC;

	lpsm->tymed = TYMED_HGLOBAL;
	lpsm->hGlobal = ::GlobalAlloc( GHND | GMEM_DDESHARE, size );
	memcpy( (LPTSTR) ::GlobalLock( lpsm->hGlobal ), data, size );
	::GlobalUnlock( lpsm->hGlobal );
	lpsm->pUnkForRelease = NULL;

	return S_OK;
#else
	if( lpfe == NULL || lpsm == NULL )
		return E_INVALIDARG;
	if( m_hData == NULL )
		return OLE_E_NOTRUNNING;
	if( lpfe->cfFormat != m_cfFormat
		|| !(lpfe->tymed & TYMED_HGLOBAL)
		|| lpfe->lindex != -1
		|| lpfe->dwAspect != DVASPECT_CONTENT)
		return DV_E_FORMATETC;

	lpsm->tymed = TYMED_HGLOBAL;
	lpsm->hGlobal = m_hData;
	lpsm->pUnkForRelease = NULL;

	return S_OK;
#endif
}

STDMETHODIMP CDataObject::GetDataHere( LPFORMATETC lpfe, LPSTGMEDIUM lpsm )
{
#ifdef ENABLED_YEBISUYA_ADDITION		//Feb. 26, 2001, fixed by yebisuya sugoroku
	if( lpfe == NULL || lpsm == NULL || lpsm->hGlobal == NULL )
		return E_INVALIDARG;
	if( data == NULL )
		return OLE_E_NOTRUNNING;

	if( lpfe->lindex != -1 )
		return DV_E_LINDEX;
	if( lpfe->tymed != TYMED_HGLOBAL
		|| lpsm->tymed != TYMED_HGLOBAL )
		return DV_E_TYMED;
	if( size > ::GlobalSize( lpsm->hGlobal ) )
		return STG_E_MEDIUMFULL;
	if( lpfe->dwAspect != DVASPECT_CONTENT )
		return DV_E_DVASPECT;
	if( lpfe->cfFormat != m_cfFormat )
		return DV_E_FORMATETC;

	memcpy( ::GlobalLock( lpsm->hGlobal ), data, size );
	::GlobalUnlock( lpsm->hGlobal );

	return S_OK;
#else
	if( lpfe == NULL || lpsm == NULL || lpsm->hGlobal == NULL )
		return E_INVALIDARG;
	if( m_hData == NULL )
		return OLE_E_NOTRUNNING;
	if( lpfe->cfFormat != m_cfFormat
		|| !(lpfe->tymed & TYMED_HGLOBAL)
		|| lpfe->lindex != -1
		|| lpfe->dwAspect != DVASPECT_CONTENT )
		return DV_E_FORMATETC;

	DWORD nSize = ::GlobalSize( m_hData );
	if( nSize > ::GlobalSize( lpsm->hGlobal ) )
		return DATA_E_FORMATETC;

	LPVOID lpSource = ::GlobalLock( m_hData );
	LPVOID lpDest = ::GlobalLock( lpsm->hGlobal );
	memcpy( lpDest, lpSource, nSize );
	::GlobalUnlock(lpsm->hGlobal);
	::GlobalUnlock( m_hData );

	return S_OK;
#endif
}

STDMETHODIMP CDataObject::QueryGetData( LPFORMATETC lpfe )
{
	if( lpfe == NULL )
		return E_INVALIDARG;
#ifdef ENABLED_YEBISUYA_ADDITION		//Feb. 26, 2001, fixed by yebisuya sugoroku
	if( data == NULL )
		return OLE_E_NOTRUNNING;
#else
	if( m_hData == NULL )
		return OLE_E_NOTRUNNING;
#endif

	if( lpfe->cfFormat != m_cfFormat
		|| lpfe->ptd != NULL
		|| lpfe->dwAspect != DVASPECT_CONTENT
		|| lpfe->lindex != -1
		|| !(lpfe->tymed & TYMED_HGLOBAL) )
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


/*[EOF]*/
