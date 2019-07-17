/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <cstring>
//#include <zconf.h>
#include <netdb.h>
#include <map>
#include "whatsappServer.h"
#include <unistd.h>
#include <algorithm>
#include <iostream>


#define MAX_CONECTIONS 10
#define WRONG_INPUT 1
#define HOSTBYNAME_ERR 2
#define SOCKET_ERR 3
#define BIND_ERR 4
#define CONNECTION_ERR 5
#define READ_ERR 6
#define LEGAL 7
char* fail = "FAIL";
char* success = "SUCCESS";
fd_set clientsfds;
bool toExit = false;
char SEND_TO_CLIENT = 's';
using namespace std;
string EXIT_MSG = "*";
#define ERROR_NUM -1
Server::Server(int portNum)
{
    this->groupNames = vector<string>();
    this->clientMap = map<string, int>();
    this->allGroups = map<string, vector<string>>();

}


/**
 * checks if client is in a group.
 * @return
 */
bool Server::findClientInGroup(string name, vector<string> group)
{
    for(string client : group)
    {
        if (client == name)
        {
            return true;
        }
    }
    return false;
}


/**
 * creates a group if possible.
 */
void Server::create_group_of_clients(string &groupName, vector<string> &list_of_client_names, int portNum,
                                     const string creator)
{
    if(checkName(groupName) != LEGAL)
    {
        print_create_group(true, false, creator ,groupName);
        int writeCorrectly = writeToClient(portNum, fail);
        if (!writeCorrectly)
        {
            print_error("write", ERROR_NUM);
            return;
        }
        return;
    }
    for (auto clientName : list_of_client_names)
    {
        if (!clientMap.count(clientName))
        {
            print_create_group(true, false, creator ,groupName);
            int writeCorrectly = writeToClient(portNum, fail);
            if (!writeCorrectly)
            {
                print_error("write", ERROR_NUM);
                return;
            }
            return;
        }
    }
    groupNames.push_back(groupName);
    allGroups[groupName] = list_of_client_names;
    bool clientInGroup = findClientInGroup(creator, allGroups[groupName]);
    if(! clientInGroup)
    {
        allGroups[groupName].push_back(creator);
    }

    print_create_group(true, true, creator ,groupName);
    int writeCorrectly = writeToClient(portNum, success);
    if (!writeCorrectly)
    {
        print_error("write", ERROR_NUM);
        return;
    }

}

/**
 * servers send implementation.
 * @param sendToHim
 * @param message
 */
void Server::serverSend(string clientName, string sendToHim, string message)
{
    bool sendToClient = findClientInClientsMap(sendToHim, clientMap);
    if (sendToClient)
    {
        char* stringInFormat = convertToFormat(SEND_TO_CLIENT, message.c_str(), clientName);
        ssize_t writeCorrectly = write(clientMap[sendToHim], stringInFormat, strlen(stringInFormat));
        if (!writeCorrectly)
        {
            print_error("write", ERROR_NUM);
            return;
        }
        ssize_t writeCorrectlyToClient = write(clientMap[clientName], success, strlen(success));
        if (!writeCorrectlyToClient)
        {
            print_error("write", ERROR_NUM);
            return;
        }
        print_send(true, true, clientName, sendToHim, message);
        free(stringInFormat);
        return;
    }

    bool sendToGroup = findClientInGroup(sendToHim, groupNames);
    if (sendToGroup)
    {
        bool isClientInside = findClientInGroup(clientName, allGroups[sendToHim]);
        if (! isClientInside)
        {
            print_send(true, false, clientName, sendToHim, message);
            ssize_t writeCorrectly = write(clientMap[clientName], fail, strlen(fail));
            if (!writeCorrectly)
            {
                print_error("write", ERROR_NUM);
                return;
            }
            return;
        }
        char* stringInFormat = convertToFormat(SEND_TO_CLIENT, message.c_str(), clientName);
        for (auto toClient : allGroups[sendToHim])
        {
            if (toClient != clientName)
            {
                ssize_t writeCorrectly = write(clientMap[toClient], stringInFormat, strlen(stringInFormat));
                if (!writeCorrectly)
                {
                    print_error("write", ERROR_NUM);
                    return;
                }
            }
        }
        free(stringInFormat);
        print_send(true, true, clientName, sendToHim, message);
        ssize_t writeCorrectly = write(clientMap[clientName], success, strlen(success));
        if (!writeCorrectly)
        {
            print_error("write", ERROR_NUM);
            return;
        }
        return;
    }

        //not client or group. is it possible??
    else
    {
        ssize_t sendProblem = write(clientMap[clientName], fail, strlen(fail));
        if (!sendProblem)
        {
            print_error("write", ERROR_NUM);
            return;
        }
        print_send(true, false, clientName, sendToHim, message);
        return;
    }
}


/**
 * is it a legal new name?
 */
