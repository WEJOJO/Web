#include "get_next_line.h"
#include "StateCode.hpp"

t_fdnode	*ft_getfdnode(t_fdnode **node, int fd)
{
	t_fdnode	*temp;
	t_fdnode	*new_node;

	temp = *node;
	if (!temp)
	{
		temp = ft_makenewnode(fd);
		*node = temp;
		return (*node);
	}
	while (temp)
	{
		if ((temp->fd) == fd)
			return (temp);
		if (!(temp->next))
		{
			new_node = ft_makenewnode(fd);
			if (!new_node)
				return (NULL);
			temp->next = new_node;
			new_node->prev = temp;
		}
		temp = temp->next;
	}
	return (temp);
}

t_fdnode	*ft_makenewnode(int fd)
{
	t_fdnode	*newfdnode;

	newfdnode = (t_fdnode *)malloc(sizeof(t_fdnode));
	if (!newfdnode)
		return (NULL);
	newfdnode->fd = fd;
	newfdnode->next = NULL;
	newfdnode->prev = NULL;
	newfdnode->save = NULL;
	newfdnode->cntn = 0;
	return (newfdnode);
}

int	ft_delnode(t_fdnode *node, t_fdnode **h)
{
	if (node == *h)
		*h = node->next;
	if (node->prev)
		node->prev->next = node->next;
	if (node->next)
		node->next->prev = node->prev;
	node->next = NULL;
	node->prev = NULL;
	node->cntn = 0;
	node->fd = -1;
	if (node->save)
		free(node->save);
	node->save = NULL;
	free(node);
	node = NULL;
	return (0);
}

int	ft_makesave(t_fdnode **h, t_fdnode *t, int fd, char *bf)
{
	ssize_t	i[2];

	i[0] = 1;
	while (!(t->cntn) && i[0] > 0)
	{
		i[0] = read(fd, bf, BUFFER_SIZE);
		if (i[0] == - 1)
			throw ReadFail();
		i[1] = 0;
		while (i[1] < i[0])
			if (bf[i[1]++] == '\n')
				t->cntn++;
		if (!(t->save))
		{
			t->save = (char *)malloc((i[0] + 1) * sizeof(char));
			if (i[0] <= 0 || !(t->save))
				return (ft_delnode(t, h));
			i[1] = 0;
			while (i[1] < i[0] + 1)
				t->save[i[1]++] = '\0';
		}
		i[1] = i[0];
		while (i[1] != BUFFER_SIZE && bf[i[1]])
			bf[i[1]++] = '\0';
		t->save = ft_strjoin(t->save, bf, i[0]);
	}
	return (1);
}

char	*get_next_line(int fd)
{
	static t_fdnode	*head;
	t_fdnode		*temp;
	char			*answer;
	char			bf[BUFFER_SIZE + 1];
	ssize_t			i;

	if (fd < 0 || BUFFER_SIZE <= 0)
		return (NULL);
	i = 0;
	while (i <= BUFFER_SIZE)
		bf[i++] = '\0';
	temp = ft_getfdnode(&head, fd);
	if (!temp)
		return (NULL);
	if (!(ft_makesave(&head, temp, fd, bf)))
		return (NULL);
	answer = ft_makeanswer(&(temp->save));
	if (temp->cntn)
		temp->cntn--;
	if (!answer || !(temp->save))
		ft_delnode(temp, &head);
	return (answer);
}