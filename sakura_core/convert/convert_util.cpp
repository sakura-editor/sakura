#include "stdafx.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       変換テーブル                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//全角カナ
static const wchar_t tableZenKana_Normal[]=
	L"アイウエオカキクケコサシスセソタチツテトナニヌネノ"	//普通
	L"ハヒフヘホマミムメモヤユヨラリルレロワヲン"			//      ※「ヰ」「ヱ」は除外。(半角が存在しないので)
	L"ァィゥェォッャュョ"									//小    ※「ヵ」「ヶ」「ヮ」は除外。(半角が存在しないので)
;
static const wchar_t tableZenKana_Dakuten[]=
	L"ヴガギグゲゴザジズゼゾダヂヅデド"						//濁点
	L"バビブベボ" L"\u30f7\u30fa"							//※後半2文字は「ワ゛」「ヲ゛」  ※「ヰ゛」(\u30f8)「ヱ゛」(\u30f9)は除外。(半角が存在しないので)
;
static const wchar_t tableZenKana_HanDakuten[]=
	L"パピプペポ"											//半濁点
;
static const wchar_t tableZenKana_Kigo[]=
	L"ー゛゜。、「」・"										//記号
;

//半角カナ
static const wchar_t tableHanKana_Normal[]=
	L"ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉ"
	L"ﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜｦﾝ"
	L"ｧｨｩｪｫｯｬｭｮ"
;
static const wchar_t tableHanKana_Dakuten[]=
	L"ｳｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄ"
	L"ﾊﾋﾌﾍﾎ" L"ﾜｦ"
;
static const wchar_t tableHanKana_HanDakuten[]=
	L"ﾊﾋﾌﾍﾎ"
;
static const wchar_t tableHanKana_Kigo[] =
	L"ｰﾞﾟ｡､｢｣･"
;

//全角英記号。文字の並びに深い意味はありません。バックスラッシュは無視。
static const wchar_t tableZenKigo[] =
	L"　，．"
	L"＋－＊／％＝｜＆"
	L"＾￥＠；："
	L"”‘’＜＞（）｛｝［］"
	L"！？＃＄￣＿"
;

//半角英記号
static const wchar_t tableHanKigo[] =
	L" ,."
	L"+-*/%=|&"
	L"^\\@;:"
	L"\"`'<>(){}[]"
	L"!?#$~_"
;

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	wcschr の結果をインデックスで受け取る版。
	c が見つかったときは、その位置を idx に格納し、true を返す。
	見つからないときは idx を変更せずに false を返す。
