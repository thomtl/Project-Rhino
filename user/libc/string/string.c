#include "../include/string.h"

int strcmp(char s1[], char s2[])
{
    int i;
    for (i = 0; s1[i] == s2[i]; i++)
    {
        if (s1[i] == '\0')
            return 0;
    }
    return s1[i] - s2[i];
}

char *strcpy(char *strDest, char *strSrc)
{
    char *temp = strDest;
    while ((*strDest++ = *strSrc++))
        ; // or while((*strDest++=*strSrc++) != '\0');
    return temp;
}

size_t strlen(const char *str)
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

void reverse(char s[])
{

    for (int i = 0, j = (int)(strlen(s) - 1); i < j; i++, j--)
    {
        int c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

char *strchr(const char *s, int c)
{
    while (*s != (char)c)
        if (!*s++)
            return 0;
    return (char *)s;
}

size_t strcspn(const char *s1, const char *s2)
{
    size_t ret = 0;
    while (*s1)
        if (strchr(s2, *s1))
            return ret;
        else
            s1++, ret++;
    return ret;
}

size_t strspn(const char *s1, const char *s2)
{
    size_t ret = 0;
    while (*s1 && strchr(s2, *s1++))
        ret++;
    return ret;
}

char *strtok(char *str, const char *restrict delim)
{
    static char *p = 0;
    if (str)
        p = str;
    else if (!p)
        return 0;
    str = p + strspn(p, delim);
    p = str + strcspn(str, delim);
    if (p == str)
        return p = 0;
    p = *p ? *p = 0, p + 1 : 0;
    return str;
}

void *memset(void *bufptr, int value, size_t size)
{
    unsigned char *buf = (unsigned char *)bufptr;
    for (size_t i = 0; i < size; i++)
        buf[i] = (unsigned char)value;
    return bufptr;
}
