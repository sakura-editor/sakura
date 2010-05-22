/*
	2008.05.18 kobake CShareData ���番��
*/

#pragma once

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
