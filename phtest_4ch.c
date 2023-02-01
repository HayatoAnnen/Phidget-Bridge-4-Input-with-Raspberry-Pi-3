#include <phidget22.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define DATA_INTERVAL_MS		(100)
#define PHIDGET_BRIDGE_GAIN		(BRIDGE_GAIN_128)
#define PHIDGET_BRIDGE_NUM_CH	(4)
#define NUM_INIT_CONV			(100)
#define SCALE_VOLTAGERATIO		(1000)

//Declare your Phidget channels and other variables
static PhidgetVoltageRatioInputHandle voltageRatioInput[PHIDGET_BRIDGE_NUM_CH];

static double g_voltageRatio[PHIDGET_BRIDGE_NUM_CH];
static double zero_offset[PHIDGET_BRIDGE_NUM_CH];

static FILE *fcsv;

//Declare any event handlers here. These will be called every time the associated event occurs.

static void CCONV onVoltageRatioChange(PhidgetVoltageRatioInputHandle ch, void * ctx, double voltageRatio) {
	int channel;
	//static uint32_t conv_cnt	= 0;
	static bool voltageRatioUpdate[PHIDGET_BRIDGE_NUM_CH]	= { 0 };

	//Getting the channel number to distinguish between Phidgets
	Phidget_getChannel((PhidgetHandle)ch, &channel);
	if (channel >= 0 && channel < PHIDGET_BRIDGE_NUM_CH) {
		g_voltageRatio[channel]		= (voltageRatio - zero_offset[channel]) * SCALE_VOLTAGERATIO;
		voltageRatioUpdate[channel]	= true;
		
		if (voltageRatioUpdate[0] & voltageRatioUpdate[1]
			& voltageRatioUpdate[2] & voltageRatioUpdate[3]) {
			
			printf("\rv = [ %9.6lf %9.6lf %9.6lf %9.6lf ]",
					g_voltageRatio[0], g_voltageRatio[1], g_voltageRatio[2], g_voltageRatio[3]);
			voltageRatioUpdate[0]	= false;
			voltageRatioUpdate[1]	= false;
			voltageRatioUpdate[2]	= false;
			voltageRatioUpdate[3]	= false;
			
			if (fcsv != NULL) {
				fprintf(fcsv, "%9.6lf, %9.6lf, %9.6lf, %9.6lf\n",
						g_voltageRatio[0], g_voltageRatio[1], g_voltageRatio[2], g_voltageRatio[3]);
			}
			/*
			conv_cnt++;
			if (conv_cnt >= 10 * 5) {
				Phidget_close((PhidgetHandle)voltageRatioInput[0]);
				Phidget_close((PhidgetHandle)voltageRatioInput[1]);
				Phidget_close((PhidgetHandle)voltageRatioInput[2]);
				Phidget_close((PhidgetHandle)voltageRatioInput[3]);

				PhidgetVoltageRatioInput_delete(&voltageRatioInput[0]);
				PhidgetVoltageRatioInput_delete(&voltageRatioInput[1]);
				PhidgetVoltageRatioInput_delete(&voltageRatioInput[2]);
				PhidgetVoltageRatioInput_delete(&voltageRatioInput[3]);
			}
			*/
		}
	}
}

static void CCONV onVoltageRatioChangeInit(PhidgetVoltageRatioInputHandle ch, void * ctx, double voltageRatio) {
	int channel;
	static int init_conv_cnt	= 0;
	static bool voltageRatioUpdate[PHIDGET_BRIDGE_NUM_CH]	= { 0 };
	static double sum_voltageRatio[PHIDGET_BRIDGE_NUM_CH]	= { 0 };

	//Getting the channel number to distinguish between Phidgets
	Phidget_getChannel((PhidgetHandle)ch, &channel);
	if (channel >= 0 && channel < PHIDGET_BRIDGE_NUM_CH) {
		sum_voltageRatio[channel]		+= voltageRatio;
		voltageRatioUpdate[channel]	= true;
		
		if (voltageRatioUpdate[0] & voltageRatioUpdate[1]
			& voltageRatioUpdate[2] & voltageRatioUpdate[3]) {
			init_conv_cnt++;
			if (init_conv_cnt >= NUM_INIT_CONV) {
				PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(voltageRatioInput[0], onVoltageRatioChange, NULL);
				PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(voltageRatioInput[1], onVoltageRatioChange, NULL);
				PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(voltageRatioInput[2], onVoltageRatioChange, NULL);
				PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(voltageRatioInput[3], onVoltageRatioChange, NULL);
				
				zero_offset[0]		= sum_voltageRatio[0] / NUM_INIT_CONV;
				zero_offset[1]		= sum_voltageRatio[1] / NUM_INIT_CONV;
				zero_offset[2]		= sum_voltageRatio[2] / NUM_INIT_CONV;
				zero_offset[3]		= sum_voltageRatio[3] / NUM_INIT_CONV;
				
				fcsv	= fopen("loadcell.csv", "w");
				if (fcsv == NULL) {
					printf("File open error! : loadcell.csv\n");
				} else {
					fprintf(fcsv, "CH 0, CH 1, CH 2, CH 3\n");
				}
			}
			
			voltageRatioUpdate[0]	= false;
			voltageRatioUpdate[1]	= false;
			voltageRatioUpdate[2]	= false;
			voltageRatioUpdate[3]	= false;
		}
	}
}

