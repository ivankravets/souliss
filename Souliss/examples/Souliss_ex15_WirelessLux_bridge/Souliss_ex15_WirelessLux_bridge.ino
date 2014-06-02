/**************************************************************************
	Souliss - Wireless Lux
	
	In this example there are two nodes (bridge and wireless) either with
	two light that can be controlled via Android (or any other user interface)
	and pushbuttons.
	
	Applicable for : 
			- Lights controller via relay,
			- Plugs and other ON/OFF devices.
	
	Each device has
		- A coil (relay or other) on Pin 4 and Pin 5
		- A couple of inputs on Pin 2 and 3, pulldown required
				
***************************************************************************/
#include "bconf/inSketch.h"

// Select one of the following boards
//
//		QC_BOARDTYPE			Description
//
//		0x02					Freaklabs Chibiduino with Ethernet Shield (W5100)
//		0x40					Freaklabs Chibiduino with ENC28J60 Ethernet Shield
//		0x44					Arduino Ethernet (or Ethernet Shield) with Nordic nRF24L01 or nRF24L01+
//		0x45					Arduino with ENC28J60 Ethernet Shield and  Nordic nRF24L01 or nRF24L01+
//
#define	QC_BOARDTYPE				0x44	

// This board bridges data from the wireless node and move then over the Ethernet/WiFi
// network, select gateway option (enable 0x01)
#define	QC_GATEWAYTYPE				0x01	

// Isn't used any additional interface
#define	QC_INTERFACE				0x00	

#include "Souliss.h"
#include "Typicals.h"
#include <SPI.h>

#define	network_bridge		0x0011
#define network_address_1	0x6511
#define network_address_2	0x6512
#define network_my_subnet	0xFF00
#define network_my_supern	0x0000

#define LIGHT1_NODE1			0			 
#define LIGHT2_NODE1			1			 
#define LIGHT1_NODE2			0			 
#define LIGHT2_NODE2			1			 

// define the shared memory map
U8 memory_map[MaCaco_MEMMAP];

// flag 
U8 data_changed = 0;

#define time_base_fast		10				// Time cycle in milliseconds
#define time_base_slow		10000			// Time cycle in milliseconds
#define num_phases			255				// Number of phases

U8 phase_speedy=0, phase_fast=0, phase_slow=0;
unsigned long tmr_fast=0, tmr_slow=0;  

void setup()
{	
	// Setup the network configuration
	//
	//	The node last byte is the same as vNet address, as example 192.168.1.17
	Souliss_SetAddress(network_bridge, network_my_subnet, network_my_supern);	
	Souliss_SetAddress(network_address_1, network_my_subnet, network_my_supern);	
	
	// List the wireless node to be in touch
	Souliss_SetLocalAddress(memory_map, network_bridge);
	Souliss_SetRemoteAddress(memory_map, network_address_2, 1);
	
	// Set the typical logic to handle the lights
	Souliss_SetT11(memory_map, LIGHT1_NODE1);
	Souliss_SetT11(memory_map, LIGHT2_NODE1);
	
	// Define inputs, outputs pins
	pinMode(2, INPUT);					// Hardware pulldown required
	pinMode(3, INPUT);					// Hardware pulldown required	
	pinMode(4, OUTPUT);					// Power to relay coil for light 1
	pinMode(5, OUTPUT);					// Power to relay coil for light 2
}

void loop()
{   
	if(abs(millis()-tmr_fast) > time_base_fast)
	{	
		tmr_fast = millis();
		phase_fast = (phase_fast + 1) % num_phases;
	  
		// Execute the code every 3 time_base_fast		
		if (!(phase_fast % 3))
		{
			// Use Pin2 as ON/OFF command
			Souliss_DigIn(2, Souliss_T1n_ToogleCmd, memory_map, LIGHT1_NODE1);	
			
			// Use Pin3 as ON/OFF command
			Souliss_DigIn(3, Souliss_T1n_ToogleCmd, memory_map, LIGHT2_NODE1);		
			
			// Execute the logic
			Souliss_Logic_T11(memory_map, LIGHT1_NODE1, &data_changed);
			Souliss_Logic_T11(memory_map, LIGHT2_NODE1, &data_changed);

			// Use Pin4 as output on the electrical load
			Souliss_DigOut(4, Souliss_T1n_Coil, memory_map, LIGHT1_NODE1);	

			// Use Pin5 as output on the electrical load
			Souliss_DigOut(5, Souliss_T1n_Coil, memory_map, LIGHT2_NODE1);						
		} 
		
		// Execute the code every 5 time_base_fast		  
		if (!(phase_fast % 5))
		{   
			// Retreive data from the communication channel
			Souliss_CommunicationData(memory_map, &data_changed);					
		}

		// Execute the code every 31 time_base_fast		  
		if (!(phase_fast % 71))
		{   
			// Get logic typicals once and at every refresh
			Souliss_GetTypicals(memory_map);
		}
		
		// Execute the code every 51 time_base_fast		  
		if (!(phase_fast % 51))
		{   
			// Open a communication channel with remote nodes
			Souliss_CommunicationChannels(memory_map);
		}		
	}
	else if(abs(millis()-tmr_slow) > time_base_slow)
	{	
		tmr_slow = millis();
		phase_slow = (phase_slow + 1) % num_phases;

		// Execute the code every time_base_slow
		if (!(phase_slow % 1))
		{                 
			// The timer handle timed-on states
			Souliss_T11_Timer(memory_map, LIGHT1_NODE1);
			Souliss_T11_Timer(memory_map, LIGHT2_NODE1);						
		} 	  
	}		
} 
