#include "stdafx.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       変換テーブル                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//全角カナ
static const wchar_t tableZenkata_Normal[]=
	L"アイウエオカキクケコサシスセソタチツテトナニヌネノ"	//普通
	L"ハヒフヘホマミムメモヤユヨラリルレロワヲン"			//      ※「ヰ」「ヱ」は除外。(半角が存在しないので)
	L"ァィゥェォッャュョ"									//小    ※「ヵ」「ヶ」「ヮ」は除外。(半角が存在しないので)
;
static const wchar_t tableZenkata_Dakuten[]=
	L"ヴガギグゲゴザジズゼゾダヂヅデド"						//濁点
	L"バビブベボ" L"\u30f7\u30fa"							//※後半2文字は「ワ゛」「ヲ゛」  ※「ヰ゛」(\u30f8)「ヱ゛」(\u30f9)は除外。(半角が存在しないので)
;
static const wchar_t tableZenkata_HanDakuten[]=
	L"パピプペポ"											//半濁点
;
static const wchar_t tableZenkata_Kigo[]=
	L"ー゛゜。、「」・"										//記号
;

//半角カナ
static const wchar_t tableHankata_Normal[]=
	L"ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉ"
	L"ﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜｦﾝ"
	L"ｧｨｩｪｫｯｬｭｮ"
;
static const wchar_t tableHankata_Dakuten[]=
	L"ｳｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄ"
	L"ﾊﾋﾌﾍﾎ" L"ﾜｦ"
;
static const wchar_t tableHankata_HanDakuten[]=
	L"ﾊﾋﾌﾍﾎ"
;
static const wchar_t tableHankata_Kigo[] =
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
inline wchar_t ZenhiraToZenkata_(wchar_t c){ return ((c>=L'ぁ' && c<=L'\u3096') || (c>=L'ゝ' && c<=L'ゞ'))? L'ァ'+(c-L'ぁ'): c; }
void Convert_ZenhiraToZenkata(wchar_t* pData, int nLength)
{
	wchar_t* p=pData;
	wchar_t* q=p+nLength;
	while(p<q){
		*p = ZenhiraToZenkata_(*p);
		p++;
	}
}

//! 全角カタカナ→全角ひらがな (文字数は不変)
inline wchar_t ZenkataToZenhira_(wchar_t c){ return ((c>=L'ァ' && c<=L'ヶ') || (c>=L'ヽ' && c<=L'ヾ'))? L'ぁ'+(c-L'ァ'): c; }
void Convert_ZenkataToZenhira(wchar_t* pData, int nLength)
{
	wchar_t* p=pData;
	wchar_t* q=p+nLength;
	while(p<q){
		*p = ZenkataToZenhira_(*p);
		p++;
	}
}


//! 全角英数→半角英数 (文字数は不変)
inline wchar_t ZeneisuToHaneisu_(wchar_t c)
{
	int n;

	if(0){}
	else if(c>=L'Ａ' && c<=L'Ｚ'){ c=L'A'+(c-L'Ａ'); }
	else if(c>=L'ａ' && c<=L'ｚ'){ c=L'a'+(c-L'ａ'); }
	else if(c>=L'０' && c<=L'９'){ c=L'0'+(c-L'０'); }
	//一部の記号も変換する
	else if(wcschr_idx(tableZenKigo,c,&n)){ c=tableHanKigo[n]; }

	return c;
}
void Convert_ZeneisuToHaneisu(wchar_t* pData, int nLength)
{
	wchar_t* p=pData;
	wchar_t* q=p+nLength;
	while(p<q){
		*p = ZeneisuToHaneisu_(*p);
		p++;
	}
}


//! 半角英数→全角英数 (文字数は不変)
inline wchar_t HaneisuToZeneisu_(wchar_t c)
{
	int n;

	if(0){}
	else if(c>=L'A' && c<=L'Z'){ c=L'Ａ'+(c-L'A'); }
	else if(c>=L'a' && c<=L'z'){ c=L'ａ'+(c-L'a'); }
	else if(c>=L'0' && c<=L'9'){ c=L'０'+(c-L'0'); }
	//一部の記号も変換する
	else if(wcschr_idx(tableHanKigo,c,&n)){ c=tableZenKigo[n]; }

	return c;
}
void Convert_HaneisuToZeneisu(wchar_t* pData, int nLength)
{
	wchar_t* p=pData;
	wchar_t* q=p+nLength;
	while(p<q){
		*p = HaneisuToZeneisu_(*p);
		p++;
	}
}


/*!
	全角カタカナ→半角カタカナ
	濁点の分だけ、文字数は増える可能性がある。最大で2倍になる。
	pDstにはあらかじめ十分なメモリを確保しておくこと。
*/
void Convert_ZenkataToHankata(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst, int* nDstLength)
{
	const wchar_t* src = pSrc;
	const wchar_t* src_end = src + nSrcLength;
	wchar_t* dst = pDst;
	int n;

	while(src<src_end){
		wchar_t c=*src;
		if(0){}
		//ヒットする文字があれば変換を行う
		else if(wcschr_idx(tableZenkata_Normal    ,c,&n)){ *dst++=tableHankata_Normal[n];                  }
		else if(wcschr_idx(tableZenkata_Dakuten   ,c,&n)){ *dst++=tableHankata_Dakuten[n];    *dst++=L'ﾞ'; }
		else if(wcschr_idx(tableZenkata_HanDakuten,c,&n)){ *dst++=tableHankata_HanDakuten[n]; *dst++=L'ﾟ'; }
		else if(wcschr_idx(tableZenkata_Kigo      ,c,&n)){ *dst++=tableHankata_Kigo[n];                    }
		//無変換
		else { *dst++=c; }
		src++;
	}
	*dst=L'\0';
	*nDstLength = dst - pDst;
}

