//Dylan Tribble - dat307

#include <iostream>
#include <fstream>
#include <math.h>
#include <cmath>
#include <cstdlib>
#include <cstring>
//#include <bits/stdc++.h>
using namespace std;

/*
Each function is essentially the same outside of how much the window size increases on failure to send
*/
void bepLatency(){
    //creates and opens binaryLatency file
	ofstream binaryLatency;
	binaryLatency.open("binaryLatency.txt");

	//loop for number of devices from 100 to 6000 increasing by 100
	for(int number_of_devices = 100; number_of_devices <= 6000; number_of_devices += 100) {

		int avg_latency = 0; //initializes the average latency variable

		//test backoff 10 times
		for(int trial = 0; trial < 10; trial++) {

			int devices = number_of_devices;
			int windows_size = 2;
			int latency = 0;

			//loopthe backoff until all devices succeed
			while (devices > 0) {

				int Device_Choices[windows_size]; //creates an array to store number of devices selecting a slot
				memset(Device_Choices, 0, sizeof(Device_Choices)); //initializes array to 0

				//generates a random number for each unsuccessful device and increments that array index
				for(int i = 0; i < devices; i++){

					int random_var = rand()%windows_size;
					Device_Choices[random_var] = Device_Choices[random_var] + 1;
				}

				//Check if send is successful
				for (int i = 0; i < windows_size; i++){

					//checks if only 1 device is in a given slot
					if (Device_Choices[i] == 1){

						devices--; //decreases the number of devices sending

						//when no devices remain, latency = last slot + all other windows
						if (devices == 0){

							latency += i;
						}
					}
				}

				//if devices remain increase latency
				if (devices > 0) {

					latency += windows_size;
				}

				windows_size *= 2; //Increases window size

			}

			avg_latency += latency; //adds curreny latency to avg
		}

		binaryLatency << number_of_devices << " " << avg_latency/10 << "\n"; //writes num of devices and average latency to file
	}

	binaryLatency.close(); //closes the text file
}
void linearLatency(){
	ofstream linearLatency;
	linearLatency.open("linearLatency.txt");

	for(int number_of_devices = 100; number_of_devices <= 6000; number_of_devices += 100) {

		int avg_latency = 0;

		for(int trial = 0; trial < 10; trial++) {

			int devices = number_of_devices;
			int windows_size = 2;
			int latency = 0;

			while (devices > 0) {

				int Device_Choices[windows_size];
				memset(Device_Choices, 0, sizeof(Device_Choices));

				for(int i = 0; i < devices; i++){

					int random_var = rand()%windows_size;
					Device_Choices[random_var] = Device_Choices[random_var] + 1;
				}

				for (int i = 0; i < windows_size; i++){

					if (Device_Choices[i] == 1){

						devices--;

						if (devices == 0){

							latency += i;
						}
					}
				}

				if (devices > 0) {

					latency += windows_size;
				}

				windows_size++; //Increases window size by 1
			}

			avg_latency += latency;
		}

		linearLatency << number_of_devices << " " << avg_latency/10 << "\n";
	}

	linearLatency.close();
}
void logLatency(){
	ofstream loglogLatency;
	loglogLatency.open("loglogLatency.txt");

	for(int number_of_devices = 100; number_of_devices <= 6000; number_of_devices += 100) {

		int avg_latency = 0;

		for(int trial = 0; trial < 10; trial++) {

			int devices = number_of_devices;
			int windows_size = 2;
			int latency = 0;

			while (devices > 0) {

				int Device_Choices[windows_size];
				memset(Device_Choices, 0, sizeof(Device_Choices));

				for(int i = 0; i < devices; i++){

					int random_var = rand()%windows_size;
					Device_Choices[random_var] = Device_Choices[random_var] + 1;
				}

				for (int i = 0; i < windows_size; i++){

					if (Device_Choices[i] == 1){

						devices--;

						if (devices == 0){

							latency += i;
						}
					}
				}

				if (devices > 0) {

					latency += windows_size;
				}

				windows_size = (1 + (1 / log2(windows_size))) * windows_size; //Logarithmically increases window size
			}

			avg_latency += latency;
		}

		loglogLatency << number_of_devices << " " << avg_latency/10 << "\n";
	}

	loglogLatency.close(); //closes file
}


int main() {
    linearLatency();
    bepLatency();
    logLatency();


	return 0;

}
