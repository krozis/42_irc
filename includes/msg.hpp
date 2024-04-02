#ifndef MESSAGES_HPP
# define MESSAGES_HPP

# include <ctime>

/* #region Colors */
// Display colors

# define RED "\e[31m"
# define CYAN "\e[36m"
# define YELLOW "\e[43m"
# define GREY "\e[90m"
# define NO_COLOR "\e[0m"
/* #endregion */

/* #region Macros */
//Macros MSG

# define MSG_DEV(msg, val) std::cout << CYAN << "-> " << msg << val << NO_COLOR << std::endl
# define MSG_ERR(msg) std::cerr << RED << "Error: " << msg << NO_COLOR << std::endl
# define MSG_IN(msg) std::cout << YELLOW << msg << NO_COLOR << std::endl

// Misc

# define SVR_PREFIX to_string(":") + SVR_NAME
/* #endregion */

/* #region Servers Messages */
// Server

# define MSG_SVR_CREATED	"IRC server created."
# define MSG_SVR_START		"IRC server starting..."
# define MSG_SVR_STARTED	"IRC server started."
# define MSG_SVR_WELCOME	"Welcome to this IRC Server!"
# define MSG_SVR_SHUTDOWN	"Shutting down IRC server..."
# define MSG_SVR_END		"IRC server turned OFF"
# define MSG_SVR_EXIT_SIG	"Exit signal received, program will leave..."

// Client

# define MSG_CLT_CONNECTED(socket)			"A new client has been connected on socket " + to_string(socket) + "!"
# define MSG_CLT_DISCONNECTED(socket)		"Client on socket " + to_string(socket) + " has been disconnected."
# define MSG_CLT_NICK(socket, nick)			"User at socket " + to_string(socket) + " is now known as " + nick
# define MSG_CLT_USER(socket, name, real)	"User at socket " + to_string(socket) + "'s name is " + name + " (" + real + ")"
# define MSG_CLT_SVRSHUTDOWM				SVR_PREFIX + " :server now turned OFF."
# define MSG_CLT_QUIT(nick)					SVR_PREFIX + " " + nick + " :Good by, " + nick + "!"

// Channel

# define MSG_CHAN_CREATED(nick, chan)	"New channel created by " + nick + ": " + chan

// Dev

# define MSG_DEV_PORT_IS_SET 			"PORT HAS BEEN SET: "
# define MSG_DEV_SVR_SOC_CREATED		"SERVER SOCKET CREATED: "
# define MSG_DEV_SVR_SOC_SETOPT			"OPTION SO_REUSEADDR SET ON SERVER SOCKET: "
# define MSG_DEV_SVR_SOC_NONBLOCK		"SERVER SOCKER SET UP TO NONBLOCK"
# define MSG_DEV_SVR_IP_SETUP			"SERVER IP CONFIGURED: "
# define MSG_DEV_SVR_SOC_IPv4_BINDED	"SERVER SOCKET BINDED TO IPv4 ADDRESS"
# define MSG_DEV_SVR_SOC_LISTEN			"SERVER SOCKET LISTENING MODE ENABLED"

// Misc

# define SEND_NICK(full, nick)						":" + full + " NICK :" + nick
# define SEND_INVIT(full, nick, chan)				":" + full + " INVITE " + nick + " :" + chan		// to send to invited user
# define SEND_PM(from, to, msg)						":" + from + " PRIVMSG " + to + " :" + msg
# define SEND_PONG(nick)							SVR_PREFIX + " PONG " + SVR_NAME + " :" + nick		// response to PING
# define SEND_PART(full, chan)						":" + full + " PART " + chan
# define SEND_PART_MSG(full, chan, msg)				":" + full + " PART " + chan + " :" + msg
# define SEND_JOIN(full, chan)						":" + full + " JOIN :" + chan
# define SEND_QUIT(full, nick)						":" + full + " QUIT :Quit: " + nick
# define SEND_QUIT_MSG(full, msg)					":" + full + " QUIT :Quit: " + msg
# define SEND_KICK(full, chan, target, nick)		":" + full + " KICK " + chan + " " + target + " :" + nick
# define SEND_KICK_MSG(full, chan, target, msg)		":" + full + " KICK " + chan + " " + target + " :" + msg
# define SEND_MODE_USER(full, nick, mode)			":" + full + " MODE " + nick + " :" + mode
# define SEND_MODE_CHAN(full, chan, mode)			":" + full + " MODE " + chan + " " + mode
# define SEND_TOPIC(full, chan, topic)					":" + full + " TOPIC " + chan + " :" + topic

