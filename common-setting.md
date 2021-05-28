# `共通設定` の仕様まとめ

## `全般` タブ

|GROUPBOX|設定項目|タイトル|ID|コントロールタイプ|
|--|--|--|--|--|
|カーソル|→|フリーカーソル|IDC_CHECK_FREECARET|CHECKBOX|
|カーソル|→|単語単位で移動するときに単語の両端に止まる|IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_WORD|CHECKBOX|
|カーソル|→|段落単位で移動するときに段落の両端に止まる|IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_PARAGRAPH|CHECKBOX|
|カーソル|→|マウスクリックでのアクティブ化ではカーソルを移動しない|IDC_CHECK_NOMOVE_ACTIVATE_BY_MOUSE|CHECKBOX|
|カーソル形状|カーソル形状|Windows風|IDC_RADIO_CARETTYPE0|RADIOBUTTON|
|カーソル形状|カーソル形状|MS-DOS風|IDC_RADIO_CARETTYPE1|RADIOBUTTON|
|タスクトレイ|→|タスクトレイを使う|IDC_CHECK_USETRAYICON|CHECKBOX|
|タスクトレイ|→|タスクトレイに常駐|IDC_CHECK_STAYTASKTRAY|CHECKBOX|
|タスクトレイ|左クリックメニューのショートカットキー|左クリックメニューのショートカットキー|IDC_STATIC|LTEXT|
|タスクトレイ|左クリックメニューのショートカットキー|HotKey1|IDC_HOTKEY_TRAYMENU|msctls_hotkey32|
|-|→|同時に複数の編集用ウィンドウを閉じるとき確認|IDC_CHECK_CLOSEALLCONFIRM|CHECKBOX|
|-|→|サクラエディタの全終了で編集用 ウィンドウを閉じるとき確認|IDC_CHECK_EXITCONFIRM|CHECKBOX|
|スクロール|スクロール行数|行数|IDC_STATIC|LTEXT|
|スクロール|スクロール行数|-|IDC_EDIT_REPEATEDSCROLLLINENUM|EDITTEXT|
|スクロール|スクロール行数|-|IDC_SPIN_REPEATEDSCROLLLINENUM|msctls_updown32|
|スクロール|→|少し滑らかにする|IDC_CHECK_REPEATEDSCROLLSMOOTH|CHECKBOX|
|スクロール|組み合わせてホイール操作した時ページスクロールする|組み合わせてホイール操作した時ページスクロールする|IDC_STATIC|LTEXT|
|スクロール|組み合わせてホイール操作した時ページスクロールする|-|IDC_COMBO_WHEEL_PAGESCROLL|COMBOBOX|
|スクロール|組み合わせてホイール操作した時横スクロールする|組み合わせてホイール操作した時横スクロールする|IDC_STATIC|LTEXT|
|スクロール|組み合わせてホイール操作した時横スクロールする|-|IDC_COMBO_WHEEL_HSCROLL|COMBOBOX|
|スクロール|→|画面キャッシュを使う|IDC_CHECK_MEMDC|CHECKBOX|
|履歴|ファイルの履歴MAX|ファイルの履歴MAX|IDC_STATIC|LTEXT|
|履歴|ファイルの履歴MAX|-|IDC_EDIT_MAX_MRU_FILE|EDITTEXT|
|履歴|ファイルの履歴MAX|-|IDC_SPIN_MAX_MRU_FILE|msctls_updown32|
|履歴|ファイルの履歴MAX|履歴をクリア|IDC_BUTTON_CLEAR_MRU_FILE|PUSHBUTTON|
|履歴|フォルダの履歴MAX|フォルダの履歴MAX|IDC_STATIC|LTEXT|
|履歴|フォルダの履歴MAX|-|IDC_EDIT_MAX_MRU_FOLDER|EDITTEXT|
|履歴|フォルダの履歴MAX|-|IDC_SPIN_MAX_MRU_FOLDER|msctls_updown32|
|履歴|フォルダの履歴MAX|履歴をクリア|IDC_BUTTON_CLEAR_MRU_FOLDER|PUSHBUTTON|
