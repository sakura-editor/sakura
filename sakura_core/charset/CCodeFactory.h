#pragma once

class CCodeBase;

class CCodeFactory{
public:
	//! eCodeType�ɓK������ CCodeBase�C���X�^���X �𐶐�
	static CCodeBase* CreateCodeBase(
		ECodeType	eCodeType,		//!< �����R�[�h
		int			nFlag			//!< bit 0: MIME Encode���ꂽ�w�b�_��decode���邩�ǂ���
	);
};

