#include <std_include.hpp>
#include "com.hpp"

namespace utils::com
{
	namespace
	{
		class _
		{
		public:
			_()
			{
				if(FAILED(CoInitialize(nullptr)))
				{
					throw std::runtime_error("Failed to initialize the component object model");
				}
			}

			~_()
			{
				CoUninitialize();
			}
		} __;
	}

	bool select_folder(std::string& out_folder, const std::string& title, const std::string& selected_folder)
	{
		IFileOpenDialog* file_dialog = nullptr;
		if(FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&file_dialog))))
		{
			throw std::runtime_error("Failed to create co instance");
		}

		const auto $1 = gsl::finally([file_dialog]()
		{
			file_dialog->Release();
		});

		DWORD dwOptions;
		if(FAILED(file_dialog->GetOptions(&dwOptions)))
		{
			throw std::runtime_error("Failed to get options");
		}

		if(FAILED(file_dialog->SetOptions(dwOptions | FOS_PICKFOLDERS)))
		{
			throw std::runtime_error("Failed to set options");
		}

		std::wstring wide_title(title.begin(), title.end());
		if(FAILED(file_dialog->SetTitle(wide_title.data())))
		{
			throw std::runtime_error("Failed to set title");
		}

		if (!selected_folder.empty())
		{
			file_dialog->ClearClientData();

			std::wstring wide_selected_folder(selected_folder.begin(), selected_folder.end());
			for (auto& chr : wide_selected_folder)
			{
				if (chr == L'/')
				{
					chr = L'\\';
				}
			}

			IShellItem* shell_item = nullptr;
			if(FAILED(SHCreateItemFromParsingName(wide_selected_folder.data(), NULL, IID_PPV_ARGS(&shell_item))))
			{
				throw std::runtime_error("Failed to create item from parsing name");
			}

			if (FAILED(file_dialog->SetDefaultFolder(shell_item)))
			{
				throw std::runtime_error("Failed to set default folder");
			}
		}

		const auto result = file_dialog->Show(nullptr);
		if(result == HRESULT_FROM_WIN32(ERROR_CANCELLED))
		{
			return false;
		}

		if (FAILED(result))
		{
			throw std::runtime_error("Failed to show dialog");
		}

		IShellItem* result_item = nullptr;
		if(FAILED(file_dialog->GetResult(&result_item)))
		{
			throw std::runtime_error("Failed to get result");
		}

		const auto $2 = gsl::finally([result_item]()
		{
			result_item->Release();
		});

		PWSTR raw_path = nullptr;
		if(FAILED(result_item->GetDisplayName(SIGDN_FILESYSPATH, &raw_path)))
		{
			throw std::runtime_error("Failed to get path display name");
		}

		const auto $3 = gsl::finally([raw_path]()
		{
			CoTaskMemFree(raw_path);
		});

		std::wstring result_path = raw_path;
		out_folder = std::string(result_path.begin(), result_path.end());

		return true;
	}
}
