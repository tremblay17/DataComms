//Dylan Tribble dat307

#include <stdlib.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "packet.h"
#include <math.h>
#include <poll.h>
#include "client.h"

using namespace std;

#define payLen 30
#define packetLen 50
#define FLIP_SEQNUM(seqnum) ((seqnum) = (seqnum) == 1 ? 0 : 1)

client::client(char * filename, char * seqlog, char * acklog){

	// file input/output
	ofileSeq = new ofstream(seqlog);
	ofileAck = new ofstream(acklog);
    myIStream = new ifstream(filename);

}

// send the next segment
int client::sendData(int seqNumber, int socket, sockaddr_in & saddr, char *dataChunk){	// socket should be CEsocket
	char spacket[packetLen];   // for serializing the packet to send

	packet mySendPacket(1, seqNumber, strlen(dataChunk), dataChunk); // make the packet to be sent
	//mySendPacket.printContents();

	memset(spacket, 0, packetLen); // serialize the packet to be sent
	mySendPacket.serialize(spacket);

	if(sendto(socket, spacket, sizeof(spacket), 0, (struct sockaddr *)&saddr, sizeof(saddr)) < 0){
			cout << "Failed to send payload.\n";
			return 0;
	}
	return 1;
}

int main(int argc, char *argv[]){
    if(argc != 5) cout << "Usage: client <serverHostName> <receivePort> <sendPort> <path/to/file.txt>" << endl;
    ofstream seqNumLogFile;
    ofstream ackLogFile;
    ifstream inputFile;
    std::string inputFileName = argv[4];
    char dataChunk[payLen];

	// declare and setup server
	struct hostent *em_host;            // pointer to a structure of type hostent
	em_host = gethostbyname(argv[1]);   // host name for emulator
	if(em_host == NULL){                // failed to obtain server's name
		cout << "Failed to obtain server.\n";
		exit(EXIT_FAILURE);
	}

  // ******************************************************************
  // ******************************************************************

  // client sets up datagram socket for sending
  int CESocket = socket(AF_INET, SOCK_DGRAM, 0);
  if(CESocket < 0){
  	cout << "Error: failed to open datagram socket.\n";
  }

  // set up the sockaddr_in structure for sending
  struct sockaddr_in CE;
  socklen_t CE_length = sizeof(CE);
  bzero(&CE, sizeof(CE));
  CE.sin_family = AF_INET;
  bcopy((char *)em_host->h_addr, (char*)&CE.sin_addr.s_addr, em_host->h_length);  // both using localhost so this is fine
  char * end;
  int em_rec_port = strtol(argv[2], &end, 10);  // get emulator's receiving port and convert to int
  CE.sin_port = htons(em_rec_port);             // set to emulator's receiving port

  // ******************************************************************
  // ******************************************************************

  // client sets up datagram socket for receiving
  int ECSocket = socket(AF_INET, SOCK_DGRAM, 0);
  if(ECSocket < 0){
  	cout << "Error: failed to open datagram socket.\n";
  }

  // set up the sockaddr_in structure for receiving
  struct sockaddr_in EC;
  socklen_t EC_length = sizeof(EC);
  bzero(&EC, sizeof(EC));
  EC.sin_family = AF_INET;
  EC.sin_addr.s_addr = htonl(INADDR_ANY);
  char * end2;
  int cl_rec_port = strtol(argv[3], &end2, 10);  // client's receiving port and convert to int
  EC.sin_port = htons(cl_rec_port);             // set to emulator's receiving port

  // do the binding
  if (bind(ECSocket, (struct sockaddr *)&EC, EC_length) == -1){
  		cout << "Error in binding.\n";
  }

  // ******************************************************************
  // ******************************************************************

  inputFile.open(inputFileName,std::ios::in);
  if(!inputFile.is_open())
    throw std::runtime_error("Could not open input file.");
  seqNumLogFile.open("clientseqnum.log", std::ios::out | std::ios::trunc);
  if (!seqNumLogFile.is_open())
	throw std::runtime_error("Could not open seq num log file.");
  ackLogFile.open("clientack.log", std::ios::out | std::ios::trunc);
  if (!ackLogFile.is_open())
	throw std::runtime_error("Could not open ack log file.");

  char slog[] = "clientseqnum.log";
  char alog[] = "clientack.log";
  client myClient(argv[4], slog, alog);

  int seqnum =0;
  struct pollfd fds[1];
  fds[0].fd = ECSocket;
  fds[0].events = POLLIN;
  while(inputFile.is_open()){
      inputFile.readsome(dataChunk, payLen);
      dataChunk[inputFile.gcount()]='\0';

      int pollResult;

      myClient.sendData(seqnum, CESocket, CE, dataChunk);
      seqNumLogFile << seqnum << std::endl;
      for (;;) {
          pollResult = poll(fds, 1, 2000);  // Wait for 2 seconds for data
          if (pollResult == -1) {
              cout << "Error in poll." << endl;
              break;
          } else if (pollResult == 0) {
              // Timeout, send the same packet again
              //cout << "Timeout, resending the packet." << endl;
              //if(strlen(dataChunk) == 0) break;
              //if(strlen(dataChunk) == 0) break;
              myClient.sendData(seqnum, CESocket, CE, dataChunk);
          } else {
              // Break out of the loop when a response is received
              break;
          }
      }
      FLIP_SEQNUM(seqnum);
      // This is where I handle getting back an ACK from the server
      char serialized[512];
      memset(serialized, 0, 512);
      recvfrom(ECSocket, serialized, 512, 0, (struct sockaddr *)&EC, &EC_length);
      //cout << "Received packet and deserialized to obtain the following: " << endl << endl;

      char ackload[512];
      memset(ackload, 0, 512);
      packet rcvdPacket(0,0,0,ackload);
      rcvdPacket.deserialize(serialized);
      ackLogFile << const_cast<packet&>(rcvdPacket).getSeqNum() << std::endl;
      //rcvdPacket.printContents();

      if(strlen(dataChunk)==0) break;
  }

  char spacket[packetLen];   // for serializing the packet to send

  //Send EOT to server
  packet mySendPacket(3, seqnum, 0, NULL); // make the packet to be sent
  //mySendPacket.printContents();

  memset(spacket, 0, 0); // serialize the packet to be sent
  mySendPacket.serialize(spacket);
  seqNumLogFile << seqnum << std::endl;

  if(sendto(CESocket, spacket, sizeof(spacket), 0, (struct sockaddr *)&CE, sizeof(CE)) < 0){
	  cout << "Failed to send payload.\n";
	  return 0;
  }

  char serialized[512];
  memset(serialized, 0, 512);
  recvfrom(ECSocket, serialized, 512, 0, (struct sockaddr *)&EC, &EC_length);
  //cout << "Received packet and deserialized to obtain the following: " << endl << endl;

  char ackload[512];
  memset(ackload, 0, 512);
  packet rcvdPacket(0,0,0,ackload);
  rcvdPacket.deserialize(serialized);
  ackLogFile << const_cast<packet&>(rcvdPacket).getSeqNum() << std::endl;
  //rcvdPacket.printContents();

  inputFile.close();
  ackLogFile.close();
  seqNumLogFile.close();
  close(ECSocket);
  close(CESocket);
  return 0;
}
