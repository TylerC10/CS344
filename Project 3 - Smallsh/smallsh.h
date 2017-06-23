#ifndef SMALLSH_H
#define SMALLSH_H


void catchSIGINT(int signal);
void showStatus(int status);
void catchSigStop(int signal);
#endif