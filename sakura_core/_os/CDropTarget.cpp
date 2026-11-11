/*!	@file
	@brief Drag & Drop

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani, Yebisuya Sugoroku
	Copyright (C) 2002, aroka
	Copyright (C) 2008, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "_os/CDropTarget.h"

#include "basis/primitive.h"
#include "window/CEditWnd.h"	// 2008.06.20 ryoji
#include "view/CEditView.h"// 2002/2/3 aroka
#include "_main/global.h"
#include "CClipboard.h"
#include "CSelectLang.h"

CDropTarget::CDropTarget(CEditView* pCEditView)
	: m_pcEditView{ pCEditView }
{
	return;
}

CDropTarget::~CDropTarget()
{
	return;
}

void CEditView::EnterDragMode()
{
	_SetDragMode(true);
}

bool CEditView::IsDragFile() const
{
	return
		!IsDragText() &&
		m_Formats.contains(CF_HDROP);
}

bool CEditView::IsDragText() const
{
	return
		m_Formats.contains(CClipboard::GetSakuraFormat()) ||
		m_Formats.contains(CF_UNICODETEXT) ||
		m_Formats.contains(CF_TEXT);
}

IFACEMETHODIMP CDropTarget::DragEnter(
	LPDATAOBJECT pDataObject,
	DWORD dwKeyState,
	POINTL pt,
	LPDWORD pdwEffect
)
{
	DEBUG_TRACE( L"CDropTarget::DragEnter()\n" );
	return m_pcEditView->DragEnter( pDataObject, dwKeyState, pt, pdwEffect );
}

IFACEMETHODIMP CDropTarget::DragOver(
	DWORD dwKeyState,
	POINTL pt,
	LPDWORD pdwEffect
)
{
	return m_pcEditView->DragOver( dwKeyState, pt, pdwEffect );
}

IFACEMETHODIMP CDropTarget::DragLeave()
{
	return m_pcEditView->DragLeave();
}

IFACEMETHODIMP CDropTarget::Drop(
	LPDATAOBJECT pDataObject,
	DWORD dwKeyState,
	POINTL pt,
	LPDWORD pdwEffect
)
{
	return m_pcEditView->Drop( pDataObject, dwKeyState, pt, pdwEffect );
}

void CEditView::LeaveDragMode()
{
	_SetDragMode(false);

	// 非アクティブ時は表示状態を非アクティブに戻す
	if (!::GetActiveWindow()) {
		OnKillFocus();
	}
}

/* static */ DWORD CDropSource::DoDragDrop(
	LPDATAOBJECT pDataObject,
	DWORD dwDesiredEffects,
	bool bLeft
)
{
	DWORD dwEffects;

	const auto dropSource = Base::make_instance<Me>(bLeft);

	if (FAILED(::DoDragDrop(
			pDataObject,
			dropSource,
			dwDesiredEffects,
			&dwEffects
		)))
	{
		dwEffects = DROPEFFECT_NONE;
	}

	return dwEffects;
}

IFACEMETHODIMP CDropSource::QueryContinueDrag(
	BOOL bEscapePressed,
	DWORD dwKeyState
)
{
	if (const auto altButtonPressed = dwKeyState & (m_bLeft ? MK_RBUTTON : MK_LBUTTON);
		bEscapePressed || altButtonPressed)
		return DRAGDROP_S_CANCEL;

	if (const auto targetButtonPressed = dwKeyState & (m_bLeft ? MK_LBUTTON : MK_RBUTTON);
		!targetButtonPressed)
		return DRAGDROP_S_DROP;

	return S_OK;
}

IFACEMETHODIMP CDropSource::GiveFeedback(
	DWORD dropEffect [[maybe_unused]]
)
{
	return DRAGDROP_S_USEDEFAULTCURSORS;
}

