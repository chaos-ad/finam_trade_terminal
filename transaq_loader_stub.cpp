#include "transaq_loader.h"

#include <deque>
#include <boost/thread.hpp>

typedef bool  (*callback_t)(char* data);
typedef char* (*send_command_t)(const char* data);
typedef bool  (*free_memory_t)(char* data);
typedef bool  (*set_callback_t)(callback_t callback);

class fake_server
{
public :
    fake_server()
        : stopping(0)
        , thread(boost::bind(&fake_server::start, this))
    {}

    ~fake_server()
    {
        stop();
        thread.join();
    }

    void start()
    {
        for(;!stopping;)
        {
            boost::this_thread::sleep(boost::posix_time::seconds(1));
            if(callback && !messages.empty())
            {
                char * ptr = new char[messages.front().size() + 1];
                strncpy(ptr, messages.front().c_str(), messages.front().size() + 1);
                callback(ptr);
                messages.pop_front();
            }
        }
    }

    void stop()
    {
        stopping = true;
    }

    bool set_callback(callback_t new_callback)
    {
        callback = new_callback;
    }

    char* send_command(const char* data)
    {
        std::string str(data);
        messages.push_back(str);
        char * ptr = new char[3];
        strncpy(ptr, "ok", 3);
        return ptr;
    }

    bool free_memory(char * data)
    {
        delete [] data;
        return true;
    }

private :
    bool                        stopping;
    boost::thread               thread;

    std::deque<std::string>     messages;
    callback_t                  callback;
};

static boost::shared_ptr<fake_server>& instance()
{
    static boost::shared_ptr<fake_server> instance;
    return instance;
}

bool free_memory(char * data)
{
    return instance()->free_memory(data);
}

char* send_command(const char* data)
{
    return instance()->send_command(data);
}

bool set_callback(callback_t callback)
{
    return instance()->set_callback(callback);
}

namespace transaq {
namespace loader {

void* load_library(std::string const& path)
{
    if(!instance())
        instance().reset(new fake_server());
    return 0;
}

void unload_library(void * library)
{
    if (instance())
        instance().reset();
}

void* load_function(void * library, std::string const& name)
{
    if (name == "FreeMemory")
    {
        return reinterpret_cast<void*>(free_memory);
    }
    if (name == "SendCommand")
    {
        return reinterpret_cast<void*>(send_command);
    }
    if (name == "SetCallback")
    {
        return reinterpret_cast<void*>(set_callback);
    }
    return 0;
}

} // namespace loader
} // namespace transaq