static void CCONV onAttach(PhidgetHandle ch, void * ctx) {
	int channel;

	//Getting the channel number to distinguish between Phidgets
	Phidget_getChannel(ch, &channel);
	printf("Attach [%d]!\n", channel);
}

static void CCONV onDetach(PhidgetHandle ch, void * ctx) {
	int channel;

	//Getting the channel number to distinguish between Phidgets
	Phidget_getChannel(ch, &channel);
	printf("Detach [%d]!\n", channel);
}

int main(int argc, char *argv[])
{
	//Create your Phidget channels
	PhidgetVoltageRatioInput_create(&voltageRatioInput[0]);
	PhidgetVoltageRatioInput_create(&voltageRatioInput[1]);
	PhidgetVoltageRatioInput_create(&voltageRatioInput[2]);
	PhidgetVoltageRatioInput_create(&voltageRatioInput[3]);

	//Set addressing parameters to specify which channel to open (if any)
	Phidget_setChannel((PhidgetHandle)voltageRatioInput[0], 0);
	Phidget_setChannel((PhidgetHandle)voltageRatioInput[1], 1);
	Phidget_setChannel((PhidgetHandle)voltageRatioInput[2], 2);
	Phidget_setChannel((PhidgetHandle)voltageRatioInput[3], 3);

	//Assign any event handlers you need before calling open so that no events are missed.
	PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(voltageRatioInput[0], onVoltageRatioChangeInit, NULL);
	Phidget_setOnAttachHandler((PhidgetHandle)voltageRatioInput[0], onAttach, NULL);
	Phidget_setOnDetachHandler((PhidgetHandle)voltageRatioInput[0], onDetach, NULL);
	PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(voltageRatioInput[1], onVoltageRatioChangeInit, NULL);
	Phidget_setOnAttachHandler((PhidgetHandle)voltageRatioInput[1], onAttach, NULL);
	Phidget_setOnDetachHandler((PhidgetHandle)voltageRatioInput[1], onDetach, NULL);
	PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(voltageRatioInput[2], onVoltageRatioChangeInit, NULL);
	Phidget_setOnAttachHandler((PhidgetHandle)voltageRatioInput[2], onAttach, NULL);
	Phidget_setOnDetachHandler((PhidgetHandle)voltageRatioInput[2], onDetach, NULL);
	PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(voltageRatioInput[3], onVoltageRatioChangeInit, NULL);
	Phidget_setOnAttachHandler((PhidgetHandle)voltageRatioInput[3], onAttach, NULL);
	Phidget_setOnDetachHandler((PhidgetHandle)voltageRatioInput[3], onDetach, NULL);

	//Open your Phidgets and wait for attachment
	Phidget_openWaitForAttachment((PhidgetHandle)voltageRatioInput[0], 5000);
	Phidget_openWaitForAttachment((PhidgetHandle)voltageRatioInput[1], 5000);
	Phidget_openWaitForAttachment((PhidgetHandle)voltageRatioInput[2], 5000);
	Phidget_openWaitForAttachment((PhidgetHandle)voltageRatioInput[3], 5000);
	
	//Do stuff with your Phidgets here or in your event handlers.
	PhidgetVoltageRatioInput_setBridgeGain(voltageRatioInput[0], PHIDGET_BRIDGE_GAIN);
	PhidgetVoltageRatioInput_setBridgeGain(voltageRatioInput[1], PHIDGET_BRIDGE_GAIN);
	PhidgetVoltageRatioInput_setBridgeGain(voltageRatioInput[2], PHIDGET_BRIDGE_GAIN);
	PhidgetVoltageRatioInput_setBridgeGain(voltageRatioInput[3], PHIDGET_BRIDGE_GAIN);

	PhidgetVoltageRatioInput_setDataInterval(voltageRatioInput[0], DATA_INTERVAL_MS);
	PhidgetVoltageRatioInput_setDataInterval(voltageRatioInput[1], DATA_INTERVAL_MS);
	PhidgetVoltageRatioInput_setDataInterval(voltageRatioInput[2], DATA_INTERVAL_MS);
	PhidgetVoltageRatioInput_setDataInterval(voltageRatioInput[3], DATA_INTERVAL_MS);

	//Wait until Enter has been pressed before exiting
	getchar();
	printf("\n\n");
	if (fcsv != NULL) {
		fclose(fcsv);
	}

	//Close your Phidgets once the program is done.
	Phidget_close((PhidgetHandle)voltageRatioInput[0]);
	Phidget_close((PhidgetHandle)voltageRatioInput[1]);
	Phidget_close((PhidgetHandle)voltageRatioInput[2]);
	Phidget_close((PhidgetHandle)voltageRatioInput[3]);

	PhidgetVoltageRatioInput_delete(&voltageRatioInput[0]);
	PhidgetVoltageRatioInput_delete(&voltageRatioInput[1]);
	PhidgetVoltageRatioInput_delete(&voltageRatioInput[2]);
	PhidgetVoltageRatioInput_delete(&voltageRatioInput[3]);
	
	return 0;
}
