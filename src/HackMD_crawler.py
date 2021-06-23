import requests
import time
import datetime as dt
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
import os
import json


def get_updated_time(time_html):
    month_to_num = { "Jan":1, "Feb":2, "Mar":3, "Apr":4,  "May":5,  "Jun":6,
                     "Jul":7, "Aug":8, "Sep":9, "Oct":10, "Nov":11, "Dec":12 }
    updated_time = time_html.split('text-uppercase ui-status-lastchange')[1].split('title')[1].split("\"")[1].replace(',', '').split(" ")
    if "月" in updated_time[0]:
        year = updated_time[0].split('年')[0]
        month = updated_time[0].split('年')[1].split('月')[0]
        day = updated_time[0].split('年')[1].split('月')[1].split('日')[0]
        hour, minute = updated_time[1].split(':')
        time_tuple = dt.datetime(int(year), int(month), int(day), int(hour), int(minute), 0).timetuple()

    else:
        _, month, day, year, hr_min, noon = updated_time
        hour, minute = hr_min.split(':')
        month = month_to_num[month]
        time_tuple = dt.datetime(int(year), int(month), int(day), int(hour), int(minute), 0).timetuple()
    return time.mktime(time_tuple) # in seconds


            
def get_summary(content, keyword):
    summary = None
    paragraph = content.split('\n')
    summaries = []
    for p in paragraph:
        if keyword in p:
            tokens = p.split(keyword)
            for i in range(len(tokens)-1):
                summaries += [ "..." + tokens[i] + keyword + tokens[i+1] + "..." ]
    if not os.path.isfile("HackMD_cache.json"):
        with open("HackMD_cache.json", "w") as f:
            json.dump({url: summaries}, f)

    f = open("HackMD_cache.json", 'r')
    url_keyword_content = json.load(f)  
    if url in url_keyword_content:
        for s in summaries:
            if s not in url_keyword_content[url]:
                summary = s if summary == None else summary
                url_keyword_content[url] += [s]
    else:
        url_keyword_content[url] = summaries
    
    f.close()
    with open("HackMD_cache.json", "w") as f:
        json.dump(url_keyword_content, f)
    
    return summary


# status: error (status code is not 200) 
#         empty (website had not updated) -> return None
#         success

# add multiple keywords

def get_hackmd_update(url, keywords, last_update_time):
    r = requests.get(url)
    if r.status_code != 200: # error handling
        update = {}
        update["error_msg"] = "Error, status code: {}".format(r.status_code)
        update["status"] = "error"
        return [ [update]*len(keywords) ]
    options = webdriver.ChromeOptions()
    options.add_argument('--headless')
    options.add_argument('--disable-gpu') 
    driver = webdriver.Chrome("./chromedriver", options=options)
    driver.get(url)
    time_html = driver.execute_script("return document.getElementsByClassName('ui-lastchangeuser dn')[0].innerHTML")
    updated_time = get_updated_time(time_html)
    if updated_time < last_update_time:
        return []
    updates = []
    title = driver.title
    for keyword in keywords:
        if keyword == None:
            updates += [ {"url": url, "publised_time": updated_time, "status": "success", "title": title} ]
        else:
            content = driver.find_element_by_class_name("ui-view-area").text
            summary = get_summary(content, keyword)
            if summary != None:
                updates += [ {"url": url, "publised_time": updated_time, "status": "success", "content": summary, "title": title} ]
             
    return updates


def get_hackmd_update_v2(url, keyword, last_update_time):
    update = {"url": url}
    if keyword != None:
        update["keyword"] = keyword
    r = requests.get(url)
    if r.status_code != 200: # error handling
        update["error_msg"] = "Error, status code: {}".format(r.status_code)
        update["status"] = "error"
        return update
    update["status"] = "success"
    options = webdriver.ChromeOptions()
    options.add_argument('--headless')
    options.add_argument('--disable-gpu') 
    driver = webdriver.Chrome("./chromedriver", options=options)
    driver.get(url)
    time_html = driver.execute_script("return document.getElementsByClassName('ui-lastchange text-uppercase')[0].innerHTML")
    
    updated_time = get_updated_time(time_html)
    if updated_time < last_update_time:
        return None
    update["published_time"] = updated_time
    if keyword != None:
        content = driver.find_element_by_class_name("ui-view-area").text

    return update
