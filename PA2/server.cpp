//Russell Smith res565

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
#include <string>
#include <sstream>
#include "packet.h"
#include <math.h>

using namespace std;

#define FLIP_SEQNUM(seqnum) ((seqnum) = (seqnum) == 1 ? 0 : 1)

int main(int argc, char *argv[]){
    if(argc != 5) cout << "Usage: server <clientHost> <receivePort> <sendPort> <path/to/output.txt>.";
    std::stringstream outputFile;
    ofstream arrivalLog;
    std::string outputFileName = argv[4];
    std::string arrivalLogName = "arrival.log";
    std::string dataFromClient;

	// ******************************************************************
	// ******************************************************************

	// sets up datagram socket for receiving from emulator
	int ESSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(ESSocket < 0){
		cout << "Error: failed to open datagram socket.\n";
	}

	// set up the sockaddr_in structure for receiving
	struct sockaddr_in ES;
	socklen_t ES_length = sizeof(ES);
	bzero(&ES, sizeof(ES));
	ES.sin_family = AF_INET;
	ES.sin_addr.s_addr = htonl(INADDR_ANY);
	char * end;
	int sr_rec_port = strtol(argv[2], &end, 10);  // server's receiving port and convert to int
	ES.sin_port = htons(sr_rec_port);             // set to emulator's receiving port

	// do the binding
	if (bind(ESSocket, (struct sockaddr *)&ES, ES_length) == -1)
		cout << "Error in binding.\n";

	// ******************************************************************
	// ******************************************************************

	// declare and setup server
	struct hostent *em_host;            // pointer to a structure of type hostent
	em_host = gethostbyname(argv[1]);   // host name for emulator

	if(em_host == NULL){                // failed to obtain server's name
		cout << "Failed to obtain server.\n";
		exit(EXIT_FAILURE);
	}

	int SESocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(SESocket < 0){
		cout << "Error in trying to open datagram socket.\n";
		exit(EXIT_FAILURE);
	}

	// setup sockaddr_in struct
	struct sockaddr_in SE;
	memset((char *) &SE, 0, sizeof(SE));
	SE.sin_family = AF_INET;
	bcopy((char *)em_host->h_addr, (char*)&SE.sin_addr.s_addr, em_host->h_length);
	int em_rec_port = strtol(argv[3], &end, 10);
	SE.sin_port = htons(em_rec_port);


	// ******************************************************************
	// ******************************************************************

  std::ofstream dataFile(outputFileName, std::ios::out | std::ios::trunc );
  if(!dataFile.is_open())
    throw std::runtime_error("Could not open data file.");
  arrivalLog.open(arrivalLogName, std::ios::out | std::ios::trunc);
  if(!arrivalLog.is_open())
    throw std::runtime_error("Could not open arrival log.");
  int prevSeqNum = 0;

  for(;;)
  {
    char payload[512];
    memset(payload, 0, 512);
    char serialized[512];
    memset(serialized, 0, 512);
    packet rcvdPacket(0,0,0,payload);
    for (int i=0; i<1; i++) { // loop is redundant
        if (recvfrom(ESSocket, serialized, 512, 0, (struct sockaddr *) &ES, &ES_length)==-1)
        cout << "Failed to receive.\n";

        //cout << "Received packet and deserialized to obtain the following: " << endl << endl;
        rcvdPacket.deserialize(serialized);
        //rcvdPacket.printContents();

    }
    arrivalLog << const_cast<packet&>(rcvdPacket).getSeqNum() << std::endl;
    if(rcvdPacket.getSeqNum() == prevSeqNum)
    {
        //Record Received Data
        // sending back an ACK
        //cout << "I'm now going to send back an ACK.\n";

        packet myAckPacket(0, prevSeqNum, 0, NULL); // make the packet to be sent
        //myAckPacket.printContents();

        char spacket[100];   // for serializing the packet to send
        memset(spacket, 0, 100); // serialize the packet to be sent
        myAckPacket.serialize(spacket);

        //Record Data to buffer
        outputFile.write(rcvdPacket.getData(), rcvdPacket.getLength());

        if(sendto(SESocket, spacket, strlen(spacket), 0, (struct sockaddr *)&SE, sizeof(struct sockaddr_in)) < 0){
            cout << "Failed to send payload.\n";
            return 0;
        }
        continue;
    }

    FLIP_SEQNUM(prevSeqNum);
    if(rcvdPacket.getType() == 3){
        // sending back an ACK
        //cout << "I'm now going to send back an ACK.\n";

        packet myAckPacket(2, prevSeqNum, 0, NULL); // make the packet to be sent
        //myAckPacket.printContents();

        char spacket[100];   // for serializing the packet to send
        memset(spacket, 0, 100); // serialize the packet to be sent
        myAckPacket.serialize(spacket);



        if(sendto(SESocket, spacket, strlen(spacket), 0, (struct sockaddr *)&SE, sizeof(struct sockaddr_in)) < 0){
            cout << "Failed to send payload.\n";
            return 0;
        }
        break;
    }
    // sending back an ACK
    //cout << "I'm now going to send back an ACK.\n";

    packet myAckPacket(0, prevSeqNum, 0, NULL); // make the packet to be sent
    //myAckPacket.printContents();

    char spacket[100];   // for serializing the packet to send
    memset(spacket, 0, 100); // serialize the packet to be sent
    myAckPacket.serialize(spacket);


    if(sendto(SESocket, spacket, strlen(spacket), 0, (struct sockaddr *)&SE, sizeof(struct sockaddr_in)) < 0){
        cout << "Failed to send payload.\n";
        return 0;
    }

   //Record data to buffer
   outputFile.write(rcvdPacket.getData(), rcvdPacket.getLength());
  }
  //write data to output file
  dataFile << outputFile.str() << std::endl;
  //cout << "I will now shut down.\n";

  dataFile.close();
  arrivalLog.close();
  close(SESocket);
  close(ESSocket);
  return 0;
}
