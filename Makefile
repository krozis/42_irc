# Name
NAME	=	ircserv

#Colors
ifneq ($(OS),Windows_NT)
	RED			=	"\e[31m"
	GREEN		=	"\e[32m"
	END_COLOR	=	"\e[0m"
	BOLD		=	"\e[1m"
else
	RED			=	""
	GREEN		=	""
	END_COLOR	=	""
	BOLD		=	""
endif

# Compilation
include .env
CXX			=	c++
FLAGS		=	$(STDFLAGS) $(CXXFLAGS) $(DEPFLAGS) $(ENVFLAGS)
DEPFLAGS	=	-MMD -MP
CXXFLAGS	=	-Wall -Wextra -Werror
STDFLAGS	=	-std=c++98
ENVFLAGS	=	-DOPLOGIN=\"$(OPLOGIN)\" -DOPPASS=\"$(OPPASS)\"
INCLUDE		=	-I$(INC_DIR)

# Commands
RM	=	rm -rf

# Files
INC_DIR =	includes
SRC_DIR	=	srcs
OBJ_DIR	=	obj
OBJS 	=	$(addprefix $(OBJ_DIR)/,$(SRCS:.cpp=.o))
DEPS	=	$(OBJS:.o=.d)
SRCS	=	main.cpp \
			Server.cpp \
			User.cpp \
			Commands.cpp \
			Channel.cpp \

# Rules
all:	$(NAME)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o:	$(SRC_DIR)/%.cpp | $(OBJ_DIR)
				@$(CXX) $(FLAGS) $(INCLUDE) -o $@ -c $<

$(NAME):	$(OBJ_DIR) $(OBJS)
		@$(CXX) $(FLAGS) -o $@ $(OBJS)
		@echo $(GREEN)$(BOLD)$(NAME) $(END_COLOR)$(GREEN)successfully created$(END_COLOR)

clean:
		@$(RM) $(OBJ_DIR)
		@echo $(RED)Object files directory removed $(END_COLOR)

fclean:	clean
		@$(RM) $(NAME)
		@echo $(RED)$(BOLD)$(NAME) $(END_COLOR)$(RED)removed $(END_COLOR)

re:	fclean
	@$(MAKE)  --no-print-directory all
	@echo $(GREEN)Cleaned and rebuild $(BOLD)$(NAME)!$(END_COLOR)

.PHONY: all clean fclean re