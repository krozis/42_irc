#include "Commands.hpp"

/* #region EXTERNAL FUNCTIONS */

/**
 * @brief Initialises the given command list with all commands that are available
 */
void	initCommands(Server *server, std::map<std::string, Command *> &commands)
{
	commands["QUIT"] = new Quit(server);
	commands["PASS"] = new Pass(server);
	commands["NICK"] = new Nick(server);
	commands["USER"] = new UserCMD(server);
	commands["PING"] = new Ping(server);		//REGISTRATION NEEDED
	commands["JOIN"] = new Join(server);		//REGISTRATION NEEDED
	commands["PART"] = new Part(server);		//REGISTRATION NEEDED
	commands["KICK"] = new Kick(server);		//REGISTRATION NEEDED
	commands["INVITE"] = new Invite(server);	//REGISTRATION NEEDED
	commands["PRIVMSG"] = new Privmsg(server);	//REGISTRATION NEEDED
	commands["TOPIC"] = new Topic(server);		//REGISTRATION NEEDED
	commands["MODE"] = new Mode(server);		//REGISTRATION NEEDED
	commands["OPER"] = new Oper(server);		//REGISTRATION NEEDED
	commands["POWEROFF"] = new Poweroff(server);//SERVER OPERATOR ONLY
}

/**
 * @brief Redirects to all different Commands
 */
void	execute(Server	*server, User *user, s_msg &msg)
{
	if (msg.cmd == "QUIT")
		(*server->getCommands()["QUIT"]).execute(user, msg);
	else if (msg.cmd == "PASS")
		(*server->getCommands()["PASS"]).execute(user, msg);
	else if (msg.cmd == "NICK")
		(*server->getCommands()["NICK"]).execute(user, msg);
	else if (msg.cmd == "USER")
		(*server->getCommands()["USER"]).execute(user, msg);
	else if (msg.cmd == "PING")
		(*server->getCommands()["PING"]).execute(user, msg);
	else if (msg.cmd == "JOIN")
		(*server->getCommands()["JOIN"]).execute(user, msg);
	else if (msg.cmd == "PART")
		(*server->getCommands()["PART"]).execute(user, msg);
	else if (msg.cmd == "KICK")
		(*server->getCommands()["KICK"]).execute(user, msg);
	else if (msg.cmd == "INVITE")
		(*server->getCommands()["INVITE"]).execute(user, msg);
	else if (msg.cmd == "PRIVMSG")
		(*server->getCommands()["PRIVMSG"]).execute(user, msg);
	else if (msg.cmd == "TOPIC")
		(*server->getCommands()["TOPIC"]).execute(user, msg);
	else if (msg.cmd == "MODE")
		(*server->getCommands()["MODE"]).execute(user, msg);
	else if (msg.cmd == "OPER")
		(*server->getCommands()["OPER"]).execute(user, msg);
	else if (msg.cmd == "POWEROFF")
		(*server->getCommands()["POWEROFF"]).execute(user, msg);
	else if (msg.cmd == "CAP")
		;	// Just ignore CAP request
	else
		user->sendToClient(ERR_UNKNOWNCOMMAND(user->getNickname(), msg.cmd));
}

/**
 * @brief Cleans the given command list
 */
void	deleteCommands(std::map<std::string, Command *> &commands)
{
	std::map<std::string, Command *>::iterator it = commands.begin();
	std::map<std::string, Command *>::iterator end = commands.end();

	while (it != end)
	{
		delete it->second;
		it++;
	}
}
/* #endregion */

/* #region COMMAND */

Command::Command(Server	*server): _server(server) {   }
Command::~Command() {  }
/* #endregion */ 

/* #region QUIT */

Quit::Quit(Server *server) : Command(server) {  }
Quit::~Quit() {  }

void	Quit::execute(User *user, s_msg &msg)
{
	if (!msg.trailing.empty())
		user->setLeavingMessage(msg.trailing);
	user->sendToClient(MSG_CLT_QUIT(user->getNickname()));
	_server->disconnectClient(user);
}
/* #endregion */

/* #region PASS */

Pass::Pass(Server *server): Command(server) {  }
Pass::~Pass() {  }

void	Pass::execute(User *user, s_msg &msg)
{
	if (msg.args.empty())
		user->sendToClient(ERR_NEEDMOREPARAMS(user->getNickname(), "PASS"));
	else if (user->getStatus() == REGISTERED || user->getStatus() == PASSWORDACCEPTED)
		user->sendToClient(ERR_ALREADYREGISTERED(user->getNickname()));
	else if (_server->getPassword() != msg.args.at(0))
		user->sendToClient(ERR_PASSWDMISMATCH(user->getNickname()));
	else
		user->setStatus(PASSWORDACCEPTED);
}
/* #endregion */

/* #region NICK */

