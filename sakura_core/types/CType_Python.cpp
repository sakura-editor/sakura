/*! @file
	@brief Pythonアウトライン解析

	@author genta
	@date 2007.02.24 新規作成
*/
/*
	Copyright (C) 2007, genta

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.

*/
#include "StdAfx.h"
#include "doc/CEditDoc.h"
#include "doc/CDocOutline.h"
#include "doc/logic/CDocLine.h"
#include "outline/CFuncInfoArr.h"


/*!
	関数に用いることができる文字かどうかの判定
	
	@date 2007.02.08 genta

	@note 厳密には1文字目に数字を使うことは出来ないが，
		それは実行してみれば明らかにわかることなので
		そこまで厳密にチェックしない
*/
inline bool Python_IsWordChar( wchar_t c ){
	return ( L'_' == c ||
			(L'a' <= c && c <= L'z' )||
			(L'A' <= c && c <= L'Z' )||
			(L'0' <= c && c <= L'9' )
		);
}



/*! pythonのパース状態を管理する構造体

	解析中に解析関数の間を引き渡される．
	このクラスは現在の状態と，文字列の性質を保持する．
	解析位置は解析関数間でパラメータとして渡されるので
	この中では保持しない．

	[状態遷移]
	開始 : STATE_NORMAL

	STATE_NORMAL/STATE_CONTINUE→STATE_CONTINUEの遷移
	- 継続行マーク有り

	STATE_NORMAL/STATE_CONTINUE→STATE_NORMALの遷移
	- 継続行マークがなく行末に達した
	- コメントに達した

	STATE_NORMAL→STATE_STRINGの遷移
	- 引用符あり

	STATE_STRING→STATE_NORMALの遷移
	- 規定の文字列終了記号
	- short stringで文字列の終了を示す引用符も継続行マークもなく行末に達した

	@date 2007.02.12 genta
*/
struct COutlinePython {
	enum {
		STATE_NORMAL,	//!< 通常行 : 行頭を含む
		STATE_STRING,	//!< 文字列中
		STATE_CONTINUE,	//!< 継続行 : 前の行からの続きなので行頭とはみなされない
	} m_state;
	
	int m_quote_char;	//!<	引用符記号
	bool m_raw_string;	//!<	エスケープ記号無視ならtrue
	bool m_long_string;	//!<	長い文字列中ならtrue

	COutlinePython();

	/*	各状態における文字列スキャンを行う
		Scan*が呼びだされるときは既にその状態になっていることが前提．
		ある状態から別の状態に移るところまでを扱う．
		別の状態に移る判定がややこしいばあいは，Enter*として関数にする．
	*/	
	int ScanNormal( const wchar_t* data, int linelen, int start_offset );
	int ScanString( const wchar_t* data, int linelen, int start_offset );
	int EnterString( const wchar_t* data, int linelen, int start_offset );
	void DoScanLine( const wchar_t* data, int linelen, int start_offset );
	
	bool IsLogicalLineTop(void) const { return STATE_NORMAL == m_state; }
};

void CType_Python::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("Python") );
	_tcscpy( pType->m_szTypeExts, _T("py") );

	//設定
	pType->m_cLineComment.CopyTo( 0, L"#", -1 );					/* 行コメントデリミタ */
	pType->m_eDefaultOutline = OUTLINE_PYTHON;						/* アウトライン解析方法 */
	pType->m_nKeyWordSetIdx[0] = 16;								/* キーワードセット */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;			/* 半角数値を色分け表示 */
	pType->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp = true;	//対括弧の強調をデフォルトON
	pType->m_nStringType = STRING_LITERAL_PYTHON;
	pType->m_bStringLineOnly = true; // 文字列は行内のみ
}

/*!コンストラクタ: 初期化

	初期状態をSTATE_NORMALに設定する．
*/
COutlinePython::COutlinePython()
	: m_state( STATE_NORMAL ),
	m_raw_string( false ),
	m_long_string( false )
{
}

