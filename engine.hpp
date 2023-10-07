#include <iostream>
#include <string>
#include <vector>
#include "ascii.hpp"
using namespace std;
// Version of this program
#define ASCIIDISPLAY_VERSION "ALPHA_0.0";

// =========================================
// Display Engine written by Nathaniel Kang
// ----------------------------------------
// Created for the ease of management of
// on-screen geometry for ASCII window-based
// programs such as menu screens or games.
// 
// A character structure (CharStruct) is a
// data structure meant to hold a shape of
// a sort formed by characters on the screen.
//
// This display engine was made mostly for
// ASCII games in mind, since each character
// structure has a collision code intended for
// coordinate interaction with other objects.
// =========================================

// A color that stores the RGB values as a char each.
class CharColorB {
	unsigned char r, g, b;
	public:
		CharColorB(const unsigned char red, 
		const unsigned char grn, const unsigned char blu) {
			r = red;
			g = grn;
			b = blu;
		}
};

// A color that stores the RGB values as a short each.
class CharColor2B {
	unsigned short r, g, b;
	public:
		CharColor2B(const unsigned short red,
		const unsigned short grn, const unsigned short blu) {
			r = red;
			g = grn;
			b = blu;
		}
};

// A data structure that contains information on
// a pattern on which to put on screen.
class CharStruct {
	protected:
		// collision code to determine certain things
		// such as whether the tile is passable, or
		// is at a certain elevation, etc.
		// This is intended to be used with hexadecimal digits.
		unsigned int collCode;
		unsigned short xp, yp;
	public:
		CharStruct() {
			collCode = 0;
			xp = 0; yp = 0;
		}

		CharStruct(const unsigned int collision, 
		const unsigned short xPos, const unsigned short yPos) {
			collCode = collision;
			xp = xPos; yp = yPos;
		}

		virtual ~CharStruct() {};
		// ========
		// Virtuals
		// ========

		// draw the structure on the ASCIIWindow screen 
		virtual void draw(ASCIIWindow &win, const unsigned short xo, const unsigned short yo) {}
		// write the structure to a char array
		virtual void write(unsigned char** chars, 
		const unsigned short xo, const unsigned short yo,
		const unsigned short xMax, const unsigned short yMax) {}
		// what character is at a certain position, or ' ' if there is none
		virtual unsigned char charAt(const unsigned short x, const unsigned short y) 
		{ return 0; } // null default
		// whether a given coordinate intersects with the structure's collision boundaries
		virtual bool inColl(const unsigned short x, const unsigned short y) { return false; }
		// This should be the exact name of the class
		virtual const string type() { return "CharStruct"; } 

		// ===================
		// Getters and setters
		// ===================
		
		// collision code
		void setCollisionCode(const unsigned int code)
		{ collCode =  code; }
		const unsigned int collisionCode() { return collCode; }
		
		// set position, returns false if no change
		bool setX(const unsigned short x) {
			if(x == xp) return false;
			xp = x; return true; 
		}
		bool setY(const unsigned short y) { 
			if(y == yp) return false;
			yp = y; return true;
		}
		// x and y positions
		const unsigned short posX() { return xp; }
		const unsigned short posY() { return yp; }
};

// A single tile and the most simple CharStruct.
// Useful for single tile obstructions, doorways, events, etc.
class CollChar : public CharStruct {
	protected:
		unsigned char chr;
	public:
		CollChar() : CharStruct() {
			chr = '0';
		}

		CollChar(const unsigned char character) : CharStruct() {
			chr = character;
		}

		CollChar(const unsigned int collision, const unsigned char character) 
		: CharStruct(collision, 0, 0) {
			chr = character;
		}

		CollChar(const unsigned int collision, const unsigned char character,
		const unsigned short xPos, const unsigned short yPos)
		: CharStruct(collision, xPos, yPos) {
			chr = character;
		}

		// simple, draws a single character on the screen if in bounds
		void draw(ASCIIWindow &win,
		const unsigned short xo, const unsigned short yo) override {
			const unsigned short
				x = (xp + xo),
				y = (yp + yo);
			if(win.inBounds(x, y))
				win.writeAtNR(x, y, chr);
		}