static bool	isNickValid(std::string const &nick)
{
	// max size for a nickname is 9 characters
	if (nick.length() > 9)
		return (false);

	// First character must be a letter OR '|' OR '^' OR '~'
	if (!isalpha(nick[0]) && nick[0] != '|' && nick[0] != '^' && nick[0] != '~')
		return (false);

	// Rest countains only allowed characters
	for (std::string::const_iterator it = nick.begin(); it != nick.end(); ++it)
	{
		if (!isalnum(*it) && *it != '-' && *it != '[' && *it != ']' && *it != '\\' && *it != '{' && *it != '}' && *it != '|' && *it != '^' && *it != '_')
			return (false);
	}

	// Correct format of Nickname
	return (true);
}

Nick::Nick(Server* server) : Command(server) {  }
Nick::~Nick() {  }

void	Nick::execute(User *user, s_msg &msg)
{
	// No argument given
	if (msg.args.empty() || msg.args[0].empty())
		user->sendToClient(ERR_NONICKNAMEGIVEN(user->getNickname()));
	else
	{
		// Nickname with unvalid format
		if (!isNickValid(msg.args[0]))
			user->sendToClient(ERR_ERRONEUSNICKNAME(user->getNickname(), msg.args[0]));

		// Nickname already in use
		else if (_server->getUserWithNickname(msg.args[0]))
			user->sendToClient(ERR_NICKNAMEINUSE(user->getNickname(), msg.args[0]));
		
		// Nickname is accepted
		else
		{
			if (user->getStatus() == PASSWORDACCEPTED)
			{
				user->setNickname(msg.args[0]);
				user->setStatus(NICKNAMEISOK);
			}
			else if (user->getStatus() == USERNAMEISOK)
			{
				user->setNickname(msg.args[0]);
				user->setStatus(REGISTERED);
				user->welcome();
			}
			else if (user->getStatus() == NICKNAMEISOK || user->getStatus() == REGISTERED) 
			{
				if (user->getStatus() == REGISTERED)
					user->sendToClient(SEND_NICK(user->getFullname(), msg.args[0]));
				user->setNickname(msg.args[0]);
			}
			msg_log(MSG_CLT_NICK(user->getSocketFd(), user->getNickname()));
		}
	}
}
/* #endregion */

/* #region USER */

static bool	isUsernameValid(std::string const &username)
{
	// max size for an username is 9 characters
	if (username.length() > 9)
		return (false);	

	// First character must be a letter
	if (!isalpha(username[0]))
		return (false);

	// Rest countains only allowed characters
	for (std::string::const_iterator it = username.begin(); it != username.end(); ++it)
	{
		if (!isalnum(*it) && *it != '-' && *it != '[' && *it != ']' && *it != '\\' && *it != '{' && *it != '}' && *it != '|' && *it != '^' && *it != '_')
			return (false);
	}

	// valid username
	return (true);
}

static bool	isRealnameValid(std::string const &realname)
{
	for (std::string::const_iterator it = realname.begin(); it != realname.end(); ++it)
	{
		if (!isalnum(*it) && *it != ' ' && *it != '-' && *it != '[' && *it != ']' && *it != '\\' && *it != '{' && *it != '}' && *it != '|' && *it != '^' && *it != '_')
			return (false);
	}
	return (true);
}

UserCMD::UserCMD(Server *server) : Command(server) {  }
UserCMD::~UserCMD() {  }

void	UserCMD::execute(User *user, s_msg &msg)
{
	// Not enough parameters or too much parameters or no trailing
	if (!(msg.args.size() == 3 && !msg.trailing.empty()))
		user->sendToClient(ERR_NEEDMOREPARAMS(user->getNickname(), "USER"));
	
	// Username with unvalid format
	else if (!isUsernameValid(msg.args[0]))
		user->sendToClient(ERR_INVALIDUSERNAME(msg.args[0]));

	// Realname with unvalid format
	else if (!isRealnameValid(msg.trailing))
		user->sendToClient(ERR_INVALIDREALNAME(msg.trailing));

	// Already Registered
	else if (user->getStatus() == REGISTERED)
		user->sendToClient(ERR_ALREADYREGISTERED(user->getNickname()));

	// Accept to change USER names
	else
	{
		if (user->getStatus() == PASSWORDACCEPTED)
		{
			user->setUsername(msg.args[0]);
			user->setRealname(msg.trailing);
			user->setStatus(USERNAMEISOK);
		}
		else if (user->getStatus() == NICKNAMEISOK)
		{
			user->setUsername(msg.args[0]);
			user->setRealname(msg.trailing);
			user->setStatus(REGISTERED);
			user->welcome();
		}
		else if (user->getStatus() == USERNAMEISOK)
		{
			user->setUsername(msg.args[0]);
			user->setRealname(msg.trailing);
		}
		msg_log(MSG_CLT_USER(user->getSocketFd(), user->getUsername(), user->getRealname()));
	}
}
/* #endregion */