/*! @brief Python文字列の入り口で文字列種別を決定する

	文字列の種類を適切に判別し，内部状態を設定する．
	start_offsetは開始引用符を指していること．

	- 引用符1つ: short string
	- 引用符3つ: long string
	- 引用符の前にrかRあり : raw string

	@param[in] data 対象文字列
	@param[in] linelen データの長さ
	@param[in] start_offset 調査開始位置
	
	@return 調査後の位置

	@invariant
		m_state != STATE_STRING

	@note 引用符の位置で呼びだせば，抜けた後は必ずSTATE_STRINGになっているはず．
		引用符以外の位置で呼びだした場合は何もしないで抜ける．
*/
int COutlinePython::EnterString( const wchar_t* data, int linelen, int start_offset )
{
	assert( m_state != STATE_STRING );

	int col = start_offset;
	//	文字列開始チェック
	if( data[ col ] == '\"' || data[ col ] == '\'' ){
		int quote_char = data[ col ];
		m_state = STATE_STRING;
		m_quote_char = quote_char;
		//	文字列の開始
		if( col >= 1 &&
			( data[ col - 1 ] == 'r' || data[ col - 1 ] == 'R' )){
			//	厳密には直前がSHIFT_JISの2バイト目だと誤判定する可能性があるが
			//	そういう動かないコードは相手にしない
			m_raw_string = true;
		}
		else {
			m_raw_string = false;
		}
		if( col + 2 < linelen &&
			data[ col + 1 ] == quote_char &&
			data[ col + 2 ] == quote_char ){
				m_long_string = true;
				col += 2;
		}
		else {
			m_long_string = false;
		}
		++col;
	}
	return col;
}

/*! @brief Pythonプログラムの処理

	プログラム本体部分の処理．文字列の開始，継続行，コメント，通常行末をチェックする．
	行頭判定が終わった後で引き渡されるので，関数・クラス定義は考慮しなくて良い．
	
	以下の場合に処理を終了する
	- 行末: STATE_NORMALとして処理終了
	- コメント: STATE_NORMALとして処理終了
	- 文字列の開始: EnterString() にて文字列種別の判定を行った後STATE_STRINGとして処理終了
	- 継続行: STATE_CONTINUEとして処理終了

	@param[in] data 対象文字列
	@param[in] linelen データの長さ
	@param[in] start_offset 調査開始位置
	
	@invaliant
		m_state == STATE_NORMAL || m_state == STATE_CONTINUE
	
	@return 調査後の位置
*/
int COutlinePython::ScanNormal( const wchar_t* data, int linelen, int start_offset )
{
	assert( m_state == STATE_NORMAL || m_state == STATE_CONTINUE );
	bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;

	for( int col = start_offset; col < linelen; ++col ){
		int nCharChars = CNativeW::GetSizeOfChar( data, linelen, col );
		if( 1 < nCharChars ){
			col += (nCharChars - 1);
			continue;
		}
		//	コメント
		if( data[col] == '#' ){
			//	コメントは行末と同じ扱いなので
			//	わざわざ独立して扱う必要性が薄い
			//	ここで片を付けてしまおう
			m_state = STATE_NORMAL;
			break;
		}
		//	文字列
		else if( data[col] == '\"' || data[col] == '\'' ){
			return EnterString( data, linelen, col );
		}
		else if( data[col] == '\\' ){	//	継続行かもしれない
			//	CRかCRLFかLFで行末
			//	最終行には改行コードがないことがあるが，それ以降には何もないので影響しない
			if(
				( linelen - 2 == col && 
				( data[ col + 1 ] == WCODE::CR && data[ col + 2 ] == WCODE::LF )) ||
				( linelen - 1 == col && 
				( WCODE::IsLineDelimiter(data[ col + 1 ], bExtEol) ))
			){
				m_state = STATE_CONTINUE;
				break;
			}
		}
	}
	return linelen;
}


/*! @brief python文字列(1行)を調査する

	与えられた状態からPython文字列の状態変化を追い，
	最終的な状態を決定する．
	
	文字列の開始判定はEnterString()関数で処理済みであり，その結果が
	m_state, m_raw_string, m_long_string, m_quote_charに与えられている．
	
	m_raw_stringがtrueならbackslashによるエスケープ処理を行わない
	m_long_stringならm_quote_charが3つ続くまで文字列となる．

	@param[in] data 対象文字列
	@param[in] linelen データの長さ
	@param[in] start_offset 調査開始位置
	
	@return 調査後の位置
	
	@invariant
		m_state==STATE_STRING

	@author genta
	@date 2007.02.12 新規作成
	@date 2007.03.23 genta 文字列の継続行の処理を追加

*/
int COutlinePython::ScanString( const wchar_t* data, int linelen, int start_offset )
{
	assert( m_state == STATE_STRING );
	bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;

	int quote_char = m_quote_char;
	for( int col = start_offset; col < linelen; ++col ){
		int nCharChars = CNativeW::GetSizeOfChar( data, linelen, col );
		if( 1 < nCharChars ){
			col += (nCharChars - 1);
			continue;
		}
		//	rawモード以外ではエスケープをチェック
		//	rawモードでも継続行はチェック
		if( data[ col ] == '\\' && col + 1 < linelen ){
			wchar_t key = data[ col + 1 ];
			if( ! m_raw_string ){
				if( key == L'\\' ||
					key == L'\"' ||
					key == L'\'' ){
					++col;
					//	ignore
					continue;
				}
			}
			if( WCODE::IsLineDelimiter(key, bExtEol) ){
				// \r\nをまとめて\nとして扱う必要がある
				if( col + 1 >= linelen ||
					data[ col + 2 ] == key ){
					// 本当に行末
					++col;
					continue;
				}
				else if( data[ col + 2 ] == WCODE::LF ){
					col += 2;	//	 CRLF
				}
			}
		}
		//	short string + 改行の場合はエラーから強制復帰
		else if( WCODE::IsLineDelimiter(data[ col ], bExtEol) ){
			//あとで
			if( ! m_long_string ){
				//	文字列の末尾を発見した
				m_state = STATE_NORMAL;
				return col + 1;
			}
		}
		//	引用符が見つかったら終了チェック
		else if( data[ col ] == quote_char ){
			if( ! m_long_string ){
				//	文字列の末尾を発見した
				m_state = STATE_NORMAL;
				return col + 1;
			}
			//	long stringの場合
			if( col + 2 < linelen &&
				data[ col + 1 ] == quote_char &&
				data[ col + 2 ] == quote_char ){
				m_state = STATE_NORMAL;
				return col + 3;
			}
		}
	}
	return linelen;
}

