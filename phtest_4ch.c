#include <phidget22.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#define DATA_INTERVAL_MS		(8)	// Should be a multiple of 8
#define PHIDGET_BRIDGE_GAIN		(BRIDGE_GAIN_128)
#define PHIDGET_BRIDGE_NUM_CH	(4)
#define NUM_OFFSET_CNT			(100)
#define SCALE_VOLTAGERATIO		(1000)

//Declare your Phidget channels and other variables
static PhidgetVoltageRatioInputHandle voltageRatioInput[PHIDGET_BRIDGE_NUM_CH];

static double g_vr_res[PHIDGET_BRIDGE_NUM_CH]		= { 0 };
static double g_vr_offset[PHIDGET_BRIDGE_NUM_CH]	= { 0 };

static FILE *fcsv;

//Declare any event handlers here. These will be called every time the associated event occurs.

static void CCONV onVoltageRatioChange(PhidgetVoltageRatioInputHandle ch, void * ctx, double voltageRatio)
{
	int channel, i;
	static int res_cnt	= 0;
	static bool vr_res_update[PHIDGET_BRIDGE_NUM_CH]	= { false };

	//Getting the channel number to distinguish between Phidgets
	Phidget_getChannel((PhidgetHandle)ch, &channel);
	if (channel >= 0 && channel < PHIDGET_BRIDGE_NUM_CH) {
		g_vr_res[channel]		= voltageRatio * SCALE_VOLTAGERATIO - g_vr_offset[channel];
		vr_res_update[channel]	= true;
		
		for (i = 0; i < PHIDGET_BRIDGE_NUM_CH; i++) {
			if (vr_res_update[i] == false) {
				return;
			}
		}
		
		res_cnt++;
		printf("\r%9.6lf %9.6lf %9.6lf %9.6lf", g_vr_res[0], g_vr_res[1], g_vr_res[2], g_vr_res[3]);
		if (fcsv != NULL) {
			fprintf(fcsv, "%9.6lf,%9.6lf,%9.6lf,%9.6lf\n", g_vr_res[0], g_vr_res[1], g_vr_res[2], g_vr_res[3]);
		}
		for (i = 0; i < PHIDGET_BRIDGE_NUM_CH; i++) {
			vr_res_update[i] = false;
		}
		if (res_cnt >= 1000) {
			Phidget_close((PhidgetHandle)voltageRatioInput[0]);
			Phidget_close((PhidgetHandle)voltageRatioInput[1]);
			Phidget_close((PhidgetHandle)voltageRatioInput[2]);
			Phidget_close((PhidgetHandle)voltageRatioInput[3]);

			PhidgetVoltageRatioInput_delete(&voltageRatioInput[0]);
			PhidgetVoltageRatioInput_delete(&voltageRatioInput[1]);
			PhidgetVoltageRatioInput_delete(&voltageRatioInput[2]);
			PhidgetVoltageRatioInput_delete(&voltageRatioInput[3]);
		}
	}
}

