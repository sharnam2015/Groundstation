// Pratham Ground Station Rotor Code  - IIT Bombay Student Satellite team
//Copyrights with IITB Student Satellite Team
//written in 2016
#include <stdlib.h>
#include <fstream> 
#include <iostream>
#include <string>
#include <cstdio>
#include <unistd.h>
using namespace std; 


const char* predictQuery = "predict -f TIGRISAT";
const char* getRotctlQuery = "sudo rotctl -m 601 -r /dev/ttyACM0 -s 9600 get_pos";
int defaultAzimuth;
int mode;
//1 if 222 and 305 belong to [begin,end] then use ulta elevation and ulta azi and default azi to be 270(act) 114(cont)
//2 if only 222 belongs then default = contAzi = 141; act = 180
//3 if only 305 belongs then default = contAzi = 77; act = 0 
//use either 2 or 3 if none of the above conditions. better not use 1 because there is a limit to the min elevation in that mode

void dataLog(string res);	
int getSatEle(string res);	//get elevation angle of satellite using predict data
int getSatAzi(string res);	//get azimuth angle of satellite using predict data
int getRotAzi();
int getRotEle();
void setElevationAzimuth(int elevation, int azimuth);
double actualAzimuth(double cAzi);
double actualElevation(double cEle);
double controlAzimuth(double aAzi);
double controlElevation(double aEle);
void set(int elevation, int azimuth);
int main() {
	int elevation, azimuth;
	int k=1;
	char result[128];
	int def = 0;
	cout<<"Tracking mode rules:\n1 if azi values 222 and 305 both belong to [beginning,end] of satellite pass; \n2 if only 222 belongs; \n3 if only 305 belongs; \neither 2 or 3 if neither of these two values are crossed by satellite\n";
	cout<<"Enter tracking mode:...";
	cin>>mode;
	//set default position
	cout<<"Enter 1 to start from the default position of set mode(use this if earlier runs of the program had used different modes or if you're unsure) ; \nEnter 0 otherwise\n";
	cin>>def;
	if(def != 0) {
		if(mode == 1) set(30, 270);
		if (mode == 2) set(15, 180);
		if(mode == 3) set(15, 0);
	}
	while(k<25) {
		//k++;
		try{
			FILE* pipe = popen(predictQuery, "r");
        	fgets(result, 128, pipe);
			pclose(pipe);
			} catch(std::exception e){
				cout<<"Error in connecting with predict!";
		} 
		cout<<"Data : "<<result<<"\n";
		elevation = getSatEle(result);
		cout<<"Elevation is:"<<elevation<<"\n";
		azimuth = getSatAzi(result);
		cout<<"Azimuth is:"<<azimuth<<"\n";

		dataLog(result);
		set(elevation, azimuth);
	}
	return 1;
}

int getSatEle(string info) {	
	int count = 0;
	for(int i=0; info[i]!='*'; i++) {
		if(info[i]==' ') {
			while(info[i]==' ') i++;
			count++;
			if(count==4) {
				char ele[3];
				int j=0;
				while(info[i]!=' ') {
					ele[j] = info[i];
					j++;
					i++;
				}
				return atoi((const char*)ele);
			}
		}
	}
	return 6;	
}

int getSatAzi(string info) {	
	int count = 0;
	for(int i=0; info[i]!='*'; i++) {
		if(info[i]==' ') {
			while(info[i]==' ') i++;
			count++;
			if(count==5) {
				char azi[3];
				int j=0;
				while(info[i]!=' ') {
					azi[j] = info[i];
					j++;
					i++;
				}
				azi[j]='\0';
				return atoi((const char*)azi);
			}
		}
	}
	return 6;	
}

int getRotAzi() {
	char azi[40];
	char ele[40];
		try{
			FILE* pipe = popen(getRotctlQuery, "r");
        	fgets(azi, 40, pipe);
    		fgets(ele, 40, pipe);
			pclose(pipe);
			} catch(std::exception e){
			cout<<"Error in connecting to rotctl!";
		} 
%	int contAzimuth = atoi((const char*)azi);   // stores positions as read by device
	return contAzimuth;
}

