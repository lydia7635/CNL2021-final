import feedparser
import webbrowser
import requests
import json
import time
# import re

def get_RSS_feed(url):
    url_file = requests.get(url)
    filename = 'source_code.txt'
    ff = open(filename, "wb").write(url_file.content)
    
    file = open(filename)
    found = False
    RSS_fead = ''
    line = file.readline()
    while (line and not found):
        split_lines = line.replace('<', ' ').replace('>', ' ').replace(',', ' ').split()
        line_idx = 0
        for line_idx in range(len(split_lines)):
            if "RSS" in split_lines[line_idx]:
                found = True
                RSS_feed = split_lines[line_idx+1].replace('"', ' ').split()
                break
        line = file.readline()
    file.close()
    return RSS_feed[1]

# def write_to_file(updates):
#     f_out = open('updates.txt', 'w', encoding='utf-8')

#     for u in updates:
#         for key in u.keys():
#             f_out.write(key)
#             f_out.write("\n")
#             if (key == "published_time"):
#                 f_out.write(str(u[key]))
#             else:
#                 f_out.write(u[key])
#             f_out.write("\n")
#     f_out.close()

def get_updates(url, keywords, last_updated_time):
    MAX_SUMMARY_LEN = 256
    RSS_feed = get_RSS_feed(url)
    feed = feedparser.parse(RSS_feed)
    feed_entries = feed.entries

    updates = []
    for entry in feed.entries:
        # print(entry.keys())
        # print(type(entry))
        dic = {}
        dic['url'] = url
        dic['published_time'] = ''
        dic['keyword'] = ''
        dic['content'] = ''   # link + summary
        dic['title'] = ''
    
        if ('published_parsed' in entry):
            secs = int(time.mktime(entry.published_parsed))
            dic['published_time'] = secs # published
            # print("sec = ", dic['published_time'], ", asctime = ", time.asctime(time.localtime(secs)))
        if ('title' in entry):
            dic['title'] = entry.title
        if ('link' in entry):
            dic['content'] = "link: " + entry.link
        if ('summary' in entry):
            summary = entry.summary
            summaries = summary.split("</p>")
            s = summaries[0].replace("<p>", "").replace("</p>", "").replace("</strong>", "").replace("<strong>", "").replace("<h3>", "").replace("</h3>", "").replace("</pre>", "").replace("<pre>", "")
            if (len(s) > MAX_SUMMARY_LEN):
                s = s[0:MAX_SUMMARY_LEN]
            dic['content'] = dic['content'] + ", summary: " + s
        
        # drop earlier updates
        if (last_updated_time > secs):
            continue

        for keyword in keywords:
            new_dic = dict(dic)
            new_dic['keyword'] = keyword
            # keyword matches
            if (keyword == None):
                del new_dic['keyword']
                updates.append(new_dic)
            elif (((keyword in dic['title'])) or ((keyword in dic['content']))):
                updates.append(new_dic)

    # write_to_file(updates)

    return updates

# url = "https://allaboutdataanalysis.medium.com/"
# get_updates(url, [None], 0)

