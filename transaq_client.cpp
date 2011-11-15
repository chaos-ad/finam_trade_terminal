#include <deque>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/asio/signal_set.hpp>

#include "get_time.hpp"
#include "tcp_socket.hpp"
#include "ssl_socket.hpp"
#include "transaq_client.h"
#include "transaq_wrapper.h"

namespace
{
    void throw_error(const boost::system::error_code& err)
    {
        if (err)
        {
            std::clog << get_time() << ": Error: " << err.message() << std::endl;
            boost::system::system_error e(err);
            boost::throw_exception(e);
        }
    }
}

namespace transaq
{

    template <typename socket_type>
    class client
    {
    public :
        client(boost::program_options::variables_map const& options)
            : send_size(0)
            , recv_size(0)
            , sigset(service, SIGINT, SIGTERM)
            , socket(service, options)
        {
            std::string dllpath = options["dllpath"].as<std::string>();
            wrapper::start(boost::bind(&client::handle_data, this, _1), dllpath);
            sigset.async_wait(boost::bind(&client::handle_signal, this, _1, _2));
            start_read();
        }

        ~client()
        {
            wrapper::stop();
        }

        void run()
        {
            service.run();
        }

        void stop()
        {
            service.stop();
        }

    private :
        void start_read()
        {
            boost::asio::async_read
            (
                socket.get(),
                boost::asio::buffer(&recv_size, sizeof(recv_size)),
                boost::bind(&client::handle_read_size, this, _1)
            );
        }

        void handle_read_size(boost::system::error_code err)
        {
            throw_error(err);
            recv_size = ntohl(recv_size);
            recv_buffer.resize(recv_size, 0);
            boost::asio::async_read
            (
                socket.get(),
                boost::asio::buffer(recv_buffer),
                boost::bind(&client::handle_read_data, this, _1)
            );
        }

        void handle_read_data(boost::system::error_code err)
        {
            throw_error(err);
            std::string command(recv_buffer.data(), recv_buffer.size());
            std::clog << get_time() << ": Sending command:" << std::endl << command << std::endl << std::endl;
            write( transaq::wrapper::send_command(command) );
            start_read();
        }

        void handle_signal(boost::system::error_code const& err, int signal_number)
        {
            if(!err)
            {
                stop();
            }
        }

        bool handle_data(std::string const& data)
        {
            // called from different thread
            service.post(boost::bind(&client::write, this, data));
            return true;
        }

        void write(std::string const& data)
        {
            send_buffer.push_back(data);
            start_write();
        }

        void start_write()
        {
            if (!send_size && !send_buffer.empty())
            {
                send_size = htonl(send_buffer.front().size());
                boost::array<boost::asio::const_buffer, 2> buffers;
                buffers[0] = boost::asio::buffer(&send_size, sizeof(send_size));
                buffers[1] = boost::asio::buffer(send_buffer.front());
                boost::asio::async_write
                (
                    socket.get(),
                    buffers,
                    boost::bind(&client::handle_write, this, _1)
                );
            }
        }

        void handle_write(boost::system::error_code err)
        {
            throw_error(err);
            send_buffer.pop_front();
            send_size = 0;
            start_write();
        }

    private :
        uint32_t                        recv_size;
        std::vector<char>               recv_buffer;

        uint32_t                        send_size;
        std::deque<std::string>         send_buffer;

        boost::asio::io_service         service;
        boost::asio::signal_set         sigset;

        socket_type                     socket;
    };

    /////////////////////////////////////////////////////////////////////////

    void start(boost::program_options::variables_map & options)
    {
        if (!options["secure"].as<bool>())
        {
            client<tcp_socket> c(options); c.run();
        }
        else
        {
            client<ssl_socket> c(options); c.run();
        }
    }

}
