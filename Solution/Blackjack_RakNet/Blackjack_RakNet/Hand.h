#ifndef HAND_H
#define HAND_H
#include <string>


// includes raknet header files
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>
#include "Card.h"

// adds the library files for RakNet
#pragma comment(lib, "LibStatic.lib")
#pragma comment(lib, "ws2_32.lib")



class Hand{
public:
	// constructor
	Hand();

	void ClearHand(); // clear the hand
	void AddCard(Card adding); // add card to the hand
	int GetValue(); // return hand value
	RakNet::RakString PrintFirst(); // return first card in hand for printing
	RakNet::RakString PrintHand(); // return hand string for printing
private:
	Card cards[52];
	int cardCount;
};
#endif