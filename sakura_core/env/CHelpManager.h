/*
	2008.05.18 kobake CShareData ���番��
*/
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
#ifndef SAKURA_CHELPMANAGER_979D10EB_B07B_466F_BB28_AB5A7259E2EA9_H_
#define SAKURA_CHELPMANAGER_979D10EB_B07B_466F_BB28_AB5A7259E2EA9_H_

// �v��s��`
// #include "DLLSHAREDATA.h"


//!�w���v�Ǘ�
class CHelpManager{
public:
	CHelpManager()
	{
		m_pShareData = &GetDllShareData();
	}
	//�w���v�֘A	//@@@ 2002.2.3 YAZAKI
	bool			ExtWinHelpIsSet( CTypeConfig nType = CTypeConfig(-1) );		//	�^�C�v��nType�̂Ƃ��ɁA�O���w���v���ݒ肳��Ă��邩�B
	const TCHAR*	GetExtWinHelp( CTypeConfig nType = CTypeConfig(-1) );		//	�^�C�v��nType�̂Ƃ��́A�O���w���v�t�@�C�������擾�B
	bool			ExtHTMLHelpIsSet( CTypeConfig nType = CTypeConfig(-1) );	//	�^�C�v��nType�̂Ƃ��ɁA�O��HTML�w���v���ݒ肳��Ă��邩�B
	const TCHAR*	GetExtHTMLHelp( CTypeConfig nType = CTypeConfig(-1) );		//	�^�C�v��nType�̂Ƃ��́A�O��HTML�w���v�t�@�C�������擾�B
	bool			HTMLHelpIsSingle( CTypeConfig nType = CTypeConfig(-1) );	//	�^�C�v��nType�̂Ƃ��́A�O��HTML�w���v�u�r���[�A�𕡐��N�����Ȃ��v��ON�����擾�B
private:
	DLLSHAREDATA* m_pShareData;
};

#endif /* SAKURA_CHELPMANAGER_979D10EB_B07B_466F_BB28_AB5A7259E2EA9_H_ */
/*[EOF]*/
