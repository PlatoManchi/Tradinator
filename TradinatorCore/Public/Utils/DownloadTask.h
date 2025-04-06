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

private:
	void DownloadFile(DownloadRequest request);
};

