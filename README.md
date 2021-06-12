# CNL2021-final

## RSS_reader.py
### usage:
```
import RSS_reader
updates = RSS_reader.get_updates(url)
```

### return format:
* A list
	* updates = [u1, u2, ..., u_n]
* u1: A dictionary
	* keys: title, link, published_time, summary
	* the value may be None, e.g., Some RSS_feed doesn't have key 'summary'
