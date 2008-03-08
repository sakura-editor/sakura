/*!	@file
	@brief Drag & Drop

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani, Yebisuya Sugoroku
	Copyright (C) 2002, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDropTarget;
class CYbInterfaceBase;
class COleLibrary;

#ifndef _CEDITDROPTARGET_H_
#define _CEDITDROPTARGET_H_

#include <windows.h>
class CEditView;// 2002/2/3 aroka ヘッダ軽量化

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class COleLibrary
{
	friend class CYbInterfaceBase;
private:
	DWORD m_dwCount;
	COleLibrary();
public:
	~COleLibrary();
private:
	void Initialize();
	void UnInitialize();
};



class CYbInterfaceBase
{
private:
	static COleLibrary m_olelib;
protected:
	CYbInterfaceBase();
	~CYbInterfaceBase();
	static HRESULT QueryInterfaceImpl( IUnknown*, REFIID, REFIID, void** );
};


template<class BASEINTERFACE>
class CYbInterfaceImpl : public BASEINTERFACE, public CYbInterfaceBase
{
private:
	static REFIID m_owniid;
public:
	CYbInterfaceImpl(){AddRef();}
	STDMETHOD( QueryInterface )( REFIID riid, void** ppvObj )
	{return QueryInterfaceImpl( this, m_owniid, riid, ppvObj );}
	STDMETHOD_( ULONG, AddRef )( void )
	{return 1;}
	STDMETHOD_( ULONG, Release )( void )
	{return 0;}
};


class CDropTarget : public CYbInterfaceImpl<IDropTarget>
{
public:
	/*
	||  Constructors
	*/
	CDropTarget( CEditView* );
	~CDropTarget();
	/*
	||  Attributes & Operations
	*/
private: // 2002/2/10 aroka アクセス権変更
	HWND			m_hWnd_DropTarget;
	CEditView*		m_pCEditView;
//	void*			m_pCEditView;
	//	static REFIID	m_owniid;
public:
	LPDATAOBJECT	m_pDataObject;// 2002/2/10 aroka ここに移動
	BOOL			Register_DropTarget( HWND );
	BOOL			Revoke_DropTarget( void );
	STDMETHODIMP	DragEnter( LPDATAOBJECT, DWORD, POINTL , LPDWORD );
	STDMETHODIMP	DragOver( DWORD, POINTL, LPDWORD );
	STDMETHODIMP	DragLeave( void );
	STDMETHODIMP	Drop( LPDATAOBJECT, DWORD, POINTL, LPDWORD );
protected:
	/*
	||  実装ヘルパ関数
	*/
};


class CDropSource : public CYbInterfaceImpl<IDropSource> {
private:
	BOOL m_bLeft;
public:
	CDropSource( BOOL bLeft ):m_bLeft( bLeft ){}

	STDMETHOD( QueryContinueDrag )( BOOL bEscapePressed, DWORD dwKeyState );
	STDMETHOD( GiveFeedback )( DWORD dropEffect );
};


class CDataObject : public CYbInterfaceImpl<IDataObject> {
private:
	typedef struct {
		CLIPFORMAT cfFormat;
		//Feb. 26, 2001, fixed by yebisuya sugoroku
		LPBYTE			data;	//データ
		unsigned int	size;	//データサイズ。バイト単位。
	} DATA, *PDATA;

	int m_nFormat;
	PDATA m_pData;

public:
	CDataObject (LPCWSTR lpszText ):
		m_nFormat(0),
		m_pData(NULL)
	{
		SetText( lpszText );
	}
	~CDataObject(){SetText( NULL );}
	void	SetText( LPCWSTR lpszText );
	DWORD	DragDrop( BOOL bLeft, DWORD dwEffects );

	STDMETHOD( GetData )( LPFORMATETC, LPSTGMEDIUM );
	STDMETHOD( GetDataHere )( LPFORMATETC, LPSTGMEDIUM );
	STDMETHOD( QueryGetData )( LPFORMATETC );
	STDMETHOD( GetCanonicalFormatEtc )( LPFORMATETC, LPFORMATETC );
	STDMETHOD( SetData )( LPFORMATETC, LPSTGMEDIUM, BOOL );
	STDMETHOD( EnumFormatEtc )( DWORD, LPENUMFORMATETC* );
	STDMETHOD( DAdvise )( LPFORMATETC, DWORD, LPADVISESINK, LPDWORD );
	STDMETHOD( DUnadvise )( DWORD );
	STDMETHOD( EnumDAdvise )( LPENUMSTATDATA* );
};

///////////////////////////////////////////////////////////////////////
#endif /* _CEDITDROPTARGET_H_ */


/*[EOF]*/
