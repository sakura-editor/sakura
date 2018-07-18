#include "StdAfx.h"
#include "CConvert_ToUpper.h"

// 大文字へ変換
//	変換できないキリル文字ёの修正	2010/6/5 Uchi
//	ラテンアルファベットの拡張対応(ラテン補助、拡張AB、拡張追加)	2010/6/5 Uchi
bool CConvert_ToUpper::DoConvert(CNativeW* pcData)
{
	WCHAR* p = pcData->GetStringPtr();
	WCHAR* end = p + pcData->GetStringLength();
	while(p < end){
		WCHAR& c=*p++;
		// a-z → A-Z
		if(c>=0x0061 && c<=0x007A){
			c=0x0041+(c-0x0061);
		}
		// ａ-ｚ → Ａ-Ｚ
		else if( c>=0xFF41 && c<=0xFF5A){
			c=0xFF21+(c-0xFF41);
		}
		// ギリシャ文字変換
		else if( c>=0x03B1 && c<=0x03C9){
			c=0x0391+(c-0x03B1);
		}
		// キリル文字変換
		else if( c>=0x0430 && c<=0x044F){
			c=0x0410+(c-0x0430);
		}
		// キリル文字（拡張）変換
		else if (c>=0x0450 && c<=0x045F) {
			c=0x0400+(c-0x0450);
		}
		// iso8859-1(Latin-1 Supplement)変換
		else if (c>=0x00E0 && c<=0x00FE && c!=0x00F7) {
			c=0x00C0+(c-0x00E0);
		}
		else if (c == 0x00FF) {
			c = 0x0178;
		}
		// Latin Extended-A 変換(含東欧等)
		else if (c==0x0131) { // 2014.11.25 0x0131=>0x0130は間違い。0x0131=>0x0049に変更
			c = 0x0049; // I
		}
		else if ((c>=0x0100 && c<=0x0137) || (c>=0x014A && c<=0x0177)) {
			c = (c & ~0x0001);
		}
		else if (((c>=0x0139 && c<=0x0148) || (c>=0x0179 && c<=0x017E)) && ((c&1) == 0)) {
			c--;
		}
		// Latin Extended-B 変換(含Pinyin用)
		else if ((c>=0x0180 && c<=0x0185) || (c>=0x0198 && c<=0x0199) || (c>=0x01A0 && c<=0x01A5) || (c>=0x01AC && c<=0x01AD) || (c>=0x01B8 && c<=0x01B9) || (c>=0x01BC && c<=0x01BD) || (c>=0x01DE && c<=0x01EF) || (c>=0x01F4 && c<=0x01F5) || (c>=0x01F8 && c<=0x01FF)) {
			c = (c & ~0x0001);
		}
		else if (((c>=0x0187 && c<=0x018c) || (c>=0x0191 && c<=0x0192) || (c>=0x01A7 && c<=0x01A8) || (c>=0x01AF && c<=0x01B0) || (c>=0x01B3 && c<=0x01B6) || (c>=0x01CD && c<=0x01DC)) && ((c&1) == 0)) {
			c--;
		}
		else if (c == 0x01C6 || c == 0x01C9 || c == 0x01CC || c == 0x01F3) {
			c-=2;
		}
		else if (c == 0x01C5 || c == 0x01C8 || c == 0x01CB || c == 0x01F2) {
			// Title Letter
			c--;
		}
		else if (c == 0x01DD) {
			c = 0x018E;
		}
		else if (c == 0x0195) {
			c = 0x01F6;
		}
		else if (c == 0x01BF) {
			c = 0x01F7;
		}
		// Latin Extended Additional 変換(含ベトナム語用)
		else if ((c>=0x1E00 && c<=0x1E95) || (c>=0x1EA0 && c<=0x1EFF)) {
			c = (c & ~0x0001);
		}
	}
	return true;
}
