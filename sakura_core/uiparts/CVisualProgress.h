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
#ifndef SAKURA_CVISUALPROGRESS_64023BB9_BFA3_45B6_9E06_553B0F5EDCC5_H_
#define SAKURA_CVISUALPROGRESS_64023BB9_BFA3_45B6_9E06_553B0F5EDCC5_H_

#include "doc/CDocListener.h"
class CWaitCursor;

class CVisualProgress : public CDocListenerEx, public CProgressListener{
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CVisualProgress();
	virtual ~CVisualProgress();

	//���[�h�O��
	void OnBeforeLoad(SLoadInfo* sLoadInfo);
	void OnAfterLoad(const SLoadInfo& sLoadInfo);

	//�Z�[�u�O��
	void OnBeforeSave(const SSaveInfo& sSaveInfo);
	void OnFinalSave(ESaveResult eSaveResult);

	//�v���O���X��M
	void OnProgress(int nPer);

protected:
	//�����⏕
	void _Begin();
	void _Doing(int nPer);
	void _End();
private:
	CWaitCursor* m_pcWaitCursor;
};

#endif /* SAKURA_CVISUALPROGRESS_64023BB9_BFA3_45B6_9E06_553B0F5EDCC5_H_ */
/*[EOF]*/
