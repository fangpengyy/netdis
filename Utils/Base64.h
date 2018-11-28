# ifndef _UT_BASE64_H_
# define _UT_BASE64_H_

# include <stddef.h>

extern int base64_encode(unsigned char * szBuffer, size_t  aiLen, char* szB64text, int& aiB64len);
extern int base64_decode(char *b64message, unsigned char **buffer, size_t *length);

# endif
