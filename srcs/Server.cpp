#include "Server.hpp"

/* #region Signal handling */
bool	stopSignalReceived = false;

void	handleSignal(int signal)
{
	(void)signal;
	msg_log(MSG_SVR_EXIT_SIG);
	stopSignalReceived = true;
}
/* #endregion */

/* #region Contructor/Destructor */

/**
 * @brief Construct a new Server:: Server object
 * 
 * @param port which port to connect to the server
 * @param password password to allow you to connect to the server
 */
Server::Server(std::string const &port, std::string const &password):
	_password(password), _serverSocket(ERROR), _nbOfClients(0)
{
	setEndian();
	setPort(port);
	memset(&_addrServer, 0, sizeof(_addrServer));
	initCommands(this, _commands);
	msg_log(MSG_SVR_CREATED);
}

/**
 * @brief Destroy the Server:: Server object
 */
Server::~Server()
{
	msg_log(MSG_SVR_SHUTDOWN);

	// clear all users
	disconnectAllClients();

	// clear all commands
	deleteCommands(_commands);

	msg_log(MSG_SVR_END);
}
/* #endregion */

/* #region PRIVATE */

/**
 * @brief Check if port given is correct and set the attribute
 * @param port Given port to check and convert
 * @note A valid port must be a number between 1024 and 65535 (included).
 */
void	Server::setPort(std::string const &port)
{
	char* endptr;
    errno = 0;
    long int tmp_port = std::strtol(port.c_str(), &endptr, 10);

	try
	{
		if ((errno == ERANGE && (tmp_port == LONG_MAX || tmp_port == LONG_MIN)) || (errno != 0 && tmp_port == 0))
			throw std::out_of_range("Port out of valid range: " + port);
		if (*endptr != '\0')
			throw std::invalid_argument("unvalid port: " + port);
		if (tmp_port < 0 || tmp_port > PORT_MAX)
			throw std::out_of_range("Port out of valid range: " + port);
		if (tmp_port < PORT_MIN)
			throw std::out_of_range("Port is forbidden: " + port);
        _port = static_cast<unsigned int>(tmp_port);
		MSG_DEV(MSG_DEV_PORT_IS_SET, _port);
	}
	catch (const std::exception& e)
	{
		throw std::invalid_argument(std::string(ERR_SVR_PORT) + e.what());
	}
}

/**
 * @brief Detect the system endianness and set the attribute to BIG_ENDIAN or LITTLE_ENDIAN
 * @note Endian must be determined in order to communicate correctly in networks
 */
void	Server::setEndian()
{
	int	n = 1;
	if( *(char *) & n == 1)
		_endian = LITTLE_ENDIAN;
	else
		_endian = BIG_ENDIAN;
}

/**
 * @brief Creation and configuration of the server's socket
 * @note - 1) in socket() : AF_INET for IPv4 protocol, SOCK_STREAM since we use TCP protocol, and 0 is because there is only one protocol available for UNIX domain.
 * @note - 2) in setsockopt() : Allow socket fd to be reusable and prevent it from blocking the port (https://beej.us/guide/bgnet/html/#getaddrinfoprepare-to-launch).
 * @note - 3) IP addr setup: AF_INET for IPv4 protcol, INNADDR_ANY for listen on every available network interface.
 * @note - 4) The listening socket is set ton non blocking mode. Sockets from incoming connections will inherit this state.
 * @note - 5) When socket is created, it needs to be assigned a "name". Thats what bind() will do.
 */