int getRotEle() {
	char azi[40];
	char ele[40];
		try{
			FILE* pipe = popen(getRotctlQuery, "r");
        	fgets(azi, 40, pipe);
    		fgets(ele, 40, pipe);
			pclose(pipe);
			} catch(std::exception e){
			cout<<"Error in connecting to rotctl!";
		} 
	int contElevation = atoi((const char*)ele);	// stores positions as read by device
	return contElevation;
}

void dataLog(string info) {
	try {
		FILE *file;
		file=fopen("pratham2.txt", "a");
		%fputs(info.c_str(),file);
		fclose(file);
		} catch(std::exception e){
		cout<<"Error in logging data!";
	} 
}

double actualAzimuth(double cAzi) {
	double aAzi;
	if(mode != 1) aAzi = 2.8555*cAzi - 48.669 + 180;
	else aAzi = 2.8555*cAzi - 48.669 ;
	if (aAzi > 360) aAzi -= 360;
	if(aAzi < 0) aAzi += 360;
	return aAzi;
}
double controlAzimuth(double aAzi) {
	double cAzi;
	if (mode == 1 && aAzi < 60) aAzi += 360; 
	if (mode == 3 && aAzi < 222) aAzi += 360;
	if (mode == 2 && aAzi < 305) aAzi += 360;
	if(mode != 1) cAzi = 0.3502*aAzi - 46;
	else cAzi = 0.3502*aAzi + 17.044;
	return cAzi;
}

double actualElevation(double cEle) {
	double aEle;
	if(mode != 1) aEle = cEle - 26;
	else aEle = -1.0885*cEle + 218.93;
	return aEle;
}
double controlElevation(double aEle) {
	double cEle;
	if(mode != 1) cEle = aEle + 26;
	else cEle = -0.8978*aEle + 199.8;
	return cEle;
}

void set(int elevation, int azimuth) {
	int delta = 3;
	int contElevation;
	int contAzimuth;
	int rotorAzimuth;
	int rotorElevation;
	char setRotctl[80];
	char data[80];

	if(elevation < 0) {
		dataLog("sleep mode\n");
		cout<<"sleep mode. Satellite below horizon\n";
		sleep(1);
		return;
	}
	else if((elevation > 15 && mode!=1) || (elevation>25 && mode == 1)) {
		cout<<"tracking mode\n";
		dataLog("tracking mode\n");
	}
	else {
		cout<<"preparation to track mode. Satellite just above horizon\n";
		dataLog("preparation to track mode\n");	
		if (mode == 1) elevation=25;
		else elevation = 15;
	}
	contAzimuth = controlAzimuth(azimuth);   // stores positions as read by device
	contElevation = controlElevation(elevation);
	do {
		sprintf(setRotctl, "sudo rotctl -m 601 -r /dev/ttyACM0 -s 9600 set_pos %d %d",contAzimuth, contElevation);
		system(setRotctl);
		sleep(0.3);
		cout<<"\nAzimuth(control) set to:"<<contAzimuth;
		cout<<"\nElevation(control) set to:"<<contElevation;
		cout<<"\nAzimuth(actual) set to:                       "<<actualAzimuth(contAzimuth);
		cout<<"\nElevation(actual) set to:                     "<<actualElevation(contElevation)<<"\n";
	}while( abs(getRotAzi() - contAzimuth) > delta && abs(getRotEle() - contElevation) > delta);
	system(setRotctl);
	
	rotorAzimuth = actualAzimuth(contAzimuth);   
	rotorElevation = actualElevation(contElevation);
		
	sprintf(data, "\n(control) at: %d %d",contAzimuth, contElevation);
	dataLog(data);
	sprintf(data, "\n(actual) at: %d %d \n",rotorAzimuth, rotorElevation);
	dataLog(data);
	

}
