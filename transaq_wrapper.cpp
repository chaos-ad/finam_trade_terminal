#include "transaq_loader.h"
#include "transaq_wrapper.h"

#include <boost/shared_ptr.hpp>

#ifdef WIN32
#define STDCALL __stdcall
#else
#define STDCALL
#endif

/////////////////////////////////////////////////////////////////////////////

namespace transaq {
namespace wrapper {

class impl
{
private :
    typedef bool  (STDCALL *internal_callback_t)(char* data);
    typedef char* (STDCALL *send_command_t)(const char* data);
    typedef bool  (STDCALL *free_memory_t)(char* data);
    typedef bool  (STDCALL *set_callback_t)(internal_callback_t callback);

private :
    impl(callback_t const& callback, std::string const& libpath)
        : callback(callback)
        , library(loader::load_library(libpath), loader::unload_library)
        , free_memory_fn(loader::load_fun<free_memory_t>(library.get(), "FreeMemory"))
        , send_command_fn(loader::load_fun<send_command_t>(library.get(), "SendCommand"))
    {
        set_callback_t set_callback = loader::load_fun<set_callback_t>(library.get(), "SetCallback");
        set_callback(&impl::handle_data);
    }

public :
    static void stop()
    {
        if (instance()) {
            instance().reset();
        } else {
            throw std::runtime_error("wrapper is not running");
        }
    }

    static void start(const callback_t& callback, std::string const& path)
    {
        if(!instance().get()) {
            instance().reset(new impl(callback, path));
        } else {
            throw std::runtime_error("wrapper already started");
        }
    }

    static std::string send_command(std::string const& cmd)
    {
		boost::shared_ptr<char> result
		(
			instance()->send_command_fn(cmd.c_str()),
			instance()->free_memory_fn
		);
		return std::string(result.get());
    }

    static bool STDCALL handle_data(char * data)
    {
        boost::shared_ptr<char> pdata(data, instance()->free_memory_fn);
        return instance()->callback(std::string(pdata.get()));
    }

private :
    static boost::shared_ptr<impl>&  instance()
    {
        static boost::shared_ptr<impl> instance;
        return instance;
    }

private :
    callback_t                      callback;
    boost::shared_ptr<void>         library;
    free_memory_t                   free_memory_fn;
    send_command_t                  send_command_fn;
};

/////////////////////////////////////////////////////////////////////////////

void start(const callback_t& callback, std::string const& libpath)
{
    impl::start(callback, libpath);
}

std::string send_command(const std::string& cmd)
{
    return impl::send_command(cmd);
}

void stop()
{
    impl::stop();
}

/////////////////////////////////////////////////////////////////////////////

}
}
