
//Dylan Tribble - dat307
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fstream>
#include <string.h>
#include <unistd.h>

using namespace std;

int handshake(int handshake_socket, int port, sockaddr_in server, char payload[], socklen_t slen, hostent* s){
    int mysocket = handshake_socket;
    if ((mysocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        cout << "Error in creating socket.\n" << strerror(errno) << endl;
        return 1;
    }

    memset((char *) &server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);  // Use the desired server port
    bcopy((char *) s->h_addr, (char *) &server.sin_addr.s_addr, s->h_length);

    // Send 'abc' to initiate handshake
    if (sendto(mysocket, payload, strlen(payload), 0, (struct sockaddr *) &server, slen) == -1) {
        cout << "Error in sendto function.\n" << strerror(errno) << endl;
        close(mysocket);
        return 1;
    }

    char ack[5];
    recvfrom(mysocket, ack, sizeof(ack), 0, (struct sockaddr *) &server, &slen);
    ack[slen] = '\0';
    int random_port = atoi(ack);  // Convert the received port to an integer
    cout << "Random port: " << random_port << endl;

    // Close the initial handshake socket
    close(mysocket);
    return random_port;
}

int send_file(auto file, int random_port, sockaddr_in server, int mysocket, socklen_t slen){
    auto fileToSendPath = file;

    // Specify the server's address and port for data transmission
    server.sin_port = htons(random_port);

    // Open the file to send
    ifstream fileToSend(fileToSendPath, ios::binary);

    if (!fileToSend.is_open()) {
        cout << "Error: Unable to open the file.\n" << strerror(errno) << endl;
        close(mysocket);
        return 1;
    }

    char chunk[5];  // A buffer to read 4 characters from the file + '\0'
    char ack[5];

    while (!fileToSend.eof()) {
        fileToSend.read(chunk, 4);  // Read 4 characters from the file
        chunk[fileToSend.gcount()] = '\0';  // Null-terminate the chunk

        // Send the chunk to the server
        if (sendto(mysocket, chunk, sizeof(chunk), 0, (struct sockaddr *) &server, slen) == -1) {
            cout << "Error in sendto function.\n" << strerror(errno) << endl;
            close(mysocket);
            return 1;
        }

        // Receive the acknowledgment from the server
        int received_bytes = recvfrom(mysocket, ack, sizeof(ack)-1, 0, (struct sockaddr *) &server, &slen);
        ack[received_bytes] = '\0';
        cout << "Received acknowledgment from server: " << ack << endl;
    }
    
    char eof_marker[] = "EOF";
    if (sendto(mysocket, eof_marker, strlen(eof_marker), 0, (struct sockaddr *) &server, slen) == -1) {
        cout << "Error sending EOF marker.\n" << strerror(errno) << endl;
        close(mysocket);
        return 1;
    }
    // Close the data transmission socket
    close(mysocket);
    fileToSend.close();

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <hostname> <port> <path_of_file_to_send>" << endl;
        return 1;
    }

    struct hostent *s;
    s = gethostbyname(argv[1]);

    if (s == nullptr) {
        cerr << "Error: Unable to resolve hostname." << endl;
        return 1;
    }

    const char *fileToSendPath = argv[3];  // Get the file path from the command-line argument

    struct sockaddr_in server;
    int port = atoi(argv[2]);
    int mysocket = 0;
    socklen_t slen = sizeof(server);
    char payload[4] = "abc";  // Initiate handshake with 'abc'

    int random_port = handshake(mysocket, port, server, payload, slen, s);

    // Create a new socket for data transmission using the random port
    if ((mysocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        cout << "Error in creating data socket.\n" << strerror(errno) << endl;
        return 1;
    }

    send_file(fileToSendPath, random_port, server, mysocket, slen);

    return 0;
}

