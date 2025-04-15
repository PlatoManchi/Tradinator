#pragma once

#include <string>
#include <fstream>
#include <mutex>


#if 1
#define _LOG_TO_FILE_
#else
#endif

#if 1
#define _LOG_TO_CONSOLE_
#else
#endif


class Log
{
public:
    static Log& GetInstance();
    void SetFolderPath(std::string folder_path);
    void Write(const std::string& data);

    Log(Log& other) = delete;
    void operator=(const Log&) = delete;

    template <class T>
    Log& operator<<(const T& x);

private:
	Log();
    void OpenLogFile();
    
    std::string m_folder_path;
	


#ifdef _LOG_TO_FILE_
    std::ofstream m_file_stream;
    std::mutex m_log_to_file_mutex;
#endif // _LOG_TO_FILE_
#ifdef _LOG_TO_CONSOLE_
    std::mutex m_log_to_console_mutex;
#endif // _LOG_TO_CONSOLE_
};
