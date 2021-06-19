#include "../inc/header.h"

using namespace std;

// ********************
// ** error handling **
// ********************

string successfulReturn(string website_string)
{
    long res_code = 0;
    CURL *curl = curl_easy_init();
    if(curl) {
        CURLcode res;
        curl_easy_setopt(curl, CURLOPT_URL, website_string.c_str());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
        res = curl_easy_perform(curl);
        if(res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res_code);

            // if getting website successfully
            if(res_code == (long)200 || res_code == (long)201)
                return website_string;
            
            // if redirect
            char *new_url = NULL;
            curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &new_url);
            if(new_url)
                website_string = new_url;
            else
                website_string = "";
        }
        else {
            website_string = "";
        }
        curl_easy_cleanup(curl);
    }
    else
        website_string = "";

    return website_string;
}

bool availableService(string website_string)
{
    regex reg("^https?:\\/\\/((hackmd\\.io\\/)|(www\\.youtube\\.com\\/c(|hannel)\\/)|(medium\\.com\\/@)|([A-Za-z0-9\\-]+\\.medium\\.com))[^\\ ]*$");
    if(regex_match(website_string, reg))
        return true;
    else
        return false;
}

string returnValidUrl(string website_string)
{
    website_string = successfulReturn(website_string);
    if(website_string.empty())
        return "";
    else
        return availableService(website_string)? website_string : "";
}