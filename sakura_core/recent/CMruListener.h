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
#ifndef SAKURA_CMRULISTENER_7C175242_808A_48BB_A4C2_93FA71274B999_H_
#define SAKURA_CMRULISTENER_7C175242_808A_48BB_A4C2_93FA71274B999_H_

#include "doc/CDocListener.h"

class CMruListener : public CDocListenerEx{
public:
	//���[�h�O��
//	ECallbackResult OnCheckLoad(SLoadInfo* pLoadInfo);
	void OnBeforeLoad(SLoadInfo* sLoadInfo);
	void OnAfterLoad(const SLoadInfo& sLoadInfo);

	//�Z�[�u�O��
	void OnAfterSave(const SSaveInfo& sSaveInfo);

	//�N���[�Y�O��
	ECallbackResult OnBeforeClose();

protected:
	//�w���p
	void _HoldBookmarks_And_AddToMRU(); // Mar. 30, 2003 genta
};

#endif /* SAKURA_CMRULISTENER_7C175242_808A_48BB_A4C2_93FA71274B999_H_ */
/*[EOF]*/
