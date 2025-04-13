#include "Utils/DownloadTask.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <format>

#include <curl/curl.h>


DownloadTask::DownloadTask(std::function<void()> callback, std::string url, std::string file)
	: AsyncTask()
    , m_url(url)
    , m_file_path (file)
{
    m_callback = callback;
    m_human_readable_description = "Downloading ";

    m_worker_list.push_back(std::function<void()>(
        [url, file, this]() 
        {
            DownloadFile({url, file});
        }
    ));
}

size_t write_data(void* ptr, size_t size, size_t nmemb, void* stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE*)stream);
    return written;
}

void DownloadTask::DownloadFile(DownloadRequest request)
{
    std::chrono::time_point start = std::chrono::steady_clock::now();

    CURL* curl_handle;
    FILE* pagefile;

    curl_global_init(CURL_GLOBAL_ALL);

    /* init the curl session */
    curl_handle = curl_easy_init();

    /* set URL to get here */
    curl_easy_setopt(curl_handle, CURLOPT_URL, request.url.c_str());

    /* Switch on full protocol/debug output while testing */
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, false);

    /* disable progress meter, set to 0L to enable it */
    //curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

    /* send all data to this function  */
    // Need this to prevent curl from using stdout as output
    //curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

    /* open the file */
    fopen_s(&pagefile, request.file_path.c_str(), "wb");
    if (pagefile) {

        /* write the page body to this file handle */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);

        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, true);
        // Install the callback function
        //curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, std::bind(&DownloadTask::progress_func, this));

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "application/json");
        curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);

        /* get it! */
        CURLcode res = curl_easy_perform(curl_handle);

        int prtall = 0;
        if (CURLE_OK == res) {
            curl_off_t val;

            /* check for bytes downloaded */
            //res = curl_easy_getinfo(curl_handle, CURLINFO_SIZE_DOWNLOAD_T, &val);
            //if ((CURLE_OK == res) && (val > 0))
            //    printf("Data downloaded: %lu bytes.\n", (unsigned long)val);
            //
            ///* check for total download time */
            //res = curl_easy_getinfo(curl_handle, CURLINFO_TOTAL_TIME_T, &val);
            //if ((CURLE_OK == res) && (val > 0))
            //    printf("Total download time: %lu.%06lu sec.\n",
            //        (unsigned long)(val / 1000000), (unsigned long)(val % 1000000));
            //
            ///* check for average download speed */
            //res = curl_easy_getinfo(curl_handle, CURLINFO_SPEED_DOWNLOAD_T, &val);
            //if ((CURLE_OK == res) && (val > 0))
            //    printf("Average download speed: %lu kbyte/sec.\n",
            //        (unsigned long)(val / 1024));

            if (prtall) {
                /* check for Name resolution time */
                //res = curl_easy_getinfo(curl_handle, CURLINFO_NAMELOOKUP_TIME_T, &val);
                //if ((CURLE_OK == res) && (val > 0))
                //    printf("Name lookup time: %lu.%06lu sec.\n",
                //        (unsigned long)(val / 1000000), (unsigned long)(val % 1000000));
                //
                ///* check for connect time */
                //res = curl_easy_getinfo(curl_handle, CURLINFO_CONNECT_TIME_T, &val);
                //if ((CURLE_OK == res) && (val > 0))
                //    printf("Connect time: %lu.%06lu sec.\n",
                //        (unsigned long)(val / 1000000), (unsigned long)(val % 1000000));
            }
        }
        else {
            std::cout << "Error while fetching " << request.url.c_str() << "\n    " << curl_easy_strerror(res) << std::endl;
            fprintf(stderr, "Error while fetching '%s' : %s\n",
                request.url.c_str(), curl_easy_strerror(res));
        }

        /* close the header file */
        fclose(pagefile);
    }

    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);

    curl_global_cleanup();
}


std::string DownloadTask::GetHumanReadableDescription() const
{
    return std::format("{} \n  URL: \'{}\'\n  File: {}"
        , m_human_readable_description
        , m_url
        , m_file_path);
}