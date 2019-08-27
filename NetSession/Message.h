#pragma once

#include <iostream>
#include <climits>

namespace NetSession
{
#pragma pack(push, 1)   

    class Message : public std::enable_shared_from_this<Message>
    {
    public:
        enum { HEADER_LENGTH = 4 }; 
		enum { SIZE_LENGTH = 2 };
		enum { CMD_LENGTH = 2 };
        enum { MAX_LENGTH = USHRT_MAX };

    private:
        char data[MAX_LENGTH];
        std::size_t data_length;

    public:
        Message()
            : data_length(0)
        {
            memset(data, 0, MAX_LENGTH);
        }

		Message(const void* _data)
			: data_length(0)
		{
			memset(data, 0, MAX_LENGTH);

			size_t size = ((unsigned short*)_data)[0];

			if (size + HEADER_LENGTH > MAX_LENGTH)
				assert(false);

			memcpy_s(data, size + HEADER_LENGTH, _data, size + HEADER_LENGTH);
			data_length += size;
		}

		void operator=(const Message& rhs)
		{
			memset(data, 0, MAX_LENGTH);

			operator<<(rhs.body());
			close(rhs.cmd());
		}

		// ToKnow : 헤더에 정보를 기록하고 패킷을 완성시킨다. 마지막에 반드시 호출해야함
		void close(unsigned short cmd)
		{
			char header[HEADER_LENGTH] = "";
			memcpy_s(header, SIZE_LENGTH, &data_length, SIZE_LENGTH);
			memcpy_s(header+SIZE_LENGTH, SIZE_LENGTH, &cmd, SIZE_LENGTH);

			memcpy_s(data, HEADER_LENGTH, header, HEADER_LENGTH);
		}

		template <typename T>
        Message&& operator << (const T& _data)
        {
			size_t size = sizeof(_data);

			if ( size > MAX_LENGTH - HEADER_LENGTH - data_length )
                assert(false);

            memcpy_s(data + HEADER_LENGTH + data_length, size, &_data, size);
			data_length += size;

            return std::move(*this);
        }

		void write (const void* _data, unsigned short length)
		{
			if ( length > MAX_LENGTH - HEADER_LENGTH - data_length )
				assert(false);

			memcpy_s(data + HEADER_LENGTH + data_length, length, _data, length);
			data_length += length;
		}

		const unsigned short size() const
		{
			return ((unsigned short*)data)[0] + HEADER_LENGTH;
		}

		const unsigned short cmd() const
		{
			return ((unsigned short*)data)[1];
		}

        const void* body() const
        {
            return data + HEADER_LENGTH;
        }

		const void* tail(size_t in, size_t& out) const
		{
			out = data_length - HEADER_LENGTH - in;
			return data + HEADER_LENGTH + in;
		}

		const void* get() const
		{
			return data;
		}
    };

#pragma pack(pop)  
}