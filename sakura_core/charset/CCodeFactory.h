/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CCODEFACTORY_953F01EB_9094_453E_92FC_35456A0E11A9_H_
#define SAKURA_CCODEFACTORY_953F01EB_9094_453E_92FC_35456A0E11A9_H_

class CCodeBase;

class CCodeFactory{
public:
	//! eCodeTypeに適合する CCodeBaseインスタンス を生成
	static CCodeBase* CreateCodeBase(
		ECodeType	eCodeType,		//!< 文字コード
		int			nFlag			//!< bit 0: MIME Encodeされたヘッダをdecodeするかどうか
	);
};

#endif /* SAKURA_CCODEFACTORY_953F01EB_9094_453E_92FC_35456A0E11A9_H_ */
/*[EOF]*/
