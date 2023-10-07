#include <ncurses.h> // -lncurses
#include <iostream>
#include <string>
#include <vector>
using namespace std;
// Version of this program
#define ASCIIWIN_VERSION "ALPHA_0.0";

// ==========================================
// ASCII Gui Window written by Nathaniel Kang
// ------------------------------------------
// The purpose of this project is to provide
// an interface to make working with ncurses
// a little bit easier for me.
//
// It may be a little unnecessary but it's
// easier for me to keep track of this 
// structure than the ncurses methods.
//
// In the future I will have it contain the
// necessary libraries for Windows systems
// so that it can easily be compiled on both
// unix and Windows systems.
// ==========================================

enum Colors {
	BLK, BLU, GRN, CYN, RED, MGT, YLW, WHT,
	BBLK, BBLU, BGRN, BCYN, BRED, BMGT, BYLW, BWHT
};

// A command line window that can be interfaced with to show ASCII images.
class ASCIIWindow {
	bool instanced; // whether the window is instanced or not
	bool realTime; // whether the window is in real time or not
	unsigned char r, g, b; // colors
	unsigned short wd, ht; // window dimensions in chars
	unsigned short posX, posY; // location of the cursor
	WINDOW *cWindow; // C window

	// error
	class WindowError : public runtime_error {
		public:
			WindowError(string message) : runtime_error(message){}
	};
	public:
		// short w - The width of the window in command-line characters
		// short h - The height of the window in command-line characters
		ASCIIWindow(const unsigned short w, const unsigned short h) {
			wd = w;
			ht = h;
			instanced = false;
			realTime = false;
		}
		
		~ASCIIWindow() {
			if (instanced) endwin();
		}

		// Returns true if the window was successfully built.
		bool build() {
			// do not build if instanced
			if (instanced == true ) return false;
			// do not instance if terminal is too small
			//if(LINES < ht || COLS < wd) 
			//{
			//	cout << LINES << " " << COLS << "\n";
			//	string msg = "Terminal window too small, should be at least ";
			//	msg += to_string(wd); msg += 'x'; msg += to_string(ht);
			//	throw WindowError(msg);
			//}
			// instance the window now
			cWindow = initscr(); // initialize
			start_color(); // Enable color in window
			cbreak();
			noecho(); // do not echo
			for(unsigned short y=0; y<ht; y++) {
				for(unsigned short x=0; x<wd; x++)
					addch(' ');
				addch('\n');
			}
			refresh();

			instanced = true;
			return true;
		}
		
		// Returns true if the window was successfully closed.
		bool close() {
			// Will not destroy if not instanced
			if (instanced == false) return false;

			// destroy the window
			endwin();
			instanced = false;
			return false;
		}

		// ===========================
		// Misc  functions
		// ===========================

		// Wrapper for curs_set
		// 0 is invisible, 1 is normal, 2 is emphasized
		// TODO this has an error handling thing so patch that up
		void cursVis(const unsigned char val) // byte int
		{ curs_set(val); }
		
		// Wrapper for getch
		// If real time is enabled and no key is pressed then returns -1
		char getKey() { return getch(); }
		
		// Destroys the buffer by calling getch until it returns -1
		unsigned short killBuf() {
			char bufIn = 0; // The input character type
			unsigned short ct = 0; // How many chars that this function consumed
			while(bufIn != -1) {
				bufIn = getch();
				ct++;
			}
			return ct;
		}

		// ===========================
		// Writing to screen functions
		// ===========================
		
		// Wrapper for addch()
		void addChar(const char c) {
			addch(c);
			refresh();
		}
		
		// TODO Write color character without changing cursor color
		void addColorChar(const char c, 
				const unsigned char red, const unsigned char grn, 
				const unsigned char blu) {
			
		}

		// Wrapper for default box
		void boxDef() { box(cWindow, 0, 0); }

		// Wrapper for custom box
		void boxAt(const unsigned short x, const unsigned short y) //custom box
		{ box(cWindow, y, x); }
		
		// Wrapper for clear
		void clearWin() {
			clear();
		}
		
		// Stop real-time input so that the window waits for key input before cycling.
		bool exitRealTime() {
			nodelay(cWindow, false);
			realTime = false;
			return !realTime;
		}
		
		// Initialize window for real-time key input instead of waiting.
		bool initRealTime() {
			nodelay(cWindow, true);
			realTime = true;
			return realTime;
		}

		// Write a single character at the cursor's current position.
		void writeChar(const char c) {
			addch(c);
		}
		
		// writeAt but does not return the cursor to original position.
		// Use when the cursor is hidden
		void writeAtNR(const unsigned short x, const unsigned short y, const char c) {
			// throw error if the string position is out of window bounds
			if(x >= wd || y >= ht) 
				throw WindowError("Character out of window bounds");
			// write character to position
			move(y, x);
			writeChar(c);
			posX = x;
			posY = y;
		}

