#include "ft_irc.hpp"

int	main(int ac, char **av)
{
	try
	{
		if (ac != 3)
			throw std::invalid_argument(ERR_SVR_USAGE(std::string(av[0])));

		Server	ircserv(av[1], av[2]);

		ircserv.start();
	}
	catch (const std::exception& e)
	{
		MSG_ERR(e.what());
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}