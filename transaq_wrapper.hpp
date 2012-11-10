#pragma once

#include <string>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

/////////////////////////////////////////////////////////////////////////////

namespace transaq {

class wrapper
{
private:
	typedef boost::function<bool (std::string const&)> callback_t;

public:
	wrapper(callback_t const& callback, std::string const& libpath, std::string const& logdir, int32_t loglevel);
	~wrapper();

public:
    std::string send_command(std::string const& cmd);

private:
	class impl;
	boost::shared_ptr<impl> impl_;
};

/////////////////////////////////////////////////////////////////////////////

} // namespace transaq
