#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "ft_irc.hpp"

/* #region Definitions */
typedef std::list<User*>::iterator	user_iterator;
/* #endregion */

class User;

class Channel
{
	private:

		//Attributes
		Server			*_server;
		std::string		_channelName;

		channelModes	_modes;
		std::string		_topic;				// (+t and TOPIC)
		std::string		_password;			// (+k)
		int				_maxUsers;			// (+l)

		std::list<User*>	_operators;		// (+o)
		std::list<User*>	_normalUsers;
		std::list<User*>	_invitedUsers;	// (+i)

		User	*findUserFromList(std::list<User *> &role, std::string nickname);
		bool	isChannelEmpty();

	public:
		
		/* #region Constructor/Destructor */
		Channel(Server *server, std::string name, User *user); //Without key
		Channel(Server *server, std::string name, User *user, std::string const &password); //With key
		~Channel();
		/* #endregion */

		/* #region GETTERS */
		std::string	const	&getChannelName() const;
		std::string	const	&getPassword()const;
		std::string	const	&getTopic() const;
		int					getMaxUsers() const;

		bool	isInvited(std::string const &nickname);
		bool	isOperator(std::string const &nickname);
		bool	isNormal(std::string const &nickname);

		int		getTotalUsers() const;
		bool	isPasswordCorrect(std::string const &pass);
		/* #endregion */

		/* #region SETTERS */
		void			setLevel(User *user, userLevel lvl);
		void			setPassword(std::string const &password);
		void			setTopic(std::string const &topic);
		void			setMaxUsers(int max);
		/* #endregion */

		/* #region MODES */
		// mode : INVITE_ONLY, KEY, LIMIT or TOPIC
		void		setMode(channelModes mode, modeType type);
		bool		isMode(channelModes mode) const;	// test if a mode is activated
		std::string	listModes(bool isMember) const;		// list mods for channel adherents

		/* #endregion */

		//todo : welcomeUser() must be perfected
		//todo : Adduser() invitation messages
		//todo : setUserLevel() messages
		/* #region User Add/Set/Remove */
		void	addUser(User *user, userLevel lvl);
		void	removeUser(User *user);
		void	setUserLevel(User *user, userLevel lvl);
		
		User	*findUserInChannel(std::string nickname);
		
		void	displayUsers(User *);
		void	welcomeUser(User *user);
		/* #endregion */

		void				sendToChannel(User *user, std::string const &msg);
		std::string const	sendTopic(User *user) const;

	private:

		//UNUSED COPLIEN
		Channel();
		Channel(Channel const &toCopy);
		Channel	&operator=(Channel const &toAssign);

};

#endif
