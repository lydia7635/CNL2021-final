import requests
from . import HackMD_crawler
from . import RSS_reader

if __name__ == '__main__':

    print("[Python] Executing get_updates.py")
    requests_file = open("request.txt", "r")

    
    reqs =  {}    # key: url, value: {"keywords":set(...), "clients":{}}

    updates = []  #
    
    line = requests_file.readline()
    last_updated_time = line
    last_updated_time = int(last_updated_time)

    
    
    lines = requests_file.readlines()
    while line:
        client_id, n = line.split(' ')    
        for i in range(n):
            lines = requests_file.readlines()
            req = line.strip('\n').split(' ')
            url = req[0]
            if url not in reqs:
                reqs[url] = {}
                reqs[url]["keywords"] = set()
                reqs[url]["clients"] = {}
            if len(req) == 1:
                reqs[url]["keywords"].add(None)
                if None not in reqs[url]["clients"]:
                    reqs[url]["clients"][None] = []
                reqs[url]["clients"][None] += [client_id]
            else:
                for k in req[1:]:
                    req[url]["keywords"].add(k)
                    if k not in reqs[url]["clients"]:
                        reqs[url]["clients"][k] = []
                    reqs[url]["clients"][k] += [client_id]
        lines = requests_file.readlines()
    requests_file.close()
        
    for url in reqs:
        if ("hackmd.io" in url):
            update = HackMD_crawler.get_hackmd_update(url, reqs[url]["keywords"], last_updated_time)
        elif ("medium.com" in url or "youtube.com" in url):
            update = RSS_reader.get_updates(url, reqs[url]["keywords"], last_updated_time)
        if update is not None:
            updates += [update]
    

    print("[Python] Complete checking, now dump all updates.")

    updates_file = open("updates.txt", "w")
    keys = ["url", "publish_time", "keyword", "content"]
    for update in updates:
        url = update["url"]
        keyword = None if "keyword" not in update else update["keyword"]
        for client_id in reqs[url]["clients"][keyword]:
            updates_file.write("<start>\n")
            updates_file.write("\t <client_id> {}\n".format(client_id))
            if update["status"] == "error":
                updates_file.write("\t <error> {}\n".format(update["error_msg"]))
            for k in keys:
                if k in update:
                    updates_file.write("\t <{}> {}\n".format(k, update[k]))
            updates_file.write("<end>\n")
    updates_file.close()

    print("[Python] Exit.")
