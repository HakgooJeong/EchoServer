#pragma once

#include "Client.h"

namespace NetServer
{
	class ClientMgr 
	{
	public:
		ClientMgr();

		std::map<__int64, Client::ptrClient>& get();
		void update();
		void clear();

		void addProc(boost::function<void()>&& func);
		void add(Client::ptrClient client);
		
		void remove(Client::ptrClient client);
		void remove(__int64& key);
		void broadcast(void* data);
		Client::ptrClient find(std::string& guid);

		template <typename T>
		std::shared_ptr<T>	getImpl(__int64& key);

	private:
		std::map<__int64, Client::ptrClient>		clients;
		std::vector < boost::function<void()> >		procs;
		boost::mutex								mutex;
	};

	template <typename T>
	std::shared_ptr<T> ClientMgr::getImpl(__int64& key)
	{
		boost::unique_lock<boost::mutex> scoped_lock(mutex);

		Clients::const_iterator iter = clients.find(key);
		if (iter == clients.end())
		{
			return std::make_shared<std::shared_ptr<T> >(nullptr);
		}

		return std::dynamic_pointer_cast<T, Client>((*iter).second);
	}
}