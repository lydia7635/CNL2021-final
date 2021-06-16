import HackMD_crawler

if __name__ == '__main__':

    print("[Python] Executing get_updates.py")
    keys = ["url", "publish_time", "keyword", "content"]
    updates = []
    requests_file = open("request.txt", "r")

    last_updated_time = requests_file.readline()
    last_updated_time = int(last_updated_time)

    lines = requests_file.readlines()
    for line in lines:
        print("[Python] Check for update...")
        args = line.strip('\n').split(' ')
        if len(args) == 1:
            url = args
        elif len(args) == 2:
            url, keyword = args
        update = HackMD_crawler.get_hackmd_update(url, keyword, last_updated_time)
        if update is not None:
            updates += [update]
    requests_file.close()

    print("[Python] Complete checking, now dump all updates.")

    updates_file = open("updates.txt", "w")
    for update in updates:
        updates_file.write("<start>\n")
        if update["status"] == "error":
            updates_file.write("\t <error> {}\n".format(update["error_msg"]))
        for k in keys:
            if k in update:
                updates_file.write("\t <{}> {}\n".format(k, update[k]))
        updates_file.write("<end>\n")
    updates_file.close()

    print("[Python] Exit.")
