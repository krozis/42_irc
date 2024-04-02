#ifndef SERVER_HPP
# define SERVER_HPP

# include "ft_irc.hpp"		//FT_IRC global header

class User;
class Command;
class Channel;

class Server
{
	private:
		//Attributes
		
		unsigned int		_port;				// 1024 to 65535
		const std::string	_password;

		int					_serverSocket;
		int					_endian;			// BIG_ENDIAN or LITTLE_ENDIAN
		struct sockaddr_in	_addrServer;		// server address

		std::vector<pollfd>	_fds;				// List of socket FD that poll() must watch
		int					_nbOfClients;		// Total clients connected, not including server

		std::map<int, User *>				_users;		//int is FD	
		std::map<std::string, Command *>	_commands;
		std::vector<Channel *>				_channels;

		//--------------------------------------------------------------
		//Methods

		//init and setup

		void	setPort(std::string const &port);
		void	setEndian();
		void	setServerSocket();
	
		//events handle
		
		void	handlePollEvents();
		void	handleNewConnection();
		void	handleIncomingData(int &clientfd);

		//tools
		void	addToPoll(int fd, bool isServer);
		void	deleteFromPoll(int fd);
		void	deleteUser(int fd);
		void	disconnectAllClients();

		//private getters
		
		User	*getUserwithFd(int client_socket);

		//parsing  //general format is :
		//  :prefix COMMAND arg1 arg2 ... :trailing
		// prefix is optionnal and can be the server name or an user name
		// trailing is a secial arg that can countain spaces and has ":" just before
		void	parseIncomingData(std::string const &buffer, std::vector<s_msg> &fullMsg);
		s_msg	parseLine(std::string const &line);

		//--------------------------------------------------------------
		//UNUSED COPLIEN

		Server();								
		Server(Server &to_copy);				
		Server	&operator=(Server &to_assign);



	public:
		Server(std::string const &port, std::string const &password);
		~Server();

		void	start();
		void	shutdown();

		void	disconnectClient(User *client);
		void	newChannel(std::string const &name, User *user);
		void	newChannel(std::string const &name, User *user, std::string const &key);
		void	deleteChannel(Channel *channel);
		Channel *findChannel(std::string const &name);
		


		//--------------------------------------------------------------
		//Getters

		std::string const					&getPassword() const;
		std::map<std::string, Command *>	&getCommands();
		User								*getUserWithNickname(std::string const &nickname);

		//--------------------------------------------------------------
		//Setters


		//--------------------------------------------------------------
		//DEPRECATED

		//std::string	getServerName() const;
};


#endif