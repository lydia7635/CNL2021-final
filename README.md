# CNL2021-final
## broker: client 端
👩: 表示 client 需要實作的部份

### TO-DO

- [x] client 登入/ 註冊
- [x] 修改/查看規則
- [x] ~~產生 thread~~ 及時丟訊息給 client

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

### 修改/查看規則

* 新增規則
    * website 一定要填
    * keyword 可不填，在 broker 以 `*` 儲存
    * [ ] 確認 website 是否合法
* 刪除規則
    * website 不填即代表刪除所有規則
    * keyword 不填代表刪除該 website 下所有規則
    * 都有填只會刪除對應規則
* 印出規則
    * 在 broker 端顯示
    * [ ] 討論封包格式

### 從 queue 取出並丟訊息給 client