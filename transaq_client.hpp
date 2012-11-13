#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>

#include "base_client.hpp"
#include "base_strategy.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace transaq {

/////////////////////////////////////////////////////////////////////////////

class client : public base_client
{
public:
    typedef boost::shared_ptr<base_strategy> strategy_ptr_t;

public:
    client(boost::program_options::variables_map const& options);
    ~client();

    void run();
    void stop();

    void add_strategy(std::string const& name, strategy_ptr_t const& strategy);
    void del_strategy(std::string const& name);

public:
    virtual void subscribe_ticks(bool filter, std::map<int32_t, int32_t> const& args);

private:
    class impl;
    boost::shared_ptr<impl> impl_;
};
    
/////////////////////////////////////////////////////////////////////////////

} // namespace transaq
