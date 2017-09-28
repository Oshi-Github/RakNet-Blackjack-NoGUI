#include <string>
#include "Hand.h"
#include "Card.h"


// includes raknet header files
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>

// adds the library files for RakNet
#pragma comment(lib, "LibStatic.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace std;

Hand::Hand(){
	cardCount = 0;
}

// empty hand
void Hand::ClearHand(){
	cardCount = 0;
}

// add a card to the hand
void Hand::AddCard(Card adding){
	Hand::cardCount++;
	Hand::cards[cardCount - 1] = adding;
}

// get the value of the hand by adding each cards value
int Hand::GetValue(){
	int lowvalue = 0; // value for low ace (1)
	int highvalue = 0; // value for high ace (11)

	for(int i = 0; i < Hand::cardCount; i++){
		if(Hand::cards[i].GetValue() == 11){
			// if the card is ace, apply to values correctly
			lowvalue += 1;
			highvalue += 11;
		}else{
			lowvalue += Hand::cards[i].GetValue();
			highvalue += Hand::cards[i].GetValue();
		}
	}

	// if high ace takes the hand over 21, use low ace value instead
	if(highvalue > 21){ return lowvalue; } else { return highvalue; }
}

// print first card in hand
RakNet::RakString Hand::PrintFirst(){
	return Hand::cards[0].GetName();
}

// print whole hand
RakNet::RakString Hand::PrintHand(){
	RakNet::RakString returning = "";

	// loop through hand adding each card name to the string
	for(int i = 0; i < Hand::cardCount; i++){
		returning += Hand::cards[i].GetName();
		if(i < (cardCount - 1)){ returning += ", "; }
	}
	return returning;
}