void	Server::setServerSocket()
{
	// 1) IPv4 socket creation with NONBLOCK flag
	if ((_serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == ERROR)
		throw std::runtime_error("unable to create socket");
	MSG_DEV(MSG_DEV_SVR_SOC_CREATED, _serverSocket);

	// 2) socket is allowed to be reusable
	int	enable = 1;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == ERROR)
		throw std::runtime_error("unable to configure socket to 'reusable'");
	MSG_DEV(MSG_DEV_SVR_SOC_SETOPT, _serverSocket);

	// 3) IP address/port setup
	_addrServer.sin_family = AF_INET;
	_addrServer.sin_addr.s_addr = INADDR_ANY;
	if (_endian == BIG_ENDIAN)
		_addrServer.sin_port = _port;
	else
		_addrServer.sin_port = htons(_port);
	MSG_DEV(MSG_DEV_SVR_IP_SETUP, inet_ntoa(_addrServer.sin_addr));

	// 4) set socket to non blocking mode
	if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) == ERROR)
		throw std::runtime_error("unable to set socket to non blocking mode");
	MSG_DEV(MSG_DEV_SVR_SOC_NONBLOCK, "");

	// 5) Binding socket to addr/port
	if (bind(_serverSocket, (const struct sockaddr *)&_addrServer, sizeof(_addrServer)) == ERROR)
		throw std::runtime_error(std::string("unable to bind server socket to address") + strerror(errno));
	MSG_DEV(MSG_DEV_SVR_SOC_IPv4_BINDED, "");

	// 6) Enabling listening mode for this socket
	if (listen(_serverSocket, MAX_CLIENTS) == ERROR)
		throw std::runtime_error("unable to enable listening mode for the server socket");
	MSG_DEV(MSG_DEV_SVR_SOC_LISTEN, "");
}

/**
 * @brief Checks the result of poll()
 * 
 * @param numEventsReady number of events "ready" that poll() returned
 */
void	Server::handlePollEvents()
{
	// launch poll()
	if (poll(_fds.data(), _fds.size(), TIMEOUT) == ERROR && stopSignalReceived == false)
		throw std::runtime_error("poll error: " + std::string(strerror(errno)));

	// Search in each pollfd if events happened
	for (size_t i = 0; i < _fds.size(); i++)
	{
		//event detected
		if (_fds[i].revents & POLLIN)
		{
			if (_fds[i].fd == _serverSocket)
				handleNewConnection();
			else
				handleIncomingData(_fds[i].fd);
		}
	}
}

/**
 * @brief Handle the connection from a new client
 * 
 * @param socket_fd the FD where poll
 */
void	Server::handleNewConnection()
{
	// 1) Creation of the client address/socket
	sockaddr_in		clientAddr = {};
	socklen_t		clientAddr_len = sizeof(clientAddr);
	int	clientSocket = accept(_serverSocket, (struct sockaddr*)&clientAddr, &clientAddr_len);
	if (clientSocket == ERROR)
	{
		MSG_ERR(strerror(errno));
		return ;
	}

	// 2 Store the client's hostname
	char	clientHostname[NI_MAXHOST];

	int		res = getnameinfo((struct sockaddr *) &clientAddr, sizeof(clientAddr), clientHostname, NI_MAXHOST, NULL, 0, NI_NUMERICSERV);
	if (res != 0)
	{
		close(clientSocket);
		MSG_ERR(strerror(errno));
		return ;
	}

	// 3 - add to poll list
	addToPoll(clientSocket, false);

	// 4 - create the User and add it to the list
	User	*newUser = new User(this, clientSocket, clientHostname);
	_users[clientSocket] = newUser;

	// 5 - console message
	msg_log(MSG_CLT_CONNECTED(clientSocket));

	// 6 - Change client's status to CONNECTED
	newUser->setStatus(CONNECTED);
}

/**
 * @brief Handle when POLLIN detected in a client
 * 
 * @param clientfd client's socket FD
 */
