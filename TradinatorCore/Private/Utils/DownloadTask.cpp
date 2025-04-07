#include "Utils/DownloadTask.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <format>

#include <curl/curl.h>


DownloadTask::DownloadTask(std::function<void()> callback, std::string url, std::string file)
	: AsyncTask("Downloading", callback)
    , m_url(url)
    , m_file_path (file)
{
    m_worker_list.push_back(std::function<void()>(
        [url, file, this]() 
        {
            DownloadFile({url, file});
        }
    ));
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, FILE* userp)
{
    size_t written = fwrite(contents, size, nmemb, userp);
    std::cout << "Written : " << written << " bytes" << std::endl;
    std::cout << contents << std::endl;
    // return -1 to cancel the download
    return written;
}

int progress_func(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded)
{
    // It's here you will write the code for the progress message or bar
    // ensure that the file to be downloaded is not empty
    // because that would cause a division by zero error later on
    //std::cout << "TotalToDownload: " << TotalToDownload << "  NowDownloaded : " << NowDownloaded << std::endl;
    if (TotalToDownload <= 0.0) {
        return 0;
    }

    // how wide you want the progress meter to be
    int totaldotz = 40;
    double fractiondownloaded = NowDownloaded / TotalToDownload;
    // part of the progressmeter that's already "full"
    int dotz = (int)round(fractiondownloaded * totaldotz);

    // create the "meter"
    int ii = 0;
    printf("%3.0f%% [", fractiondownloaded * 100);
    // part  that's full already
    for (; ii < dotz; ii++) {
        printf("=");
    }
    // remaining part (spaces)
    for (; ii < totaldotz; ii++) {
        printf(" ");
    }
    // and back to line begin - do not forget the fflush to avoid output buffering problems!
    printf("]\r");
    fflush(stdout);
    // if you don't return 0, the transfer will be aborted - see the documentation
    return 0;
}

void DownloadTask::DownloadFile(DownloadRequest request)
{
    std::cout << "Download Request: " << std::endl << "URL : " << request.url << std::endl << "Filename: " << request.file_path << std::endl;

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
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);

    /* open the file */
    fopen_s(&pagefile, request.file_path.c_str(), "wb");
    if (pagefile) {

        /* write the page body to this file handle */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);

        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, FALSE);
        // Install the callback function
        curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, progress_func);

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "application/json");
        curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);

        /* get it! */
        CURLcode res = curl_easy_perform(curl_handle);

        int prtall = 0;
        if (CURLE_OK == res) {
            curl_off_t val;

            /* check for bytes downloaded */
            res = curl_easy_getinfo(curl_handle, CURLINFO_SIZE_DOWNLOAD_T, &val);
            if ((CURLE_OK == res) && (val > 0))
                printf("Data downloaded: %lu bytes.\n", (unsigned long)val);

            /* check for total download time */
            res = curl_easy_getinfo(curl_handle, CURLINFO_TOTAL_TIME_T, &val);
            if ((CURLE_OK == res) && (val > 0))
                printf("Total download time: %lu.%06lu sec.\n",
                    (unsigned long)(val / 1000000), (unsigned long)(val % 1000000));

            /* check for average download speed */
            res = curl_easy_getinfo(curl_handle, CURLINFO_SPEED_DOWNLOAD_T, &val);
            if ((CURLE_OK == res) && (val > 0))
                printf("Average download speed: %lu kbyte/sec.\n",
                    (unsigned long)(val / 1024));

            if (prtall) {
                /* check for name resolution time */
                res = curl_easy_getinfo(curl_handle, CURLINFO_NAMELOOKUP_TIME_T, &val);
                if ((CURLE_OK == res) && (val > 0))
                    printf("Name lookup time: %lu.%06lu sec.\n",
                        (unsigned long)(val / 1000000), (unsigned long)(val % 1000000));

                /* check for connect time */
                res = curl_easy_getinfo(curl_handle, CURLINFO_CONNECT_TIME_T, &val);
                if ((CURLE_OK == res) && (val > 0))
                    printf("Connect time: %lu.%06lu sec.\n",
                        (unsigned long)(val / 1000000), (unsigned long)(val % 1000000));
            }
        }
        else {
            fprintf(stderr, "Error while fetching '%s' : %s\n",
                request.url.c_str(), curl_easy_strerror(res));
        }

        /* close the header file */
        fclose(pagefile);
    }

    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);

    curl_global_cleanup();

    std::chrono::time_point end = std::chrono::steady_clock::now();
    std::cout << "Download took " << std::to_string(std::chrono::duration<double>(end - start).count()) << "s" << std::endl;
}


std::string DownloadTask::GetHumanReadableDescription() const
{
    return std::format("{} \'{}\' and saving at {}"
        , m_human_readable_description
        , m_url
        , m_file_path);
}