int Server::checkName(string name)
{
    for(string groupName: this->groupNames)
    {
        if (groupName == name)
        {
            return 0;
        }
    }
    for(auto clientName: this->clientMap)
    {
        if (clientName.first == name)
        {
            return 1;
        }
    }
    return LEGAL;
}


/**
 * entering client details to server.
 * @param socket
 * @return
 */
int Server::initializeClient(int socket)
{
    char buffer[WA_MAX_MESSAGE];
    int newSocketfd = accept(socket, nullptr, nullptr);
    if (newSocketfd == -1)
    {
        print_fail_connection();
        return CONNECTION_ERR;
    }

    memset(&buffer, 0, WA_MAX_MESSAGE + 1);
    if (read(newSocketfd, buffer, WA_MAX_MESSAGE) == -1)
    {
        print_error("read", READ_ERR);
        return READ_ERR;
    }

    int isLegalName = checkName(buffer);
    if (isLegalName != LEGAL)
    {
        print_dup_connection();
        if (write(newSocketfd, "not legal", strlen("not legal")) == -1)
        {
            print_error("write", READ_ERR);
            return READ_ERR;
        }
        return -4;
    }
    print_connection_server(buffer);
    if (write(newSocketfd, "is legal\0", strlen("is legal\0")) == -1)
    {
        print_error("write", READ_ERR);
        return READ_ERR;
    }
    clientMap[buffer] = newSocketfd;
    return newSocketfd;
}


/**
 * creates a new socket with a port number as parameter.
 * @return socket fd
 */
int Server::createNewSocket(int port_num)
{
    char hostname[WA_MAX_NAME + 1];

    struct sockaddr_in sa{};
    struct hostent *hp;

    memset(&sa, 0, sizeof(struct sockaddr_in));
    gethostname(hostname, WA_MAX_NAME);
    hp = gethostbyname(hostname);

    if (hp == nullptr)
    {
        print_error("gethostbyname", HOSTBYNAME_ERR);
        return (HOSTBYNAME_ERR);
    }

    sa.sin_family = AF_INET;
    memcpy(&sa.sin_addr, hp->h_addr, hp->h_length);
    sa.sin_port= htons(port_num);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        print_error("socket", SOCKET_ERR);
        return SOCKET_ERR;
    }

    if (bind(sockfd , (struct sockaddr *)&sa , sizeof(struct sockaddr_in)) < 0)
    {
        close(sockfd);
        print_error("bind", BIND_ERR);
        return BIND_ERR;
    }

    listen(sockfd, MAX_CONECTIONS);

    return sockfd;
}

/**
 * function that writes to server.
 * @return
 */
int Server::writeToClient(int sockfd, char *buffer)
{
    int n = write(sockfd, buffer, strlen(buffer));
    if (n < 0)
    {
        return 0;
    }
    return 1;
}


/**
 * checks for a name that appears twice.
 * @return list_of_client_names
 */
vector<string> Server::checkForDuplicate(vector<string> list_of_client_names)
{
    sort(list_of_client_names.begin(), list_of_client_names.end());
    vector<string> newVec;
    for (unsigned int i = 0; i < list_of_client_names.size() - 1; i++)
    {
        if(list_of_client_names[i] != list_of_client_names[i + 1])
        {
            string& client = list_of_client_names.at(i);
            newVec.push_back(client);
        }
    }

    unsigned int lastClientIndex = list_of_client_names.size() - 1;
    string& client = list_of_client_names.at(lastClientIndex);
    newVec.push_back(client);
    return newVec;
}


/**
 * server care of the who request.
 */
void Server::serverWho(string clientName, int portNum)
{
    vector<std::string> clients;
    for (auto clientProfile : clientMap) {
        clients.push_back(clientProfile.first);
    }
    sort(clients.begin(), clients.end());

    string connectedClients = "";
    for (auto clientProfile : clientMap)
    {
        auto clientNm = clientProfile.first;
        connectedClients += clientNm;
        if (clientNm != clientMap.rbegin()->first)
        {
            connectedClients += ",";
        }
    }
    const char* send = connectedClients.c_str();
    ssize_t n = write(portNum, send, strlen(send));
    if (n < 0)
    {
        print_error("write", ERROR_NUM);
        return;
    }
    print_who_server(clientName);
}


/**
 * finds client in clients map.
 * @return true or false.
 */
bool Server::findClientInClientsMap(string checkClient, map<string, int> clientsMap)
{
    for (auto client : clientsMap)
    {
        if (client.first == checkClient)
        {
            return true;
        }
    }
    return false;
}


/**
 * exits a client withe the specific portNum.
 * @param clientName
 */
