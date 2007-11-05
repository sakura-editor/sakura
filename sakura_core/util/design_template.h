/*
2007.10.23 kobake

�f�U�C���p�^�[���I�ȃ��m��u����Ɨǂ��Ȃ��B
���Ȃ݂� TSingleInstance �̓V���O���g���p�^�[���Ƃ͎��Ĕ�Ȃ郂�m�ł����B
*/

#pragma once

/*!
	1�����C���X�^���X�����݂��Ȃ��N���X����̃C���X�^���X�擾�C���^�[�t�F�[�X��static�Œ񋟁B
	Singleton�p�^�[���Ƃ͈قȂ�AInstance()�Ăяo���ɂ��A�C���X�^���X��������������Ȃ��_�ɒ��ӁB

	2007.10.23 kobake �쐬
*/
template <class T>
class TSingleInstance{
public:
	//���J�C���^�[�t�F�[�X
	static T* Instance(){ return gm_instance; } //!< �쐬�ς݂̃C���X�^���X��Ԃ��B�C���X�^���X�����݂��Ȃ���� NULL�B

protected:
	//��2�ȏ�̃C���X�^���X�͑z�肵�Ă��܂���Bassert���j�]�����o���܂��B
	TSingleInstance(){ assert(gm_instance==NULL); gm_instance=static_cast<T*>(this); }
	~TSingleInstance(){ assert(gm_instance); gm_instance=NULL; }
private:
	static T* gm_instance;
};
template <class T>
T* TSingleInstance<T>::gm_instance = NULL;