		void write(unsigned char** chars, const unsigned short xo, const unsigned short yo,
		const unsigned short xMax, const unsigned short yMax) override {
			const unsigned short
				x = (xp + xo),
				y = (yp + yo);
			if(x < xMax && y < yMax)
				chars[x][y] = chr;
		}

		unsigned char charAt(const unsigned short x, const unsigned short y) override {
			if(inColl(x, y)) return chr;
			else return 0; // null char if none
		}
		
		// also simple, check if the coords are equal
		bool inColl(const unsigned short x, const unsigned short y) override {
			return (x == xp && y == yp);
		}

		const string type() override { return "CollChar"; }

		// operator overload

		friend bool operator==(const CollChar f, const CollChar l) {
			return ((f.chr == l.chr) && (f.xp == l.xp) && (f.yp == l.yp));
		}
		
		// Get and set
		const unsigned char getChar() { return chr; }
		void setChar(const unsigned char ch) { chr = ch; }

};

// A line of chars, such as for a wall or ceiling.
class Line : public CharStruct {
	protected:
		// character to draw the line out of.
		// this actually is an ASCII char and not a byte int
		unsigned char chr;
		unsigned short len;
		bool vert; // if false then horizontal
	public:
		Line(const short length) : CharStruct() {
			chr = '0';
			len = length;
			vert = false;
		}

		Line(const unsigned char character, const short length) : CharStruct() {
			chr = character;
			len = length;
			vert = false;
		}

		Line(const unsigned char character, const short length, const bool vertical)
		: CharStruct() {
			chr = character;
			len = length;
			vert = vertical;
		}

		Line(const unsigned int collision, const unsigned char character, const short length, 
		const unsigned short xPos, const unsigned short yPos, const bool vertical)
		: CharStruct(collision, xPos, yPos) {
			chr = character;
			len = length;
			vert = vertical;
		}

		// overrides for CharStruct funcs

		void draw(ASCIIWindow &win,
		const unsigned short xo, const unsigned short yo) override {
			unsigned short
				x = (xo + xp),
				y = (yo + yp);
			// always increment the pos but only write to it
			// if it is within window bounds
			for(unsigned short i=0; i<len; i++) {
				if(win.inBounds(x, y))
					win.writeAtNR(x, y, chr);
				vert ? y++ : x++;
			}
		}

		void write(unsigned char** chars, const unsigned short xo, const unsigned short yo,
		const unsigned short xMax, const unsigned short yMax) override {
			unsigned short
				x = (xo + xp),
				y = (yo + yp);
			// always increment the pos but only write to it
			// if it is within window bounds
			for(unsigned short i=0; i<len; i++) {
				if(x < xMax && y < yMax)
					chars[x][y] = chr;
				vert ? y++ : x++;
			}
		}

		unsigned char charAt(const unsigned short x, const unsigned short y) override {
			if(inColl(x,y)) return chr;
			else return 0;
		}

		bool inColl(const unsigned short x, const unsigned short y) override {
			// i:input, s:self, Pa:parallel, Pp:perpendicular
			unsigned short iPa, sPa, iPp, sPp;
			iPa = vert ? x : y;
			sPa = vert ? xp : yp;
			iPp = vert ? y : x;
			sPp = vert ? yp : xp;
			if(iPa == sPa) // if input coord is along line's plane
				return ((iPp >= sPp) && // greater than left side
				(iPp <= sPp + len-1)); // less than right side
			return false;
		}

		const string type() override { return "Line"; }
		
		// getter methods
		const unsigned short length() { return len; }
		const bool vertical() { return vert; }
		const bool horizontal() { return !vert; }

		// operator overload

		friend bool operator==(const Line& f, const Line& l) {
			return ((f.chr == l.chr) && (f.len == l.len) &&
				(f.xp == l.xp) && (f.yp == l.yp) &&
				(f.vert == l.vert));
		}
};