void Server::serverExit(string clientName, int portNum)
{
    for (auto &group : allGroups)
    {
        int index = 0;
        for (auto &client : group.second)
        {
            if (clientName == client)
            {
                group.second.erase(group.second.begin() + index);
                break;
            }
        }
    }
    auto it = clientMap.find(clientName);
    clientMap.erase(it);
    print_exit(true, clientName);
    FD_CLR(portNum, &clientsfds);
}


/**
 * length of the buffer.
 */
int Server::lengthOfBuffer(char *buffer)
{
    int count = 0;
    for (int i = 0; buffer[i] != '\n'; ++i)
    {
        count++;
    }
    return count;
}


/**
 * converts the message to the right format.
 * @return new format
 */
char *Server::convertToFormat(char operation, const char *message, string clientName)
{
    size_t maxLength = WA_MAX_NAME + WA_MAX_MESSAGE + 2;
    auto *send = static_cast<char *>(malloc(maxLength * sizeof(char)));
    bzero(send, maxLength);
    send[0] = operation;
    int i = 0;
    for (i; clientName[i] != '\0'; ++i)
    {
        send[i + 1] = clientName[i];
    }
    char DIFF = '*';
    send[i + 1] = DIFF;
    i++;
    for (int j = 0; message[j] != '\0'; ++j)
    {
        send[i + j + 1] = message[j];
    }
    return send;
}


int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        print_server_usage();
        return WRONG_INPUT;
    }

    int port_num = std::stoi(argv[1]);
    Server server(port_num);
    int new_socket = server.createNewSocket(port_num);
    fd_set tempfds;
    FD_ZERO(&clientsfds);
    FD_SET(new_socket, &clientsfds);
    FD_SET(STDIN_FILENO, &clientsfds);

    while (true)
    {
        if (toExit)
        {
            break;
        }
        tempfds = clientsfds;
        if (select(WA_MAX_GROUP + 1, &tempfds, nullptr, nullptr, nullptr) < 0)
        {
            print_error("select", ERROR_NUM);
            return ERROR_NUM;
        }

        if (FD_ISSET(new_socket, &tempfds))
        {
            int sockfd = server.initializeClient(new_socket);
            if (sockfd != -1)
            {
                FD_SET(sockfd, &clientsfds);
            }
        }

        if (FD_ISSET(STDIN_FILENO, &tempfds))
        {
            command_type commandT;
            string name;
            string message;
            vector<std::string> clients;
            char msg[WA_MAX_MESSAGE];
            bzero(msg, WA_MAX_MESSAGE);
            ssize_t readCorrectly = read(STDIN_FILENO, msg, WA_MAX_MESSAGE);
            if (readCorrectly == -1)
            {
                print_error("read", ERROR_NUM);
                return ERROR_NUM;
            }
            int lenOfBuff = server.lengthOfBuffer(msg);
            string cleanMessage(msg, lenOfBuff);
            parse_command(cleanMessage, commandT, name, message, clients);
            if (commandT == EXIT)
            {
                // for each connected client - execute the exit command
                for (auto clientProfile : server.clientMap)
                {
                    const char ex = '*';
                    int wroteCorr = write(clientProfile.second, &ex, strlen(&ex));
                    if (!wroteCorr)
                    {
                        print_error("write", ERROR_NUM);
                        return ERROR_NUM;
                    }
                    server.serverExit(clientProfile.first, clientProfile.second);
                }
                print_exit();
                close(new_socket);
                toExit = true;
                exit(0);
            }
        }
        else
        {
            for (auto clientProfile : server.clientMap)
            {
                if (FD_ISSET(clientProfile.second, &tempfds))
                {
                    int sock = clientProfile.second;
                    command_type commandT;
                    string name;
                    string message;
                    vector<std::string> clients;
                    char buffer[WA_MAX_MESSAGE];
                    bzero(buffer, WA_MAX_MESSAGE);
                    ssize_t n = read(sock, &buffer, WA_MAX_MESSAGE + 3);
                    if (n < 0)
                    {
                        print_error("read", ERROR_NUM);
                        continue;
                    }

                    int lenOfBuff = server.lengthOfBuffer(buffer);
                    string cleanMessage(buffer, lenOfBuff);
                    parse_command(cleanMessage, commandT, name, message, clients);

                    if (commandT == CREATE_GROUP)
                    {
                        vector<string> noDuplicateClients = server.checkForDuplicate(clients);
                        server.create_group_of_clients(name, noDuplicateClients, sock, clientProfile.first);
                        break;
                    }
                    else if (commandT == SEND)
                    {
                        server.serverSend(clientProfile.first, name, message);
                        break;
                    }

                    else if (commandT == WHO)
                    {
                        server.serverWho(clientProfile.first, sock);
                        break;
                    }
                    else if (commandT == EXIT)
                    {
                        server.serverExit(clientProfile.first, sock);
                        break;
                    }

                    else
                    {
                        print_invalid_input();
                    }
                }
            }


        }
    }

    close(new_socket);
    return(new_socket);
}
