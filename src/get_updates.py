import requests
import HackMD_crawler
import RSS_reader

if __name__ == '__main__':

    print("[Python] Executing get_updates.py")
    requests_file = open("request.txt", "r")

    reqs =  {}    # key: url, value: {"keywords":set(...), "clients":{}}

    updates = []  #
    
    line = requests_file.readline()
    last_updated_time = line
    last_updated_time = int(last_updated_time)    
    
    line = requests_file.readline()
    while line:
        client_id, n = line.split(' ')
        for i in range(int(n)):
            line = requests_file.readline()
            req = line.strip('\n').split(' ')
            url = req[0]
            if req[0] == '':
                continue
            if url not in reqs:
                reqs[url] = {}
                reqs[url]["keywords"] = set()
                reqs[url]["clients"] = {}
            if len(req) == 1 or (len(req) > 1 and req[1] == ''):
                reqs[url]["keywords"].add(None)
                if None not in reqs[url]["clients"]:
                    reqs[url]["clients"][None] = []
                reqs[url]["clients"][None] += [client_id]
            else:
                for k in req[1:]:
                    if len(k) > 0:
                        reqs[url]["keywords"].add(k)
                        if k not in reqs[url]["clients"]:
                            reqs[url]["clients"][k] = []
                        reqs[url]["clients"][k] += [client_id]
        line = requests_file.readline()
    requests_file.close()
        
    for url in reqs:
        print("last_update_time:", last_updated_time)
        if ("hackmd.io" in url):
            update = HackMD_crawler.get_hackmd_update(url, reqs[url]["keywords"], last_updated_time)
        elif ("medium.com" in url or "youtube.com" in url):
            update = RSS_reader.get_updates(url, reqs[url]["keywords"], last_updated_time)
        if update is not None:
            updates += update
    

    print("[Python] Complete checking, now dump all updates.")

    updates_file = open("updates.txt", "w")
    keys = ["url", "published_time", "title", "keyword", "content"]
    for update in updates:
        print("update", update)
        url = update["url"]
        keyword = None if "keyword" not in update else update["keyword"]
        for client_id in reqs[url]["clients"][keyword]:
            updates_file.write("<start>\n")
            updates_file.write("\t <client_id> {}\n".format(client_id))
            if "status" in update:
                if update["status"] == "error":
                    updates_file.write("\t <error> {}\n".format(update["error_msg"]))
            for k in keys:
                if k in update:
                    if update[k] != '':
                        updates_file.write("\t <{}> {}\n".format(k, update[k]))
            updates_file.write("<end>\n")
    updates_file.close()

    print("[Python] Exit.")