/* #region PING */

Ping::Ping(Server *server): Command(server) {   }
Ping::~Ping() {  }

void	Ping::execute(User *user, s_msg &msg)
{
	// User is not registered
	if (user->getStatus() != REGISTERED)
		user->sendToClient(ERR_NOTREGISTERED(user->getNickname(), "PING"));

	// No arg given
	else if (msg.args.empty())
		user->sendToClient(ERR_NOORIGIN(user->getNickname()));

	// Default PING response
	else
		user->sendToClient(SEND_PONG(user->getNickname()));
}
/* #endregion */

/* #region JOIN */

static bool	isChanNameValid(std::string const &name)
{
	// max size is set to 64 char
	if (name.length() > 64)
		return (false);

	// first char must be '#' or '&'
	if (name[0] != '#' && name[0] != '&')
		return (false);
	
	// rest must be alphanumerical OR "-" OR "_" OR "."
	for (std::string::const_iterator it = name.begin() + 1; it != name.end(); ++it)
	{
		if (!isalnum(*it) && *it != '-' && *it != '.'&& *it != '_')
			return (false);
	}

	// format is correct
	return (true);
}

static void	parseJoin(std::vector<std::string> &chan_list, std::vector<std::string> &key_list, std::vector<std::string> const &args)
{
	// get every channel in the 1st argument
	std::istringstream iss_chan(args[0]);
	std::string chan;
	while (std::getline(iss_chan, chan, ','))
		chan_list.push_back(chan);

	// leave if no key provided
	if (args.size() == 1)
		return ;

	// get every key in the 2nd argument
	std::istringstream iss_key(args[1]);
	std::string key;
	while (std::getline(iss_key, key, ','))
		key_list.push_back(key);
}

Join::Join(Server *server): Command(server) {  }
Join::~Join() {  }

void	Join::execute(User *user, s_msg &msg)
{
	// User is not registered
	if (user->getStatus() != REGISTERED)
		user->sendToClient(ERR_NOTREGISTERED(user->getNickname(), "JOIN"));

	// No argument given
	else if (msg.args.empty())
		user->sendToClient(ERR_NEEDMOREPARAMS(user->getNickname(), "JOIN"));

	// JOIN 0 means PART from all channels
	else if (msg.args[0] == "0")
		user->leaveAllChannels("PART");

	else
	{
		std::vector<std::string>	chan_list;
		std::vector<std::string>	key_list;

		parseJoin(chan_list, key_list, msg.args);

		for (std::vector<std::string>::iterator it = chan_list.begin(); it != chan_list.end(); it++)
		{
			// Channel's name is invalid
			if (!isChanNameValid(*it))
				user->sendToClient(ERR_BADCHANNAME(user->getNickname(), *it));

			else
			{
				Channel *channel = _server->findChannel(*it);

				// Channel already exist
				if (channel)
				{
					// User already in channel -> just ignore
					if (channel->findUserInChannel(user->getNickname()))
						(void)user;

					// Channel is full (+l)
					else if (channel->getMaxUsers() >= channel->getTotalUsers())
						user->sendToClient(ERR_CHANNELISFULL(user->getNickname(), channel->getChannelName()));

					// Channel needs invitation and not invited
					else if (channel->isMode(INVITE_ONLY) && !channel->isInvited(user->getNickname()))
						user->sendToClient(ERR_INVITEONLYCHAN(user->getNickname(), channel->getChannelName()));

					else
					{
						// Channel needs password
						if (channel->isMode(KEY))
						{
							// Password not given
							if (key_list.empty())
								user->sendToClient(ERR_BADCHANNELKEY(user->getNickname(), channel->getChannelName()));
							else
							{
								// Password given but incorrect
								if (!channel->isPasswordCorrect(key_list.front()))
									user->sendToClient(ERR_BADCHANNELKEY(user->getNickname(), channel->getChannelName()));
								else
									channel->addUser(user, NORMAL);
								key_list.erase(key_list.begin());
							}
						}

						// Channel doesn't need key
						else
						{
							// if a key was given, delete it
							if (!key_list.empty())
								key_list.erase(key_list.begin());
							channel->addUser(user, NORMAL);
						}
					}
				}

				// Channel doesn't exist
				else
				{
					// key is provided
					if (!key_list.empty())
					{
						_server->newChannel(*it, user, key_list.front());
						key_list.erase(key_list.begin());
					}

					// no key provided
					else
					{
						_server->newChannel(*it, user);
					}
				}
			}
		}
	}
}

/* #endregion */

/* #region PART */

static void	parsePart(std::vector<std::string> &chan_list, std::string const &args)
{
	// get every channel in the 1st argument
	std::istringstream iss_chan(args);
	std::string chan;
	while (std::getline(iss_chan, chan, ','))
		chan_list.push_back(chan);
}

Part::Part(Server *server) : Command(server) {  }
Part::~Part() {  }

