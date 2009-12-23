/*!	@file
	@brief Erlang �A�E�g���C�����
	
	@author genta
	@date 2009.08.10 created
	
*/
/*
	Copyright (C) 2009, genta

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

#include "stdafx.h"
#include "doc/CDocOutline.h"
#include "outline/CFuncInfoArr.h"

/** Erlang �A�E�g���C����� �Ǘ������
*/
struct COutlineErlang {
	enum {
		STATE_NORMAL,	//!< ��͒��łȂ�
		STATE_FUNC_CANDIDATE_FIN,	//!< �֐��炵������(�s����atom)����͍ς�
		STATE_FUNC_ARGS1,	//!< �ŏ��̈����m�F��
		STATE_FUNC_ARGS,	//!< 2�߈ȍ~�̈����m�F��
		STATE_FUNC_ARGS_FIN,//!< �֐��̉�͂�����
		STATE_FUNC_FOUND,	//!< �֐��𔭌��D�f�[�^�̎擾���\
	} m_state;

	wchar_t m_func[64];	//!< �֐���(Arity�܂�) = �\����
	CLogicInt m_lnum;	//!< �֐��̍s�ԍ�
	int m_argcount;		//!< �������������̐�
	wchar_t m_parenthesis[32];	//!< ���ʂ̃l�X�g���Ǘ��������
	int m_parenthesis_ptr;	//!< ���ʂ̃l�X�g���x��
	
	COutlineErlang();
	bool parse( const wchar_t* buf, int linelen, CLogicInt linenum );
	
	const wchar_t* ScanFuncName( const wchar_t* buf, const wchar_t* end, const wchar_t* p );
	const wchar_t* EnterArgs( const wchar_t* end, const wchar_t* p );
	const wchar_t* ScanArgs1( const wchar_t* end, const wchar_t* p );
	const wchar_t* ScanArgs( const wchar_t* end, const wchar_t* p );
	const wchar_t* EnterCond( const wchar_t* end, const wchar_t* p );
	const wchar_t* GetFuncName() const { return m_func; }
	CLogicInt GetFuncLine() const { return m_lnum; }

private:
	// helper functions
	bool IS_ATOM_HEAD( wchar_t wc )
	{
		return ( L'a' <= wc && wc <= L'z' )
			|| ( wc == L'_' ) || ( wc == L'@' );
	}

	bool IS_ALNUM( wchar_t wc )
	{
		return IS_ATOM_HEAD(wc) || ( L'A' <= wc && wc <= L'Z' ) || ( L'0' <= wc && wc <= L'9' );
	}

	bool IS_COMMENT( wchar_t wc )
	{
		return ( wc == L'%' );
	}

	bool IS_SPACE( wchar_t wc )
	{
		return ( wcschr( L" \t\r\n", wc ) != 0 );
	}
	
	void build_arity(int);
};

COutlineErlang::COutlineErlang() :
	m_state( STATE_NORMAL ), m_lnum( 0 ), m_argcount( 0 )
{
}

/** �֐����̎擾

	@param[in] buf �s(�擪����)
	@param[in] end buf����
	@param[in] p ��͂̌��݈ʒu
	
	�֐�����atom�Datom�� �������A���t�@�x�b�g�C_, @ �̂����ꂩ����n�܂�
	�p�������񂩁C���邢�̓V���O���N�H�[�e�[�V�����ň͂܂ꂽ������D
*/
const wchar_t* COutlineErlang::ScanFuncName( const wchar_t* buf, const wchar_t* end, const wchar_t* p )
{
	assert( m_state == STATE_NORMAL );

	if( p > buf || ! ( IS_ATOM_HEAD( *p ) || *p == L'\'' )) {
		return end;
	}
	
	if( *p == L'\'' ){
		do {
			++p;
		} while( *p != L'\'' && p < end );
		if( p >= end ){
			// invalid atom
			return p;
		}
		++p;
	}
	else {
		do {
			++p;
		} while( IS_ALNUM( *p ) && p < end );
	}
	
	int buf_len = sizeof( m_func ) / sizeof( m_func[0]);
	const wchar_t *head = buf;
	int len = p - buf;
	if( buf[0] == L'\'' ){
		++buf;
		len -= 2;
		--buf_len;
	}
	len = len < buf_len - 1 ? len : buf_len - 1;
	wcsncpy( m_func, buf, len );
	m_func[len] = L'\0';
	m_state = STATE_FUNC_CANDIDATE_FIN;
	return p;
}

/** �p�����[�^�̔���

	@param[in] end buf����
	@param[in] p ��͂̌��݈ʒu
	
	�֐����̎擾���������C�p�����[�^�擪�̊��ʂ�T���D
*/
const wchar_t* COutlineErlang::EnterArgs( const wchar_t* end, const wchar_t* p )
{
	assert( m_state == STATE_FUNC_CANDIDATE_FIN );

	while( IS_SPACE( *p ) && p < end )
		p++;
	
	if( p >= end )
		return end;

	if( IS_COMMENT( *p )){
		return end;
	}
	else if( *p == L'(' ){ // )
		m_state = STATE_FUNC_ARGS1;
		m_argcount = 0;
		m_parenthesis_ptr = 1;
		m_parenthesis[0] = *p;
		++p;

		return p;
	}

	// not a function
	m_state = STATE_NORMAL;
	return end;
}

