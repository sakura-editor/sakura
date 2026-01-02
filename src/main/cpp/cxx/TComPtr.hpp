/*!	@file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

namespace cxx {

/*!
 * MSVCのコンパイラCOM対応クラス _com_ptr_t を模倣するテンプレート
 * 
 * 必要最小限の機能のみ実装する
 */
template<typename Interface>
class TComPtr final {
private:
	using InterfaceHolder = Microsoft::WRL::ComPtr<Interface>;
    InterfaceHolder m_pInterface = nullptr;

	using Me = TComPtr<Interface>;

public:
	TComPtr() = default;
    ~TComPtr() = default;

	/* implicit */ TComPtr(std::nullptr_t) noexcept {}

	template<typename U>
	explicit TComPtr(_In_opt_ U* other) {
		*this = other;
	}

	/*!
	 * インターフェースを作成する
	 */
    HRESULT CreateInstance(
		const CLSID& rclsid,
		IUnknown* pOuter = nullptr,
		DWORD dwClsContext = CLSCTX_INPROC_SERVER
	) noexcept
    {
		// 以前のインターフェースを解放する
		m_pInterface.Reset();

		// SDK関数を使ってインスタンスを作成する
		Microsoft::WRL::ComPtr<IUnknown> pUnknown;
		_com_util::CheckError(CoCreateInstance(rclsid, pOuter, dwClsContext, IID_PPV_ARGS(&pUnknown)));

		// 生成したIUnknownから希望のインターフェースを取得する
		_com_util::CheckError(pUnknown.As(&m_pInterface));

		return S_OK;
    }

	Interface* Detach()
    {
		return m_pInterface.Detach();
	}

    Interface* GetInterfacePtr() const
    {
        return m_pInterface.Get();
    }

    Interface& GetInterfaceRef() const
    {
        if (m_pInterface) {
            return *GetInterfacePtr();
        }

		throw _com_error(E_POINTER);
    }

	Me& operator = (std::nullptr_t) noexcept {
		m_pInterface.Reset();
		return *this;
	}

	template<typename U>
	Me& operator = (_In_opt_ U* other) {
		using OtherType = Microsoft::WRL::ComPtr<U>;
		if (other) {
			OtherType pOther(other);
			_com_util::CheckError(pOther.As(&m_pInterface));
			return *this;
		}
		return operator = (nullptr);
	}

	/*!
	 * インターフェース型への暗黙変換を行う
	 */
	/* implicit */ operator Interface* () const noexcept
    {
        return GetInterfacePtr();
    }

	Interface& operator*() const
    {
		return GetInterfaceRef();
	}

	Interface** operator&() noexcept
    {
        return m_pInterface.ReleaseAndGetAddressOf();
    }

    Interface* operator->() const
    {
        if (m_pInterface) {
            return GetInterfacePtr();
        }

		throw _com_error(E_POINTER);
    }
};

} // namespace cxx