void	Part::execute(User *user, s_msg &msg)
{
	// User is not registered
	if (user->getStatus() != REGISTERED)
		user->sendToClient(ERR_NOTREGISTERED(user->getNickname(), "PART"));

	// No parameters given
	else if (msg.args.empty())
			user->sendToClient(ERR_NEEDMOREPARAMS(user->getNickname(), "PART"));

	else
	{
		std::vector<std::string>	chan_list;

		parsePart(chan_list, msg.args[0]);

		// Run through every channel
		for (std::vector<std::string>::iterator it = chan_list.begin(); it != chan_list.end(); it++)
		{
			Channel	*channel = _server->findChannel(*it);

			// Channel doesn't exist
			if (!channel)
				user->sendToClient(ERR_NOSUCHCHANNEL(user->getNickname(), *it));

			// User not in the channel
			else if (!channel->findUserInChannel(user->getNickname()))
				user->sendToClient(ERR_NOTONCHANNEL(user->getNickname(), channel->getChannelName()));

			// Part is allowed
			else
			{
				if (!msg.trailing.empty())
					user->setLeavingMessage(msg.trailing);
				user->leaveChannel(channel, NULL, "PART");
			}
		}
	}
}
/* #endregion */

/* #region KICK */

static void	parseKick(std::vector<std::string> &chan_list, std::vector<std::string> &user_list, std::vector<std::string> const &args)
{
	// get every channel in the 1st argument
	std::istringstream iss_chan(args[0]);
	std::string chan;
	while (std::getline(iss_chan, chan, ','))
		chan_list.push_back(chan);
	

	// get every user in the 2nd argument
	std::istringstream iss_user(args[1]);
	std::string user;
	while (std::getline(iss_user, user, ','))
		user_list.push_back(user);
}

Kick::Kick(Server* server) : Command(server) {  }
Kick::~Kick() {  }

void	Kick::execute(User *user, s_msg &msg)
{
	// User is not registered
	if (user->getStatus() != REGISTERED)
		user->sendToClient(ERR_NOTREGISTERED(user->getNickname(), "KICK"));

	// Not enough params
	else if (msg.args.empty() || msg.args.size() != 2)
		user->sendToClient(ERR_NEEDMOREPARAMS(user->getNickname(), "KICK"));
	else
	{
		std::vector<std::string>	chan_list;
		std::vector<std::string>	user_list;

		parseKick(chan_list, user_list, msg.args);

		// Run through every channel
		for (std::vector<std::string>::iterator it = chan_list.begin(); it != chan_list.end(); it++)
		{
			Channel	*channel = _server->findChannel(*it);

			// Channel is invalid
			if (!channel)
				user->sendToClient(ERR_NOSUCHCHANNEL(user->getNickname(), msg.args[0]));
			
			// Channel is valid
			else
			{
				// Kick launcher not in channel
				if (!channel->findUserInChannel(user->getNickname())) 
					user->sendToClient(ERR_NOTONCHANNEL(user->getNickname(), channel->getChannelName()));

				// Kick launcher not operator
				else if (!channel->isOperator(user->getNickname()))
						user->sendToClient(ERR_CHANOPRIVSNEEDED(user->getNickname(), channel->getChannelName()));
				
				// Kick launcher is allowed to KICK 
				else
				{
					// For every user in argument
					for (std::vector<std::string>::iterator it2 = user_list.begin(); it2 != user_list.end(); it2++)
					{
						User	*target = _server->getUserWithNickname(*it2);

						// Target doesn't exist
						if (!target)
							user->sendToClient(ERR_NOSUCHNICK(user->getNickname(), *it2));

						// Target not in the channel
						else if (!channel->findUserInChannel(target->getNickname()))
							user->sendToClient(ERR_USERNOTINCHANNEL(user->getNickname(), target->getNickname(), channel->getChannelName()));

						// Kick
						else // kick
						{
							target->setLeavingMessage(msg.trailing);
							target->leaveChannel(channel, user, "KICK");
						}
					}
				}
			}
		}
	}
}
/* #endregion */

/* #region INVITE */

Invite::Invite(Server *server) : Command(server) {  }
Invite::~Invite() {  }

/**
 * @brief Command INVITE
 * 
 * Parameters: <nickname> <channel>
 *
 * The INVITE command is used to invite a user to a channel.  The
   parameter <nickname> is the nickname of the person to be invited to
   the target channel <channel>.  There is no requirement that the
   channel the target user is being invited to must exist or be a valid
   channel.  However, if the channel exists, only members of the channel
   are allowed to invite other users.  When the channel has invite-only
   flag set, only channel operators may issue INVITE command.
 */