/** �擪�p�����[�^�̔���

	@param[in] end buf����
	@param[in] p ��͂̌��݈ʒu
	
	�p�����[�^��0��1�ȏ�̔��ʂ̂��߂ɏ�Ԃ�݂��Ă���D
*/
const wchar_t* COutlineErlang::ScanArgs1( const wchar_t* end, const wchar_t* p )
{
	assert( m_state = STATE_FUNC_ARGS1 );
	
	while( IS_SPACE( *p ) && p < end )
		p++;

	if( p >= end )
		return end;

	if( *p == /* ( */ L')' ){
		// no argument
		m_state = STATE_FUNC_ARGS_FIN;
		p++;
	}
	else if( IS_COMMENT( *p )){
		return end;
	}
	else {
		// argument found
		m_state = STATE_FUNC_ARGS;
		++m_argcount;
	}
	return p;
}

/** �p�����[�^�̉�͂ƃJ�E���g

	@param[in] end buf����
	@param[in] p ��͂̌��݈ʒu
	
	�p�����[�^����͂���D�p�����[�^�̐��Ɩ����̕����ʂ𐳂������ʂ��邽�߂ɁC
	���p���C���ʁC�p�����[�^�̋�؂�̃J���}�ɒ��ڂ���D
	���p���͉��s���܂ނ��Ƃ��ł��Ȃ��D
*/
const wchar_t* COutlineErlang::ScanArgs( const wchar_t* end, const wchar_t* p )
{
	assert( m_state = STATE_FUNC_ARGS );

	const int parptr_max = sizeof( m_parenthesis ) / sizeof( m_parenthesis[0] );
	wchar_t quote = L'\0'; // �擪�ʒu��ۑ�
	for(const wchar_t* head = p ; p < end ; p++ ){
		if( quote ){
			if( *p == quote )
				quote = L'\0';
		}
		else {
			if( wcschr( L"([{", *p )){	//)
				// level up
				if( m_parenthesis_ptr < parptr_max ){
					m_parenthesis[ m_parenthesis_ptr ] = *p;
				}
				m_parenthesis_ptr++;
			}
			else if( wcschr( L")]}", *p )){	//)
				wchar_t op;
				switch( *p ){
					case L')': op = L'('; break;
					case L']': op = L'['; break;
					case L'}': op = L'{'; break;
					default:
						PleaseReportToAuthor(NULL, _T("COutlineErlang::ScanArgs ���m�̊���"));
						break;
				}
				// level down
				--m_parenthesis_ptr;
				while( 1 <= m_parenthesis_ptr && m_parenthesis_ptr < parptr_max ){
					if( m_parenthesis[ m_parenthesis_ptr ] != op ){
						// if unmatch then skip
						--m_parenthesis_ptr;
					}
					else{
						break;
					}
				}
				
				// check level
				if( m_parenthesis_ptr == 0 ){
					m_state = STATE_FUNC_ARGS_FIN;
					++p;
					return p;
				}
			}
			else if( *p == L',' && m_parenthesis_ptr == 1 ){
				++m_argcount;
			}
			else if( *p == L';' ){
				//	�Z�~�R�����͕����̕��̋�؂�D
				//	�p�����[�^���ɂ͌���Ȃ��̂ŁC��͂����s���Ă���
				//	���ʂ̕��Y�ꂪ�l������̂ŁC�d�؂蒼��
				m_state = STATE_NORMAL;
				return end;
			}
			else if( *p == L'.' ){
				//	�s���I�h�͎��̖������C�����_�Ƃ��Ďg����D
				if( p > head && ( L'0' <= p[-1] && p[-1] <= L'9' )){
					//	�����_��������Ȃ��̂ŁC���̂܂܂ɂ���
				}
				else {
					//	�����̓r���ŕ��������ꂽ�͉̂�͂����s���Ă���
					//	���ʂ̕��Y�ꂪ�l������̂ŁC�d�؂蒼��
					m_state = STATE_NORMAL;
					return end;
				}
			}
			else if( *p == L'"' ){
				quote = L'"';
			}
			else if( *p == L'\'' ){
				quote = L'\'';
			}
			else if( IS_COMMENT( *p )){
				return end;
			}
		}
	}
	return p;
}