void	Server::handleIncomingData(int &clientfd)
{
	int					bytesReceived;
	char				buffer[BUFFER_SIZE];
	std::vector<s_msg>	fullMsg;
	User	*user = _users.find(clientfd)->second;
	static std::string	incompleteLine;
	
	memset(buffer, 0, sizeof(buffer));
	bytesReceived = recv(clientfd, buffer, sizeof(buffer) -1, 0);
	if (bytesReceived == ERROR || bytesReceived == 0 || bytesReceived >= 512)
	{
		disconnectClient(user);
		return ;
	}
	else
	{
		buffer[bytesReceived] = '\0';
		incompleteLine += buffer;

		if (!incompleteLine.empty() && incompleteLine[incompleteLine.size() - 1] == '\n')
		{
			// PARSE INCOMMING DATA AND LAUNCH COMMANDS
			parseIncomingData(incompleteLine, fullMsg);
			for (msg_iterator it = fullMsg.begin(); it != fullMsg.end(); it++)
			{
				//display_s_msg(*it);
				execute(this, user, *it);
			}
			incompleteLine.clear();
		}
	}
}

/**
 * @brief Parsing of incoming data : separate all lines
 * 
 * @param buffer 	raw buffer
 * @param fullMsg 	vector to fill with each line
 */
void	Server::parseIncomingData(std::string const &buffer, std::vector<s_msg> &fullMsg)
{
	std::istringstream	iss(buffer);
	std::string			line;

	while (std::getline(iss, line))
	{
		if (!line.empty())
		{
			s_msg	parsedLine = parseLine(line);
			fullMsg.push_back(parsedLine);
		}
	}
}

/**
 * @brief Parsing of incoming data : parse one line into an s_msg
 * 
 * @param line the line to analyse
 * @return s_msg filled
 */
s_msg	Server::parseLine(std::string const &line)
{
	s_msg				parsedMsg;
	std::istringstream	iss(line);

	// 0 - init s_msg to zero
	parsedMsg.prefix = "";
	parsedMsg.cmd = "";
	parsedMsg.args.clear();
	parsedMsg.trailing = "";
	parsedMsg.trailing_sign	= false;

	// 1 - extract prefix if exists
	if (line[0] == ':')
	{
		size_t	prefixEnd = line.find(' ');
		if (prefixEnd != std::string::npos)
		{
				parsedMsg.prefix = line.substr(1, prefixEnd - 1);
				iss.ignore(prefixEnd + 1);
		}
		else //Incorrect format msg
			return (parsedMsg);
	}

	// 2 - extract command
	iss >> parsedMsg.cmd;

	// 3 - extract arguments
	std::string	arg;
	while (iss >> arg)
	{
		if (arg[0] == ':')  //trailing
		{
			parsedMsg.trailing_sign = true;
			if (!parsedMsg.trailing.empty())
				parsedMsg.trailing += ' ' + arg.substr(1);
			else
				parsedMsg.trailing = arg.substr(1);
			
			// Concaténer tous les mots restants
			while (iss >> arg)
			{
				parsedMsg.trailing += ' ' + arg;
			}
			break ;
		}
		else
			parsedMsg.args.push_back(arg);
	}
	return (parsedMsg);
}
/* #endregion */

/* #region PUBLIC */

/**
 * @brief Run the IRC server
 * 
 * @note - 1) Creation and configuration of the server's network socket
 * @note - 2) Adding server's socket to POLLFD (list of sockets that poll() have to check)
 * @note - 3) Server is now running and wait for activities from clients. To leave, use the EXIT signal (CTRL + C).
 */
void	Server::start()
{
	msg_log(MSG_SVR_START);
	signal(SIGINT, handleSignal);
	signal(SIGPIPE, SIG_IGN);

	// 1 - setup of the server socket
	setServerSocket();

	// 2 - add server's socket to the list of pollfd
	addToPoll(_serverSocket, true);

	// 3 - main loop, waiting for activity on sockets with poll (or for exit signal)
	msg_log(MSG_SVR_STARTED);
	while (stopSignalReceived == false)
		handlePollEvents();
}

void	Server::shutdown() { stopSignalReceived = true; }

/* #endregion */

/* #region Client*/

/**
 * @brief Handle the disconnection of the client
 * 
 * @param client client to disconnect
 */