// A box of a single character.
class Box : public CharStruct {
	// fill - if true then the rect draws all chars inside,
	// otherwise it will only draw inside of the borders
	// collIn - whether the collision checks the inside,
	// if false then it only collides with the borders
	bool fill, collIn;
	unsigned char chr;
	unsigned short wd, ht;
	public:
		Box(const unsigned int collision, const unsigned char charac, const bool filled)
		: Box(collision, '0', 0, 0, filled, filled, 8, 8) { }

		Box(const unsigned int collision, const unsigned short xPos, const unsigned short yPos,
		const unsigned char charac, const bool filled)
		: Box(collision, '0', xPos, yPos, filled, filled, 8, 8) { }

		Box(const unsigned int collision, const unsigned char charac,
		const bool filled, const bool collideInside) 
		: Box(collision, '0', 0, 0, filled, collideInside, 8, 8) { }
		
		// if width and height not specified then it will be 8x8 chars in size
		// if charac not specified then it will default to '0'
		Box(const unsigned int collision, const unsigned short xPos, const unsigned short yPos,
		const unsigned char charac, const bool filled, const bool collideInside) 
		: Box(collision, '0', xPos, yPos, filled, collideInside, 8, 8) { }

		Box(const unsigned int collision, const unsigned char charac, const bool filled, 
		const unsigned short width, const unsigned short height)
		: Box(collision, charac, 0, 0, filled, filled, width, height) {}
		
		// if collideInside is not specified then it will be the same as whether its filled
		Box(const unsigned int collision, const unsigned char charac,
		const unsigned short xPos, const unsigned short yPos,
		const bool filled, const unsigned short width, const unsigned short height) 
		: Box(collision, charac, xPos, yPos, filled, filled, width, height) {}
		
		// if coordinates are not specified then it goes to 0, 0
		Box(const unsigned int collision, const unsigned char charac,
		const bool filled, const bool collideInside,
		const unsigned short width, const unsigned short height) 
		: Box(collision, charac, 0, 0, filled, collideInside, width, height) {}
		
		// most complex constructor
		Box(const unsigned int collision, const unsigned char charac,
		const unsigned short xPos, const unsigned short yPos,
		const bool filled, const bool collideInside,
		const unsigned short width, const unsigned short height) 
		: CharStruct(collision, xPos, yPos) {
			chr = charac;
			fill = filled;
			collIn = collideInside;
			wd = width;
			ht = height;
		}

		virtual ~Box() {}

		// overrides for CharStruct funcs

		void draw(ASCIIWindow &win,
		const unsigned short xo, const unsigned short yo) override {
			const unsigned short
				x = (xp + xo),
				y = (yp + yo);
			if(fill) {// O(xy) if filled
				for(unsigned short i=x; i<x+wd; i++)
					for(unsigned short j=y; j<y+ht; j++)
						if(win.inBounds(i, j))
							win.writeAtNR(i, j, chr);
			} else { // O(x+y) otherwise
				// draw the horizontal edges
				for(unsigned short i=x; i<x+wd; i++) {
					if(win.inBounds(i, y)) 
						win.writeAtNR(i, y, chr);
					if(win.inBounds(i, y+ht-1))
						win.writeAtNR(i, y+ht-1, chr);
				}
				// draw the vertical edges
				for(unsigned short j=y; j<y+ht; j++) {
					if(win.inBounds(x, j))
						win.writeAtNR(x, j, chr);
					if(win.inBounds(x+wd-1, j))
						win.writeAtNR(x+wd-1, j, chr);
				}
			}
		}

