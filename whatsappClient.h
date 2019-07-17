#ifndef EX4_CLIENT_H
#define EX4_CLIENT_H

#include "whatsappio.h"

using namespace std;
#define MAX_CLIENTS = 30;

class Client
{
public:
    Client(char *name, string ip, int portNum);
    void create_group(vector<string> &list_of_client_names, int portNum, string groupName);
    void send(string name, string message);
    void who();
    void clientExit();
    bool checkValidLetters(string groupName, vector<string> &clientsList);
    bool checkGroupLength(vector<string> &clients, string creator);
    void checkForDuplicate(vector<string> *list_of_client_names);
    void socketFunc(int &argc, char **argv);


    int getPort();
    string getIp();
    string getName();
    int getSockfd();
    void setSockfd(int newSockfd);
    int sockfd;

private:
    char * clientName;
    string ip;
    int port;
    char buffer[WA_MAX_MESSAGE];


    void loopFunc(int sockfd);

    int writeToServer(int sockfd, char *buffer);

    string readFromServer(int sockfd);

    bool validateName(string name);

    bool corrrectSizeInput(char buffer[256]);

    void differentCommands();

    void getMessage(char *buf, ssize_t sizeOfBuffer);

    void exitByServer();

    int lengthOfBuffer(char buffer[256]);

    void parseToVector(string &input, vector<string> &output);
};


#endif //EX4_CLIENT_H
