
//Dylan Tribble - dat307
#include<iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>

using namespace std;

int main(int argc, char *argv[]) {
  if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <port>" << endl;
        return 1;
    }    
  
  struct sockaddr_in server;
  struct sockaddr_in client;
  int port = atoi(argv[1]);
  int mysocket = 0;
  socklen_t clen = sizeof(client);
  char payload[5];

  if ((mysocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    cout << "Error in socket creation.\n";

  memset((char *) &server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  
  cout << "Binding Socket for Handshake" << endl;
  if (bind(mysocket, (struct sockaddr *)&server, sizeof(server)) == -1) {cout << "Error in binding.\n" << strerror(errno) << endl;}

  cout << "Waiting for a packet now.\n";
  if (recvfrom(mysocket, payload, sizeof(payload), 0, (struct sockaddr *)&client, &clen) == -1)
    cout << "Failed to receive.\n";
  else {
    //cout << "Received data: " << payload << endl;

    // Check if payload matches the handshake message "abc"
    if (strcmp(payload, "abc") == 0) {
      // Generate a random port number (between 1024 and 65535)
      srand(time(NULL)); // Initialize the random seed
      int random_port = rand() % (65535 - 1024 + 1) + 1024;

      // Convert the random port to a string
      char random_port_str[6]; // Maximum of 5 digits for port number
      snprintf(random_port_str, sizeof(random_port_str), "%d", random_port);

      // Send the random port number back to the client
      if (sendto(mysocket, random_port_str, strlen(random_port_str), 0, (struct sockaddr *)&client, clen) == -1) {
        cout << "Error in sendto function.\n" << strerror(errno) << endl;
        close(mysocket);
        return 1;
      }
      cout << "Generating Socket for File Transfer" << endl;
      // Create a new socket to receive data using the random port
      int dataSocket = socket(AF_INET, SOCK_DGRAM, 0);
      struct sockaddr_in dataServer;
      memset((char *) &dataServer, 0, sizeof(dataServer));
      dataServer.sin_family = AF_INET;
      dataServer.sin_port = htons(random_port);
      dataServer.sin_addr.s_addr = htonl(INADDR_ANY);
      
      int reuse = 1;
      if (setsockopt(dataSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        perror("setsockopt");
        cout << "Error setting SO_REUSEADDR: " << strerror(errno) << endl;
      }

      if (bind(dataSocket, (struct sockaddr *)&dataServer, sizeof(dataServer)) == -1) {
        cout << "Error in binding data socket.\n" << strerror(errno) << endl;
        close(dataSocket);
        return 1;
      }

      ofstream outputFile("upload.txt", ios::binary);
      cout << "Generating Output File" << endl;
      if (!outputFile.is_open()) {
        cout << "Error: Unable to open the output file.\n";
        close(dataSocket);
        return 1;
      }

      // Receive data and acknowledgments
      char ack[5];
      int received_bytes;
      cout << "Receiving File" << endl;
      do {
        received_bytes = recvfrom(dataSocket, payload, sizeof(payload), 0, (struct sockaddr *)&client, &clen);
        payload[received_bytes] = '\0';
        if (received_bytes == -1) {
          cout << "Failed to receive data.\n";
          close(dataSocket);
          outputFile.close();
          close(mysocket);
          return 1;
        }
        //cout << "Received data chunk from client: " << payload << endl;
        
        if (strcmp(payload, "EOF") == 0) {
            cout << "Received EOF marker.\n";
            break; // Exit the loop when EOF marker is received
        }
        
        // Write the received data to the output file
        outputFile.write(payload, received_bytes);

        // Send acknowledgment in uppercase
        for (int i = 0; i < received_bytes; i++) {
          ack[i] = toupper(payload[i]);
        }
        sendto(dataSocket, ack, received_bytes, 0, (struct sockaddr *)&client, clen);

        if(received_bytes < sizeof(payload) - 1) break;
        //cout << received_bytes << sizeof(payload) << endl;
      } while (true);  // Continue until an empty payload is received

      // Close the data socket and output file
      close(dataSocket);
      outputFile.close();
      
      cout << "File transfer complete.\n";
    }
  }
  
  close(mysocket);
  return 0;
}