// ft_irc

# define ERR_SVR_USAGE(prog_name)	"Usage: " + prog_name + " <port> <password>"
# define ERR_SVR_PORT				"Impossible port extraction: "

# define ERR_CLT_NO_PASS_NICK		"Password is required before using NICK."
/* #endregion */

/* #region RPL_ERR */

/**
 * Correct format for an server -> client response :
 * :<servername> <messagecode> <nickname> :<message>
 * For and example: 
 * :B&S 001 stelie :Welcome to B&S IRC server!
 *
 * @exception NOTICE for the notice messages the format is :
 * :B&S NOTICE * :***your notice message here
 * 
 * stars * replace non known values as the nickname if not already given
 */


// UNOFFICIAL RPLs and ERR

# define ERR_INVALIDUSERNAME(username)			SVR_PREFIX + " 468 " + username + ": Invalid username"
# define ERR_INVALIDREALNAME(realname)			SVR_PREFIX + " " + realname + ": Invalid realname"
# define ERR_BADCHANNAME(nickname, wrongname)	SVR_PREFIX + " 479 " + nickname + " " + wrongname + " :Invalid channel name." // Not official, used in irc.net / User if channel name doesn't start with & or #
# define ERR_SYNTAX(nickname, command)			SVR_PREFIX + " " + nickname + " " + command + " :Syntax error."


// RPL_MSG_CODE - NOT VERIFIED


// RPL_MSG_CODE : VALID

# define RPL_WELCOME(nick, full)					SVR_PREFIX + " 001 " + nick + " :Welcome to the B&S IRC server " + full
# define RPL_YOURHOST(nick)							SVR_PREFIX + " 002 " + nick + " :You host is " + SVR_NAME + ", running version 1.0."
# define RPL_CREATED(nick)							SVR_PREFIX + " 003 " + nick + " :This server was created in early 2024."
# define RPL_MYINFO(nick)							SVR_PREFIX + " 004 " + nick + " :" + SVR_NAME + " v1.0 o iklot"

# define RPL_UMODEIS(nick, modes)					SVR_PREFIX + " 221 " + nick + " " + modes
# define RPL_LUSERCLIENT(nick, nb)					SVR_PREFIX + " 251 " + nick + " :There are " + nb + " users and 0 invisible on 1 server"
# define RPL_LUSEROP(nick, nb)						SVR_PREFIX + " 252 " + nick + " " + nb + " :IRC Operators online"
# define RPL_LUSERCHANNELS(nick, nb)				SVR_PREFIX + " 254 " + nick + " " + nb + " :channels formed"
# define RPL_LUSERME(nick, nb)						SVR_PREFIX + " 255 " + nick + " :I have " + nb + " clients and 1 servers"

# define RPL_CHANNELMODEIS(nick, chan, mods)		SVR_PREFIX + " 324 " + nick + " " + chan + " +" + mods										// List mods activated on a channel
# define RPL_NOTOPIC(nick, chan)					SVR_PREFIX + " 331 " + nick + " " + chan + " :No topic set."
# define RPL_TOPIC(nick, chan, topic)				SVR_PREFIX + " 332 " + nick + " " + chan + " :" + topic
# define RPL_INVITING(nick, invited, chan)			SVR_PREFIX + " 341 " + nick + " " + invited + " " + chan									// Reply for a successful INVITE
# define RPL_NAMREPLY(nick, chan, list)				SVR_PREFIX + " 353 " + nick + " = " + chan + " :" + list
# define RPL_ENDOFNAMES(nick, chan)					SVR_PREFIX + " 366 " + nick + " " + chan + " :End of /NAMES list."
# define RPL_YOUREOPER(nick)						SVR_PREFIX + " 381 " + nick + " :You are now server Operator."

// ERR_MSG_CODE : VALID

