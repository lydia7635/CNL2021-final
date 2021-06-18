#include "info.h"
#define MAX_WEBSITE_NUM 64

class ServerData
{
	std::map<string, std::map<string, struct INFO>*>* servers;
	unsigned long long int time;
public:
	ServerData();
	~ServerData();

	void insert(string url, struct INFO a);
	void removeOld(std::map<string, struct INFO>* server_websites);
};