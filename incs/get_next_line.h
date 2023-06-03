#ifndef GET_NEXT_LINE_H
# define GET_NEXT_LINE_H

# include <unistd.h>
# include <stdlib.h>

# ifndef BUFFER_SIZE
#  define BUFFER_SIZE 1
#endif
typedef struct s_fdnode
{
	int				fd;
	int				cntn;
	char			*save;
	struct s_fdnode	*next;
	struct s_fdnode	*prev;
}			t_fdnode;

t_fdnode	*ft_getfdnode(t_fdnode **node, int fd);
t_fdnode	*ft_makenewnode(int fd);
int			ft_delnode(t_fdnode *node, t_fdnode **h);
char		*get_next_line(int fd);
char		*ft_strdup(char *s);
char		*ft_strjoin(char *s1, char *s2, size_t sizes2);
size_t		ft_strlcat(char *dst, char *src, size_t size);
size_t		ft_strlcpy(char *dst, char *src, size_t dstsize);
char		*ft_makeanswer(char **s);
int			ft_makesave(t_fdnode **h, t_fdnode *t, int fd, char *bf);

#endif