/** �֐��{�̂̋�؂�C�܂��͏������̌��o

	@param[in] end buf����
	@param[in] p ��͂̌��݈ʒu
	
	�p�����[�^�{�̂�\���L��(->)���������̊J�n�L�[���[�h(when)��
	��������C�֐������Ƃ���D
	����ȊO�̏ꍇ�͊֐��ł͂Ȃ������ƍl����D
*/
const wchar_t* COutlineErlang::EnterCond( const wchar_t* end, const wchar_t* p )
{
	while( IS_SPACE( *p ) && p < end )
		p++;

	if( p >= end )
		return end;

	if( p + 1 < end && wcsncmp( p, L"->", 2 ) == 0){
		p += 2;
		m_state = STATE_FUNC_FOUND;
	}
	else if( p + 3 < end && wcsncmp( p, L"when", 4 ) == 0 ){
		m_state = STATE_FUNC_FOUND;
		p += 4;
	}
	else if( IS_COMMENT( *p )){
		return end;
	}
	else {
		m_state = STATE_NORMAL;
	}
	return end;
}

/** �s�̉��

	@param[in] buf �s(�擪����)
	@param[in] linelen �s�̒���
	@param[in] linenum �s�ԍ�
*/
bool COutlineErlang::parse( const wchar_t* buf, int linelen, CLogicInt linenum )
{
	const wchar_t* pos = buf;
	const wchar_t* const end = buf + linelen;
	if( m_state == STATE_FUNC_FOUND ){
		m_state = STATE_NORMAL;
	}
	if( m_state == STATE_NORMAL ){
		pos = ScanFuncName( buf, end, pos );
		if( m_state != STATE_NORMAL ){
			m_lnum = linenum;
		}
	}
	while( pos < end ){
		switch( m_state ){
			case STATE_FUNC_CANDIDATE_FIN:
				pos = EnterArgs( end, pos ); break;
			case STATE_FUNC_ARGS1:
				pos = ScanArgs1( end, pos ); break;
			case STATE_FUNC_ARGS:
				pos = ScanArgs( end, pos ); break;
			case STATE_FUNC_ARGS_FIN:
				pos = EnterCond( end, pos ); break;
			default:
				PleaseReportToAuthor(NULL, _T("COutlineErlang::parse Unknown State: %d"), m_state );
				break;
		}
		if( m_state == STATE_FUNC_FOUND ){
			build_arity( m_argcount );
			break;
		}
	}
	return m_state == STATE_FUNC_FOUND;
}

/** �֐����̌��� Arity (�����̐�)��t������

	@param[in] arity �����̐�
	
	�֐����̌��� /�p�����[�^�� �̌`�ŕ������ǉ�����D
	�o�b�t�@���s������ꍇ�͂ł���Ƃ���܂ŏ������ށD
	���̂��߁C10�ȏ�̈���������ꍇ�ɁC�����̐��̉��ʌ��������邱�Ƃ�����D
*/ 
void COutlineErlang::build_arity( int arity )
{
	int len = wcslen( m_func );
	const int buf_size = sizeof( m_func ) / sizeof( m_func[0]);
	wchar_t *p = &m_func[len];
	wchar_t numstr[12];
	
	if( len + 1 >= buf_size )
		return; // no room

	numstr[0] = L'/';
	_itow( arity, numstr + 1, 10 );
	wcsncpy( p, numstr, buf_size - len - 1 );
	m_func[ buf_size - 1 ] = L'\0';
}

/** Erlang �A�E�g���C�����

	@par ��ȉ���ƕ��j
	�֐��錾��1�J�����ڂ���L�ڂ���Ă���D
	
	
	@par ��̓A���S���Y��
	1�J�����ڂ��A���t�@�x�b�g�̏ꍇ: �֐��炵���Ƃ��ĉ�͊J�n / �֐�����ۑ�
	�X�y�[�X�͓ǂݔ�΂�
	( �𔭌������� ) �܂ň����𐔂���D���̏ꍇ����q�̊��ʂƕ�������l��
	-> �܂��� when ������Ί֐���`�ƌ��Ȃ�(���̍s�ɂ܂������Ă��ǂ�)
	�r�� % (�R�����g) �����ꂽ��s���܂œǂݔ�΂�
*/
void CDocOutline::MakeFuncList_Erlang( CFuncInfoArr* pcFuncInfoArr )
{

	COutlineErlang erl_state_machine;
	CLogicInt	nLineCount;

	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		CLogicInt nLineLen;

		const wchar_t* pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( erl_state_machine.parse( pLine, nLineLen, nLineCount )){
			/*
			  �J�[�\���ʒu�ϊ�
			  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
			  ��
			  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
			*/
			CLayoutPoint ptPosXY;
			m_pcDocRef->m_cLayoutMgr.LogicToLayout(
				CLogicPoint(CLogicInt(0), erl_state_machine.GetFuncLine()),
				&ptPosXY
			);
			pcFuncInfoArr->AppendData(
				erl_state_machine.GetFuncLine() + CLogicInt(1),
				ptPosXY.GetY2() + CLayoutInt(1),
				erl_state_machine.GetFuncName(),
				0,
				0
			);
		}
	}
}
