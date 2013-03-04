/*!	@file
	@brief MessageBox用関数

	@author Norio Nakatani

	@date 2013/03/03 Uchi Debug1.hから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_MESSAGEBOX_2D6EF6BC_3D8C_427B_8AFB_E8903838A1ED_H_
#define SAKURA_MESSAGEBOX_2D6EF6BC_3D8C_427B_8AFB_E8903838A1ED_H_

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 メッセージボックス：実装                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//2007.10.02 kobake メッセージボックスの使用はデバッグ時に限らないので、「Debug～」という名前を廃止

//テキスト整形機能付きMessageBox
SAKURA_CORE_API int VMessageBoxF( HWND hwndOwner, UINT uType, LPCTSTR lpCaption, LPCTSTR lpText, va_list& v );
SAKURA_CORE_API int MessageBoxF ( HWND hwndOwner, UINT uType, LPCTSTR lpCaption, LPCTSTR lpText, ... );


//                ユーザ用メッセージボックス                   //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//デバッグ用メッセージボックス
#define MYMESSAGEBOX MessageBoxF

//一般の警告音
#define DefaultBeep()   ::MessageBeep(MB_OK)

//エラー：赤丸に「×」[OK]
int ErrorMessage  (HWND hwnd, LPCTSTR format, ...);
//(TOPMOST)
int TopErrorMessage  (HWND hwnd, LPCTSTR format, ...);
#define ErrorBeep()     ::MessageBeep(MB_ICONSTOP)

//警告：三角に「！」[OK]
int WarningMessage   (HWND hwnd, LPCTSTR format, ...);
int TopWarningMessage(HWND hwnd, LPCTSTR format, ...);
#define WarningBeep()   ::MessageBeep(MB_ICONEXCLAMATION)

//情報：青丸に「i」[OK]
int InfoMessage   (HWND hwnd, LPCTSTR format, ...);
int TopInfoMessage(HWND hwnd, LPCTSTR format, ...);
#define InfoBeep()      ::MessageBeep(MB_ICONINFORMATION)

//確認：吹き出しの「？」 [はい][いいえ] 戻り値:IDYES,IDNO
int ConfirmMessage   (HWND hwnd, LPCTSTR format, ...);
int TopConfirmMessage(HWND hwnd, LPCTSTR format, ...);
#define ConfirmBeep()   ::MessageBeep(MB_ICONQUESTION)

//その他メッセージ表示用ボックス[OK]
int OkMessage  (HWND hwnd, LPCTSTR format, ...);
int TopOkMessage  (HWND hwnd, LPCTSTR format, ...);

//タイプ指定メッセージ表示用ボックス
int CustomMessage(HWND hwnd, UINT uType, LPCTSTR format, ...);
//(TOPMOST)
int TopCustomMessage(HWND hwnd, UINT uType, LPCTSTR format, ...);

//作者に教えて欲しいエラー
int PleaseReportToAuthor(HWND hwnd, LPCTSTR format, ...);


///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_MESSAGEBOX_2D6EF6BC_3D8C_427B_8AFB_E8903838A1ED_H_ */



