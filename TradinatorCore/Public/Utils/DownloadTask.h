#pragma once

#include "AsyncTask.h"



class DownloadTask : public AsyncTask
{
public:
	struct DownloadRequest
	{
		std::string url;
		std::string file_path;
	};

	DownloadTask(std::function<void()> callback, std::string url, std::string file_path);

protected:
	virtual std::string GetHumanReadableDescription() const;

private:
	void DownloadFile(DownloadRequest request);

	std::string m_url;
	std::string m_file_path;
};

