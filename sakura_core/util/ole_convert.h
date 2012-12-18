/*!	@file
	@brief OLE�^�iVARIANT, BSTR�Ȃǁj�̕ϊ��֐�

*/
#ifndef SAKURA_OLE_CONVERT_208FE8C1_C742_4ED8_9C9C_25841915706BD_H_
#define SAKURA_OLE_CONVERT_208FE8C1_C742_4ED8_9C9C_25841915706BD_H_

#include <string>
#include "_os/OleTypes.h"

bool variant_to_wstr( VARIANT v, std::wstring& wstr );	// VARIANT�ϐ���BSTR�Ƃ݂Ȃ��Awstring�ɕϊ�����
bool variant_to_int( VARIANT v, int& n );	// VARIANT�ϐ��𐮐��Ƃ݂Ȃ��Aint�ɕϊ�����

#endif /* SAKURA_OLE_CONVERT_208FE8C1_C742_4ED8_9C9C_25841915706BD_H_ */
/*[EOF]*/
