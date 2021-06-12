import feedparser
import webbrowser
import requests


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

def get_updates(url):
    RSS_feed = get_RSS_feed(url)
    feed = feedparser.parse(RSS_feed)
    feed_entries = feed.entries

    updates = []
    for entry in feed.entries:
        # print(entry.keys())
        # print(type(entry))
        dic = {}
        dic['title'] = None
        dic['link'] = None
        dic['published_time'] = None
        dic['summary'] = None
    
        if ('title' in entry):
            dic['title'] = entry.title
        if ('link' in entry):
            dic['link'] = entry.link
        if ('published_parsed' in entry):
            dic['published_time'] = entry.published_parsed
        if ('summary' in entry):
            dic['summary'] = entry.summary
        updates.append(dic)

    return updates

# url = "https://www.youtube.com/channel/UC2ggjtuuWvxrHHHiaDH1dlQ"
# updates = get_updates(url)
# print (updates[0])