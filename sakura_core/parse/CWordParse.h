//2007.09.30 kobake CDocLineMgr から分離

#pragma once

//! 文字種類識別子
enum ECharKind{
	CK_NULL,			//!< NULL
	CK_TAB,				//!< タブ 0x9<=c<=0x9
	CK_CR,				//!< CR = 0x0d 
	CK_LF,				//!< LF = 0x0a 

	CK_SPACE,			//!< 半角のスペース 0x20<=c<=0x20
	CK_CSYM,			//!< 識別子に使用可能な文字 (英数字、アンダースコア)
	CK_KATA,			//!< 半角のカタカナ 0xA1<=c<=0xFD
	CK_ETC,				//!< 半角のその他

	CK_ZEN_SPACE,		//!< 全角スペース
	CK_ZEN_NOBASU,		//!< 伸ばす記号 0x815B<=c<=0x815B 'ー'
	CK_ZEN_CSYM,		//!< 全角版、識別子に使用可能な文字 (英数字、アンダースコア)

	CK_ZEN_KIGO,		//!< 全角の記号
	CK_HIRA,			//!< ひらがな
	CK_ZEN_KATA,		//!< 全角カタカナ
	CK_GREEK,			//!< ギリシャ文字
	CK_ZEN_ROS,			//!< ロシア文字:
	CK_ZEN_SKIGO,		//!< 全角の特殊記号
	CK_ZEN_ETC,			//!< 全角のその他（漢字など）
};

class CWordParse{
public:
	//2001.06.23 N.Nakatani
	//2007.09.30 kobake     CDocLineMgrから移動
	/*!
		@brief 現在位置の単語の範囲を調べる staticメンバ
		@author N.Nakatani
		@retval true	成功 現在位置のデータは「単語」と認識する。
		@retval false	失敗 現在位置のデータは「単語」とは言いきれない気がする。
	*/
	static bool WhereCurrentWord_2(
		const wchar_t*	pLine,			//[in]  調べるメモリ全体の先頭アドレス
		CLogicInt		nLineLen,		//[in]  調べるメモリ全体の有効長
		CLogicInt		nIdx,			//[out] 調査開始地点:pLineからの相対的な位置
		CLogicInt*		pnIdxFrom,		//[out] 単語が見つかった場合は、単語の先頭インデックスを返す。
		CLogicInt*		pnIdxTo,		//[out] 単語が見つかった場合は、単語の終端の次のバイトの先頭インデックスを返す。
		CNativeW*		pcmcmWord,		//[out] 単語が見つかった場合は、現在単語を切り出して指定されたCMemoryオブジェクトに格納する。情報が不要な場合はNULLを指定する。
		CNativeW*		pcmcmWordLeft	//[out] 単語が見つかった場合は、現在単語の左に位置する単語を切り出して指定されたCMemoryオブジェクトに格納する。情報が不要な場合はNULLを指定する。
	);

	//! 現在位置の文字の種類を調べる
	static ECharKind WhatKindOfChar(
		const wchar_t*	pData,
		int				pDataLen,
		int				nIdx
	);

	//	pLine（長さ：nLineLen）の文字列から次の単語を探す。探し始める位置はnIdxで指定。
	static bool SearchNextWordPosition(
		const wchar_t*	pLine,
		CLogicInt		nLineLen,
		CLogicInt		nIdx,		//	桁数
		CLogicInt*		pnColmNew,	//	見つかった位置
		BOOL			bStopsBothEnds	//	単語の両端で止まる
	);
};

SAKURA_CORE_API BOOL IsURL( const wchar_t*, int, int* );/* 指定アドレスがURLの先頭ならばTRUEとその長さを返す */
SAKURA_CORE_API BOOL IsMailAddress( const wchar_t*, int, int* );	/* 現在位置がメールアドレスならば、NULL以外と、その長さを返す */
