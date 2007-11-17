#pragma once

//�X�g���[�����N���X
class CStream{
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CStream(const TCHAR* tszPath, const TCHAR* tszMode);
	CStream();
	virtual ~CStream();

	//���Z�q
	operator bool() const{ return Good(); }

	//����
	void Open(const TCHAR* tszPath, const TCHAR* tszMode);
	void Close();
	void SeekSet(	//!< �V�[�N
		long offset	//!< �X�g���[���擪����̃I�t�Z�b�g 
	);
	void SeekEnd(   //!< �V�[�N
		long offset //!< �X�g���[���I�[����̃I�t�Z�b�g
	);

	//���
	virtual bool Good() const{ return m_fp!=NULL && !Eof(); }
	bool Eof() const{ return m_fp==NULL || feof(m_fp); }
protected:
	FILE* m_fp;
};

