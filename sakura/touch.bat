@echo off

REM �w��t�@�C���̃^�C���X�^���v�����ݎ����ɍX�V����
REM VC++2005/2008 �ŕs�K�v�Ƀr���h�̖₢���킹���o��̂�������邽�߁A
REM �r���h��̃C�x���g�� .manifest �̎������X�V����̂Ɏg�p�B
REM ���₢���킹�����炵���������Ȃ̂ŃG���[�������Ă�����I���� 0 �ŏI��

if %1.==. goto end
if not exist %1 goto end
echo touch %1
copy /b %1+,,%1 > nul
:end
exit 0
