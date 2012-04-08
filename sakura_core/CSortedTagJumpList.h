/*!	@file
	@brief �^�O�W�����v���X�g

	@author MIK
	@date 2005.3.31
*/
/*
	Copyright (C) 2005, MIK, genta

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

#ifndef SAKURA_CSORTED_TAGJUMP_LIST_H_
#define SAKURA_CSORTED_TAGJUMP_LIST_H_

#define MAX_TAG_STRING_LENGTH _MAX_PATH	//�Ǘ����镶����̍ő咷

/*!	@brief �_�C���N�g�^�O�W�����v�p�������ʂ��\�[�g���ĕێ�����D
	@author MIK
*/
class CSortedTagJumpList {
public:
	CSortedTagJumpList(int max);
	~CSortedTagJumpList();

	int AddBaseDir( const TCHAR* baseDir );
	BOOL AddParamA( const ACHAR* keyword, const ACHAR* filename, int no, ACHAR type, const ACHAR* note, int depth, const int baseDirId );
	BOOL GetParam( int index, TCHAR* keyword, TCHAR* filename, int* no, TCHAR* type, TCHAR* note, int* depth, TCHAR* baseDir );
	int GetCount( void ){ return m_nCount; }
	void Empty( void );
	bool IsOverflow( void ){ return m_bOverflow; }

	typedef struct tagjump_info_t {
		struct tagjump_info_t*	next;	//!< ���̃��X�g
		TCHAR*	keyword;	//!< �L�[���[�h
		TCHAR*	filename;	//!< �t�@�C����
		int		no;			//!< �s�ԍ�
		TCHAR	type;		//!< ���
		TCHAR*	note;		//!< ���l
		int		depth;		//!< (�����̂ڂ�)�K�w
		int		baseDirId;	//!< �t�@�C�����̃x�[�X�f�B���N�g��
	} TagJumpInfo;

	TagJumpInfo* GetPtr( int index );

	/*!	@brief �Ǘ����̍ő�l���擾����

		@date 2005.04.22 genta �ő�l���ς�
	*/
	int GetCapacity(void) const { return m_MAX_TAGJUMPLIST; }

private:
	TagJumpInfo*	m_pTagjump;	//!< �^�O�W�����v���
	std::vector<std::tstring> m_baseDirArr;	//!< �x�[�X�f�B���N�g�����
	int				m_nCount;	//!< ��
	bool			m_bOverflow;	//!< �I�[�o�[�t���[
	
	//	2005.04.22 genta �ő�l���ς�
	const int		m_MAX_TAGJUMPLIST;	//!< �Ǘ�������̍ő吔

	void Free( TagJumpInfo* item );
};

#endif	//SAKURA_CSORTED_TAGJUMP_LIST_H_