		void write(unsigned char** chars, const unsigned short xo, const unsigned short yo,
		const unsigned short xMax, const unsigned short yMax) override {
			const unsigned short
				x = (xp + xo),
				y = (yp + yo);
			if(fill) {// O(xy) if filled
				for(unsigned short i=x; i<x+wd; i++)
					for(unsigned short j=y; j<y+ht; j++)
						if(i < xMax && j < yMax)
							chars[i][j] = chr;
			} else { // O(x+y) otherwise
				// draw the horizontal edges
				for(unsigned short i=x; i<x+wd; i++) {
					if(i < xMax && y < yMax) 
						chars[i][y] = chr;
					if(i < xMax && y+ht-1 < yMax)
						chars[i][y+ht-1] = chr;
				}
				// draw the vertical edges
				for(unsigned short j=y; j<y+ht; j++) {
					if(x < xMax && j < yMax)
						chars[x][j] = chr;
					if(x+wd-1 < xMax && j < yMax)
						chars[x+wd-1][j] = chr;
				}
			}
		}
		
		// Collision bounds may be inside the box even if the chars are not.
		// This is determined by collIn and may be used to see if something is
		// inside, for example, an area of effect.
		bool inColl(const unsigned short x, const unsigned short y) override {
			bool inBox = (x >= xp && x < (xp + wd))	
			&& (y >= yp && y < (yp + ht));
			if(collIn) return inBox;
			bool onBorder = (x == xp || y == yp) || 
			(x == (xp + wd - 1) || y == (yp + wd - 1));
			return onBorder;
		}
	
		// Not an override, but placed it here because it has a similar function
		// Checks of a coord is within the actual chars and not just the collision bounds
		bool inChars(const unsigned short x, const unsigned short y) {
			bool inBox = (x >= xp && x < (xp + wd))	
			&& (y >= yp && y < (yp + ht));
			if(fill) return inBox;
			bool onBorder = (x == xp || y == yp) || 
			(x == (xp + wd - 1) || y == (yp + wd - 1));
			return onBorder;
		}

		const string type() override { return "Box"; }
		// operator overloads

		friend bool operator==(const Box& f, const Box& l) {
			return ((f.chr == l.chr) && (f.wd == l.wd) && (f.ht == l.ht) &&
				(f.xp == l.xp) && (f.yp == l.yp) &&
				(f.fill == l.fill) && (f.collIn == l.collIn));
		}
};

// A grid of a certain size where each individual character
// is stored seperately. Use for more detailed patterns
// of which it is inefficient to use many different charstructs.
class StoredGrid : public CharStruct {
	char** chrs;
	bool** coll;
	public:
		const string type() override { return "StoredGrid"; }
};

// A group of char structs, used when building rooms or levels.
// Suggested use is to use them as layers.
// Do not add structs of a different collision code or it will not work as expected.
class CharStructGroup : public CharStruct {
	vector<CharStruct *> structs;
	public:
		
		~CharStructGroup() {
			for(unsigned short i=0; i<structs.size(); i++)
				delete(structs[i]);
		}
		// Override methods

		// Draw every structure in the group
		void draw(ASCIIWindow &win,
		const unsigned short xo, const unsigned short yo) override {
			for(unsigned short i=0; i<structs.size(); i++)
				structs[i] -> draw(win, xo, yo);
		}

		// Draw every structure in the group
		void write(unsigned char** chars, const unsigned short xo, const unsigned short yo,
		const unsigned short xMax, const unsigned short yMax) override {
			for(unsigned short i=0; i<structs.size(); i++)
				structs[i] -> write(chars, xo, yo, xMax, yMax);
		}

		// Check every structure in the group
		// Return true on the first struct it collides with
		bool inColl(const unsigned short x, const unsigned short y) override {
			for(unsigned short i=0; i<structs.size(); i++)
				if(structs[i] -> inColl(x, y)) return true;
			return false;
		}
		
		// Returns the first visible char at the coordinates or 0 if none exist
		unsigned char charAt(const unsigned short x, const unsigned short y) override {
			for(unsigned short i=0; i<structs.size(); i++) {
				unsigned char chr = structs[i] -> charAt(x, y);
				if(chr != 0) return chr;
			} return 0;
		}

		const string type() override { return "CharStructGroup"; }

		// Unique methods

		void add(CharStruct * structure) {
			structs.push_back(structure);
		}

		bool remove(const unsigned short index) {
			if(index > structs.size()) return false;
			structs.erase(structs.begin() + index);
			return true;
		}

