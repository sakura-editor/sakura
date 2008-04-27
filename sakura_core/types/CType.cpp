#include "stdafx.h"
#include "CType.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        CTypeConfig                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
STypeConfig* CTypeConfig::GetTypeConfig()
{
	return &GetDllShareData().GetTypeSetting(*this);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          CType                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
void CType::InitTypeConfig(int nIdx)
{
	DLLSHAREDATA* pShareData = &GetDllShareData();

	//�K��l���R�s�[
	pShareData->GetTypeSetting(CTypeConfig(nIdx)) = pShareData->GetTypeSetting(CTypeConfig(0));

	//�C���f�b�N�X��ݒ�
	CTypeConfig(nIdx)->m_nIdx = nIdx;

	//�ʐݒ�
	InitTypeConfigImp(CTypeConfig(nIdx).GetTypeConfig());
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        CShareData                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	@brief ���L������������/�^�C�v�ʐݒ�

	�^�C�v�ʐݒ�̏���������

	@date 2005.01.30 genta CShareData::Init()���番���D
*/
void CShareData::InitTypeConfigs(DLLSHAREDATA* pShareData)
{
	CType* table[] = {
		new CType_Basis(),	//��{
		new CType_Text(),	//�e�L�X�g
		new CType_Cpp(),	//C/C++
		new CType_Html(),	//HTML
		new CType_Sql(),	//PL/SQL
		new CType_Cobol(),	//COBOL
		new CType_Java(),	//Java
		new CType_Asm(),	//�A�Z���u��
		new CType_Awk(),	//awk
		new CType_Dos(),	//MS-DOS�o�b�`�t�@�C��
		new CType_Pascal(),	//Pascal
		new CType_Tex(),	//TeX
		new CType_Perl(),	//Perl
		new CType_Vb(),		//Visual Basic
		new CType_Rich(),	//���b�`�e�L�X�g
		new CType_Ini(),	//�ݒ�t�@�C��
		new CType_Other1(),	//�ݒ�17
		new CType_Other2(),	//�ݒ�18
		new CType_Other3(),	//�ݒ�19
		new CType_Other4(),	//�ݒ�20
	};
	for(int i=0;i<_countof(table);i++){
		table[i]->InitTypeConfig(i);
		SAFE_DELETE(table[i]);
	}
}
