#include "get_next_line.h"

char	*ft_strdup(char *s)
{
	char	*st;
	size_t	len;
	size_t	i;
	size_t	index;

	i = 0;
	if (!(*s))
		return (NULL);
	len = 0;
	while (s[len])
		len++;
	st = (char *)malloc((len + 1) * sizeof(char));
	if (!(st))
		return (NULL);
	index = 0;
	while (index < (len + 1))
		st[index++] = '\0';
	while (s[i])
	{
		st[i] = s[i];
		i++;
	}
	st[i] = s[i];
	return (st);
}

char	*ft_strjoin(char *s1, char *s2, size_t sizes2)
{
	char	*dst;
	size_t	lens1;
	size_t	index;

	if (!s1 || !s2)
		return (NULL);
	lens1 = 0;
	while (s1[lens1])
		lens1++;
	dst = (char *) malloc ((lens1 + sizes2 + 1) * sizeof(char));
	if (!(dst))
	{
		free((char *)s1);
		return (NULL);
	}
	index = 0;
	while (index < (lens1 + sizes2 + 1))
		dst[index++] = '\0';
	ft_strlcpy (dst, s1, lens1 + 1);
	ft_strlcat (dst, s2, lens1 + sizes2 + 1);
	index = 0;
	while (s1[index])
		s1[index++] = '\0';
	free(s1);
	return (dst);
}

size_t	ft_strlcat(char *dst, char *src, size_t size)
{
	char		*stdst;
	char		*stsrc;
	size_t		i;
	size_t		dstlen;

	dstlen = 0;
	while (dst[dstlen])
		dstlen++;
	stdst = dst;
	stsrc = src;
	i = 0;
	if (dstlen > size)
		return (0);
	while (*stdst)
		stdst++;
	while (*stsrc && dstlen + 1 + i < size)
	{
		*stdst = *stsrc;
		stdst++;
		stsrc++;
		i++;
	}
	*stdst = '\0';
	return (0);
}

size_t	ft_strlcpy(char *dst, char *src, size_t dstsize)
{
	size_t		i;
	char		*st;
	char		*stdst;

	if (!dst)
		return (0);
	st = src;
	stdst = dst;
	if (dstsize == 0)
		return (0);
	i = 0;
	while (*st && ((i + 1) < dstsize))
	{
		*stdst = *st;
		stdst++;
		st++;
		i++;
	}
	*stdst = '\0';
	return (0);
}

char	*ft_makeanswer(char **s)
{
	char	*dst;
	size_t	index;
	char	*tsave;
	size_t	i;

	index = 0;
	while ((*s)[index] && (*s)[index] != '\n')
		index++;
	if ((*s)[index])
		index++;
	if (!(*s) || index <= 0)
		return (NULL);
	dst = (char *) malloc ((index + 1) * sizeof(char));
	if (dst)
	{
		i = 0;
		while (i < index + 1)
			dst[i++] = '\0';
	}
	ft_strlcpy(dst, *s, index + 1);
	tsave = *s;
	*s = ft_strdup(*s + (sizeof(char) * index));
	free (tsave);
	return (dst);
}