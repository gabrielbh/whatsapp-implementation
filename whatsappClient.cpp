#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <algorithm>
#include <iostream>
#define ERROR_NUM -1

#include "whatsappClient.h"
char* fail = "FAIL";
char GET_MESSAGE = 's';


/**
 * function that creates a group if its a legal one.
 */
void Client::create_group(vector<string> &list_of_client_names, int portNum, string groupName)
{
//    Server server(portNum);
    bool isLegalLetter = checkValidLetters(groupName, list_of_client_names);
    bool lengthValidity = checkGroupLength(list_of_client_names, this->clientName);
    if (! isLegalLetter || ! lengthValidity)
    {
        print_create_group(false, false, clientName ,groupName);
        return;
    }
    int wroteCorrectly = writeToServer(sockfd, buffer);
    if (!wroteCorrectly)
    {
        print_error("write", ERROR_NUM);
        return;
    }

    bzero(buffer,WA_MAX_MESSAGE);
    string readCorrectly = readFromServer(sockfd);
    print_create_group(false, readCorrectly != fail, clientName , groupName);
}


/**
 * sends a message if its a legal one.
 */
void Client::send(string name, string message)
{
    bool isLegalLetter = validateName(name);
    bool sentToSelf = (this->clientName == name);
    if ((!isLegalLetter) || sentToSelf)
    {
        print_send(false, false, clientName, name, message);
        return;
    }

    int wroteCorrectly = writeToServer(sockfd, buffer);
    if (!wroteCorrectly)
    {
        print_error("write", ERROR_NUM);
        return;
    }

    bzero(buffer,WA_MAX_MESSAGE);

    string readCorrectly = readFromServer(sockfd);
    if (readCorrectly == fail)
    {
        print_send(false, false, clientName, name, message);
        return;
    }

    print_send(false, true, clientName, name, message);
}


/**
 * Sends a request (to the server) to receive a list of
 * currently connected client names (alphabetically order),
 * separated by comma without spaces.
 */
void Client::who()
{
    vector<string> aVec =  vector<string>();
    int wroteCorrectly = writeToServer(sockfd, buffer);
    if (!wroteCorrectly)
    {
        print_error("write", ERROR_NUM);
        return;
    }

    bzero(buffer,WA_MAX_MESSAGE);

    string readCorrectly = readFromServer(sockfd);
    if (readCorrectly.empty())
    {
        print_who_client(false, aVec);
        return;
    }
    parseToVector(readCorrectly, aVec);
    print_who_client(true, aVec);
}


/**
 * parsing string to vector.
 */
void Client::parseToVector(std::string& input, std::vector<std::string>& output)
{
    const char *s;
    char *saveptr;
    char c[WA_MAX_INPUT];
    bzero(c, WA_MAX_INPUT);
    strcpy(c, input.c_str());
    s = strtok_r(c, ",", &saveptr);
    if (s == NULL)
    {
        output.emplace_back(input);
    }
    else
    {
        while (s != NULL)
        {
            output.emplace_back(s);
            s = strtok(NULL, ",");
        }
        if (s == NULL && saveptr == NULL)
        {
//            output.emplace_back(saveptr);
            return;
        }
        else
        {
            output.emplace_back(saveptr);
        }
    }
}


/**
 * Unregisters the client from the server and removes
 * it from all groups.
 * @return
 */
void Client::clientExit()
{
    ssize_t exitCorrectly = writeToServer(sockfd, buffer);
    if (exitCorrectly == -1)
    {
        print_error("write", ERROR_NUM);
        return;
    }
    print_exit(false, clientName);
    close(sockfd);
    exit(0);
}


/**
 * check valid name or group letters.
 */
bool Client::validateName(string name)
{
    for (char c : name)
    {
        if (c < '0' || (c > '9' && c < 'A') || (c > 'Z' && c < 'a') || c > 'z')
        {
            return false;
        }
    }
    return true;
}


/**
 * check input validity.
 * @return true if valid, false otherwise.
 */
bool Client::checkValidLetters(string groupName, vector<string> &clientsList) {
    for (string client : clientsList)
    {
        for (char cn : client)
        {
            if (cn < '0' || (cn > '9' && cn < 'A') || (cn > 'Z' && cn < 'a') || cn > 'z')
            {
                if (cn != '\n')
                {
                    return false;
                }
            }
        }
    }
    return validateName(groupName);
}


/**
 * checks if list is not empty, and with more than one person.
 * @param creator who created the group.
 * @return true or false.
 */
bool Client::checkGroupLength(vector<string> &clients, string creator)
{
    if (clients.empty())
    {
        return false;
    }
    int numOfClients = 0;
    for (auto client : clients)
    {
        if (client != creator)
        {
            numOfClients++;
        }
    }
    if (!numOfClients)
    {
        return false;
    }
    return true;
}


/**
 * checks for a name that appears twice.
 * @return list_of_client_names
 */
void Client::checkForDuplicate(vector<string> *list_of_client_names)
{
    sort(list_of_client_names->begin(), list_of_client_names->end());
    vector<string> newVec;
    for (unsigned int i = 0; i < list_of_client_names->size() - 1; i++)
    {
        if(list_of_client_names[i] != list_of_client_names[i + 1])
        {
            string& client = list_of_client_names->at(i);
            newVec.push_back(client);
        }
    }

    unsigned int lastClientIndex = list_of_client_names->size() - 1;
    string& client = list_of_client_names->at(lastClientIndex);
    newVec.push_back(client);
    *list_of_client_names = newVec;
}


/**
 * client constractor.
 */
Client::Client(char *name, string cIp, int portNum)
{
    if (! validateName(name))
    {
        print_fail_connection();
        exit(1);
    }
    clientName = name;
    ip = cIp;
    port = portNum;
}


