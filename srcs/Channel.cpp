#include "Channel.hpp"

/* #region Constructor/Destructor  */

Channel::Channel(Server *server, std::string name, User *user):
_server(server), _channelName(name), _modes(0), _maxUsers(0)
{
	msg_log(MSG_CHAN_CREATED(user->getNickname(), _channelName));
	_topic.empty();
	_password.empty();
	addUser(user, OPERATOR);
}

Channel::Channel(Server *server, std::string name, User *user, std::string const &password):
_server(server), _channelName(name), _modes(0), _password(password), _maxUsers(0)
{
	msg_log(MSG_CHAN_CREATED(user->getNickname(), _channelName));
	_topic.empty();
	setMode(KEY, PLUS);
	addUser(user, OPERATOR);
}

Channel::~Channel()
{
	// Remove the channel from the lists of all invited users
	for (std::list<User *>::iterator it = _invitedUsers.begin(); it != _invitedUsers.end(); it++)
		(*it)->removeInvitedChannel(this);

	msg_log("Channel " + _channelName + " channel has been deleted.");
}
/* #endregion */

/* #region PUBLIC */

/**
 * @brief Sends a message to all users (operators and normal) of the channel
 * If the given user isn't NULL, send to all except him
 */
void	Channel::sendToChannel(User *user, std::string const &msg)
{
	for (std::list<User *>::iterator it = _operators.begin(); it != _operators.end(); it++)
	{
		if (user == NULL || (*it)->getNickname() != user->getNickname())
				(*it)->sendToClient(msg);
	}
	for (std::list<User *>::iterator it = _normalUsers.begin(); it != _normalUsers.end(); it++)
	{
		if (user == NULL || (*it)->getNickname() != user->getNickname())
				(*it)->sendToClient(msg);
	}
}

std::string	const	Channel::sendTopic(User *user) const
{
	if (_topic.empty())
		return (RPL_NOTOPIC(user->getNickname(), _channelName));
	return (RPL_TOPIC(user->getNickname(), _channelName, _topic));
}
/* #endregion */

/* #region User */

/**
 * @brief Adds a new user of the desired lvl to the channel
 */
void	Channel::addUser(User *user, userLevel lvl)
{
	// Invite an user
	if (lvl == INVITED)
	{
		// if already invited, do nothing
		if (isInvited(user->getNickname()))
			return ;
		_invitedUsers.push_back(user);
		user->addInvitedChannel(this);
	}
	else
	{


		// If user was invited, delete the invitation
		if (isInvited(user->getNickname()))
			_invitedUsers.remove(user);

		// check lvl and add to the correct list
		if (lvl == NORMAL)
			_normalUsers.push_back(user);
		else if (lvl == OPERATOR)
			_operators.push_back(user);

		//Send the message ":fullname JOIN :#channel"
		sendToChannel(NULL, SEND_JOIN(user->getFullname(), _channelName));

		user->addJoinedChannel(this);
		msg_log(user->getNickname() + " joined the channel " + _channelName);
		welcomeUser(user);
	}
}

void	Channel::setUserLevel(User *user, userLevel lvl)
{
	if (lvl == NORMAL && !isNormal(user->getNickname()))
	{
		_operators.remove(user);
		_normalUsers.push_back(user);
	}
	else if (lvl == OPERATOR && !isOperator(user->getNickname()))
	{
		_normalUsers.remove(user);
		_operators.push_back(user);
	}
}

/**
 * @brief Remove an user from Channel
 */
void	Channel::removeUser(User *user)
{
	if (isInvited(user->getNickname()))
		_invitedUsers.remove(user);
	else
	{
		if (isOperator(user->getNickname()))
			_operators.remove(user);
		else
			_normalUsers.remove(user);
		msg_log(user->getNickname() + " has left the channel " + _channelName);

		if (isChannelEmpty())
			_server->deleteChannel(this);
	}

}

/**
 * @brief Finds an user with his nickname and returns a pointer to this user
 * 
 * @param nickname the user to find
 */
User	*Channel::findUserInChannel(std::string nickname)
{
	User *user = NULL;
	user = findUserFromList(_operators, nickname);
	if (user == NULL)
		user = findUserFromList(_normalUsers, nickname);
	return user;
}