/*!
	全角→半角
	濁点の分だけ、文字数は増える可能性がある。最大で2倍になる。
	pDstにはあらかじめ十分なメモリを確保しておくこと。
*/
void Convert_ToHankaku(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst, int* nDstLength)
{
	const wchar_t* src = pSrc;
	const wchar_t* src_end = src + nSrcLength;
	wchar_t* dst = pDst;
	int n;

	while(src<src_end){
		wchar_t c=*src;
		//全角英数を半角英数に変換する
		wchar_t d = ZeneisuToHaneisu_(c);
		if(d != c){ *dst++ = d; }
		else {
			//小さい「か」「け」や「ゝ」「ゞ」は全角カタカナ（「ヵ」「ヶ」「ヽ」「ヾ」）には変換できても半角カタカナまでは変換できないので無変換
			if(c>=L'\u3095' && c<=L'\u309F'){ *dst++ = c; }
			else{
				//全角ひらがなを全角カタカナにしてから半角カタカナに変換する
				c = ZenhiraToZenkata_(c);
				if(0){}
				//ヒットする文字があれば変換を行う
				else if(wcschr_idx(tableZenkata_Normal    ,c,&n)){ *dst++=tableHankata_Normal[n];                  }
				else if(wcschr_idx(tableZenkata_Dakuten   ,c,&n)){ *dst++=tableHankata_Dakuten[n];    *dst++=L'ﾞ'; }
				else if(wcschr_idx(tableZenkata_HanDakuten,c,&n)){ *dst++=tableHankata_HanDakuten[n]; *dst++=L'ﾟ'; }
				else if(wcschr_idx(tableZenkata_Kigo      ,c,&n)){ *dst++=tableHankata_Kigo[n];                    }
				//無変換
				else { *dst++=c; }
			}
		}
		src++;
	}
	*dst=L'\0';
	*nDstLength = dst - pDst;
}

/*!
	半角カタカナ→全角カタカナ
	濁点の分だけ、文字数は減る可能性がある。最小で2分の1になる。
	pDstにはあらかじめ十分なメモリを確保しておくこと。
*/
void Convert_HankataToZenkata(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst, int* nDstLength)
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
		else if(next==L'ﾞ' && wcschr_idx(tableHankata_Dakuten   ,c,&n)){ *dst++=tableZenkata_Dakuten[n];    src++; }
		else if(next==L'ﾟ' && wcschr_idx(tableHankata_HanDakuten,c,&n)){ *dst++=tableZenkata_HanDakuten[n]; src++; }
		//それ以外の文字チェックを行う
		else if(              wcschr_idx(tableHankata_Normal    ,c,&n)){ *dst++=tableZenkata_Normal[n];            }
		else if(              wcschr_idx(tableHankata_Kigo      ,c,&n)){ *dst++=tableZenkata_Kigo[n];              }
		//無変換
		else { *dst++=c; }
		src++;
	}
	*dst=L'\0';
	*nDstLength = dst - pDst;
}

/*!
	半角カタカナ→全角ひらがな
	濁点の分だけ、文字数は減る可能性がある。最小で2分の1になる。
	pDstにはあらかじめ十分なメモリを確保しておくこと。
*/
void Convert_HankataToZenhira(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst, int* nDstLength)
{
	const wchar_t* src = pSrc;
	const wchar_t* src_end = src + nSrcLength;
	wchar_t* dst = pDst;
	int n;

	while(src<src_end){
		wchar_t c=*src;
		wchar_t next=(src+1<src_end)?*(src+1):0; //次の1文字を先読み
		bool hit = true;	//半角カタカナ→全角カタカナ変換を実施したかどうかを示すフラグ
		if(0){}
		//濁点、半濁点のチェックを先行して行う
		//※「ﾜﾞ」「ｦﾞ」は１字の全角カタカナには変換できても全角ひらがなまでは変換できないので濁点を切り離して変換
		else if(              wcschr_idx(L"ﾜｦ"                  ,c,&n)){ *dst++=L"ワヲ"[n];            }
		else if(next==L'ﾞ' && wcschr_idx(tableHankata_Dakuten   ,c,&n)){ *dst++=tableZenkata_Dakuten[n];    src++; }
		else if(next==L'ﾟ' && wcschr_idx(tableHankata_HanDakuten,c,&n)){ *dst++=tableZenkata_HanDakuten[n]; src++; }
		//それ以外の文字チェックを行う
		else if(              wcschr_idx(tableHankata_Normal    ,c,&n)){ *dst++=tableZenkata_Normal[n];            }
		else if(              wcschr_idx(tableHankata_Kigo      ,c,&n)){ *dst++=tableZenkata_Kigo[n];              }
		//無変換
		else { *dst++=c; hit = false; }
		if(hit){ *(dst-1)=ZenkataToZenhira_(*(dst-1)); }	//半角カタカナから変換した全角カタカナを全角ひらがなに変換（※もともと全角だったカタカナは無変換）
		src++;
	}
	*dst=L'\0';
	*nDstLength = dst - pDst;
}