/**
 * port getter.
 */
int Client::getPort()
{
    return port;
}


/**
 * function that writes to server.
 * @return
 */
int Client::writeToServer(int sockfd, char *buffer)
{
    int n = write(sockfd, buffer, strlen(buffer));
    if (n < 0)
    {
        print_error("write", ERROR_NUM);
        return 0;
    }
    return 1;
}


/**
 * function that reads from server.
 * @return
 */
string Client::readFromServer(int sockfd)
{
    char *message = new char[WA_MAX_MESSAGE];
    ssize_t lenOfMessage = recv(sockfd, message, WA_MAX_MESSAGE, 0);
    if (lenOfMessage < 0)
    {
        print_error("recv", ERROR_NUM);
        return "";    }
    string cleanMessage(message, lenOfMessage);
    delete[] message;
    return cleanMessage;
}


void Client::socketFunc(int &argc, char **argv)
{
    int portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    if (argc < 4)
    {
        print_client_usage();
    }
    portno = atoi(argv[3]);
    this->port = portno;

    server = gethostbyname(argv[2]);
    if (server == NULL)
    {
        print_error("gethostbyname", ERROR_NUM);
        return;
    }
    this->clientName = argv[1];
    this->ip = argv[2];
    this->port = portno;

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = server->h_addrtype;
    memcpy((char *)&serv_addr.sin_addr , server->h_addr , server->h_length);
    serv_addr.sin_port = htons((u_short)portno);

    int sockfd = socket(server->h_addrtype, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        print_error("socket", sockfd);
        return;
    }

    int connecting = connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
    if (connecting < 0)
    {
        print_error("connect", sockfd);
        close(sockfd);
        return;
    }

    int wroteCorrectly = writeToServer(sockfd, clientName);
    if (!wroteCorrectly)
    {
        print_error("write", static_cast<int>(wroteCorrectly));
        return;
    }

    bzero(buffer,WA_MAX_MESSAGE);
    string readCorrectly = readFromServer(sockfd);
    string n = "not legal";
    if (readCorrectly == n)
    {
        close(sockfd);
        print_dup_connection();
        exit(1);
    }
    print_connection();
    loopFunc(sockfd);
}


/**
 * loops on the client, until he exits.
 */
void Client::loopFunc(int sockFd)
{
    this->sockfd = sockFd;
    fd_set curFds, fds;
    FD_ZERO(&fds);
    FD_SET(this->sockfd, &fds);
    FD_SET(STDIN_FILENO, &fds);
    while(true)
    {
        curFds = fds;
        if (int n = select(1 + this->sockfd, &curFds, nullptr, nullptr, nullptr) < 0)
        {
            print_error("select", n);
            exit(1);
        }

        if (FD_ISSET(STDIN_FILENO, &curFds))
        {
            command_type commandT;
            string name;
            string message;
            string com;
            vector<std::string> clients;
            bzero(buffer, WA_MAX_MESSAGE);
            fgets(buffer, WA_MAX_MESSAGE - 1, stdin);
            int lenOfBuff = lengthOfBuffer(buffer);
            string cleanMessage(buffer, lenOfBuff);
            parse_command(cleanMessage, commandT, name, message, clients);
            if(commandT == CREATE_GROUP)
            {
                create_group(clients, getPort(), name);
            }

            else if(commandT == SEND)
            {
                send(name, message);
            }

            else if(commandT == WHO)
            {
                who();
            }
            else if(commandT == EXIT)
            {
                clientExit();
            }
            else
            {
                print_invalid_input();
            }
        }

        if (FD_ISSET(this->sockfd, &curFds))
        {
            differentCommands();
        }
    }

}


/**
 * command that comes from server or other client.
 */
void Client::differentCommands()
{
    auto bufSize = WA_MAX_NAME + WA_MAX_MESSAGE + 2;
    char operation[bufSize];
    bzero(operation, bufSize);
    ssize_t  receive = recv(sockfd, &operation, bufSize, 0);
    if (receive < 0)
    {
        print_error("recv", static_cast<int>(receive));
        return;
    }
    if(operation[0] == GET_MESSAGE)
    {
        getMessage(operation, receive);
    }
    else if(operation[0] == '*')
    {
        clientExit();
    }
    else
    {
        return;
    }
}


/**
 * implementation of message receiving.
 */
void Client::getMessage(char *buf, ssize_t sizeOfBuffer)
{
    char message[WA_MAX_MESSAGE];
    char name[WA_MAX_NAME];
    bzero(message, WA_MAX_MESSAGE);
    bzero(name, WA_MAX_NAME);
    string cleanMessage(buf, sizeOfBuffer);
    int i = 1;
    for (i; buf[i] != '\0'; i++)
    {
        if (buf[i] != '*')
        {
            name[i - 1] = buf[i];
            continue;
        }
        break;
    }

    for (int j = 0;  buf[i + j + 1] != '\0' ; j++)
    {
        message[j] = buf[i + j + 1];
    }
    string theMessage(message);
    string theName(name);
    print_message(theName, theMessage);
}


/**
 * length of the buffer.
 * @param buffer
 * @return
 */
int Client::lengthOfBuffer(char *buffer)
{
    int count = 0;
    for (int i = 0; buffer[i] != '\n'; ++i)
    {
        count++;
    }
    return count;
}


int main(int argc, char *argv[])
{
    char * name = argv[1];
    string ip = "132.65.126.115";
    int port = 8888;



    if (argc != 4)
    {
        print_client_usage();
    }
    Client client(name, ip, port);
    client.socketFunc(argc, argv);
    close(client.sockfd);

    return 0;
}






