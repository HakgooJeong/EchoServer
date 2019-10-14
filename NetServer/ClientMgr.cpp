#include "stdafx.h"

#include "ClientMgr.h"

namespace NetServer
{
	ClientMgr::ClientMgr()
	{
		clear();
	}

	std::map<__int64, Client::ptrClient>& ClientMgr::get()
	{
		return clients;
	}

	void ClientMgr::update()
	{
		for each (auto proc in procs)
		{
			proc();
		}
	}

	void ClientMgr::clear()
	{
		boost::unique_lock<boost::mutex> scoped_lock(mutex);

		for (std::map<__int64, Client::ptrClient>::iterator iter = clients.begin();
			iter != clients.end();
			++iter)
		{
			if ((*iter).second->isConnected())
			{
				(*iter).second->getSession()->close("Clear Clients");
			}
		}
	}

	void ClientMgr::addProc(boost::function<void()>&& func)
	{
		procs.push_back(func);
	}

	void ClientMgr::add(Client::ptrClient client)
	{
		boost::unique_lock<boost::mutex> scoped_lock(mutex);

		clients.insert(std::make_pair(client->getKey(), client));
	}

	void ClientMgr::remove(Client::ptrClient client)
	{
		boost::unique_lock<boost::mutex> scoped_lock(mutex);

		if ( nullptr == client )
		{
			return;
		}

		clients.erase(client->getKey());
	}

	void ClientMgr::remove(__int64& key)
	{
		boost::unique_lock<boost::mutex> scoped_lock(mutex);

		clients.erase(key);
	}

	void ClientMgr::broadcast(const void* data)
	{
		boost::unique_lock<boost::mutex> scoped_lock(mutex);

		for (std::map<__int64, Client::ptrClient>::iterator iter = clients.begin();
			iter != clients.end();
			++iter)
		{
			(*iter).second->send(data);
		}
	}

	Client::ptrClient ClientMgr::find(const std::string& guid)
	{
		boost::unique_lock<boost::mutex> scoped_lock(mutex);

		for (std::map<__int64, Client::ptrClient>::iterator iter = clients.begin();
			iter != clients.end();
			++iter)
		{
			if ( (false == (*iter).second->getGuid().empty()) && 0 == guid.compare((*iter).second->getGuid()))
			{
				return (*iter).second;
			}
		}

		return nullptr;
	}
}