#include <windows.h>
#include "transaq_wrapper.hpp"

#ifdef WIN32
#define STDCALL __stdcall
#else
#define STDCALL
#endif

/////////////////////////////////////////////////////////////////////////////

namespace transaq {

class wrapper::impl
{
public:
    typedef wrapper::callback_t callback_t;
    typedef bool  (STDCALL *internal_callback_t)(char* data, void*);
    typedef char* (STDCALL *send_command_t)(const char* data);
    typedef bool  (STDCALL *free_memory_t)(char* data);
    typedef bool  (STDCALL *set_callback_t)(internal_callback_t callback, void * userdata);
    typedef char* (STDCALL *uninitialize_t)();
    typedef char* (STDCALL *initialize_t)(const char* dir, int level);
    typedef char* (STDCALL *set_log_level_t)(int level);

public:
    impl(callback_t const& callback, std::string const& libpath, std::string const& logdir, int32_t loglevel)
        : library_(library::load(libpath), library::unload)
		, callback_fn_(callback)
        , initialize_fn_(library::load_fun<initialize_t>(library_.get(), "Initialize"))
        , uninitialize_fn_(library::load_fun<uninitialize_t>(library_.get(), "UnInitialize"))
        , set_log_level_fn_(library::load_fun<set_log_level_t>(library_.get(), "SetLogLevel"))
        , set_callback_fn_(library::load_fun<set_callback_t>(library_.get(), "SetCallbackEx"))
        , free_memory_fn_(library::load_fun<free_memory_t>(library_.get(), "FreeMemory"))
        , send_command_fn_(library::load_fun<send_command_t>(library_.get(), "SendCommand"))
    {
		boost::shared_ptr<char> error(initialize_fn_(logdir.c_str(), loglevel), free_memory_fn_);
        if (error)
        {
            throw std::runtime_error(error.get());
        }

		internal_callback_t int_callback = &impl::handle_data_raw;
        set_callback_fn_(int_callback, static_cast<void*>(this));
    }

    ~impl()
    {
        uninitialize_fn_();
    }

public:
    std::string send_command(std::string const& cmd)
    {
		boost::shared_ptr<char> result(send_command_fn_(cmd.c_str()), free_memory_fn_);
		return std::string(result.get());
    }

private:
    static bool STDCALL handle_data_raw(char * data, void * ptr)
    {
        return static_cast<impl*>(ptr)->handle_data(data);
    }

    bool handle_data(char * data)
    {
        boost::shared_ptr<char> pdata(data, free_memory_fn_);
        return callback_fn_(std::string(pdata.get()));
    }

private:
	struct library 
	{
		static void unload(void * library) 
		{
			FreeLibrary(static_cast<HMODULE>(library));
		}

		static void* load(std::string const& path)
		{
			HMODULE module = LoadLibraryA(path.c_str());
			if (!module)
			{
				throw std::runtime_error("library not found");
			}
			return static_cast<void*>(module);
		}

		static void* load_fun_raw(void * library, std::string const& name)
		{
			void * address = GetProcAddress(static_cast<HMODULE>(library), name.c_str());
			if (!address)
			{
				throw std::runtime_error("function not found");
			}
			return address;
		}

		template <class fun_t>
		static fun_t load_fun(void * library, std::string const& name)
		{
			return reinterpret_cast<fun_t>(load_fun_raw(library, name));
		}
	};

private:
    boost::shared_ptr<void>         library_;
	callback_t                      callback_fn_;
    initialize_t                    initialize_fn_;
    uninitialize_t                  uninitialize_fn_;
    set_log_level_t                 set_log_level_fn_;
    set_callback_t                  set_callback_fn_;
    free_memory_t                   free_memory_fn_;
    send_command_t                  send_command_fn_;
};

/////////////////////////////////////////////////////////////////////////////

wrapper::wrapper(callback_t const& callback, std::string const& libpath, std::string const& logdir, int32_t loglevel)
    : impl_(new impl(callback, libpath, logdir, loglevel))
{}

wrapper::~wrapper() {}

std::string wrapper::send_command(std::string const& cmd)
{
    return impl_->send_command(cmd);
}

/////////////////////////////////////////////////////////////////////////////

} // namespace transaq

