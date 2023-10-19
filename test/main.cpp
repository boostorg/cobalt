#include <boost/cobalt.hpp>
#include <boost/cobalt/main.hpp>

namespace cobalt = boost::cobalt;

auto cobalt_task() -> cobalt::task<int>
{
  co_return 0;
}


cobalt::main co_main(int argc, char * argv[])
{
  constexpr std::size_t nb = 10000000;

  for (std::size_t i = 0;i <nb; ++i)
  {

    co_await cobalt_task();
  }
  co_return 0;  
}