void	Invite::execute(User *user, s_msg &msg)
{
	// User is not registered
	if (user->getStatus() != REGISTERED)
		user->sendToClient(ERR_NOTREGISTERED(user->getNickname(), "INVITE"));

	// Not enough arguments given
	else if (msg.args.empty())
		user->sendToClient(ERR_NEEDMOREPARAMS(user->getNickname(), "INVITE"));

	// Too much arguments given
	else if (msg.args.size() != 2)
		user->sendToClient(ERR_SYNTAX(user->getNickname(), "INVITE"));

	// Syntax Correct
	else
	{
		User	*target = _server->getUserWithNickname(msg.args[0]);
		Channel	*channel = _server->findChannel(msg.args[1]);

		// target user doesn't exist
		if (target == NULL)
			user->sendToClient(ERR_NOSUCHNICK(user->getNickname(), msg.args[0]));

		// channel doesn't exist
		else if (channel == NULL || channel->isInvited(user->getNickname()))
			return ;

		// Target is not already on channel -> invite it
		// check if already invited is done in inviteUser().
		else if (channel->findUserInChannel(target->getNickname()) == NULL)
		{
			channel->addUser(target, INVITED);
			target->sendToClient(SEND_INVIT(user->getFullname(), user->getNickname(), channel->getChannelName()));
			user->sendToClient(RPL_INVITING(user->getNickname(), target->getNickname(), channel->getChannelName()));
		}
	}
}
/* #endregion */

/* #region PRIVMSG */

static void	parsePrivmsg(std::vector<std::string> &recipient_list, std::string const &args)
{
	if (args.find(',') == std::string::npos)
        recipient_list.push_back(args);  // Only one recipient
	else
	{
		std::istringstream	iss(args);
		std::string			recipient;
		while (std::getline(iss, recipient, ','))
			recipient_list.push_back(recipient);
	}
}

Privmsg::Privmsg(Server *server): Command(server) {  }
Privmsg::~Privmsg() {  }

void	Privmsg::execute(User *user, s_msg &msg)
{
	// user is not registered
	if (user->getStatus() != REGISTERED)	
		user->sendToClient(ERR_NOTREGISTERED(user->getNickname(), "PRIVMSG"));
	
	// no argument given
	else if (msg.args.empty())
		user->sendToClient(ERR_NORECIPIENT(user->getNickname()));
	
	// no message given (trailing)
	else if (msg.trailing.empty())
		user->sendToClient(ERR_NOTEXTTOSEND(user->getNickname()));
	
	// multiple arguments given = syntax error
	else if (msg.args.size() != 1)
		user->sendToClient(ERR_SYNTAX(user->getNickname(), "PRIVMSG"));
	
	//Syntax is good
	else
	{
		// parse the recipient list
		std::vector<std::string>	recipient_list;

		parsePrivmsg(recipient_list, msg.args[0]);
		//case there is only ',' in recipient list
		if (recipient_list.empty())
			user->sendToClient(ERR_NORECIPIENT(user->getNickname()));

		// process with the filled recipient list
		else
		{
			// For each recipient
			for (std::vector<std::string>::const_iterator it = recipient_list.begin(); it != recipient_list.end(); ++it)
			{
				std::string const	&recipient = *it;
				
				// Recipient is a channel
				if (recipient[0] == '#' || recipient[0] == '&')
				{
					Channel	*channel = _server->findChannel(recipient);
			
					// Channel doesn't exists
					if (channel == NULL)
						user->sendToClient(ERR_NOSUCHCHANNEL(user->getNickname(), recipient));
					
					// User is not in channel
					else if (!channel->findUserInChannel(user->getNickname()))
						user->sendToClient(ERR_CANNOTSENDTOCHAN(user->getNickname(), recipient));
					
					// Send to channel
					else
						channel->sendToChannel(user, SEND_PM(user->getFullname(), channel->getChannelName(), msg.trailing));
				}

				// Recipient is an user
				else
				{
					User	*target = _server->getUserWithNickname(recipient);

					// target doesn't exist
					if (target == NULL)
						user->sendToClient(ERR_NOSUCHNICK(user->getNickname(), recipient));

					// send to target
					else
						target->sendToClient(SEND_PM(user->getFullname(), target->getNickname(), msg.trailing));
				}

			}
		}
		
	}
}
/* #endregion */

/* #region TOPIC */

Topic::Topic(Server *server) : Command(server) {  }
Topic::~Topic() {  }