*/
bool wcschr_idx(const wchar_t* str, wchar_t c, int* idx)
{
	const wchar_t* p=wcschr(str,c);
	if(p)*idx=(int)(p-str);
	return p!=NULL;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         変換実装                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //



//! 全角ひらがな→全角カタカナ (文字数は不変)
void Convert_ZenhiraToZenkana(wchar_t* pData, int nLength)
{
	wchar_t* p=pData;
	wchar_t* q=p+nLength;
	while(p<q){
		wchar_t& c=*p;

		if(c>=L'ぁ' && c<=L'\u3096'){ c=L'ァ'+(c-L'ぁ'); } //※\3096は「ヶ」のひらがな版。

		p++;
	}
}

//! 全角カタカナ→全角ひらがな (文字数は不変)
void Convert_ZenkanaToZenhira(wchar_t* pData, int nLength)
{
	wchar_t* p=pData;
	wchar_t* q=p+nLength;
	while(p<q){
		wchar_t& c=*p;

		if(c>=L'ァ' && c<=L'ヶ'){ c=L'ぁ'+(c-L'ァ'); }

		p++;
	}
}


//! 全角英数→半角英数 (文字数は不変)
void Convert_ZenEisuu_To_HanEisuu(wchar_t* pData, int nLength)
{
	wchar_t* p=pData;
	wchar_t* q=p+nLength;
	int n;
	while(p<q){
		wchar_t& c=*p;

		if(0){}
		else if(c>=L'Ａ' && c<=L'Ｚ'){ c=L'A'+(c-L'Ａ'); }
		else if(c>=L'ａ' && c<=L'ｚ'){ c=L'a'+(c-L'ａ'); }
		else if(c>=L'０' && c<=L'９'){ c=L'0'+(c-L'０'); }
		//一部の記号も変換する
		else if(wcschr_idx(tableZenKigo,c,&n)){ c=tableHanKigo[n]; }

		p++;
	}
}


//! 半角英数→全角英数 (文字数は不変)
void Convert_HanEisuu_To_ZenEisuu(wchar_t* pData, int nLength)
{
	wchar_t* p=pData;
	wchar_t* q=p+nLength;
	int n;
	while(p<q){
		wchar_t& c=*p;

		if(0){}
		else if(c>=L'A' && c<=L'Z'){ c=L'Ａ'+(c-L'A'); }
		else if(c>=L'a' && c<=L'z'){ c=L'ａ'+(c-L'a'); }
		else if(c>=L'0' && c<=L'9'){ c=L'０'+(c-L'0'); }
		//一部の記号も変換する
		else if(wcschr_idx(tableHanKigo,c,&n)){ c=tableZenKigo[n]; }

		p++;
	}
}


/*!
	全角カタカナ→半角カタカナ
	濁点の分だけ、文字数は増える可能性がある。最大で2倍になる。
	pDstにはあらかじめ十分なメモリを確保しておくこと。
*/
void Convert_ZenKana_To_HanKana(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst)
{
	const wchar_t* src = pSrc;
	const wchar_t* src_end = src + nSrcLength;
	wchar_t* dst = pDst;
	int n;

	while(src<src_end){
		wchar_t c=*src;
		if(0){}
		//ヒットする文字があれば変換を行う
		else if(wcschr_idx(tableZenKana_Normal    ,c,&n)){ *dst++=tableHanKana_Normal[n];                  }
		else if(wcschr_idx(tableZenKana_Dakuten   ,c,&n)){ *dst++=tableHanKana_Dakuten[n];    *dst++=L'ﾞ'; }
		else if(wcschr_idx(tableZenKana_HanDakuten,c,&n)){ *dst++=tableHanKana_HanDakuten[n]; *dst++=L'ﾟ'; }
		else if(wcschr_idx(tableZenKana_Kigo      ,c,&n)){ *dst++=tableHanKana_Kigo[n];                    }
		//無変換
		else { *dst++=c; }
		src++;
	}
	*dst=L'\0';
}

/*!
	半角カタカナ→全角カタカナ
	濁点の分だけ、文字数は減る可能性がある。最小で2分の1になる。
	pDstにはあらかじめ十分なメモリを確保しておくこと。
*/
void Convert_HankanaToZenkana(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst)
{
	const wchar_t* src = pSrc;
	const wchar_t* src_end = src + nSrcLength;
	wchar_t* dst = pDst;
	int n;

	while(src<src_end){
		wchar_t c=*src;
		wchar_t next=(src+1<src_end)?*(src+1):0; //次の1文字を先読み
		if(0){}
		//濁点、半濁点のチェックを先行して行う
		else if(next==L'ﾞ' && wcschr_idx(tableHanKana_Dakuten   ,c,&n)){ *dst++=tableZenKana_Dakuten[n];    src++; }
		else if(next==L'ﾟ' && wcschr_idx(tableHanKana_HanDakuten,c,&n)){ *dst++=tableZenKana_HanDakuten[n]; src++; }
		//それ以外の文字チェックを行う
		else if(              wcschr_idx(tableHanKana_Normal    ,c,&n)){ *dst++=tableZenKana_Normal[n];            }
		else if(              wcschr_idx(tableHanKana_Kigo      ,c,&n)){ *dst++=tableZenKana_Kigo[n];              }
		//無変換
		else { *dst++=c; }
		src++;
	}
	*dst=L'\0';
}
