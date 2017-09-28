#include "Deck.h"
#include "Card.h"

#include <random>
#include <time.h>

// includes raknet header files
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>

// adds the library files for RakNet
#pragma comment(lib, "LibStatic.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace std;

// constructor
Deck::Deck(){
	currentCard = -1;
	GenerateSuit(0, 'H');
	GenerateSuit(13, 'D');
	GenerateSuit(26, 'C');
	GenerateSuit(39, 'S');
	Shuffle(150);
}

// generate a set of cards for a suit
void Deck::GenerateSuit(int startindex, char suit){
	Deck::cards[startindex] = Card('A', suit);
	Deck::cards[startindex+1] = Card('2', suit);
	Deck::cards[startindex+2] = Card('3', suit);
	Deck::cards[startindex+3] = Card('4', suit);
	Deck::cards[startindex+4] = Card('5', suit);
	Deck::cards[startindex+5] = Card('6', suit);
	Deck::cards[startindex+6] = Card('7', suit);
	Deck::cards[startindex+7] = Card('8', suit);
	Deck::cards[startindex+8] = Card('9', suit);
	Deck::cards[startindex+9] = Card('T', suit);
	Deck::cards[startindex+10] = Card('J', suit);
	Deck::cards[startindex+11] = Card('Q', suit);
	Deck::cards[startindex+12] = Card('K', suit);
}

// shuffle the deck count times
// swap 2 cards around [count] times, effective shuffle decently performant depending on how high count is
void Deck::Shuffle(int count){
	srand(time(NULL)); // setup random seed
	while(count > 0){
		Card temp;
		int swap = rand() % 52;
		int swap2 = rand() % 52;
		temp = Deck::cards[swap];
		Deck::cards[swap] = Deck::cards[swap2];
		Deck::cards[swap2] = temp;

		count--;
	}
}

Card Deck::NextCard(){
	Deck::currentCard++;
	if(Deck::currentCard > 51){ Deck::currentCard = 0; Deck::Shuffle(150); }
	return Deck::cards[currentCard];
}