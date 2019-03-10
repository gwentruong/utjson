#ifndef __TOKEN_H
#define __TOKEN_H

int   is_structural(char c);                // Check if token is structural
int   is_whitespace(char c);
char *rm_leading_whitespaces(char *buf);    // Delete whitespace in between
char *tokenizer(char *buf_in, char *token); // Take apart each tokens

#endif
