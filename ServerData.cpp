#include <iostream>
#include <time.h>
#include <map>
#include <limits>

using namespace std;
using std::string;

#include "ServerData.h"


string get_server(string s) {
	string d = "/";
	size_t pos = 0;
	string token = "";
	while ((pos = s.find(d)) != string::npos) {
		token = s.substr(0, pos);
		if (token.find(".") != string::npos) {
			break;
		}
		s.erase(0, pos + d.length());
	}
	return token;
}

ServerData::ServerData() {
	this->servers = new std::map<string, std::map<string, struct INFO>*>;
	this->time = 0;
}

ServerData::~ServerData() {
	delete (this->servers);
}

void ServerData::insert(string url, struct INFO update) {
	string server_url = get_server(url);
	std::map<string, struct INFO>* server_websites;
	// add a new server
	if (servers->find(server_url) == servers->end()) {
		server_websites = new std ::map<string, struct INFO>;
		servers->insert(std::pair<string, std::map<string, struct INFO>*>(server_url, server_websites));
	}
	else {	// server exists
		server_websites = (*servers)[server_url];
	}
	update.time_stamp = time;

	// add a new website
	if (server_websites->find(url) == server_websites->end())
		server_websites->insert(std::pair<string, struct INFO>(url, update));
	else {	// website updates
		(*server_websites)[url] = update;
	}

#ifdef DEBUG
	cout << "title: " << (*(*servers)[server_url])[url].title << endl;
#endif

	if ( server_websites->size() > MAX_WEBSITE_NUM) {
		removeOld(server_websites);
	}
	time++;
}

void ServerData::removeOld(std::map<string, struct INFO>* server_websites) {
	unsigned long long int min_time_stamp = std::numeric_limits<unsigned long long int>::max();
	string remove_candidate;

	for (const auto& website : *server_websites) {
		if (min_time_stamp > website.second.time_stamp) {
			min_time_stamp = website.second.time_stamp;
			remove_candidate = website.first;
		}
	}

	server_websites->erase(remove_candidate);

#ifdef DEBUG
	cout << "remove: " << remove_candidate << endl;
#endif
}