void	Server::disconnectClient(User *client)
{
	int	clientFD = client->getSocketFd();

	// 1) remove client from all Channels
	client->leaveAllChannels("QUIT");

	// 2) remove client form pollfd list
	deleteFromPoll(clientFD);

	// 3) remove client from Users list
	deleteUser(clientFD);

	// 4) close socket
	close(clientFD);

	// 5) delete
	delete client;

	msg_log(MSG_CLT_DISCONNECTED(clientFD));
}

/**
 * @brief For each client, send the 
 * 
 */
void	Server::disconnectAllClients()
{
	client_iterator it = _users.begin();
	while (it != _users.end())
	{
		client_iterator	current = it;
		it++;
		User	*client =  current->second;
		client->sendToClient(MSG_CLT_SVRSHUTDOWM);
		disconnectClient(client);
	}
}

/* #endregion */

/* #region Channels */

/**
 * @brief Creates a new channel without a key
 */
void	Server::newChannel(std::string const &name, User *user)
{
	Channel	*chan = new Channel(this, name, user);

	_channels.push_back(chan);
}

/**
 * @brief Creates a new channel with a key
 */
void	Server::newChannel(std::string const &name, User *user, std::string const &key)
{
	Channel	*chan = new Channel(this, name, user, key);

	_channels.push_back(chan);
}

/**
 * @brief Remove a channel from the Server's list and liberate memory
 */
void	Server::deleteChannel(Channel *channel)
{
	for (channel_iterator it = _channels.begin(); it != _channels.end();)
	{
		if (*it == channel)
		{
			delete *it;
			it = _channels.erase(it);
			break ;
		}
		else
			it++;
	}
}

/**
 * @brief Searchs and returns a Channel * from his name
 */
Channel	*Server::findChannel(std::string const &name)
{
	if (name.empty())
		return (NULL);
	for (channel_iterator it = _channels.begin(); it != _channels.end(); it++)
	{
		if ((*it)->getChannelName() == name)
			return (*it);
	}
	return (NULL);
}

/* #endregion */

/* #region TOOLS */

/**
 * @brief Add a new pollfd to the list
 * 
 * @param fd the fd to enter in parameter of the new pollfd
 * @param isServer true if the pollfd concerns the server, false if not
 */
void	Server::addToPoll(int fd, bool isServer)
{
	pollfd	newPoll;

	newPoll.fd = fd;
	newPoll.events = POLLIN;
	newPoll.revents = 0;
	_fds.push_back(newPoll);
	if (!isServer)
		_nbOfClients++;
}

/**
 * @brief Delete an entry from the pollfd list
 *
 * @param fd FD parameter of the pollfd to delete
 */
void	Server::deleteFromPoll(int fd)
{
	for (pollfd_iterator it = _fds.begin(); it != _fds.end(); ++it)
	{
		if (it->fd == fd)
		{
			_fds.erase(it);
			_nbOfClients--;
			break ;
		}
	}
}

/**
 * @brief Remove an User fron the Users list
 * 
 * @param fd FD of this user
 */
void	Server::deleteUser(int fd)
{
	client_iterator it = _users.find(fd);

	if (it != _users.end())
		_users.erase(it);
}
/* #endregion */

/* #region GETTERS */

/**
 * @brief Search an user with his nickname
 * 
 * @param nickname user to search
 * @return NULL if not found or a pointer to the User if found
 */
User	*Server::getUserWithNickname(std::string const &nickname)
{
	for (client_iterator it = _users.begin(); it != _users.end(); it++)
	{
		if (!nickname.compare(it->second->getNickname()))
			return (it->second);
	}
	return (NULL);
}

std::string const &Server::getPassword() const { return _password; }

std::map<std::string, Command *> &Server::getCommands() { return _commands; }


/**
 * @brief Search an user with his socket FD (PRIVATE GETTER)
 * 
 * @param client_socket user to search
 * @return NULL if not found or a pointer to the User if found
 */
User	*Server::getUserwithFd(int client_socket)
{
	for (client_iterator it = _users.begin(); it != _users.end(); it++)
	{
		if (client_socket == it->first)
			return (it->second);
	}
	return (NULL);
}

/* #endregion */

