/*!	@file
	@brief OLE型（VARIANT, BSTRなど）の変換関数

*/
#ifndef SAKURA_OLE_CONVERT_208FE8C1_C742_4ED8_9C9C_25841915706BD_H_
#define SAKURA_OLE_CONVERT_208FE8C1_C742_4ED8_9C9C_25841915706BD_H_

#include <string>
#include "_os/OleTypes.h"

bool variant_to_wstr( VARIANT v, std::wstring& wstr );	// VARIANT変数をBSTRとみなし、wstringに変換する
bool variant_to_int( VARIANT v, int& n );	// VARIANT変数を整数とみなし、intに変換する

#endif /* SAKURA_OLE_CONVERT_208FE8C1_C742_4ED8_9C9C_25841915706BD_H_ */
/*[EOF]*/
