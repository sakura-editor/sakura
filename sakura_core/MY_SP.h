//$Id$
/*!	@file
	@brief à¯êîÇê≥ÇµÇ≠ñ≥éãÇ∑ÇÈsplitpath

	@author SUI
	@date 2002.07.15 Initial release

	$Revision$
*/
/*
	Copyright (C) 2002, SUI

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
#ifdef __cplusplus
extern "C" {
#endif

void my_splitpath ( const char *comln , char *drv,char *dir,char *fnm,char *ext );
char	*sjis_strrchr2( unsigned char *pt , const unsigned char ch1 , const unsigned char ch2 );

#ifdef __cplusplus
}
#endif
