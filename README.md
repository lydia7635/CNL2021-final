# CNL2021-final
# ：）））））））
## TODO
- UpdatesManager.cpp
    - delete 某個 url-keyword pair from requests
        - or 每次要爬就固定從昱妤那邊抓 requests
    - 增加多個 server queue
- get_update.py
    - 判斷網站的 domain 來決定要用 RSS feed or crawler
    - 確認網站的格式正確，沒有爬到奇怪的東西

---
## UpdatesManager.cpp
### Usage
```cpp
/* Declaration */
UpdatesManager updates_manager;

/* Add request （新增一個 URL-keyword pair）*/
char url[MAX_URL_LEN] = "..."
char keyword[MAX_KEYWORD_LEN] = "..."
updates_manager.AddRequest(url, keyword);

/* Send request and get updates */
updates_manager.GetUpdates();
```


### 一些 data structure
```cpp
typedef struct update {
    UPDATE_TYPE type;              // -1: error, 0 or 1: normal
    time_t publish_time;
    char url[MAX_URL_LEN];
    char keyword[MAX_KEYWORD_LEN];
    char content[MAX_CONTENT_LEN]; // optional or error message
} Update;

typedef struct request {
    char url[MAX_URL_LEN];
    char keyword[MAX_KEYWORD_LEN];
} Request;

```
----
## get_update.py
```
python3 ./get_updates.py
```
- input file: `requests.txt`
- output file: `updates.txt`

### I/O File Format
- `requests.txt`
```python
1623808383 # last updated time in seconds
https://hackmd.io/nxUPv275TvOYYu8kwY_D7A corgi
...
```

- `updates.txt`
```python
<start>
         <url> https://hackmd.io/nxUPv275TvOYYu8kwY_D7A
         <publish_time> 1623808980
         <keyword> corgi
         <content> corgicorgicorgi
<end>
```


