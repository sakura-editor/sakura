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
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CDROPTARGET_365C215D_E844_49BD_8E96_549970AE0A50_H_
#define SAKURA_CDROPTARGET_365C215D_E844_49BD_8E96_549970AE0A50_H_
#pragma once

#include <Unknwn.h>

#include "cxx/TComImpl.hpp"
#include "util/design_template.h"
#include "util/os.h"

#include <set>

class CDropTarget;

class CCaret;
class CEditWnd;	// 2008.06.20 ryoji
class CEditView;// 2002/2/3 aroka ヘッダー軽量化

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/

/*!
 * @brief D&Dターゲットクラス
 *
 * D&Dターゲット(IDropTarget)の実装です。
 */
class CDropTarget : public cxx::TComImpl<IDropTarget>
{
private:
	using Base = cxx::TComImpl<IDropTarget>;
	using Me = CDropTarget;

	static inline int dummyInt = 0;
	static inline BOOL dummyFlag = false;

public:
	// 生成関数
	template<typename T, typename... Args>
	static com_pointer_type make_instance(Args&&... args)
		requires std::constructible_from<T, Args...>
	{
		return Base::template make_instance<T>(std::forward<Args>(args)...);
	}
	
	/*
	||  Constructors
	*/
	explicit CDropTarget(
		CEditView* pcEditView = nullptr
	);

	~CDropTarget() override;

	void EnterDragMode();
	CCaret& GetCaret();
	HWND GetHwnd() const;
	bool IsDragSource() const;
	bool IsDragFile() const;
	bool IsDragText() const;
	void LeaveDragMode();
	void OnMOUSEMOVE(UINT wParam, int x, int y) const;
	HRESULT PostMyDropFiles(
		LPDATAOBJECT pDataObject
	);
	void RedrawAll();
	DWORD TranslateDropEffect(
		DWORD dwKeyState,
		DWORD dwEffect
	) const;

	IFACEMETHODIMP	DragEnter(LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect) override;
	IFACEMETHODIMP	DragOver(DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect) override;
	IFACEMETHODIMP	DragLeave() override;
	IFACEMETHODIMP	Drop(LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect) override;

	/*
	||  Attributes & Operations
	*/
private: // 2002/2/10 aroka アクセス権変更
	CEditView*		m_pcEditView = nullptr;
	int&			m_nMyIndex = dummyInt;

	using SCfArray = std::set<CLIPFORMAT>;
	SCfArray		m_Formats{};						/* ドラッグデータのクリップ形式 */

	BOOL&			m_bDragBoxData = dummyFlag;
	CLayoutPoint	m_ptCaretPos_DragEnter;				/* ドラッグ開始時のカーソル位置 */
	CLayoutInt		m_nCaretPosX_Prev_DragEnter{ 0 };	/* ドラッグ開始時のX座標記憶 */
};

/*!
 * @brief 編集ウィンドウのD&Dターゲット実装
 *
 * @date 2008/06/20 ryoji 右ボタンドロップ用
 */
class CDropTarget_CEditWnd final : public CDropTarget
{
private:
	using Base = CDropTarget;
	using Me = CDropTarget_CEditWnd;

public:
	CDropTarget_CEditWnd() = default;

	IFACEMETHODIMP	DragEnter(LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect) override;
	IFACEMETHODIMP	DragOver(DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect) override;
	IFACEMETHODIMP	DragLeave() override;
	IFACEMETHODIMP	Drop(LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect) override;
};

/*!
 * @brief D&Dソースクラス
 *
 * D&Dソース(IDropSource)の実装です。
 */
class CDropSource final : public cxx::TComImpl<IDropSource> {
private:
	using Base = cxx::TComImpl<IDropSource>;
	using Me = CDropSource;

public:
	static DWORD DoDragDrop(
		LPDATAOBJECT pDataObject,
		DWORD dwDesiredEffects,
		bool bLeft = true
	);

