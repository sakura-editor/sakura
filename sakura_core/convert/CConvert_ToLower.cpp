#include "StdAfx.h"
#include "CConvert_ToLower.h"

// 小文字へ変換
//	変換できないキリル文字Ёの修正	2010/6/5 Uchi
//	ラテンアルファベットの拡張対応(ラテン補助、拡張AB、拡張追加)	2010/6/5 Uchi
bool CConvert_ToLower::DoConvert(CNativeW* pcData)
{
	WCHAR* p = pcData->GetStringPtr();
	WCHAR* end = p + pcData->GetStringLength();
	while(p < end){
		WCHAR& c=*p++;
		// A-Z → a-z
		if(c>=0x0041 && c<=0x005A){
			c=0x0061+(c-0x0041);
		}
		// Ａ-Ｚ → ａ-ｚ
		else if( c>=0xFF21 && c<=0xFF3A){
			c=0xFF41+(c-0xFF21);
		}
		// ギリシャ文字変換
		else if( c>=0x0391 && c<=0x03A9){
			c=0x03B1+(c-0x0391);
		}
		// キリル文字変換
		else if( c>=0x0410 && c<=0x042F){
			c=0x0430+(c-0x0410);
		}
		// キリル文字（拡張）変換
		else if (c>=0x0400 && c<=0x040F) {
			c=0x0450+(c-0x0400);
		}
		// iso8859-1(Latin-1 Supplement)変換
		else if (c>=0x00C0 && c<=0x00DE && c!=0x00D7) {
			c=0x00E0+(c-0x00C0);
		}
		else if (c == 0x0178) {
			c = 0x00FF;
		}
		// Latin Extended-A 変換(含東欧等)
		else if (c==0x0130) { // 2014.11.25 0x0130=>0x0131は間違い。0x0130=>0x0069に変更
			c = 0x0069; // i
		}
		else if ((c>=0x0100 && c<=0x0137) || (c>=0x014A && c<=0x0177)) {
			c = (c | 0x0001);
		}
		else if (((c>=0x0139 && c<=0x0148) || (c>=0x0179 && c<=0x017E)) && ((c&1) == 1)) {
			c++;
		}
		// Latin Extended-B 変換(含Pinyin用 小文字しか使わないから余り意味無いけど)
		else if ((c>=0x0180 && c<=0x0185) || (c>=0x0198 && c<=0x0199) || (c>=0x01A0 && c<=0x01A5) || (c>=0x01AC && c<=0x01AD) || (c>=0x01B8 && c<=0x01B9) || (c>=0x01BC && c<=0x01BD) || (c>=0x01DE && c<=0x01EF) || (c>=0x01F4 && c<=0x01F5) || (c>=0x01F8 && c<=0x01FF)) {
			c = (c | 0x0001);
		}
		else if (((c>=0x0187 && c<=0x018c) || (c>=0x0191 && c<=0x0192) || (c>=0x01A7 && c<=0x01A8) || (c>=0x01AF && c<=0x01B0) || (c>=0x01B3 && c<=0x01B6) || (c>=0x01CD && c<=0x01DC)) && ((c&1) == 1)) {
			c++;
		}
		else if (c == 0x01C4 || c == 0x01C7 || c == 0x01CA || c == 0x01F1) {
			c+=2;
		}
		else if (c == 0x01C5 || c == 0x01C8 || c == 0x01CB || c == 0x01F2) {
			// Title Letter
			c++;
		}
		else if (c == 0x018E) {
			c = 0x01DD;
		}
		else if (c == 0x01F6) {
			c = 0x0195;
		}
		else if (c == 0x01F7) {
			c = 0x01BF;
		}
		// Latin Extended Additional 変換(含ベトナム語用)
		else if ((c>=0x1E00 && c<=0x1E95) || (c>=0x1EA0 && c<=0x1EFF)) {
			c = (c | 0x0001);
		}
	}
	return true;
}
