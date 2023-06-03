NAME = webserv
CXX = c++

#리터럴 string 무시 플래그 추가
CCFLAGS =  -std=c++98  -Wall -Werror -Wextra #-Wno-write-strings #-g3 -fsanitize=address -fsanitize=undefined

INC_LINK = -I./incs

SRCS_PATH = ./srcs

SRCS_DIRS =  utils http server socket

DIRS = $(addprefix $(OBJS_PATH)/, $(SRCS_DIRS))

SRCS_NAME = webserv.cpp Php.cpp\
		server/Config.cpp server/LocationBlock.cpp server/ServerBlock.cpp \
		socket/Socket.cpp socket/EventLoop.cpp \
		http/ParseRequest.cpp http/Request.cpp http/Response.cpp \
		utils/utils.cpp utils/get_next_line.cpp utils/get_next_line_utils.cpp utils/StateCode.cpp utils/ServerFiles.cpp \


SRCS = $(addprefix $(SRCS_PATH)/, $(SRCS_NAME))

OBJS_NAME = $(SRCS_NAME:.cpp=.obj)

OBJS_PATH = ./objs

OBJS = $(addprefix $(OBJS_PATH)/, $(OBJS_NAME))

all : $(NAME)

$(NAME) : $(DIRS) $(OBJS)
	$(CXX) $(CCFLAGS) -o $(NAME) $(OBJS)

$(DIRS) :
	@mkdir $(OBJS_PATH) 2> /dev/null || true
	@mkdir $(DIRS) 2> /dev/null || true

$(OBJS_PATH)/%.obj : $(SRCS_PATH)/%.cpp
	$(CXX) $(INC_LINK) $(CCFLAGS) -c $< -o $@

clean :
	rm -rf $(OBJS_PATH)

fclean : clean
	rm -rf $(NAME)
re :
	@make fclean
	@make all

.PHONY: all clean fclean re