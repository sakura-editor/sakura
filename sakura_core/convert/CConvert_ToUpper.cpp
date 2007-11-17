#include "stdafx.h"
#include "CConvert_ToUpper.h"

// �啶���֕ϊ�
bool CConvert_ToUpper::DoConvert(CNativeW* pcData)
{
	WCHAR* p = pcData->GetStringPtr();
	WCHAR* end = p + pcData->GetStringLength();
	while(p < end){
		WCHAR& c=*p;
		// a-z �� A-Z
		if(c>=0x0061 && c<=0x007A){
			c=0x0041+(c-0x0061);
		}
		// ��-�� �� �`-�y
		else if( c>=0xFF41 && c<=0xFF5A){
			c=0xFF21+(c-0xFF41);
		}
		// �M���V�������ϊ�
		else if( c>=0x03B1 && c<=0x03C9){
			c=0x0391+(c-0x03B1);
		}
		// ���V�A�����ϊ�
		else if( c>=0x0430 && c<=0x044F){
			c=0x0410+(c-0x0430);
		}

		p++;
	}
	return true;
}
