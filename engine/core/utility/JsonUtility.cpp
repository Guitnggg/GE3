#include "JsonUtility.h"

#include <fstream>
#include <system_error>

namespace
{
	void SetError(std::string* destination, const std::string& message)
	{
		if (destination)
		{
			*destination = message;
		}
	}
}

namespace JsonUtility
{
	bool Load(const std::filesystem::path& path, Json& json, std::string* errorMessage)
	{
		SetError(errorMessage, {});

		std::ifstream file(path, std::ios::binary);
		if (!file)
		{
			SetError(errorMessage, "JSONファイルを開けませんでした。");
			return false;
		}

		try
		{
			Json loadedJson;
			file >> loadedJson;
			json = std::move(loadedJson);
			return true;
		}
		catch (const nlohmann::json::exception& exception)
		{
			SetError(errorMessage, exception.what());
			return false;
		}
	}

	bool Save(const std::filesystem::path& path, const Json& json, int indent, std::string* errorMessage)
	{
		SetError(errorMessage, {});

		const std::filesystem::path parentPath = path.parent_path();
		if (!parentPath.empty())
		{
			std::error_code errorCode;
			std::filesystem::create_directories(parentPath, errorCode);
			if (errorCode)
			{
				SetError(errorMessage, "JSONファイルの保存先フォルダを作成できませんでした: " + errorCode.message());
				return false;
			}
		}

		std::ofstream file(path, std::ios::binary | std::ios::trunc);
		if (!file)
		{
			SetError(errorMessage, "JSONファイルを作成できませんでした。");
			return false;
		}

		try
		{
			file << json.dump(indent);
		}
		catch (const nlohmann::json::exception& exception)
		{
			SetError(errorMessage, exception.what());
			return false;
		}

		if (!file)
		{
			SetError(errorMessage, "JSONファイルへの書き込みに失敗しました。");
			return false;
		}

		return true;
	}
}
