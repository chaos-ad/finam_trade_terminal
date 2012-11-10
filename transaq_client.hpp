#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>

namespace transaq {

class client
{
public:
	client(boost::program_options::variables_map const& options);
	void run();
	void stop();

private:
	class impl;
	boost::shared_ptr<impl> impl_;
};

} // namespace transaq