	explicit CDropSource(
		bool bLeft
	)
		: m_bLeft{ bLeft }
	{
	}

	STDMETHOD( QueryContinueDrag )( BOOL bEscapePressed, DWORD dwKeyState );
	STDMETHOD( GiveFeedback )( DWORD dropEffect );

private:
	bool m_bLeft;
};

/*!
 * @brief クリップボードデータクラス
 *
 * クリップボードデータ(IDataObject)の実装です。
 * 「サクラエディタが扱えるクリップボードデータ」です。
 * SetDataをサポートしない Init Only な実装です。
 */
class CDataObject final : public cxx::TComImpl<IDataObject> {
private:
	friend class CEnumFORMATETC;	// 2008.03.26 ryoji

	struct DATA;
	using PDATA = DATA*;

	//Feb. 26, 2001, fixed by yebisuya sugoroku
	struct DATA {
		CLIPFORMAT	cfFormat = CF_UNICODETEXT;	//!< クリップボードフォーマット
		LPBYTE		data = nullptr;				//!< データ
		SIZE_T		size = 0ULL;				//!< データサイズ。バイト単位。
	};

	using Base = cxx::TComImpl<IDataObject>;
	using Me = CDataObject;

	int m_nFormat = 0;
	PDATA m_pData = nullptr;

public:
	static CLIPFORMAT cfColumnSelect;
	static CLIPFORMAT cfLineSelect1;
	static CLIPFORMAT cfLineSelect2;
	static CLIPFORMAT cfSakuraClip;

	// 生成関数
	template<typename... Args>
	static com_pointer_type make_instance(Args&&... args)
		requires std::constructible_from<Me, Args...>
	{
		return Base::template make_instance<Me>(std::forward<Args>(args)...);
	}

	explicit CDataObject(
		std::wstring_view text,
		bool bColumnSelect = false
	);

	STDMETHOD( GetData )( LPFORMATETC, LPSTGMEDIUM );
	STDMETHOD( GetDataHere )( LPFORMATETC, LPSTGMEDIUM );
	STDMETHOD( QueryGetData )( LPFORMATETC );
	STDMETHOD( GetCanonicalFormatEtc )( LPFORMATETC, LPFORMATETC );
	STDMETHOD( SetData )( LPFORMATETC, LPSTGMEDIUM, BOOL );
	STDMETHOD( EnumFormatEtc )( DWORD, IEnumFORMATETC** );
	STDMETHOD( DAdvise )( LPFORMATETC, DWORD, LPADVISESINK, LPDWORD );
	STDMETHOD( DUnadvise )( DWORD );
	STDMETHOD( EnumDAdvise )( LPENUMSTATDATA* );

	void SetText(
		std::wstring_view text,
		bool bColumnSelect = false
	);
};

/*!
 * @brief データ形式を列挙するクラス
 *
 * クリップボードに含まれるデータ形式を列挙する。
 *
 * @date 2008/03/26 ryoji 複数フォーマット対応で新規作成
 */
class CEnumFORMATETC final : public cxx::TComImpl<IEnumFORMATETC> {
private:
	using Base = cxx::TComImpl<IEnumFORMATETC>;
	using Me = CEnumFORMATETC;

	int m_nIndex = 0;
	CDataObject* m_pcDataObject;

public:
	// 生成関数
	template<typename... Args>
	static com_pointer_type make_instance(Args&&... args)
		requires std::constructible_from<Me, Args...>
	{
		return Base::template make_instance<Me>(std::forward<Args>(args)...);
	}

	explicit CEnumFORMATETC(
		CDataObject* pcDataObject
	)
		: m_pcDataObject(pcDataObject)
	{
	}

	STDMETHOD( Next )(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched);
	STDMETHOD( Skip )(ULONG celt);
	STDMETHOD( Reset )(void);
	STDMETHOD( Clone )(IEnumFORMATETC** ppenum);
};

#endif /* SAKURA_CDROPTARGET_365C215D_E844_49BD_8E96_549970AE0A50_H_ */