void	Topic::execute(User *user, s_msg &msg)
{
	// User is not registered
	if (user->getStatus() != REGISTERED)
		user->sendToClient(ERR_NOTREGISTERED(user->getNickname(), "TOPIC"));

	// No arguments given nor trailing
	else if (msg.args.empty() && msg.trailing.empty())
		user->sendToClient(ERR_NEEDMOREPARAMS(user->getNickname(), "TOPIC"));

	// No arguments given but a trailing
	else if (msg.args.empty() && !msg.trailing.empty())
		user->sendToClient(ERR_NOSUCHCHANNEL(user->getNickname(), ":" + msg.trailing));

	// arguments given
	else
	{
		Channel	*channel = _server->findChannel(msg.args[0]);

		// Channel doesnt exists
		if (!channel)
			user->sendToClient(ERR_NOSUCHCHANNEL(user->getNickname(), msg.args[0]));

		// No more arg and no trailing-> TOPIC #chan
		else if (msg.args.size() == 1 && msg.trailing_sign == false)
			user->sendToClient(channel->sendTopic(user));

		// User not in Channel and args or trailing
		else if (!channel->findUserInChannel(user->getNickname()) )
			user->sendToClient(ERR_NOTONCHANNEL(user->getNickname(), channel->getChannelName()));

		// User is in channel
		else
		{
			// User is not operator and channel is mode +t
			if (channel->isMode(TOPIC) && !channel->isOperator(user->getNickname()))
				user->sendToClient(ERR_CHANOPRIVSNEEDED(user->getNickname(), channel->getChannelName()));

			// User operator OR channel is not mode +t
			else
			{
				// un-topic command ->  TOPIC #channel :
				if (msg.args.size() == 1 && msg.trailing.empty() && msg.trailing_sign == true)
				{
					channel->setTopic("");
					channel->sendToChannel(NULL, SEND_TOPIC(user->getFullname(), channel->getChannelName(), ""));
				}
				// change topic with args
				else if (msg.args.size() > 1)
				{
					channel->setTopic(msg.args[1]);
					channel->sendToChannel(NULL, SEND_TOPIC(user->getFullname(), channel->getChannelName(), channel->getTopic()));
				}
				else
				{
					channel->setTopic(msg.trailing);
					channel->sendToChannel(NULL, SEND_TOPIC(user->getFullname(), channel->getChannelName(), channel->getTopic()));
				}
			}
		}
	}
}
/* #endregion */

/* #region MODE */

/* #region convert modeVal <-> char */

static char		type_to_c(modeType type) { return (type == PLUS ? '+' : '-'); }

/* static char 	val_to_c(modeVal val)
{
	switch (val)
	{
		case I_MODE: return ('i');
		case K_MODE: return ('k');
		case L_MODE: return ('l');
		case O_MODE: return ('o');
		case T_MODE: return ('t');
		default: return (' ');
	}
} */

static modeVal	c_to_val(char c)
{
	switch(c)
	{
		case 'i': return (I_MODE);
		case 'k': return (K_MODE);
		case 'l': return (L_MODE);
		case 'o': return (O_MODE);
		case 't': return (T_MODE);
		default: return (BAD_MODE);
	}
}
/* #endregion */

/* #region Constructor / Destructor */
Mode::Mode(Server *server): Command(server) { _last = PLUS; }
Mode::~Mode() {  }
/* #endregion */

/* #region Parsing */
static std::string	parseMode(std::queue<std::pair<char, modePair> > &mods, std::queue<std::string> &params, std::vector<std::string> &args)
{
	// If no + or - given, server considers it as a +
	modeType	last = PLUS;

	// args[0] is the target
	std::string	target = args[0];

	// if no more args, return
	if (args.size() == 1)
		return target;

	// For args[1], get every char to convert into modePair and store both
	for (std::string::iterator it = args[1].begin(); it != args[1].end(); ++it)
	{
		char	c = *it;
		if (c == '+')
			last = PLUS;
		else if (c == '-')
			last = MINUS;
		else
		{
			modePair	mp;
			mp.first = last;
			mp.second = c_to_val(c);
			std::pair<char, modePair>	result;
			result.first = c;
			result.second = mp;
			mods.push(result);
		}
	}

	// For all arguments in args, except the first, fills params
    for (size_t i = 2; i < args.size(); ++i) {
        params.push(args[i]);
    }
	
	return (target);
}


/* #endregion */

/* #region Execute */

