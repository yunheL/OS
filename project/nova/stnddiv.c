#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <math.h>

int main(int argc, char** argv) {
	FILE *fp;
	fp = fopen("ExecutionTimesSeq8_100k.txt","r");
	float time[10];
	float sum = 0.0;
	float stnDiv = 0.0;
	float avg = 0.0;
	int i;
	for(i = 0; i < 10; i++){
		fscanf(fp,"%f",&time[i]);
		sum+=time[i];
	}
	avg = sum/10;
	double power;
	for(i=0; i<10; i++){

		power = pow(time[i]-avg,2);
		stnDiv += power;
		//stnDiv += pow(time[i] - avg,2);
	}
	stnDiv = sqrt(stnDiv/10);
	printf("Avg Cycles: %f\n",avg);
	printf("Stnd Div: %f\n",stnDiv);
	fclose(fp);
	return 0;
}