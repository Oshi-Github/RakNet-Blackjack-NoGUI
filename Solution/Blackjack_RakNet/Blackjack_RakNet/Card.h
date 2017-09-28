#ifndef CARD_H
#define CARD_H
#include <string>
// includes raknet header files
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>

// adds library files for RakNet
#pragma comment(lib, "LibStatic.lib")
#pragma comment(lib, "ws2_32.lib")

class Card{
private:
	int value; // card value for calculations
	char type; // card type 'A' for Ace, '2' for 2, etc, 'J' - Jack, 'Q' - Queen, 'K' - King
	char suit; // card suit 'D' - Diamonds, 'H' - Hearts, 'S' - Spades, 'C' - Clubs
	RakNet::RakString name; // cards name for printing
public:
	// constructors
	Card();
	Card(char t, char s);
	//Card(RakNet::RakString &card);

	
	void SetValue(char t); // set the cards value using its type
	int GetValue() { return value; } // return cards value

	void SetName(char t, char s); // set the cards name using its type and suit from GetName method
	RakNet::RakString GetName(){ return name; } // return cards name
	RakNet::RakString GetNameString(char t, char s); // get name string for given type and suit
};
#endif