void	Mode::execute(User *user, s_msg &msg)
{

	/* #region Basic errors */
	// user is not registered
	if (user->getStatus() != REGISTERED)
		user->sendToClient(ERR_NOTREGISTERED(user->getNickname(), "MODE"));

	// no param given
	else if (msg.args.empty())
	user->sendToClient(ERR_NEEDMOREPARAMS(user->getNickname(), "MODE"));
	/* #endregion */

	else
	{
		std::queue<std::pair<char, modePair> >		mods;
		std::queue<std::string>						params;
		std::string									target = parseMode(mods, params, msg.args);
		Channel										*channel = _server->findChannel(target);

		_modsToSend.clear();
		_paramsToSend.clear();

	/* #region Target incorrect */
		// if target is an existing OTHER user
		if (target != user->getNickname() && _server->getUserWithNickname(target) != NULL)
			user->sendToClient(ERR_USERSDONTMATCH(user->getNickname()));

		// if target is not a channel nor an existing user
		else if (_server->getUserWithNickname(target) == NULL && !channel)
			user->sendToClient(ERR_NOSUCHCHANNEL(user->getNickname(), target));
	/* #endregion */

	/* #region Target is himself */
		// if target is the user himself
		else if (target == user->getNickname())
		{
			// no mods -> show user's modes
			if (mods.empty())
				user->sendToClient(RPL_UMODEIS(user->getNickname(), (user->isServerOp() ? "+o" : "+")));

			// there is modes
			else
			{
				bool	gotwrong = false;   // if there is unkown flags

				while (!mods.empty())
				{
					std::pair<char, modePair>	it = mods.front();
					modePair	mp = it.second;

					// MODE asked is O
					if (mp.second == O_MODE)
					{
							// if +o, RFC say "DO NOTHING", but -o is accepted
							if (mp.first == MINUS)
							{
								user->setServerOP(false);
								user->sendToClient(SEND_MODE_USER(user->getFullname(), user->getNickname(), "-o"));
							}

					}
					// MODE is anything else than o
					else
						gotwrong = true;

					// remove the treated modePair from mods
					mods.pop();
				}

				// if param isn't empty, it's like a wrong mode
				if (!params.empty())
					gotwrong = true;

				// if there is any wrong mode during tests, send 501
				if (gotwrong)
					user->sendToClient(ERR_UMODEUNKNOWNFLAG(user->getNickname()));
			}
		}
	/* #endregion */

	/* #region Target is a channel */
		// target is an existing channel
		else
		{
		/* #region MODE #channel no args */
			//no mods are given
			if (mods.empty())
			{
				// client is a member of the channel
				if (channel->findUserInChannel(user->getNickname()) != NULL)
					user->sendToClient(RPL_CHANNELMODEIS(user->getNickname(), channel->getChannelName(), channel->listModes(true)));
				else
					user->sendToClient(RPL_CHANNELMODEIS(user->getNickname(), channel->getChannelName(), channel->listModes(false)));
			}
		/* #endregion */

		/* #region User not OP */
			// user is not OP
			else if (!channel->isOperator(user->getNickname()))
				user->sendToClient(ERR_CHANOPRIVSNEEDED(user->getNickname(), channel->getChannelName()));
		/* #endregion */

			// Will treat each mode requested
			else
			{
				// Tab used to know if a mode has been already treated
				bool	happened[5] = {false};

				while (!mods.empty())
				{
					std::pair<char, modePair>	it = mods.front();
					modePair	mp = it.second;

				/* #region Unkown Mode */
					// mod is not existing
					if (mp.second == BAD_MODE)
						user->sendToClient(ERR_UNKNOWNMODE(user->getNickname(), it.first));
				/* #endregion */

				/* #region MODE i */
					// mod is i and no i mode was not called before
					else if (mp.second == I_MODE && !happened[I_MODE])
					{
						// add/remove INVITE-ONLY mode to channel
						channel->setMode(INVITE_ONLY, mp.first);

						// add the action to the list of what happened during this command MODE
						happened[I_MODE] = true;

						// fill the modes to send
						if (_last != mp.first || _modsToSend.empty())
						{
							_last = mp.first;
							_modsToSend += type_to_c(_last);
						}
						_modsToSend += 'i';
					}
				/* #endregion */

				/* #region MODE k */
					// mod is k, no k was called before, and there is param
					else if (mp.second == K_MODE && !happened[K_MODE] && !params.empty())
					{
						// +k
						if (mp.first == PLUS)
						{
							channel->setPassword(params.front());
							if (!_paramsToSend.empty())
								_paramsToSend += " ";
							_paramsToSend += params.front();
						}
						// -k
						else
						{
							if (!_paramsToSend.empty())
								_paramsToSend += " ";

							// channel had a password
							if (channel->isMode(KEY))
								_paramsToSend += channel->getPassword();
							// channel had no password;
							else
								_paramsToSend += params.front();
							channel->setPassword("");
						}

						// ADD/REMOVE the mode from channel + complete happened
						channel->setMode(KEY, mp.first);
						happened[K_MODE] = true;

						if (_last != mp.first || _modsToSend.empty())
						{
							_last = mp.first;
							_modsToSend += type_to_c(_last);
						}
						_modsToSend += 'k';

						// delete first parameter
						params.pop();
					}
				/* #endregion */

				/* #region MODE l */
					// Mode is l, called for first time
					else if (mp.second == L_MODE && !happened[L_MODE])
					{
						// no params for a +l
						if (mp.first == PLUS && params.empty())
							user->sendToClient(ERR_NEEDMOREPARAMS(user->getNickname(), "MODE +l"));
						
						// -l with no params
						else if (mp.first == MINUS && params.empty())
						{
							channel->setMaxUsers(0);
							channel->setMode(LIMIT, MINUS);
							if (_last != mp.first || _modsToSend.empty())
							{
								_last = mp.first;
								_modsToSend += type_to_c(_last);
							}
							_modsToSend += 'l';
						}
						// params given
						else
						{
							std::string	p = params.front();
							int			max = atoi(p.c_str());

							if (mp.first == MINUS)
							{
								channel->setMaxUsers(0);
								channel->setMode(LIMIT, MINUS);
							}
							else
							{
								// a correct value has been given
								if (max > 0)
								{
									channel->setMaxUsers(max);
									channel->setMode(LIMIT, PLUS);
									if (!_paramsToSend.empty())
										_paramsToSend += " ";
									_paramsToSend += params.front();
								}
							}
							if (max > 0 || mp.first == MINUS)
							{
								// fill _modsToSend
								if (_last != mp.first || _modsToSend.empty())
								{
									_last = mp.first;
									_modsToSend += type_to_c(_last);
								}
								_modsToSend += 'l';
							}
							params.pop();
						}
					}
				/* #endregion */

				/* #region MODE o */
					// Mode is o, called for first time and have params
					else if (mp.second == O_MODE && !happened[O_MODE] && !params.empty())
					{
						// user doesn't exist
						if (_server->getUserWithNickname(params.front()) == NULL)
						{
							user->sendToClient(ERR_NOSUCHNICK(user->getNickname(), params.front()));
							user->sendToClient(ERR_USERNOTINCHANNEL(user->getNickname(), params.front(), channel->getChannelName()));
						}
						// user exist but is not in channel
						else if (channel->findUserInChannel(params.front()) == NULL)
							user->sendToClient(ERR_USERNOTINCHANNEL(user->getNickname(), params.front(), channel->getChannelName()));

						// user is in channel
						else
						{
							if (mp.first == PLUS)
								channel->setUserLevel(channel->findUserInChannel(params.front()), OPERATOR);
							else
								channel->setUserLevel(channel->findUserInChannel(params.front()), NORMAL);
							happened[O_MODE] = true;
							if (_last != mp.first || _modsToSend.empty())
							{
								_last = mp.first;
								_modsToSend += type_to_c(_last);
							}
							_modsToSend += 'o';
							if (!_paramsToSend.empty())
								_paramsToSend += " ";
							_paramsToSend += params.front();
							params.pop();
						}
					}
				/* #endregion */

				/* #region MODE t */
					// mod is t and no t mode was not called before
					else if (mp.second == T_MODE && !happened[T_MODE])
					{
						// add/remove topic restrictions to channel
						channel->setMode(TOPIC, mp.first);

						// add the action to the list of what happened during this command MODE
						happened[T_MODE] = true;

						// fill the modes to send
						if (_last != mp.first || _modsToSend.empty())
						{
							_last = mp.first;
							_modsToSend += type_to_c(_last);
						}
						_modsToSend += 't';
					}
				/* #endregion */

					// remove the mode that just has been treated
					mods.pop();
				}

				// send the final message including all modifications done
				if (!_modsToSend.empty())
				{
					std::string fullMods = _modsToSend + " " + _paramsToSend;
					channel->sendToChannel(NULL, SEND_MODE_CHAN(user->getFullname(), channel->getChannelName(), fullMods));
				}
			}
		}
	}
	/* #endregion */
}
/* #endregion */

