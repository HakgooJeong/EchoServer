#pragma once 

#include "stdafx.h"

#include "NetSession.h"

namespace NetSession
{
	void TcpSession::run()
	{
		do_read();
	}

    void TcpSession::do_read()
    {
		auto buffer = std::make_shared<Buffer>();
        auto self(shared_from_this());
        socket.async_read_some(boost::asio::buffer(buffer.get(), Message::MAX_LENGTH),
            [this, self, buffer](boost::system::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					Message msg(reinterpret_cast<void*>(buffer.get()->c_array()));
					self->push(msg);

					printf("packet Inserted [key:%lld][size:%d]\n", self->getKey(), length);

					do_read();
				}
			}
		);
    }

    void TcpSession::do_write(Message& msg)
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket, boost::asio::buffer(msg.get(), msg.size()),
            [this, self](boost::system::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					printf("packet sended [Key:%lld][size:%d]\n", self->getKey(), length);
				}
			}
		);
    }

	void TcpSession::push(Message& msg)
	{
		boost::unique_lock<boost::mutex> scoped_lock(mutex);
		queue->push_back(std::move(msg));
	}

	size_t TcpSession::count()
	{
		boost::unique_lock<boost::mutex> scoped_lock(mutex);

		return queue->size();
	}

	Message&& TcpSession::pop()
	{
		boost::unique_lock<boost::mutex> scoped_lock(mutex);

		Message&& msg = std::move(queue->front());
		queue->pop_front();

		return std::move(msg);
	}

	bool TcpSession::isConnected() const
	{
		return (1 == connected);
	}

	void TcpSession::close(const char* msg)
	{
		if (0 == connected)
		{
			return;
		}

		try
		{
			socket.close();
			InterlockedExchange(&connected, 0);
			diconnector(shared_from_this(), msg);
		}
		catch (std::exception& e)
		{
			printf("[SessionCloseError] %s\n", e.what());
		}
	}

	unsigned __int64 TcpSession::getSessionTime()
	{
		return sessionTimeTick;
	}

	void TcpSession::refreshSessionTime()
	{
		sessionTimeTick = GetTickCount64();
	}

	void TcpSession::SessionTime0()
	{
		sessionTimeTick = 0;
	}
}