/* static */ CLIPFORMAT CDataObject::cfColumnSelect	= (CLIPFORMAT)::RegisterClipboardFormatW(L"MSDEVColumnSelect");
/* static */ CLIPFORMAT CDataObject::cfLineSelect1	= (CLIPFORMAT)::RegisterClipboardFormatW(L"MSDEVLineSelect");
/* static */ CLIPFORMAT CDataObject::cfLineSelect2	= (CLIPFORMAT)::RegisterClipboardFormatW(L"VisualStudioEditorOperationsLineCutCopyClipboardTag");
/* static */ CLIPFORMAT CDataObject::cfSakuraClip	= (CLIPFORMAT)::RegisterClipboardFormatW(L"SAKURAClipW");

CDataObject::CDataObject(
	std::wstring_view text,
	bool bColumnSelect
)
{
	SetText(text, bColumnSelect);
}

/** 転送対象の文字列を設定する
	@param lpszText [in] 文字列
	@param nTextLen [in] pszTextの長さ
	@param bColumnSelect [in] 矩形選択か

	@date 2008.03.26 ryoji 複数フォーマット対応
*/
void CDataObject::SetText(
	std::wstring_view text,
	bool bColumnSelect
)
{
	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString<WCHAR> _Text{ text };

	const auto lpszText = text.empty() ? nullptr : _Text.c_str();
	const auto nTextLen = text.length();

	//Feb. 26, 2001, fixed by yebisuya sugoroku
	int i;
	if( m_pData != nullptr )
	{
		for( i = 0; i < m_nFormat; i++ )
			delete [](m_pData[i].data);
		delete []m_pData;
		m_pData = nullptr;
		m_nFormat = 0;
	}
	if( lpszText != nullptr ){
		// 長さフィールドで表せない長さのときは独自形式では受け渡さない
		const bool bSakuraFormat = nTextLen <= CClipboard::SAKURAClipW_MaxLength;
		m_nFormat = 2 + (bSakuraFormat? 1: 0) + (bColumnSelect? 1: 0);	// 独自形式・矩形を含めるか
		m_pData = new DATA[m_nFormat];

		i = 0;
		m_pData[0].cfFormat = CF_UNICODETEXT;
		m_pData[0].size = (nTextLen + 1) * sizeof(wchar_t);
		m_pData[0].data = new BYTE[m_pData[0].size];
		memcpy_raw( m_pData[0].data, lpszText, nTextLen * sizeof(wchar_t) );
		*((wchar_t*)m_pData[0].data + nTextLen) = L'\0';

		i++;
		m_pData[i].cfFormat = CF_TEXT;
		m_pData[i].size = ::WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)m_pData[0].data, int(m_pData[0].size / sizeof(wchar_t)), nullptr, 0, nullptr, nullptr );
		m_pData[i].data = new BYTE[m_pData[i].size];
		::WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)m_pData[0].data, int(m_pData[0].size / sizeof(wchar_t)), (LPSTR)m_pData[i].data, int(m_pData[i].size), nullptr, nullptr );

		if( bSakuraFormat ){
			using LengthField = CClipboard::SAKURAClipW_LengthFieldType;
			i++;
			m_pData[i].cfFormat = CClipboard::GetSakuraFormat();
			m_pData[i].size = sizeof(LengthField) + nTextLen * sizeof( wchar_t );
			m_pData[i].data = new BYTE[m_pData[i].size];
			*(LengthField*)m_pData[i].data = (LengthField)nTextLen;
			memcpy_raw( m_pData[i].data + sizeof(LengthField), lpszText, nTextLen * sizeof( wchar_t ) );
		}

		if( bColumnSelect ){
			i++;
			m_pData[i].cfFormat = CDataObject::cfColumnSelect;
			m_pData[i].size = 1;
			m_pData[i].data = new BYTE[1];
			m_pData[i].data[0] = '\0';
		}
	}
}

/** IDataObject::GetData
	@date 2008.03.26 ryoji 複数フォーマット対応
*/
STDMETHODIMP CDataObject::GetData( LPFORMATETC lpfe, LPSTGMEDIUM lpsm )
{
	//Feb. 26, 2001, fixed by yebisuya sugoroku
	if( lpfe == nullptr || lpsm == nullptr )
		return E_INVALIDARG;
	if( m_pData == nullptr )
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
	memcpy_s( ::GlobalLock( lpsm->hGlobal ), m_pData[i].size, m_pData[i].data, m_pData[i].size );
	::GlobalUnlock( lpsm->hGlobal );
	lpsm->pUnkForRelease = nullptr;

	return S_OK;
}