/**
 * @brief Displays all the users, operator in the first line, normal users in the second line.
 */
void	Channel::displayUsers(User *user)
{
	
	std::string	msg;
	bool		first = true;

	for (std::list<User *>::iterator it = _operators.begin(); it != _operators.end(); it++)
	{
		if (first)
		{
			msg += "@";
			msg += (*it)->getNickname();
			first = false;
		}
		else
		{
			msg += " @";
			msg += (*it)->getNickname();
		}
	}
	first = true;
	for (std::list<User *>::iterator it = _normalUsers.begin(); it != _normalUsers.end(); it++)
	{
			msg += " ";
			msg += (*it)->getNickname();
	}
	if (!msg.empty())
	{
		user->sendToClient(RPL_NAMREPLY(user->getNickname(), _channelName, msg));
		user->sendToClient(RPL_ENDOFNAMES(user->getNickname(), _channelName));
	}
}

/**
 * @brief Welcoming message when entering the Channel, including users list.
 */
void	Channel::welcomeUser(User *user)
{
	if (!_topic.empty())
		user->sendToClient(RPL_TOPIC(user->getNickname(), _channelName, _topic));
	displayUsers(user);
}

bool	Channel::isPasswordCorrect(std::string const &pass) { return (pass == _password); }

/* #endregion */

/* #region MODES */
void	Channel::setMode(channelModes mode, modeType type) { (type == PLUS ? _modes |= mode : _modes &= ~mode); }
bool	Channel::isMode(channelModes mode) const { return ((_modes & mode) != 0); }

std::string	Channel::listModes(bool isMember) const
{
	std::string	modes = "";

	if (isMode(INVITE_ONLY))
		modes += "i";
	if (isMode(KEY))
		modes += "k";
	if (isMode(LIMIT))
		modes += "l";
	if (isMode(TOPIC))
		modes += "t";
	
	// Only members of the channel have access to details
	if (isMember)
	{
		if (isMode(KEY))
			modes += " " + _password;
		if (isMode(LIMIT))
			modes += " " + to_string(_maxUsers);
	}
	return (modes);
}
/* #endregion */

/* #region GETTERS */

std::string const	&Channel::getChannelName() const { return (_channelName); }
std::string const	&Channel::getTopic() const { return (_topic); }
std::string const	&Channel::getPassword() const { return(_password); }
int					Channel::getMaxUsers() const { return(_maxUsers); }

bool	Channel::isInvited(std::string const &nickname) { return (findUserFromList(_invitedUsers, nickname) != NULL);  }
bool	Channel::isOperator(std::string const &nickname) { return (findUserFromList(_operators, nickname) != NULL); }
bool	Channel::isNormal(std::string const &nickname) { return (findUserFromList(_normalUsers, nickname) != NULL); }

int	Channel::getTotalUsers() const { return (_operators.size() + _normalUsers.size()); }
/* #endregion */

/* #region SETTERS */

//removed setChannelName() :  RFC 1459 -> once created, can't change a channel's name.

void	Channel::setPassword(std::string const &password) { _password = password; }
void	Channel::setTopic(std::string const &topic) { _topic = topic; }
void	Channel::setMaxUsers(int max) { _maxUsers = max; }

/* #endregion */

/* #region PRIVATE */

/**
 * @brief Finds an user with his nickname in a specific list
 */
User	*Channel::findUserFromList(std::list<User *> &role, std::string nickname)
{
	user_iterator it;
	for (it = role.begin(); it != role.end(); it++){
		if((*it)->getNickname() == nickname)
			return (*it);
	}
	return NULL;
}

/**
 * @brief Verify if there is at least one user in channel and if at least one operator.
 * @returns true if channel is empty, false if not
 */
bool	Channel::isChannelEmpty()
{
	if (_operators.size() + _normalUsers.size() == 0)
		return (true);

	User	*user;
	if (_operators.size() == 0)
	{
		user = _normalUsers.front();
		setUserLevel(user, OPERATOR);
	}
	return (false);
}

/* #endregion */