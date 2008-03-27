//2007.09.24 kobake �쐬
//�ݒ�t�@�C�����̃e�L�X�g���o�͂��s�����߂̃N���X�Q�B
//.ini �� .mac �̓��o�͂������Ƃ��Ɏg���Ɨǂ��ł��B
//���u�ҏW�e�L�X�g�v���������߂ł͂Ȃ��A�����܂ł��A.ini��.mac�̂悤�ȁu�ݒ�t�@�C���v�������ړI�̃N���X�Q�ł��B
//
//���̂Ƃ����ShiftJIS�œ��o�͂��s�����A
//������UTF-8���ɂ��邱�Ƃɂ��AUNICODE�f�[�^�̌������N����Ȃ��悤�ɂ������B

#pragma once

#include <string>

#include "CStream.h"
class CCodeBase;

//�e�L�X�g���̓X�g���[�� (UTF-8, SJIS)
class CTextInputStream : public CStream{
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CTextInputStream(const TCHAR* tszPath);
	CTextInputStream();
	virtual ~CTextInputStream();

	//����
	std::wstring ReadLineW(); //!< 1�s�Ǎ��B���s�͍��

private:
	bool m_bIsUtf8; //!< UTF-8�Ȃ�true
};

//�e�L�X�g�o�̓X�g���[��
// 2008.01.26 kobake �o�͕����R�[�h��C�ӂŎw��ł���悤�ɕύX
class CTextOutputStream : public COutputStream{
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CTextOutputStream(const TCHAR* tszPath, ECodeType eCodeType = CODE_UTF8, bool bExceptionMode = false);
	virtual ~CTextOutputStream();

	//�����񏑍��B���s����ꂽ���ꍇ�́A���������'\n'���܂߂邱�ƁB(�N���X���œK�؂ȉ��s�R�[�h�ɕϊ����ďo�͂��܂�)
	void WriteString(const wchar_t* szData, int nLen = -1);
	void WriteF(const wchar_t* format, ...);

	//���l�����B(�N���X���œK���ɐ��`���ďo�͂��܂�)
	void WriteInt(int n);

private:
	CCodeBase* m_pcCodeBase;
};



//�e�L�X�g���̓X�g���[���B���΃p�X�̏ꍇ��INI�t�@�C���̃p�X����̑��΃p�X�Ƃ��ĊJ���B
class CTextInputStream_AbsIni : public CTextInputStream{
public:
	CTextInputStream_AbsIni(const TCHAR* tszPath, bool bOrExedir = true);
};
