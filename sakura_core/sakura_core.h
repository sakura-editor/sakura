
// �ȉ��� ifdef �u���b�N�� DLL ����ȒP�ɃG�N�X�|�[�g������}�N�����쐬����W���I�ȕ��@�ł��B
// ���� DLL ���̂��ׂẴt�@�C���̓R�}���h���C���Œ�`���ꂽ SAKURA_CORE_EXPORTS �V���{��
// �ŃR���p�C������܂��B���̃V���{���͂��� DLL ���g�p����ǂ̃v���W�F�N�g��ł�����`�łȂ�
// ��΂Ȃ�܂���B���̕��@�ł̓\�[�X�t�@�C���ɂ��̃t�@�C�����܂ނ��ׂẴv���W�F�N�g�� DLL
// ����C���|�[�g���ꂽ���̂Ƃ��� SAKURA_CORE_API �֐����Q�Ƃ��A���̂��߂��� DLL �͂��̃}�N
// ���Œ�`���ꂽ�V���{�����G�N�X�|�[�g���ꂽ���̂Ƃ��ĎQ�Ƃ��܂��B
#ifdef SAKURA_CORE_EXPORTS
#define SAKURA_CORE_API __declspec(dllexport)
#else
#define SAKURA_CORE_API __declspec(dllimport)
#endif

// ���̃N���X�� sakura_core.dll ����G�N�X�|�[�g����܂�
class SAKURA_CORE_API CSakura_core {
public:
	CSakura_core(void);
	// TODO: ���̈ʒu�Ƀ��\�b�h��ǉ����Ă��������B
};

extern SAKURA_CORE_API int nSakura_core;

SAKURA_CORE_API int fnSakura_core(void);


/*[EOF]*/
