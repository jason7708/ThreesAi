#pragma once
#include <array>
#include <iostream>
#include <iomanip>
#include <cmath>
/**
 * array-based board for 2048
 *
 * index (1-d form):
 *  (0)  (1)  (2)  (3)
 *  (4)  (5)  (6)  (7)
 *  (8)  (9) (10) (11)
 * (12) (13) (14) (15)
 *
 */
class board {
public:
	typedef uint32_t cell;
	typedef std::array<cell, 4> row;
	typedef std::array<row, 4> grid;
	typedef uint64_t data;
	typedef int reward;

public:
	board() : tile(), attr(0) {}    
	board(const grid& b, data v = 0) : tile(b), attr(v) {}
	board(const board& b) = default;
	board& operator =(const board& b) = default;

	operator grid&() { return tile; }
	operator const grid&() const { return tile; }
	row& operator [](unsigned i) { return tile[i]; }
	const row& operator [](unsigned i) const { return tile[i]; }
	cell& operator ()(unsigned i) { return tile[i / 4][i % 4]; }
	const cell& operator ()(unsigned i) const { return tile[i / 4][i % 4]; }

	data info() const { return attr; }
	data info(data dat) { data old = attr; attr = dat; return old; }
public: 
	// when there is a tile >= 48, bonus tile is On : For random generation of a tile 1 ~ maxtile
	bool BonusOn() {
		board now = *this;
		for(int i=0; i<16; i++) {
			if(now(i) >= 7){
				return true;
			}
		}
		return false;
	}
	cell MaxTile() {
		board& now = *this;
		cell mx = 0;
		for(int i=0; i<16; i++) {
			if(now(i) > mx){
				mx = now(i);
			}
		}
		return mx;
	}

public:
	bool operator ==(const board& b) const { return tile == b.tile; }
	bool operator < (const board& b) const { return tile <  b.tile; }
	bool operator !=(const board& b) const { return !(*this == b); }
	bool operator > (const board& b) const { return b < *this; }
	bool operator <=(const board& b) const { return !(b < *this); }
	bool operator >=(const board& b) const { return !(*this < b); }

public:

	/**
	 * place a tile (index value) to the specific position (1-d form index)
	 * return 0 if the action is valid, or -1 if not
	 */
	reward place(unsigned pos, cell tile) {
		if (pos >= 16) return -1;
		if (tile != 1 && tile != 2 && tile != 3 && tile != 4 && tile != 5 && tile != 6 && tile != 7 && tile != 8 && tile != 9 && tile != 10 && tile != 11) return -1;   //changed!  && tile !=3
		operator()(pos) = tile;
		if(tile >= 3 && tile <= 11){
			return pow(3, tile-2);
		}
		return 0;
	}

	/**
	 * apply an action to the board
	 * return the reward of the action, or -1 if the action is illegal
	 */
	reward slide(unsigned opcode) {
		switch (opcode & 0b11) {
		case 0: return slide_up();
		case 1: return slide_right();
		case 2: return slide_down();
		case 3: return slide_left();
		default: return -1;
		}
	}
	/*
	reward slide_left() {
		board prev = *this;
		reward score = 0;
		for (int r = 0; r < 4; r++) {
			auto& row = tile[r];
			int top = 0, hold = 0;
			for (int c = 0; c < 4; c++) {
				int tile = row[c];
				if (tile == 0) continue;
				row[c] = 0;
				if (hold) {
					if (tile == hold) {
						row[top++] = ++tile;
						score += (1 << tile);
						hold = 0;
					} else {
						row[top++] = hold;
						hold = tile;
					}
				} else {
					hold = tile;
				}
			}
			if (hold) tile[r][top] = hold;
		}
		return (*this != prev) ? score : -1;
	}
	*/
	int getScore() {
		int sum = 0;
		for(int r=0; r<4; r++) {
			auto& row = tile[r];
			for(int c=0; c<4; c++) {
				if(row[c] > 2 && row[c] <= 14) {
					sum += pow(3, row[c] - 2);
				}
			}
 		}
		return sum;
	}
	reward slide_left() {
		board prev = *this;
		reward score = 0;
		//compute reward
		int score_old = getScore();
		for(int r=0; r<4; r++) {
			auto& row = tile[r];
			int left = 0, com = 0; 
			for(int c=0; c<3; c++) {
				if(row[c] == 0) {
					left = 1;
				}
				if(left != 1) {
					if((row[c] == 1 && row[c+1] == 2)||(row[c] == 2 && row[c+1] == 1)||((row[c] == row[c+1])&&(row[c] != 0)&&(row[c] != 1)&&(row[c] != 2)&&(row[c] != 14))) {
						com = 1;
						left = 1;
						if(row[c]==2&&row[c+1]==1) {
						com++;
						}
					}
				}
				if(left == 1) {
					row[c] = row[c+1] + com;
					com = 0;
					if(c==2) {
						row[c+1] = 0;
					}
				}
			}
		}
		score = getScore() - score_old;

		return (*this != prev) ? score : -1;
	}

	reward slide_right() {
		reflect_horizontal();
		reward score = slide_left();
		reflect_horizontal();
		return score;
	}
	reward slide_up() {
		rotate_right();
		reward score = slide_right();
		rotate_left();
		return score;
	}
	reward slide_down() {
		rotate_right();
		reward score = slide_left();
		rotate_left();
		return score;
	}

	void transpose() {
		for (int r = 0; r < 4; r++) {
			for (int c = r + 1; c < 4; c++) {
				std::swap(tile[r][c], tile[c][r]);
			}
		}
	}

	void reflect_horizontal() {
		for (int r = 0; r < 4; r++) {
			std::swap(tile[r][0], tile[r][3]);
			std::swap(tile[r][1], tile[r][2]);
		}
	}

	void reflect_vertical() {
		for (int c = 0; c < 4; c++) {
			std::swap(tile[0][c], tile[3][c]);
			std::swap(tile[1][c], tile[2][c]);
		}
	}

	/**
	 * rotate the board clockwise by given times
	 */
	void rotate(int r = 1) {
		switch (((r % 4) + 4) % 4) {
		default:
		case 0: break;
		case 1: rotate_right(); break;
		case 2: reverse(); break;
		case 3: rotate_left(); break;
		}
	}

	void rotate_right() { transpose(); reflect_horizontal(); } // clockwise
	void rotate_left() { transpose(); reflect_vertical(); } // counterclockwise
	void reverse() { reflect_horizontal(); reflect_vertical(); }

public:
	friend std::ostream& operator <<(std::ostream& out, const board& b) {
		out << "+------------------------+" << std::endl;
		for (auto& row : b.tile) {
			out << "|" << std::dec;
			for (auto t : row) out << std::setw(6) << ((1 << t) & -2u);
			out << "|" << std::endl;
		}
		out << "+------------------------+" << std::endl;
		return out;
	}

private:
	grid tile;
	data attr;
};