		// I'll figure this out later
		//bool remove(const CharStruct structure) {
		//	for(unsigned short i=0; i<structs.size(); i++) {
		//		if(structure == structs[i]) {
		//			structs.erase(structs.begin() + i);
		//			return true;
		//		}
		//	} return false;
		//}

		unsigned short size() {
			return structs.size();
		}
};

// The class that deals with writing the character structures to the screen
class CharDisplay {	
	// w, h: width and height of the displayed screen.
	// xo, yo: x and y offsets for the display to be displaced on the ASCIIWindow
	// xs, ys: x and y scroll offsets 
	unsigned short w, h, xo, yo, xs, ys;
	bool up; // whether the screen is updated or not
	unsigned char** winChars; // has to be dynamically allocated

	// These CharStruct vectors are read from front to back meaning
	// a size 4 loadedStructs will load [0] first and [3] last,
	// where 0 will appear on the bottom and 3 will be on the top visually.
	vector<CharStruct *> structs; // Structures stored in the screen
	ASCIIWindow *win;

	protected:
		void initChars(const unsigned short width, const unsigned short height) {
			// allocate space for window characters
			cout << width << " " << height;
			winChars = new unsigned char*[width]; // create columns
			for(unsigned short i=0; i<width; i++) // create rows
				winChars[i] = new unsigned char[height];
			for(unsigned short x = 0; x<width; x++) {
				for(unsigned short y = 0; y<height; y++) {
					winChars[x][y] = ' '; // default chars are spaces
				}
			}
		}

	public:
		CharDisplay(const unsigned short width, const unsigned short height, 
			const unsigned short xOffset, const unsigned short yOffset,
			ASCIIWindow *window) {
			cout << width << " " << height;
			w = width;
			h = height;
			xo = xOffset;
			yo = yOffset;
			xs = 0; ys = 0;
			win = window;
			up = true;
			initChars(width, height);
		}

		CharDisplay(const unsigned short width, const unsigned short height,
			ASCIIWindow *window) : CharDisplay(width, height, 0, 0, window) {}

		~CharDisplay() {
			for(unsigned int i=0; i<structs.size(); i++)
				delete(structs[i]);
			// TODO test memory leak with winchars
			for(unsigned short i=0; i<w; i++)
				delete[] winChars[i];
			delete[] winChars;
		}
		
		// Redraws the screen if update is false
		void update() {
			if(up == false) {
				for(unsigned short i=0; i<w; i++) {
					for(unsigned short j=0; j<h; j++) {
						win -> writeAtNR(i, j, winChars[i][j]);			
					}
				}
			up = true;
			}
		}
		
		// ===============
		// Window settings
		// ===============
		
		// true makes real time input, false waits for keyboard input
		// gameplay typically uses real time, a menu typically waits for key input
		bool setRealTime(const bool rt) {
			bool done = rt ? win -> initRealTime() : win -> exitRealTime();
			return done;
		}
		
		// ==============================
		// Data coordinate retrieve funcs
		// ==============================

		// Whether a given coordinate has a struct with a given collcode intersecting it
		bool hasCollCode(const unsigned short x, const unsigned short y,
		const unsigned int code) {
			for(unsigned short i=structs.size() - 1; i < 65535; i--)
				if(structs[i] -> collisionCode() == code
				&& structs[i] -> inColl(x, y))
					return true;
			return false;
		}
		
		// ===================
		// Data changing funcs
		// Use clear() or writeStructs() to redraw or update() wont do anything
		// ====================================================================

		// add a struct pointer to the vector
		void addStruct(CharStruct* ptr) {
			structs.push_back(ptr);
		}
		
		// remove a struct pointer from the vector
		// return false if the index is out of bounds
		bool removeStruct(const unsigned short index) {
			if(index > structs.size()) return false;
			delete(structs[index]); // delete the object at the pointer
			structs.erase(structs.begin() + index); // remove the pointer
			return true;
		}

