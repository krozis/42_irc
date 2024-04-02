#ifndef FT_IRC_HPP
# define FT_IRC_HPP

/********************************
 *		Common includes		*
 *******************************/
# include <iostream>
# include <cstdlib>		//EXIT_SUCCESS/EXIT_FAILURE and more
# include <climits>		//MAX/MIN of types
# include <cerrno>		//using errno
# include <cstring>		//using strerror
# include <sstream>		//using string streams
# include <algorithm>	//std::find

// Server
# include <sys/socket.h>	//socket creation/usage tools
# include <sys/poll.h>		//function poll()
# include <netinet/in.h>	//contains sockaddr_in definition
# include <arpa/inet.h>		//IP representations (inet_addr(), inet_ntoa(),...)
# include <netdb.h>		//getnameinfo() + flags in handleNewConnection()

// containers
# include <vector>
# include <map>
# include <list>		// Channel
# include <queue>		// Command MODE

//not sure if global or server specific
# include <ctime>		//time and time structures manipulation
# include <fcntl.h>		//control operations on FDs and contains fcntl()
# include <csignal>		//signal() function

/********************************
 *		Configuration Values	*
 *******************************/
# define ERROR 				-1
# define SVR_NAME			"B&S"
# define SVR_SOCKET			0

//from bw: main.cpp
# define MAX_CLIENTS 		1024
# define PORT_MIN			1024
# define PORT_MAX			65535

//from bw: Server.hpp
# define BACKLOG 5
# define MAX_CONNECTIONS 1024
# define BUFFER_SIZE 1024		// RFC2812:2.3, a message has max 510 char(512 with the \r\n follwing)
# define TIMEOUT 60000 // 60 secs

// structure for a full IRC command (prefix and trailing are optional)
struct	s_msg
{
	std::string					prefix;
	std::string					cmd;
	std::vector<std::string>	args;
	std::string					trailing;
	bool						trailing_sign;
};

// Channel Modes
typedef uint8_t channelModes;

static const channelModes INVITE_ONLY = 0x01; // 0000 0001 - mode i
static const channelModes KEY = 0x02;         // 0000 0010 - mode k
static const channelModes LIMIT = 0x04;       // 0000 0100 - mode l
static const channelModes TOPIC = 0x08;       // 0000 1000 - mode t

enum	userLevel { INVITED, NORMAL, OPERATOR };
enum	modeType { PLUS, MINUS };
enum	modeVal { BAD_MODE = -1, I_MODE, K_MODE, L_MODE, O_MODE, T_MODE };
typedef	std::pair<modeType, modeVal> modePair;

// classes & Iterators

class User;
class Channel;
typedef std::vector<pollfd>::iterator		pollfd_iterator;
typedef std::map<int, User *>::iterator		client_iterator;
typedef std::vector<s_msg>::iterator		msg_iterator;
typedef std::vector<Channel *>::iterator	channel_iterator;

/********************************
 *		Project includes		*
 *******************************/
# include "msg.hpp"
# include "Server.hpp"
# include "User.hpp"
# include "Commands.hpp"
# include "Channel.hpp"

#endif