/* #endregion */

/* #region OPER */

Oper::Oper(Server *server): Command(server) {  }
Oper::~Oper() {  }

void	Oper::execute(User *user, s_msg &msg)
{
		// user is not registered
	if (user->getStatus() != REGISTERED)	
		user->sendToClient(ERR_NOTREGISTERED(user->getNickname(), "OPER"));

		// Not enough arguments given
	else if (msg.args.empty() || msg.args.size() != 2)
		user->sendToClient(ERR_NEEDMOREPARAMS(user->getNickname(), "OPER"));

		// Nickname or password is incorrect
	else if (msg.args[0] != OPLOGIN || msg.args[1] != OPPASS)
		user->sendToClient(ERR_NOOPERHOST(user->getNickname()));

		// Oper accepted
	else
	{
		user->setServerOP(true);
		user->sendToClient(RPL_YOUREOPER(user->getNickname()));
		user->sendToClient(SEND_MODE_USER(user->getFullname(), user->getNickname(), "+o"));
	}


}
/* #endregion */

/* #region POWEROFF */

/**
 * @brief This Command is added in order to have a server-OP only example
 */

Poweroff::Poweroff(Server *server): Command(server) {  }
Poweroff::~Poweroff() {  }

void	Poweroff::execute(User *user, s_msg &msg)
{
	(void)msg;
		// user is not registered
	if (user->getStatus() != REGISTERED)
		user->sendToClient(ERR_NOTREGISTERED(user->getNickname(), "POWEROFF"));

		// user is not server Operator
	else if (!user->isServerOp())
		user->sendToClient(ERR_NOPRIVILEGES(user->getNickname()));

		// user is allowed to shut down the server
	else
		_server->shutdown();
}
/* #endregion */