/*!	Python文字列を行末までスキャンして次の行の状態を決定する

	m_stateに設定された現在の状態から開始してdataをstart_offsetからlinelenに達するまで
	走査し，行末における状態をm_stateに格納する．

	現在の状態に応じてサブルーチンに解析処理を依頼する．
	サブルーチンScan**では文字列dataのstart_offsetから状態遷移が発生するまで処理を
	続け，別の状態に遷移した直後に処理済みの桁位置を返して終了する．

	この関数に戻った後は再度現在の状態に応じて処理依頼を行う．これを行末に達するまで繰り返す．

	@param[in] data 対象文字列
	@param[in] linelen データの長さ
	@param[in] start_offset 調査開始位置

*/
void COutlinePython::DoScanLine( const wchar_t* data, int linelen, int start_offset )
{
	int col = start_offset;
	while( col < linelen ){
		if( STATE_NORMAL == m_state || STATE_CONTINUE == m_state){
			col = ScanNormal( data, linelen, col );
		}
		else if( STATE_STRING == m_state ){
			col = ScanString( data, linelen, col );
		}
		else {
			//	ありえないエラー
			return;
		}
	}
}


/*!	@brief python関数リスト作成

	class, def で始まる行から名前を抜き出す．
	
	class CLASS_NAME( superclass ):
	def FUNCTION_NAME( parameters ):

	文字列とコメントを除外する必要がある．

	通常の行頭の場合に関数・クラス判定と登録処理を行う．
	Python特有の空白の数を数えてネストレベルを判定する．
	indent_levelを配列として用いており，インデントレベルごとのスペース数を格納する．
	なお，TABは8桁区切りへの移動と解釈することになっている．
	
	通常の行頭でない(文字列中および継続行)，あるいは行頭の処理完了後は
	状態機械 python_analyze_state に判定処理を依頼する．

	@par 文字列
	'' "" 両方OK
	引用符3連続でロング文字列
	直前にrかRがあったらエスケープ記号を無視(ただし改行のエスケープは有効)
	
	@par コメント
	#で始まり，行の継続はない．

	@date 2007.02.08 genta 新規作成
*/
void CDocOutline::MakeFuncList_python( CFuncInfoArr* pcFuncInfoArr )
{
	CLogicInt	nLineLen;
	CLogicInt	nLineCount;

	COutlinePython python_analyze_state;

	const int MAX_DEPTH = 10;
	bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;

	int indent_level[ MAX_DEPTH ]; // 各レベルのインデント桁位置()
	indent_level[0] = 0;	// do as python does.
	int depth_index = 0;

	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		const wchar_t*	pLine;
		int depth;	//	indent depth
		CLogicInt col = CLogicInt(0);	//	current working column position

		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		
		if( python_analyze_state.IsLogicalLineTop() ){
			//	indent check
			//	May 15, 2007 genta 桁位置colはデータオフセットdと独立にしないと
			//	文字列比較がおかしくなる
			for( depth = 0, col = CLogicInt(0); col < nLineLen; ++col ){
				//	calculate indent level
				if( pLine[col] == L' ' ){
					++depth;
				}
				else if( pLine[col] == L'\t' ){
					depth = ( depth + 8 ) & ~7;
				}
				else {
					break;
				}
			}
			if( WCODE::IsLineDelimiter(pLine[col], bExtEol) ||
				pLine[col] == L'\0' ||
				pLine[col] == L'#' ){
				//	blank line or comment line are ignored
				continue;
			}
			
			int nItemFuncId = 0;	// topic type
			if( nLineLen - col > CLogicInt(3 + 1) && wcsncmp_literal( pLine + col, L"def" ) == 0 ){
				//	"def"
				nItemFuncId = 1;
				col += CLogicInt(3); // strlen( def )
			}
			else if( nLineLen - col > CLogicInt(5 + 1) && wcsncmp_literal( pLine + col, L"class" ) == 0 ){
				// class
				nItemFuncId = 4;
				col += CLogicInt(5); // strlen( class )
			}
			else {
				python_analyze_state.DoScanLine( pLine, nLineLen, col );
				continue;
			}

			//	2006.02.28 genta 区切りチェック
			//	define, classic等が対象にならないように，後ろにスペースかタブが
			//	続くことを確認．
			//	本当は継続行として次の行に関数名を書くことも文法上は可能だが
			//	複雑になるので対応しない．
			int c = pLine[col];
			if(  c != L' ' && c != L'\t' ){
				python_analyze_state.DoScanLine( pLine, nLineLen, col );
				continue;
			}

			//	adjust current depth level
			//	関数内部の実行文のさらに奧に関数があるケースを考慮
			//	def/class以外のインデントは記録しない方がいいので
			//	見出し行と確定してからインデントレベルの判定を行う
			int i;
			for( i = depth_index; i >= 0; --i ){
				if( depth == indent_level[ i ] ){
					depth_index = i;
					break;
				}
				else if( depth > indent_level[ i ] && i < MAX_DEPTH - 1 ){
					depth_index = i + 1;
					indent_level[ depth_index ] = depth;
					break;
				}
			}

			//	2007.02.08 genta 手抜きコメント
			//	厳密には，ここで継続行を入れることが可能だが，
			//	そんなレアなケースは考慮しない
			
			//	skip whitespace
			while( col < nLineLen && C_IsSpace( pLine[col], bExtEol ))
				++col;

			int w_end;
			for( w_end = col; w_end < nLineLen
					&& Python_IsWordChar( pLine[w_end] ); ++w_end )
				;
			
			//	2007.02.08 genta 手抜きコメント
			//	厳密には，この後に括弧に囲まれた引数あるいは継承元クラスが
			//	括弧に囲まれて入り，さらに:と続くが
			//	継続行の可能性があるので，そこまでチェックしない
			
			//	ここまでで登録要件OKとみなす
			
			//	このあたりは暫定

			wchar_t szWord[512];	// 適当に大きな数(pythonでは名前の長さの上限があるのか？)
			int len = w_end - col;
			
			if( len > 0 ){
				if( len > _countof( szWord ) - 1){
					len = _countof( szWord ) - 1;
				}
				wcsncpy( szWord, pLine + col, len );
				szWord[ len ] = L'\0';
			}
			else {
				wcscpy( szWord, LSW(STR_OUTLINE_PYTHON_UNDEFINED) );
				len = 8;
			}
			if( nItemFuncId == 4  ){
				if( _countof( szWord ) - 8  < len ){
					//	後ろを削って入れる
					len = _countof( szWord ) - 8;
				}
				// class
				wcscpy( szWord + len, LSW(STR_OUTLINE_PYTHON_CLASS) );
			}
			
			/*
			  カーソル位置変換
			  物理位置(行頭からのバイト数、折り返し無し行位置)
			  →
			  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/
			CLayoutPoint ptPosXY;
			m_pcDocRef->m_cLayoutMgr.LogicToLayout(
				CLogicPoint(CLogicInt(0), nLineCount),
				&ptPosXY
			);
			pcFuncInfoArr->AppendData(
				nLineCount + CLogicInt(1),
				ptPosXY.GetY2() + CLayoutInt(1),
				szWord,
				nItemFuncId,
				depth_index
			);
			col = CLogicInt(w_end); // クラス・関数定義の続きはここから
		}
		python_analyze_state.DoScanLine( pLine, nLineLen, col );
	}
}


const wchar_t* g_ppszKeywordsPython[] = {
	L"False",
	L"None",
	L"True",
	L"and",
	L"as",
	L"assert",
	L"async",
	L"await",
	L"break",
	L"class",
	L"continue",
	L"def",
	L"del",
	L"elif",
	L"else",
	L"except",
	L"finally",
	L"for",
	L"from",
	L"global",
	L"if",
	L"import",
	L"in",
	L"is",
	L"lambda",
	L"nonlocal",
	L"not",
	L"or",
	L"pass",
	L"raise",
	L"return",
	L"try",
	L"while",
	L"with",
	L"yield",
	L"self",
};
int g_nKeywordsPython = _countof(g_ppszKeywordsPython);
