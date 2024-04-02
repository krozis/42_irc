#include "User.hpp"

/* #region Constructor/Destructor */

/**
 * @brief Construct a new User:: User object
 * 
 * @param clientSocket connection socket FD of this client
 * @param clientHostname hostname of the client
 */
User::User(Server *server, int clientSocket, std::string const &clientHostname):
	_server(server), _socket_fd(clientSocket), _hostname(clientHostname), _op(false)
{
	_status = CREATED;
	_nickname = "*";
	_username = "*";
	_realname = "*";
	_leavingMsg = "*";
	_joinedChannels.clear();
	_invitedChannels.clear();
}

User::~User()
{
	// Diseappears from all channel's invitation list
	for (channel_iterator it = _invitedChannels.begin(); it != _invitedChannels.end(); it++)
		(*it)->removeUser(this);
	_invitedChannels.clear();
}

/* #endregion */

/* #region Private */

/**
 * @brief Check is a string is equal to "*", which means that it has not been set.
 */
bool	User::isEmpty(std::string const &str) const { return str == "*"; }

/* #endregion */

/* #region Public */

/**
 * @brief Used to send a message to the client after formating it correctly
 * 
 * @param msg unformated message
 */
void	User::sendToClient(std::string const &msg)
{
	std::string	fullMsg = msg + "\r\n";
	
	if (send(_socket_fd, fullMsg.c_str(), fullMsg.length(), 0) == ERROR)
		MSG_ERR(strerror(errno));
}

void	User::welcome()
{
	sendToClient(RPL_WELCOME(_nickname, getFullname()));
	sendToClient(RPL_YOURHOST(_nickname));
	sendToClient(RPL_CREATED(_nickname));
	sendToClient(RPL_MYINFO(_nickname));
}

/* #endregion */

/* #region Channel */

void	User::addJoinedChannel(Channel *channel)	{ _joinedChannels.push_back(channel); }
void	User::addInvitedChannel(Channel *channel)	{ _invitedChannels.push_back(channel); }

void	User::removeJoinedChannel(Channel *channel)
{
	channel_iterator	it = std::find(_joinedChannels.begin(), _joinedChannels.end(), channel);
	if (it != _joinedChannels.end())
		_joinedChannels.erase(it);
}

void	User::removeInvitedChannel(Channel *channel)
{
	channel_iterator	it = std::find(_invitedChannels.begin(), _invitedChannels.end(), channel);
	if (it != _invitedChannels.end())
		_invitedChannels.erase(it);
}

/**
 * @brief Leaves the given channel, display then clears the leaving message
 */
void	User::leaveChannel(Channel *channel, User *origin, std::string const &why)
{
	// Send the correct message to the client
	if (why == "PART")
	{
		if (isEmpty(_leavingMsg))
			channel->sendToChannel(NULL, SEND_PART(getFullname(), channel->getChannelName()));
		else
			channel->sendToChannel(NULL, SEND_PART_MSG(getFullname(), channel->getChannelName(), _leavingMsg));
	}
	else if (why == "KICK")
	{
		if (isEmpty(_leavingMsg))
		{
			channel->sendToChannel(NULL, SEND_KICK(origin->getFullname(), channel->getChannelName(), _nickname, origin->getNickname()));
			msg_log(_nickname + " has been kicked from " + channel->getChannelName());
		}
		else
		{
			channel->sendToChannel(NULL, SEND_KICK(origin->getFullname(), channel->getChannelName(), _nickname, _leavingMsg));
			msg_log(_nickname + " has been kicked from " + channel->getChannelName() + " :" + _leavingMsg);
		}
	}

	// remove channel from joined list
	removeJoinedChannel(channel);

	// remove user from the channel
	channel->removeUser(this);

	// clean the leaving message
	_leavingMsg = "*";
}

/**
 * @brief Leaves all the channels that has been joined,
 * and send message to channels depending of the why parameter
 *
 * @param why "PART" or "QUIT"
 */
void	User::leaveAllChannels(std::string const &why)
{
	channel_iterator it = _joinedChannels.begin();
	while (it != _joinedChannels.end())
	{
		Channel	*channel = *it;

		// If no leaving message was set
		if (isEmpty(_leavingMsg))
		{
			if (why == "PART")
				channel->sendToChannel(this, SEND_PART(getFullname(), channel->getChannelName()));
			else
				channel->sendToChannel(this, SEND_QUIT(getFullname(), _nickname));
		}
		// If a leaving message was set
		else
		{
			if (why == "PART")
				channel->sendToChannel(this, SEND_PART_MSG(getFullname(), channel->getChannelName(), _leavingMsg));
			else
				channel->sendToChannel(this, SEND_QUIT_MSG(getFullname(), _leavingMsg));
		}
		// remove channel from _joinedChannel list
		it = _joinedChannels.erase(it);

		// remove user from channel (if last user -> channel will be deleted)
		channel->removeUser(this);
	}

	// restore values to default
	_leavingMsg = "*";
	_joinedChannels.clear();
}

/* #endregion */

/* #region GETTERS */

bool				User::isServerOp() const	{ return _op; }
int 				User::getSocketFd() const	{ return _socket_fd; }
clientStatus		User::getStatus() const		{ return _status; }
std::string const	&User::getUsername() const	{ return _username; }
std::string	const	&User::getNickname() const	{ return _nickname; }
std::string const	&User::getRealname() const	{ return _realname; }
std::string const	&User::getHostname() const	{ return _hostname; }
std::string			User::getFullname() const
{
	std::string fullMsg = _nickname;
	fullMsg += isEmpty(_username) ? "" : "!~" +_username;
	fullMsg += isEmpty(_hostname) ? "" : "@" +_hostname;

	return (fullMsg);
}

/* #endregion */

/* #region SETTERS */

void	User::setStatus(clientStatus status)			{ _status = status; }
void	User::setUsername(std::string const &username)	{ _username = username; }
void	User::setNickname(std::string const &nickname)	{ _nickname = nickname; }
void	User::setRealname(std::string const &realname)	{ _realname = realname; }
void	User::setHostname(std::string const &hostname)	{ _hostname = hostname; }
void	User::setLeavingMessage(std::string const &msg)	{ _leavingMsg = msg; }
void	User::setServerOP(bool val)						{ _op = val; }

/* #endregion */