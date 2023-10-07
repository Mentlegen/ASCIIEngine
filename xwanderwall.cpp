#include <chrono>
#include <iostream>
#include <thread>
#include <linux/input.h>
#include "engine.hpp"
using namespace std;


// Example program to demonstrate the usage of the engine.
// 
// Move your character through a maze with the arrow keys.
// He cannot pass through the walls, however he can pick up objects.
//
// Press ESC to prompt the user to quit.
struct WanderwallGame {
	// default terminal window is 80x24
	ASCIIWindow * window = new ASCIIWindow(80, 24);	
	// offset two chars down to make room for info text
	CharDisplay display = CharDisplay(80, 22, 0, 2, window);
	unsigned short px, py, // player x and y position
		mpWd, mpHt; // maximum travelable map bounds
	bool running;
	unsigned char mode; // 0 for main menu, 1 for game, 2 for pause

	// All the structs of the game
	/* collcodes for this demonstration are quite simple.
	   0 means pass through, 1 means not.*/
	//CharStructGroup walls = CharStructGroup();
	Box * surroundWorld = new Box(0x00000001, '0', false, mpWd, mpHt);
	Line * line1 = new Line(0x00000001, '0', 5, 2, 1, true);
	Line * line2 = new Line(0x00000001, '0', 5, 1, 7, false);
	CollChar * player = new CollChar(0x10000000, 'A', px, py);

	// ========================
	// Initialization functions
	// ========================

	WanderwallGame() {
		init();
	}

	~WanderwallGame() {
		end();
	}
	
	// Initializes the Wanderwall game.
	// This is called as soon as the object is created.
	bool init() {	
		// Initialize the window
		window -> build(); // build window
		window -> cursVis(0); // hide cursor
		window -> writeAt(0, 0, "===Wanderwall===");
		window -> initRealTime();
		
		// Initialize variables
		px = 25, py = 10;
		mpWd = 100, mpHt = 50;
		
		// Initialize structures
		display.addStruct(surroundWorld);
		display.addStruct(line1);
		display.addStruct(line2);
		display.addStruct(player);
		display.writeStructs();
		display.update();
		
		running = true;
		mode = 0;
		return true;
	}

	// Destructs the Wanderwall game.
	// This is called as soon as the object is destroyed.
	bool end() {
		// destroy the window
		window -> close();
		delete window;
		return true;
	}
	
	// =============
	// Frame Methods
	// =============

	// Called every frame.
	// This method is called by the thread
	const void run() {
	
		// First get the input
		char input = parseKey();
		
		if(input == 27) { // escape key
			window -> exitRealTime(); // exit real time to wait for confirmation
			window -> writeAt(0, 1, "Quit? [Y/N]             ");
			char conf = window -> getKey();
			if(conf == 'y') running = false;
			else {
				window -> initRealTime(); // resume realtime if not confirmed
				window -> writeAt(0, 1, "                        ");
			}
		}
		else { // Normal running
			bool change = tryMove(player, input);
			// updateDisp(change, input); // This only updates if screen changed
			updateDisp(true, input); // This always updates even if no screen change
		}
	}	
	
	// 
	private:
		// This function always returns the last ASCII key in the value
		char parseKey() {
			// nextKey is the next key and gets consumed every operation
			// essentially this means that nextKey always ends at -1,
			// while key is the actual key character value that gets returned
			char nextKey = window -> getKey(), 
				key = 0;
			// runs through every char value in the buffer
			while(nextKey != -1) {
				key = nextKey;
				nextKey = window -> getKey();
			}
			
			return key;
		}
		
		//char* detectKeys() {
			// TODO: Actually write this
			// If no key is pressed then return NULL
			// If there are keys being pressed then 
			// return a pointer array of all keys pressed
		//}

		// ================
		// Gameplay methods
		// ================

		// true if moved, false if not
		/* person - the collision character to attempt to move
		 * in - the input key from the keyboard*/
		bool tryMove(CollChar *person, const char in) {
			const unsigned short origX = px, origY = py;
			switch (in) {
				case 65: // up
					(py == 0) ? py = 0 : py--;
					person -> setChar('^');
					break;
				case 66: // down
					(py == mpWd) ? py = mpWd : py++;
					person -> setChar('v');
					break;
				case 67: // right
					(px == mpHt) ? px = mpHt : px++;
					person -> setChar('>');
					break;
				case 68: // left
					(px == 0) ? px = 0 : px--;
					person -> setChar('<');
					break;
			}
			// if collision code is 00-00-00-01 move character back
			if(display.hasCollCode(px, py, 0x00000001)) {
				px = origX; py = origY;
			} return (person -> setX(px) || person -> setY(py));
		}

		// Update the display
		/* change - Whether the screen changed since last frame/operation
		 * display - The reference to the display to update
		 * window - The pointer to the ASCII window to write information on*/
		void updateDisp(const bool change, const char in) {
			// whether player position has changed or not
			if(change) display.redrawStructs();
			display.update(); // update char screen
			// print Wonderwall of course
			window -> writeAt(0, 0, "===Wanderwall===");
			// print player coords
			string coords = "X:"+to_string(px)+" Y:"+to_string(py);
			window -> writeAt(0, 1, "Player Pos: "+coords);
		
			// debug
			window -> writeAt(20, 0, "DB: ");
			window -> writeAt(24, 0, "WID "+to_string(display.width()));
			window -> writeAt(24, 1, "HGT "+to_string(display.height()));
			window -> writeAt(30, 0, "CAT "+to_string(display.charAt(px, py+2)));
			window -> writeAt(30, 1, "KEY "+to_string(in));
		}

};

// ====================
// Out-of-class methods
// ====================

// This is called by the main thread every frame
const void update(WanderwallGame* wg, const unsigned short refreshRate) {
	while(wg -> running) {
		wg -> run();
		//unsigned short refresh = (refreshRate > 20) ? refreshRate : 20;
		this_thread::sleep_for(chrono::milliseconds(refreshRate));
	}
}

int main(int argc, char** argv) {
	// the game refreshes every *this* amount of milliseconds
	unsigned short refRate;
	// if no additional arguments refresh every 125 ms (8 fps)
	// otherwise refresh at an interval defined by user
	if(argc == 1) refRate = 125;
	else refRate = atoi(argv[1]);
	
	// one game instance
	WanderwallGame * game = new WanderwallGame();
	
	// main thread
	thread main(update, game, refRate);
	main.join();
	
	// end process
	delete game;
	return 0;
}
