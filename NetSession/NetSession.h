#pragma once

#include <deque>

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "Message.h"

namespace NetSession
{
    using boost::asio::ip::tcp;
    using boost::asio::ip::udp;

    class TcpSession : public std::enable_shared_from_this<TcpSession>
    {
	public:

		typedef std::shared_ptr<TcpSession>	ptrTcpSession;
		typedef std::deque<Message> Message_queue;
		typedef std::function<void(ptrTcpSession, const char*)> funcDisconnect;

    public:

        TcpSession( tcp::socket socket, 
					funcDisconnect _diconnector,
                    __int64 key = 0)
                        :	socket(std::move(socket)),
							diconnector(_diconnector),
							key(key),
							queue(new TcpSession::Message_queue())
        {
			InterlockedExchange(&connected, 1);
			sessionTimeTick = GetTickCount64();
        }

		bool isConnected() const;
		void run();
		__int64& getKey() { return key; }
		void close(const char* msg);

		void do_write(Message& msg);
		Message&& pop();
		size_t count();

		unsigned __int64 getSessionTime();
		void refreshSessionTime();
		void SessionTime0();

    private:
		volatile long connected = false;

        void do_read();
		void TcpSession::push(Message& msg);

        tcp::socket socket;
		boost::array<char, Message::Message::MAX_LENGTH> buffer;
		std::shared_ptr<TcpSession::Message_queue> queue;
		__int64 key = 0;
		boost::mutex mutex;		

		funcDisconnect diconnector;

		unsigned __int64 sessionTimeTick;
    };
}