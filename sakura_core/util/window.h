#pragma once

SAKURA_CORE_API void ActivateFrameWindow( HWND );	/* アクティブにする */

/*
||	処理中のユーザー操作を可能にする
||	ブロッキングフック(?)(メッセージ配送)
*/
SAKURA_CORE_API BOOL BlockingHook( HWND hwndDlgCancel );


#ifndef GA_PARENT
#define GA_PARENT		1
#define GA_ROOT			2
#define GA_ROOTOWNER	3
#endif
#define GA_ROOTOWNER2	100


HWND MyGetAncestor( HWND hWnd, UINT gaFlags );	// 指定したウィンドウの祖先のハンドルを取得する	// 2007.07.01 ryoji
