# CNL2021-final
## requirement
* libcurl4-openssl-dev
* C++11

## broker: client 端
👩: 表示 client 需要實作的部份

### TO-DO

- [x] client 登入/ 註冊
- [x] 修改/查看規則
- [x] ~~產生 thread~~ 丟訊息給 client

### client 登入/註冊
  
👩 開啟 client 後需先登入/註冊  
輸入一組帳號密碼，在 broker 判斷帳號代表的 client 是否存在與 is_valid
* 存在
    * is_verified == true (登入)
        * password 正確
            * is_online == true (重複登入)
                * 👩 client 接收到 STAT_MULTIPLE_LOGIN
            * is_online == false (登入成功)
                * 👩 client 接收到 STAT_SUCCESSFUL_LOGIN
                * broker: is_online = true
                * broker: fd_to_client 填入
        * password 錯誤
            * 👩 client 接收到 STAT_WRONG_PASSWORD
            * 沒有限制輸入錯誤密碼的數量
    * is_verified == false (註冊確認密碼回傳)
        * is_online == true && socket != client->locate_socket (重複登入)
            * 👩 client 接收到 STAT_MULTIPLE_LOGIN
        * password 正確(註冊成功)
            * 👩 client 接收到 STAT_SUCCESSFUL_SIGNUP
            * broker: is_verified = true
            * broker: is_online = false
            * broker: fd_to_client 留空
            * 👩 client 需重新登入
        * password 錯誤 (註冊驗證密碼錯誤)
            * 👩 client 接收到 STAT_UNSUCCESSFUL_SIGNUP
            * broker: 刪除這個 unverified client
            * broker: fd_to_client 留空
            * 👩 client 需重新註冊
* 不存在 (註冊)
    * 👩 client 接收到 STAT_CHECK_PASSWORD
    * broker: 新增一個 client 資料
    * broker: is_online = true, is_verified = false
    * broker: fd_to_client 填入
    * 如果 client 關閉連線
        * broker: 刪除這個 unverified client

### 修改/查看規則或訂閱內容結果

* 新增規則 (`RULE_CONTROL_TYPE RULE_INSERT`)
    * website 一定要填
    * keyword 可不填，表示訂閱整個網站的操作
    * 先訂閱整個網站，再訂閱特定關鍵字 => 訂閱特定關鍵字
    * 先訂閱某網站中特定關鍵字，再訂閱該網站（無關鍵字） => 訂閱整個網站
    * 網站和關鍵字不要有空格
    * [x] 確認 website 是否合法
        * 支援的網址系列：HackMD 、 YouTube 、 Medium
        * https://hackmd.io/
        * https://www.youtube.com/c/
        * https://www.youtube.com/channel/
        * https://xxxxx.medium.com/
        * https://medium.com/@xxxx
        * 會支援網址重導向
        * 需安裝 libcurl4-openssl-dev
* 刪除規則 (`RULE_CONTROL_TYPE RULE_DELETE`)
    * website 不填即代表刪除所有規則
    * keyword 不填代表刪除該 website 下所有規則
    * 都有填只會刪除對應規則
* 印出規則 (`RULE_CONTROL_TYPE RULE_LIST`)
    * 在 broker 端顯示
    * [x] 討論封包格式
    * 送給 client 端的資料
        * 一個封包最多裝 10 筆規則
        * 如果 `is_last == true` ，表示是最後一個封包。可能會有 0 ~ 9 筆規則
* 印出符合規則的訂閱內容 (`RULE_CONTROL_TYPE QUERY_CONTENT`)
    * 下一個 section

### 從 queue 取出並丟訊息給 client
* client 會收到 website, topic, content
* 只有要求的時候才會丟訊息出去

## Reference
### libcurl
* [c++ - curl.h no such file or directory - Stack Overflow](https://stackoverflow.com/questions/11471690/curl-h-no-such-file-or-directory/11471743)
* [c - hide curl_easy_perform - Stack Overflow](https://stackoverflow.com/questions/2814988/hide-curl-easy-perform)
* [CURLINFO_RESPONSE_CODE](https://curl.se/libcurl/c/CURLINFO_RESPONSE_CODE.html)
* [CURLINFO_REDIRECT_URL](https://curl.se/libcurl/c/CURLINFO_REDIRECT_URL.html)