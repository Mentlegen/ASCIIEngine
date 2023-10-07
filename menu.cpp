#include <engine.hpp>
#include <string>
#include <vector>

class Menu {
	string title; // Title of the screen
	unsigned char opts; // How many options you can select
	vector<unsigned short> opX; // x position of cursor for option [n]
	vector<unsigned short> opY; // y position of cursor for option [n]
	
	public:
		// Constructors, destructors
		Menu() {
			opts = 0;
		}

		~Menu() {
		}

		// Returns the index of the added entry
		unsigned char addEntry(unsigned short x, unsigned short y) {
			const unsigned char index = opts; // Index is 1 less than options
			opX.push_back(x);
			opY.push_back(y);
			return index;
		}

		// Return true if option operation was performed successfully
		// Return false if it failed
		virtual bool onSelect(unsigned char option) {
			// Example
			switch (option) {
				case 0: return true;
				case 1: return true;
				default: return false;
			}

		}
};
