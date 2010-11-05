#include <stdio.h>
#include <unistd.h>
#include <CoreServices/CoreServices.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>
#include "mt19937ar.h"

#define kMidiMessage_ControlChange 0xB
#define kMidiMessage_ProgramChange 0xC
#define kMidiMessage_BankMSBControl 0
#define kMidiMessage_BankLSBControl 32
#define kMidiMessage_NoteOn 0x9

OSStatus CreateAUGraph (AUGraph *outGraph, AudioUnit *outSynth) {
	OSStatus result;
	AUNode synthNode, limiterNode, outNode;
	AudioComponentDescription cd;
	cd.componentManufacturer = kAudioUnitManufacturer_Apple;
	cd.componentFlags = 0;
	cd.componentFlagsMask = 0;
	NewAUGraph (outGraph);
	cd.componentType = kAudioUnitType_MusicDevice;
	cd.componentSubType = kAudioUnitSubType_DLSSynth;
	AUGraphAddNode (*outGraph, &cd, &synthNode);
	cd.componentType = kAudioUnitType_Effect;
	cd.componentSubType = kAudioUnitSubType_PeakLimiter;  
	AUGraphAddNode (*outGraph, &cd, &limiterNode);
	cd.componentType = kAudioUnitType_Output;
	cd.componentSubType = kAudioUnitSubType_DefaultOutput;  
	AUGraphAddNode (*outGraph, &cd, &outNode);
	AUGraphOpen (*outGraph);
	AUGraphConnectNodeInput (*outGraph, synthNode, 0, limiterNode, 0);
	AUGraphConnectNodeInput (*outGraph, limiterNode, 0, outNode, 0);
	AUGraphNodeInfo(*outGraph, synthNode, 0, outSynth);
	return result;
}

int main (int argc, const char * argv[]) {
	AUGraph graph = 0;
	AudioUnit synthUnit;
	UInt8 midiChannelInUse = 0;	
    UInt8 msb_controll_value = 8;
    UInt8 program_change = 80;
    CreateAUGraph (&graph, &synthUnit);
	AUGraphInitialize (graph);
	MusicDeviceMIDIEvent(synthUnit, 
                         kMidiMessage_ControlChange << 4 | midiChannelInUse, 
                         kMidiMessage_BankMSBControl, 
                         msb_controll_value,
                         0);
    MusicDeviceMIDIEvent(synthUnit, 
                         kMidiMessage_ProgramChange << 4 | midiChannelInUse, 
                         program_change, 
                         0,
                         0); /* sine wave */
	CAShow (graph);
	AUGraphStart (graph);
    
	for (int i = 0; i < 100; i++) {
        int r = 0;
        r = rand()%88;
        //r = abs(random()*10000)%88;
        //r = lrand48()%88;
        //r = genrand_int32()%88;
		UInt32 noteNum = 21 + r;
        printf("%d\n", noteNum);
		UInt32 onVelocity = 127;
		UInt32 noteOnCommand = 	kMidiMessage_NoteOn << 4 | midiChannelInUse;
		MusicDeviceMIDIEvent(synthUnit, noteOnCommand, noteNum, onVelocity, 0);
		usleep (0.1 * 1000 * 1000);
		MusicDeviceMIDIEvent(synthUnit, noteOnCommand, noteNum, 0, 0);
	}
    usleep(3 * 1000 * 1000);
	if (graph) {
		AUGraphStop (graph);
		DisposeAUGraph (graph);
	}
	return 0;
}
