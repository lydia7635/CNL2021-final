# CNL2021-final - Pub-Sub Implementation
## Requirements
- libcurl4-openssl-dev
- `pip install -r requirements.txt`
    - feedparser
    - requests
    - selenium
- chromedriver
    ```shell
    $ wget https://chromedriver.storage.googleapis.com/2.41/chromedriver_linux64.zip
    $ unzip chromedriver_linux64.zip
    ```

## Usage
### Compile
產生 broker 與 client 。
```shell
$ make
```
### Execute
```shell
$ ./broker <port>
$ ./client <broker IP> <broker port>
```
接下來依照提示輸入指令即可。