# define ERR_NOSUCHNICK(nick, target)				SVR_PREFIX + " 401 " + nick + " " + target + " :No such nick."							// Target nickname not found
# define ERR_NOSUCHCHANNEL(nick, chan)				SVR_PREFIX + " 403 " + nick + " " + chan + " :No such channel."							// Channel doesn't exist
# define ERR_CANNOTSENDTOCHAN(nick, chan)			SVR_PREFIX + " 404 " + nick + " " + chan + " :Cannot send to channel."					// User have not the right to send to channel
# define ERR_NOORIGIN(nick)							SVR_PREFIX + " 409 " + nick + " :No origin specified."									// PING without origin
# define ERR_NORECIPIENT(nick)						SVR_PREFIX + " 411 " + nick + " :No recipient given."									// PRIVMSG with no destination for
# define ERR_NOTEXTTOSEND(nick)						SVR_PREFIX + " 412 " + nick + " :No text to send."										// PRIVMSG without the message
# define ERR_UNKNOWNCOMMAND(nick, cmd)				SVR_PREFIX + " 421 " + nick + " " + cmd + " :Unknown command"							// Command doesn't exist
# define ERR_NONICKNAMEGIVEN(nick)					SVR_PREFIX + " 431 " + nick + " :No nickname given."									// When nickname parameter for a command is not found
# define ERR_ERRONEUSNICKNAME(nick, wrong)			SVR_PREFIX + " 432 " + nick + " " + wrong + " :Nickname is invalid."					// Nickname doesn't respect rules.
# define ERR_NICKNAMEINUSE(nick, wrong)				SVR_PREFIX + " 433 " + nick + " " + wrong + " :Nickname is already in use."				// Another user already uses this nickname
# define ERR_USERNOTINCHANNEL(nick, wrong, chan)	SVR_PREFIX + " 441 " + nick + " " + wrong + " " + chan + " :User not in this channel."	// Doing channel action on an user which is not in this channel
# define ERR_NOTONCHANNEL(nick, chan)				SVR_PREFIX + " 442 " + nick + " " + chan + " :You're not in this channel."				//Try to perform an action for a channel where he's not member
# define ERR_NOTREGISTERED(nick, cmd)				SVR_PREFIX + " 451 " + nick + " " + cmd + " :You must register."						// Using a command while not registered
# define ERR_NEEDMOREPARAMS(nick, cmd)				SVR_PREFIX + " 461 " + nick + " " + cmd + " :Need more parameters"						// Not enough parameters given to the command
# define ERR_ALREADYREGISTERED(nick)				SVR_PREFIX + " 462 " + nick + " :You are already registered." 							// Returned by the server to any link which attempts to register again 
# define ERR_PASSWDMISMATCH(nick)					SVR_PREFIX + " 464 " + nick + " :Incorrect password, access denied."					// Incorrect password was given to access the server or to make OPER 
# define ERR_CHANNELISFULL(nick, chan)				SVR_PREFIX + " 471 " + nick + " " + chan + " :Cannot join channel (+l)."				// Tryin to join a channel with a limit of user that was reached
# define ERR_UNKNOWNMODE(nick, mode)				SVR_PREFIX + " 472 " + nick + " " + mode + " :Unkown mode."								// The mode requested doesnt exists.
# define ERR_INVITEONLYCHAN(nick, chan)				SVR_PREFIX + " 473 " + nick + " " + chan + " :Cannot join channel (+i)."				// Trying to join an invite-only channel when not invited
# define ERR_BADCHANNELKEY(nick, chan)				SVR_PREFIX + " 475 " + nick + " " + chan + " :Cannot join channel (+k)."				// Trying to join a channel without the correct password
# define ERR_NOPRIVILEGES(nick)						SVR_PREFIX + " 481 " + nick + " :You are not server Operator."							// Action that requires IRC operator privileges
# define ERR_CHANOPRIVSNEEDED(nick, chan)			SVR_PREFIX + " 482 " + nick + " " + chan + " :You're not channel operator."				// Action that requires channel operator privileges
# define ERR_NOOPERHOST(nick)						SVR_PREFIX + " 491 " + nick + " :Operator status refused."								// Negative response to OPER command
# define ERR_UMODEUNKNOWNFLAG(nick)					SVR_PREFIX + " 501 " + nick + " :Unkown MODE flag."										// User MODE not recognized
# define ERR_USERSDONTMATCH(nick)					SVR_PREFIX + " 502 " + nick + " :Cannot change mode for other users."					// MODE on another user

/* #endregion */

/* #region Templates/Functions */
/**
 * @brief convert compatible entry to a std::string
 * 
 * @tparam T 
 * @param value value to convert
 * @return std::string 
 */
template <typename T>
std::string to_string(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

/**
 * @brief Display message with the actual date hour
 * 
 * @param msg message to display
 */
static inline void	msg_log(std::string const &msg)
{
	time_t	time_raw;
	tm		*timeinfo;
	char	buffer[100];

	time(&time_raw);
	timeinfo = localtime(&time_raw);

	strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S :", timeinfo);
	std::string	time_str(buffer);

	std::cout << GREY << time_str << NO_COLOR << " " << msg << std::endl;
}
/* #endregion */

#endif