/** IDataObject::GetDataHere
	@date 2008.03.26 ryoji 複数フォーマット対応
*/
STDMETHODIMP CDataObject::GetDataHere( LPFORMATETC lpfe, LPSTGMEDIUM lpsm )
{
	//Feb. 26, 2001, fixed by yebisuya sugoroku
	if( lpfe == nullptr || lpsm == nullptr || lpsm->hGlobal == nullptr )
		return E_INVALIDARG;
	if( m_pData == nullptr )
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

	memcpy_s( ::GlobalLock( lpsm->hGlobal ), m_pData[i].size, m_pData[i].data, m_pData[i].size );
	::GlobalUnlock( lpsm->hGlobal );

	return S_OK;
}

/** IDataObject::QueryGetData
	@date 2008.03.26 ryoji 複数フォーマット対応
*/
STDMETHODIMP CDataObject::QueryGetData( LPFORMATETC lpfe )
{
	if( lpfe == nullptr )
		return E_INVALIDARG;
	//Feb. 26, 2001, fixed by yebisuya sugoroku
	if( m_pData == nullptr )
		return OLE_E_NOTRUNNING;

	if( lpfe->ptd != nullptr
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

	const auto pEnum = CEnumFORMATETC::make_instance(this);
	*ppenumFormatetc = pEnum;
	pEnum->AddRef();

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

/*!
 * @brief IEnumFORMATETC::Next
 *
 * @param[in] celt 取得する要素の数
 * @param[out] rgelt 取得した要素を格納する配列
 * @param[out, opt] pceltFetched 取得した要素数の格納先。（celtが1の場合のみ省略可。）
 * @refval S_OK      要求された数（celt）の要素をすべて正常に取得できた。
 * @refval S_FALSE   要求された数より少ない要素しか取得できなかった（列挙の末尾に達した）。
 * @refval E_POINTER rgelt が無効なポインタである（または celt > 1 なのに pceltFetched が NULL である）。
 *
 * @date 2008.03.26 ryoji 新規作成
 */
IFACEMETHODIMP CEnumFORMATETC::Next(
	ULONG celt,
	FORMATETC* rgelt,
	ULONG* pceltFetched
)
{
	if (0 < celt && !rgelt ||
		celt != 1 && !pceltFetched)
	{
		return E_POINTER;
	}

	if (0 == celt ||
		!m_pcDataObject ||
		m_pcDataObject->m_nFormat <= m_nIndex)
	{
		return S_FALSE;
	}

	ULONG i = celt;
	while( m_nIndex < m_pcDataObject->m_nFormat && i > 0 ){
		(*rgelt).cfFormat = m_pcDataObject->m_pData[m_nIndex].cfFormat;
		(*rgelt).ptd = nullptr;
		(*rgelt).dwAspect = DVASPECT_CONTENT;
		(*rgelt).lindex = -1;
		(*rgelt).tymed = TYMED_HGLOBAL;
		rgelt++;
		m_nIndex++;
		i--;
	}
	if( pceltFetched != nullptr )
		*pceltFetched = celt - i;

	return (i == 0)? S_OK : S_FALSE;
}

/** IEnumFORMATETC::Skip
	@date 2008.03.26 ryoji 新規作成
*/
IFACEMETHODIMP CEnumFORMATETC::Skip(
	ULONG celt
)
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
IFACEMETHODIMP CEnumFORMATETC::Reset()
{
	m_nIndex = 0;
	return S_OK;
}

/** IEnumFORMATETC::Clone
	@date 2008.03.26 ryoji 新規作成
*/
IFACEMETHODIMP CEnumFORMATETC::Clone(
	IEnumFORMATETC** ppenum [[maybe_unused]]
)
{
	return E_NOTIMPL;
}