static void CCONV onVoltageRatioGetOffset(PhidgetVoltageRatioInputHandle ch, void * ctx, double voltageRatio)
{
	int channel, i;
	static int vr_res_cnt[PHIDGET_BRIDGE_NUM_CH]	= { 0 };

	//Getting the channel number to distinguish between Phidgets
	Phidget_getChannel((PhidgetHandle)ch, &channel);
	if (channel >= 0 && channel < PHIDGET_BRIDGE_NUM_CH && vr_res_cnt[channel] < NUM_OFFSET_CNT) {
		g_vr_res[channel]		+= voltageRatio * SCALE_VOLTAGERATIO;
		vr_res_cnt[channel]++;
		
		if (vr_res_cnt[channel] == NUM_OFFSET_CNT) {
			g_vr_offset[channel]	= g_vr_res[channel] / vr_res_cnt[channel];
			for (i = 0; i < PHIDGET_BRIDGE_NUM_CH; i++) {
				if (g_vr_offset[i] == 0.0) {
					return;
				}
			}
			printf("%9.6lf %9.6lf %9.6lf %9.6lf\n", g_vr_offset[0], g_vr_offset[1], g_vr_offset[2], g_vr_offset[3]);
			
			if (fcsv != NULL) {
				fprintf(fcsv, "CH 0 offset,CH 1 offset,CH 2 offset,CH 3 offset\n");
				fprintf(fcsv, "%9.6lf,%9.6lf,%9.6lf,%9.6lf\n", g_vr_offset[0], g_vr_offset[1], g_vr_offset[2], g_vr_offset[3]);
				fprintf(fcsv, "CH 0,CH 1,CH 2,CH 3\n");
			} else {
				printf("loadcell.csv open error!\n");
			}
			
			PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(voltageRatioInput[0], onVoltageRatioChange, NULL);
			PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(voltageRatioInput[1], onVoltageRatioChange, NULL);
			PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(voltageRatioInput[2], onVoltageRatioChange, NULL);
			PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(voltageRatioInput[3], onVoltageRatioChange, NULL);
		}		
	}
}

static void CCONV onAttach(PhidgetHandle ch, void * ctx)
{
	int channel, i;
	static bool channel_init[PHIDGET_BRIDGE_NUM_CH]	= { 0 };

	//Getting the channel number to distinguish between Phidgets
	Phidget_getChannel(ch, &channel);
	printf("Attach [%d]!\n", channel);
	channel_init[channel]	= true;
	
	for (i = 0; i < PHIDGET_BRIDGE_NUM_CH; i++) {
		if (channel_init[i] != true) {
			return;
		}
	}
	
	PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(voltageRatioInput[0], onVoltageRatioGetOffset, NULL);
	PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(voltageRatioInput[1], onVoltageRatioGetOffset, NULL);
	PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(voltageRatioInput[2], onVoltageRatioGetOffset, NULL);
	PhidgetVoltageRatioInput_setOnVoltageRatioChangeHandler(voltageRatioInput[3], onVoltageRatioGetOffset, NULL);
}

static void CCONV onDetach(PhidgetHandle ch, void * ctx)
{
	int channel;

	//Getting the channel number to distinguish between Phidgets
	Phidget_getChannel(ch, &channel);
	printf("Detach [%d]!\n", channel);
}

int main(int argc, char *argv[])
{	
	fcsv	= fopen("loadcell.csv", "w");

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
	Phidget_setOnAttachHandler((PhidgetHandle)voltageRatioInput[0], onAttach, NULL);
	Phidget_setOnDetachHandler((PhidgetHandle)voltageRatioInput[0], onDetach, NULL);
	Phidget_setOnAttachHandler((PhidgetHandle)voltageRatioInput[1], onAttach, NULL);
	Phidget_setOnDetachHandler((PhidgetHandle)voltageRatioInput[1], onDetach, NULL);
	Phidget_setOnAttachHandler((PhidgetHandle)voltageRatioInput[2], onAttach, NULL);
	Phidget_setOnDetachHandler((PhidgetHandle)voltageRatioInput[2], onDetach, NULL);
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

	//Close your Phidgets once the program is done.
/*
	Phidget_close((PhidgetHandle)voltageRatioInput[0]);
	Phidget_close((PhidgetHandle)voltageRatioInput[1]);
	Phidget_close((PhidgetHandle)voltageRatioInput[2]);
	Phidget_close((PhidgetHandle)voltageRatioInput[3]);

	PhidgetVoltageRatioInput_delete(&voltageRatioInput[0]);
	PhidgetVoltageRatioInput_delete(&voltageRatioInput[1]);
	PhidgetVoltageRatioInput_delete(&voltageRatioInput[2]);
	PhidgetVoltageRatioInput_delete(&voltageRatioInput[3]);
*/
	printf("\n");
	if (fcsv != NULL) {
		fclose(fcsv);
	}
	
	return 0;
}
