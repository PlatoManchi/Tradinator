#include "Utils/Log.h"

#include <chrono>
#include <iostream>


Log::Log()
	: m_folder_path("")
{
}

Log& Log::GetInstance()
{
	static Log instance;

	return instance;
}


void Log::SetFolderPath(std::string folder_path)
{
#ifdef _LOG_TO_FILE_
	if (m_file_stream.good())
	{
		m_file_stream.close();
	}
#endif
	m_folder_path = folder_path;

	OpenLogFile();
}

void Log::OpenLogFile()
{
#ifdef _LOG_TO_FILE_
	const std::chrono::zoned_time cur_time
	{
		std::chrono::current_zone(), // may throw
		std::chrono::system_clock::now()
	};

	std::string file_name = std::format("{}/{}.log", m_folder_path, std::format("{:%d-%m-%Y %OH-%OM-%OS}", cur_time));
	m_file_stream.open(file_name.c_str());
#endif
}

void Log::Write(const std::string& data)
{
#ifdef _LOG_TO_FILE_
	{
		std::lock_guard lock(m_log_to_file_mutex);
		m_file_stream << data << std::endl;
	}
#endif // _LOG_TO_FILE_

#ifdef _LOG_TO_CONSOLE_
	{
		std::lock_guard lock(m_log_to_console_mutex);
		std::cout << data << std::endl;
	}
#endif // _LOG_TO_CONSOLE_

}


template <class T>
Log& Log::operator<<(const T& x)
{
#ifdef _LOG_TO_FILE_
	{
		std::lock_guard lock(log.m_log_to_file_mutex);
		m_file_stream << x;
	}
#endif // _LOG_TO_FILE_

#ifdef _LOG_TO_CONSOLE_
	{
		std::lock_guard lock(log.m_log_to_console_mutex);
		std::cout << x;
	}
#endif // _LOG_TO_CONSOLE_

	
	return *this;
}
