#pragma once
#include "../../Core/NativeWindow.h"
#include <wrl.h>
#include <dxgidebug.h>
#include <vector>
#include <string>


namespace GDX11
{
	class DxgiInfoManager
	{
	private:
		Microsoft::WRL::ComPtr<IDXGIInfoQueue> m_dxgiInfoQueue = nullptr;
		uint64_t m_next = 0;

	public:
		DxgiInfoManager();
		~DxgiInfoManager() = default;

		DxgiInfoManager(const DxgiInfoManager&) = delete;
		DxgiInfoManager& operator=(const DxgiInfoManager&) = delete;

		void Set();
		std::vector<std::string> GetMessages() const;
	};
}