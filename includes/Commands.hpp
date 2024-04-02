#ifndef COMMANDS_HPP
# define COMMANDS_HPP

# include "ft_irc.hpp"

class Server;
class User;

class Command
{
	protected:

		Server	*_server;

		//UNUSED COPLIEN
			Command();
			Command(const Command& toCopy);

		public:
			Command(Server *server);
			virtual ~Command();

			virtual void	execute(User *user, s_msg &msg) = 0;

};

class Quit: public Command
{
	public:

		Quit(Server *server);
		~Quit();

		void	execute(User *user, s_msg &msg);
};

class Pass: public Command
{
	public:

		Pass(Server *server);
		~Pass();

		void	execute(User *user, s_msg &msg);
};

class Nick: public Command
{
	public:

		Nick(Server *server);
		~Nick();

		void	execute(User *user, s_msg &msg);
};

class UserCMD: public Command
{
	public:

		UserCMD(Server *server);
		~UserCMD();

		void	execute(User *user, s_msg &msg);
};

class Ping: public Command
{
	public:

		Ping(Server *server);
		~Ping();

		void	execute(User *user, s_msg &msg);
};

class Join: public Command
{
	public:

		Join(Server *server);
		~Join();

		void	execute(User *user, s_msg &msg);
};

class Part: public Command
{
	public:

		Part(Server *server);
		~Part();

		void	execute(User *user, s_msg &msg);
};

class Kick: public Command
{
	public:

		Kick(Server *server);
		~Kick();

		void	execute(User *user, s_msg &msg);
};

class Invite: public Command
{
	public:

		Invite(Server *server);
		~Invite();

		void	execute(User *user, s_msg &msg);
};

class Privmsg: public Command
{
	public:

		Privmsg(Server *server);
		~Privmsg();

		void	execute(User *user, s_msg &msg);
};

class Topic: public Command
{
	public:

		Topic(Server *server);
		~Topic();

		void	execute(User *user, s_msg &msg);
};


class Mode: public Command
{
	public:

		Mode(Server *server);
		~Mode();

		void	execute(User *user, s_msg &msg);

	private:

		modeType	_last;
		std::string	_modsToSend;
		std::string	_paramsToSend;
};

class Oper: public Command
{
	public:

		Oper(Server *server);
		~Oper();

		void	execute(User *user, s_msg &msg);
};

class Poweroff: public Command
{
	public:

		Poweroff(Server *server);
		~Poweroff();

		void	execute(User *user, s_msg &msg);
};



//Methodes de classe pour lancer un check de quelle fonction utiliser
void	execute(Server *server, User *user, s_msg &msg);
void	initCommands(Server *server, std::map<std::string, Command *> &commands);
void	deleteCommands(std::map<std::string, Command *> &commands);

#endif