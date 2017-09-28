#ifndef DECK_H
#define DECK_H
#include "Card.h"

// includes raknet header files
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>

// adds the library files for RakNet
#pragma comment(lib, "LibStatic.lib")
#pragma comment(lib, "ws2_32.lib")

class Deck{
private:
	Card cards[52];
	int currentCard;
public:
	// constructor
	Deck();

	void GenerateSuit(int startindex, char suit); // generate a set of cards for a suit
	void Shuffle(int count); // shuffle the deck count times
	Card NextCard(); // return next card
};
#endif