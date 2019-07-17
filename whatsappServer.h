
//#ifndef EX4_SERVER_H
#define EX4_SERVER_H

using namespace std;
#include "whatsappio.h"


class Server
{
public:
    Server(int portNum);

    void
    create_group_of_clients(string &groupName, vector<string> &list_of_client_names, int portNum, const string creator);

    void send(string name, string message);

    void who();

    int exit();

    int checkName(string name);
    int createNewSocket(int port_num);
    int initializeClient(int socket);
//    char buffer[WA_MAX_MESSAGE];
    int writeToClient(int sockfd, char *buffer);
    int readFromClient(int sockfd, char *buffer);
//    int portNum;
    vector <string> checkForDuplicate(vector <string> list_of_client_names);
    void serverSend(string clientName, string sendToHim, string message);
    vector<string> groupNames;
//    vector<string> clientNames;
    map<string, int> clientMap;
    void serverWho(string clientName, int portNum);

    void serverExit(string clientName, int portNum);
    bool exitCommand(command_type checkForExit);
    int lengthOfBuffer(char *buffer);

private:

    map<string, vector<string>> allGroups;
    string command;
    string client;
    bool checkGroupName(string name);

    bool findClientInGroup(string name, vector <string> group);


    bool findClientInClientsMap(string checkClient, map<string, int> clientsMap);

    char *convertToFormat(char operation, const char *message, string clientName);

    void parseToVector(string &input, vector <string> &output);
};