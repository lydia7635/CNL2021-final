import feedparser
import webbrowser
import requests
import json
import time

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

def get_updates(url, keyword, last_updated_time):
    RSS_feed = get_RSS_feed(url)
    feed = feedparser.parse(RSS_feed)
    feed_entries = feed.entries

    updates = []
    for entry in feed.entries:
        # print(entry.keys())
        # print(type(entry))
        dic = {}
        dic['url'] = url
        dic['published_time'] = None
        dic['keyword'] = keyword
        dic['content'] = None   # link + summary
        dic['title'] = None
    
        if ('published_parsed' in entry):
            secs = int(time.mktime(entry.published_parsed))
            dic['published_time'] = secs # published
            # print("sec = ", dic['published_time'], ", asctime = ", time.asctime(time.localtime(secs)))
        if ('title' in entry):
            dic['title'] = entry.title
        if ('link' in entry):
            dic['content'] = "link: " + entry.link
        if ('summary' in entry):
            # dic['summary'] = entry.summary
            dic['content'] = dic['content'] + ", summary: " + entry.summary
        
        # keyword does not match
        if ((not (keyword in dic['title'])) and (not (keyword in dic['content']))):
            continue
        
        # drop earlier updates
        if (last_updated_time > secs):
            continue
        updates.append(dic)

    # write_to_file(updates)

    return updates

# url = "https://www.youtube.com/channel/UC2ggjtuuWvxrHHHiaDH1dlQ"
# get_updates(url, '', 1623388326)

