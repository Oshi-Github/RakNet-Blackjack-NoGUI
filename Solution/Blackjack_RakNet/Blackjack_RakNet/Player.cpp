#include <string>
#include "Player.h"
#include "Hand.h"


// includes raknet header files
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>

// adds the library files for RakNet
#pragma comment(lib, "LibStatic.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int startingScore = 200;

// default constructor
Player::Player(){
	Player::hand = Hand();
	Player::score = startingScore;
	Player::turnCount = 1;
	Player::currentBet = 0;
	Player::wins = 0; Player::draws = 0; Player::losses = 0;
	Player::playerID = -1;
}

// constructor with name set
Player::Player(RakNet::RakString name){
	Player::hand = Hand();
	Player::SetName(name);
	Player::score = startingScore;
	Player::turnCount = 1;
	Player::currentBet = 0;
	Player::wins = 0; Player::draws = 0; Player::losses = 0;
	Player::playerID = -1;
}