		// Write a single character at a certain position.
		void writeAt(const unsigned short x, const unsigned short y, const char c) {
			// throw error if the string position is out of window bounds
			if(x >= wd || y >= ht) 
				throw WindowError("Character out of window bounds");
			// write character to position
			move(y, x);
			writeChar(c);
			move(posY, posX); // return cursor
		}

		// Write a string starting from x, y and going right from that.
		// Will throw an error if the string is beyond the window.
		void writeAt(const unsigned short x, const unsigned short y, const string str) {
			// throw error if the string position is out of window bounds
			bool outOfBounds = (x >= wd || y >= ht);
			outOfBounds = outOfBounds || (x + str.length() > wd);
			if(outOfBounds) throw WindowError("Character out of window bounds");

			// write each letter
			for(unsigned short i=0; i<str.length(); i++) {
				move(y, x + i); // move to position to write
				writeChar(str[i]); // write a character
			}

			move(posY, posX); // return cursor to original position
		}
		
		// =========================
		// Cursor movement functions
		// =========================
		
		// Set cursor to a certain position
		void setCursor(const unsigned short x, const unsigned short y) {
			// set inherent class variables
			posX = x;
			posY = y;
			move(y, x); // actually move there now
		}

		// Move cursor by a certain amount, but not if it goes out of bounds
		// Returns false if the cursor did not move the full distance.
		bool moveCursor(const short byX, const short byY) {
			// Chaotic evil. mixed sign and unsigned arithmetic
			// Don't worry, I know exactly what I'm doing here.
			posX += byX;
			posY += byY;
			
			// whether the values are out of bounds
			// because of underflow values below 0 will
			// ALWAYS be greater than wd and height
			bool outOfBounds = (posX >= wd || posY  >= ht);

			// If out of bounds, correct the problem
			if(outOfBounds) {
				// n > 32767 really means n < 0 here because of underflow
				if(posX > 32767) posX = 0;
				if(posY > 32767) posY = 0;
				if(posX >= wd) posX = wd-1;
				if(posY >= ht) posY = ht-1;
			}
			move(posY, posX);

			return !outOfBounds;
		}

		// ===============================
		// Setter, getter, data funcs here
		// ===============================
		
		// checks
		bool inBounds(const unsigned short x, const unsigned short y) { 
			// unsigned underflow means that values
			// below 0 will wrap around and ALWAYS
			// be greater than window bounds
			return (x < wd || y < ht);
		}

		// getter functions
		unsigned short width() { return wd; }
		unsigned short height() { return ht; }
		unsigned short cursPosX() { return posX; }
		unsigned short cursPosY() { return posY; }
		bool isInstanced() { return instanced; }
		bool isRealTime() { return realTime; }

};

// DO NOT USE THIS, IT DOES NOT WORK YET
// Work in progress output stream directly to window
class WinOStream : private streambuf, public ostream {
	unsigned short x, y; // positions
	ASCIIWindow* window;
	public:
		// xPos and yPos are the coords that the buffer begins writing to screen
		WinOStream(const unsigned short xPos, const unsigned short yPos) {
			if(!set(xPos, yPos)) {
				x = 0; y = 0;
			}
		}
		
		// Sets the cursor to this position, returns false if out of bounds
		bool set(const unsigned short xPos, const unsigned short yPos) {
			if(window == NULL) return false; // if null ascii window dont do anything
			if(xPos > window -> width() // don't do anything if out of bounds
			|| yPos > window -> height()) return false;
			x = xPos; y = yPos; // set cursor, return true since successful here
			return true;
		}
		
};

// Example program to demonstrate the usage of the ASCIIWindow class.
//
// Move the cursor using the arrow keys,
// type a letter to put it on that position on the screen.
//
// Press ESC to prompt the user whether to quit.
void EX_cursorletters() {
	// 80x24 chars is default terminal size,
	// subtracting one char from each size for the border
	ASCIIWindow * window = new ASCIIWindow(79, 23);
	
	window -> build();
	window -> setCursor(0, 1);
	window -> boxDef();

	// once the window is built enter the loop
	while (1) {
		char input = window -> getKey(); // get currently typed char
		if(input == 27) { // escape key 
			window -> writeAt(0, 0, "Quit? [Y/N]"); // quit prompt
			char conf = window -> getKey(); // get currently typed char
			if(conf == 'y') break; // if y is input then end while loop
			else window -> writeAt(0, 0, "           "); // clear quit prompt
		}
		else {
			// get input for keys
			switch (input) {
				case 65: window -> moveCursor(0, -1); // up
				break;
				case 66: window -> moveCursor(0, 1); // down
				break;
				case 67: window -> moveCursor(1, 0); // left
				break;
				case 68: window -> moveCursor(-1, 0); // right
				break;
				
				default: window -> writeChar(input);
			}
			
			// cursor position
			string info = "POS: " + to_string(window -> cursPosX());
			info += " "; // write current position
			info += to_string(window -> cursPosY());
			info += "        ";
			window -> writeAt(0, 0, info);
		}
	}
	
	// once the loop is exit destroy window
	window -> close();
	delete(window);
}

