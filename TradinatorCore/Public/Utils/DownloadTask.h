#pragma once

#include "AsyncTask.h"

#include <vector>

class DownloadTask : public AsyncTask
{
public:
	struct DownloadRequest
	{
		std::string url;
		std::string file_path;
	};

	// copy and move sementics
	DownloadTask(const DownloadTask& other) = default;
	DownloadTask(DownloadTask&& other) noexcept = default;
	DownloadTask& operator = (const DownloadTask& other) = default;
	DownloadTask& operator = (DownloadTask&& other) noexcept = default;

	DownloadTask(std::function<void()> callback, std::string url, std::string file_path);

protected:
	size_t m_attempts;
	// in seconds
	std::vector<size_t> m_retry_intervels = {10, 30, 60, 120, 240, 300};

	virtual std::string GetHumanReadableDescription() const;

private:
	void DownloadFile(DownloadRequest request);

	std::string m_url;
	std::string m_file_path;
};