		// Remove a specifically called pointer
		// Returns false if the pointer was not found
		bool removeStruct(const CharStruct* ptr) {
			for(unsigned short i=0; i<structs.size(); i++) {
				if(ptr == structs[i]) {
					delete(structs[i]); // delete the object at the pointer
					structs.erase(structs.begin() + i); // remove the pointer
				}
			}
			return false;
		}

		// Gets a pointer at an index, or NULL if index is out of bounds.
		// Do NOT delete the pointer directly, use removeStruct instead
		CharStruct * getPtr(const unsigned short index) {
			if(index > structs.size()) return NULL;
			return structs[index]; // get the pointer
		}

		// Removes a struct from the list, but does NOT delete the pointer.
		// Returns the pointer instead. If index is out of bounds returns null
		// MEMORY MANAGEMENT IS UP TO YOU WHEN YOU USE THIS
		CharStruct * popStruct(const unsigned int index) {
			if(index > structs.size()) return NULL;
			CharStruct* ptr = structs[index]; // get the pointer
			structs.erase(structs.begin() + index); // remove the pointer
			return ptr;
		}
		
		// ========================================================
		// draw funcs, call update() after doing any of these below
		// ========================================================

		// Writes the structs on top of whatever is already on it
		void writeStructs() {
			// you'd better HOPE that a short stores 2 bytes on your pc
			for(unsigned short i=structs.size() - 1; i < 65535; i--) 
				structs[i] -> write(winChars, dx(), dy(), dlx(), dly());
			up = false;
		}

		// Writes a single struct on top of everything else 
		// Return false if out of bounds
		bool writeStruct(const unsigned short index) {
			if(index > structs.size()) return false;
			structs[index] -> write(winChars, dx(), dy(), dlx(), dly());
			up = false;
			return true;
		}
		
		// Writes a single struct, specified by the pointer on top of everything else
		// Return false if the specified pointer is not in the vector
		bool writeStruct(CharStruct* ptr) {
			for(unsigned short i=0; i<structs.size(); i++) {
				if(ptr == structs[i]) {
					// draw the structs
					structs[i] -> write(winChars, dx(), dy(), dlx(), dly());
					up = false; // not updated if this is true
					return true;
				}
			}
			return false;
		}
		
		// Wipes the char 2d array, leaving a blank screen when refreshed
		void clear() {
			for(unsigned short i=0; i<w; i++) {
				for(unsigned short j=0; j<h; j++) {
					winChars[i][j] = ' ';			
				}
			}
			up = false;
		}

		// clears, then rewrites structs
		void redrawStructs() {
			clear();
			writeStructs();
		}
		

		// =======================
		// Getter and setter funcs
		// =======================
		
		// whether the display is updated or not
		const bool updated() { return up; }
		// number of character structures stored by the display
		const unsigned short structCt() { return structs.size(); }
		// character at certain coordinate
		const unsigned char charAt(const unsigned short x, const unsigned short y) 
		{ return winChars[x][y]; }
		// the x and y coordinates to feed to writeStructs as the max limits
		// this is the width/height plus the display offset coordinate
		const unsigned short dlx() { return w + xo; }
		const unsigned short dly() { return h + yo; }
		// the x and y coordinates to feed to writeStructs as the offsets
		// this is the scroll offset plus the display screen offset
		const unsigned short dx() { return xs + xo; }
		const unsigned short dy() { return ys + yo; }
		//  the width and the length of the display
		const unsigned short width() { return w; }
		const unsigned short height() { return h; }

		// scroll the display in the x or y direction
		// this does not visually change anything until the structs are redrawn
		void scrollX(const short chrCt) { xs += chrCt; }
		void scrollY(const short chrCt) { ys += chrCt; }
	protected:
		class GuiWindow {
			// width, height, x position of topleft corner, y position of such
			unsigned short wd, ht, pX, pY;
			bool opened;
			public:
				GuiWindow(const unsigned short width, const unsigned short height) {
					wd = width;
					ht = height;
				}
		
				~GuiWindow() {
					
				}

				// getter funcs
				const unsigned short width() { return wd; }
				const unsigned short height() { return ht; }
				const bool open() { return opened; }
		};
};
