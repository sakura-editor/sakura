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
#ifndef SAKURA_CSTREAM_BB8B6415_7CBF_48F0_9454_2AC4D39C7CD1_H_
#define SAKURA_CSTREAM_BB8B6415_7CBF_48F0_9454_2AC4D39C7CD1_H_

class CFileAttribute;

//��O
class CError_FileOpen{};	//!< ��O�F�t�@�C���I�[�v���Ɏ��s
class CError_FileWrite{};	//!< ��O�F�t�@�C���������ݎ��s
class CError_FileRead{};	//!< ��O�F�t�@�C���ǂݍ��ݎ��s

//�X�g���[�����N���X
class CStream{
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CStream(const TCHAR* tszPath, const TCHAR* tszMode, bool bExceptionMode = false);
//	CStream();
	virtual ~CStream();

	//���Z�q
	operator bool() const{ return Good(); }

	//�I�[�v���E�N���[�Y
	void Open(const TCHAR* tszPath, const TCHAR* tszMode);
	void Close();

	//����
	void SeekSet(	//!< �V�[�N
		long offset	//!< �X�g���[���擪����̃I�t�Z�b�g 
	);
	void SeekEnd(   //!< �V�[�N
		long offset //!< �X�g���[���I�[����̃I�t�Z�b�g
	);

	//���
	virtual bool Good() const{ return m_fp!=NULL && !Eof(); }
	bool Eof() const{ return m_fp==NULL || feof(m_fp); }

	//�t�@�C���n���h��
	FILE* GetFp() const{ return m_fp; }

	//���[�h
	bool IsExceptionMode() const{ return m_bExceptionMode; }
private:
	FILE*			m_fp;
	CFileAttribute*	m_pcFileAttribute;
	bool			m_bExceptionMode;
};


class COutputStream : public CStream{
public:
	COutputStream(const TCHAR* tszPath, const TCHAR* tszMode, bool bExceptionMode = false)
	: CStream(tszPath, tszMode, bExceptionMode)
	{
	}

	//! �f�[�^�𖳕ϊ��ŏ������ށB�߂�l�͏������񂾃o�C�g���B
	int Write(const void* pBuffer, int nSizeInBytes)
	{
		int nRet = fwrite(pBuffer,1,nSizeInBytes,GetFp());
		if(nRet!=nSizeInBytes && IsExceptionMode())throw CError_FileWrite();
		return nRet;
	}
};

#endif /* SAKURA_CSTREAM_BB8B6415_7CBF_48F0_9454_2AC4D39C7CD1_H_ */
/*[EOF]*/
