#ifndef PLAYER_H
#define PLAYER_H
#include "Hand.h"
#include <string>

// includes raknet header files
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>

// adds the library files for RakNet
#pragma comment(lib, "LibStatic.lib")
#pragma comment(lib, "ws2_32.lib")

class Player{
private:
	RakNet::RakString name; // hold players name
	int score; // players score
	bool active; // whether the player is still playing or not
public:
	int currentBet;
	int turnCount;
	int wins, losses, draws;
	bool insurance;
	Hand hand; // players hand
	int playerID;
	RakNet::SystemAddress clientIP;

	// constructor
	Player();
	Player(RakNet::RakString name);

	void SetName(RakNet::RakString name){ Player::name = name; } // set name value
	RakNet::RakString GetName(){ return name; } // get name for printing etc

	void SetActive(bool active){ Player::active = active; } // set players active state
	bool GetActive(){ return active; } // get players active state

	void AddScore(int adding){ Player::score += adding; if(adding < 0){ Player::currentBet += -adding; }} // add to the players score
	int GetScore(){ return score; } // get the players score
	void SetScore(int score) { Player::score = score; }
};
#endif