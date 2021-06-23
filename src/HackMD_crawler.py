import requests
import time
import datetime as dt
from selenium import webdriver
from selenium.webdriver.chrome.options import Options


def get_updated_time(time_html):
    month_to_num = { "Jan":1, "Feb":2, "Mar":3, "Apr":4,  "May":5,  "Jun":6,
                     "Jul":7, "Aug":8, "Sep":9, "Oct":10, "Nov":11, "Dec":12 }
    updated_time = time_html.split('text-uppercase ui-status-lastchange')[1].split('title')[1].split("\"")[1].replace(',', '').split(" ")
    _, month, day, year, hr_min, noon = updated_time
    hour, minute = hr_min.split(':')
    month = month_to_num[month]
    time_tuple = dt.datetime(int(year), int(month), int(day), int(hour), int(minute), 0).timetuple()
    return time.mktime(time_tuple) # in seconds

def get_summary(content, keyword):
    pass

# status: error (status code is not 200) 
#         empty (website had not updated) -> return None
#         success

# add multiple keywords

def get_hackmd_update(url, keyword, last_update_time):
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
    time_html = driver.execute_script("return document.getElementsByClassName('ui-lastchangeuser dn')[0].innerHTML")
    updated_time = get_updated_time(time_html)
    if updated_time < last_update_time:
        return None
    update["publish_time"] = updated_time
    if keyword != None:
        content = driver.find_element_by_class_name("ui-view-area").text
        update["keyword"] = keyword
    return [update]


