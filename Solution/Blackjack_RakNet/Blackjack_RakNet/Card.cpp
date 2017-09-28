#include <string>
#include "Card.h"
// includes raknet header files
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>

// adds library files for RakNet
#pragma comment(lib, "LibStatic.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace std;

// default constructor
Card::Card(){
	type = 'A';
	suit = 'D';
	SetValue('A');
	SetName('A', 'D');
}


// constructor
Card::Card(char t, char s){
	type = t;
	suit = s;
	SetValue(t);
	SetName(t, s);
}

/*Card::Card(RakNet::RakString &card)
{
	type = (card.SubStr(0, 1)).C_String;
	suit = (card.SubStr(1, 1)).C_String;
	SetValue(type);
	SetName(type, suit);
}*/

// set the cards value using its type
void Card::SetValue(char t){
	switch(t){
	case 'A': value = 11; break;
	case '2': value = 2; break;
	case '3': value = 3; break;
	case '4': value = 4; break;
	case '5': value = 5; break;
	case '6': value = 6; break;
	case '7': value = 7; break;
	case '8': value = 8; break;
	case '9': value = 9; break;
	case 'T': value = 10; break;
	case 'J': value = 10; break;
	case 'Q': value = 10; break;
	case 'K': value = 10; break;
	}
}

// set the cards name using its type and suit from GetName method
void Card::SetName(char t, char s){
	name = GetNameString(t, s);
}

// get name string for given type and suit
RakNet::RakString Card::GetNameString(char t, char s){
	RakNet::RakString nme = ""; // local name variable, no a to prevent masking the original name

	switch(t){
	case 'A': nme = "Ace of "; break;
	case '2': nme = "2 of "; break;
	case '3': nme = "3 of "; break;
	case '4': nme = "4 of "; break;
	case '5': nme = "5 of "; break;
	case '6': nme = "6 of "; break;
	case '7': nme = "7 of "; break;
	case '8': nme = "8 of "; break;
	case '9': nme = "9 of "; break;
	case 'T': nme = "10 of "; break;
	case 'J': nme = "Jack of "; break;
	case 'Q': nme = "Queen of "; break;
	case 'K': nme = "King of "; break;
	}

	switch(s){
	case 'D': nme += "Diamonds"; break;
	case 'S': nme += "Spades"; break;
	case 'H': nme += "Hearts"; break;
	case 'C': nme += "Clubs"; break;
	}

	return nme;
}	
