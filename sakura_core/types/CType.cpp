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


/*!	@brief ���L������������/�����L�[���[�h

	�����L�[���[�h�֘A�̏���������

	@date 2005.01.30 genta CShareData::Init()���番���D
		�L�[���[�h��`���֐��̊O�ɏo���C�o�^���}�N�������ĊȌ��ɁD
*/
void CShareData::InitKeyword(DLLSHAREDATA* pShareData)
{
	/* �����L�[���[�h�̃e�X�g�f�[�^ */
	pShareData->m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx = 0;

	int nSetCount = -1;

#define PopulateKeyword(name,case_sensitive,aryname) \
	extern const wchar_t* g_ppszKeywords##aryname[]; \
	extern int g_nKeywords##aryname; \
	pShareData->m_CKeyWordSetMgr.AddKeyWordSet( (name), (case_sensitive) );	\
	pShareData->m_CKeyWordSetMgr.SetKeyWordArr( ++nSetCount, g_nKeywords##aryname, g_ppszKeywords##aryname );
	
	PopulateKeyword( L"C/C++",			true,	CPP );			/* �Z�b�g 0�̒ǉ� */
	PopulateKeyword( L"HTML",			false,	HTML );			/* �Z�b�g 1�̒ǉ� */
	PopulateKeyword( L"PL/SQL",			false,	PLSQL );		/* �Z�b�g 2�̒ǉ� */
	PopulateKeyword( L"COBOL",			true,	COBOL );		/* �Z�b�g 3�̒ǉ� */
	PopulateKeyword( L"Java",			true,	JAVA );			/* �Z�b�g 4�̒ǉ� */
	PopulateKeyword( L"CORBA IDL",		true,	CORBA_IDL );	/* �Z�b�g 5�̒ǉ� */
	PopulateKeyword( L"AWK",			true,	AWK );			/* �Z�b�g 6�̒ǉ� */
	PopulateKeyword( L"MS-DOS batch",	false,	BAT );			/* �Z�b�g 7�̒ǉ� */	//Oct. 31, 2000 JEPRO '�o�b�`�t�@�C��'��'batch' �ɒZ�k
	PopulateKeyword( L"Pascal",			false,	PASCAL );		/* �Z�b�g 8�̒ǉ� */	//Nov. 5, 2000 JEPRO ��E�������̋�ʂ�'���Ȃ�'�ɕύX
	PopulateKeyword( L"TeX",			true,	TEX );			/* �Z�b�g 9�̒ǉ� */	//Sept. 2, 2000 jepro Tex ��TeX �ɏC�� Bool�l�͑�E�������̋��
	PopulateKeyword( L"TeX2",			true,	TEX2 );			/* �Z�b�g10�̒ǉ� */	//Jan. 19, 2001 JEPRO �ǉ�
	PopulateKeyword( L"Perl",			true,	PERL );			/* �Z�b�g11�̒ǉ� */
	PopulateKeyword( L"Perl2",			true,	PERL2 );		/* �Z�b�g12�̒ǉ� */	//Jul. 10, 2001 JEPRO Perl����ϐ��𕪗��E�Ɨ�
	PopulateKeyword( L"Visual Basic",	false,	VB );			/* �Z�b�g13�̒ǉ� */	//Jul. 10, 2001 JEPRO
	PopulateKeyword( L"Visual Basic2",	false,	VB2 );			/* �Z�b�g14�̒ǉ� */	//Jul. 10, 2001 JEPRO
	PopulateKeyword( L"���b�`�e�L�X�g",	true,	RTF );			/* �Z�b�g15�̒ǉ� */	//Jul. 10, 2001 JEPRO

#undef PopulateKeyword
}

