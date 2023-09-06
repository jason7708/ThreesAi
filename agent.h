#pragma once
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <type_traits>
#include <algorithm>
#include <vector>
#include "board.h"
#include "action.h"
#include "weight.h"

class agent {
public:
	agent(const std::string& args = "") {
		std::stringstream ss("name=unknown role=unknown " + args);
		for (std::string pair; ss >> pair; ) {
			std::string key = pair.substr(0, pair.find('='));
			std::string value = pair.substr(pair.find('=') + 1);
			meta[key] = { value };
		}
	}
	virtual ~agent() {}
	virtual void open_episode(const std::string& flag = "") {}
	virtual void close_episode(const std::string& flag = "") {}
	virtual action take_action(board& b) { return action(); }
	virtual bool check_for_win(const board& b) { return false; }

public:
	virtual std::string property(const std::string& key) const { return meta.at(key); }
	virtual void notify(const std::string& msg) { meta[msg.substr(0, msg.find('='))] = { msg.substr(msg.find('=') + 1) }; }
	virtual std::string name() const { return property("name"); }
	virtual std::string role() const { return property("role"); }

protected:
	typedef std::string key;
	struct value {
		std::string value;
		operator std::string() const { return value; }
		template<typename numeric, typename = typename std::enable_if<std::is_arithmetic<numeric>::value, numeric>::type>
		operator numeric() const { return numeric(std::stod(value)); }
	};
	std::map<key, value> meta;
};

class random_agent : public agent {
public:
	random_agent(const std::string& args = "") : agent(args) {
		if (meta.find("seed") != meta.end())
			engine.seed(int(meta["seed"]));
	}
	virtual ~random_agent() {}

protected:
	std::default_random_engine engine;
};
class weight_agent : public agent {
public:
	weight_agent(const std::string& args = "") : agent(args), player_direction(0) {
		if (meta.find("init") != meta.end()) // pass init=... to initialize the weight
			init_weights(meta["init"]);
		if (meta.find("load") != meta.end()) // pass load=... to load from a specific file
			load_weights(meta["load"]);
	}
	virtual ~weight_agent() {
		if (meta.find("save") != meta.end()) // pass save=... to save to a specific file
			save_weights(meta["save"]);
	}
	weight& operator[] (size_t i) { return net[i]; }
	const weight& operator[] (size_t i) const { return net[i]; }

	int player_choose(void){
		return player_direction;
	}
	virtual action take_action(board& before) override {
		int dir = -1;
		float max_val = -1000000.f;
		for (int op : opcode) {
			board test_slide = before;
			board::reward reward = test_slide.slide(op);
			if(reward == -1){
				continue;
			}
			float value_next = predictValue(test_slide);

			if(reward + value_next >= max_val) {
				dir = op;
				max_val = reward + value_next;
			}
		}
		if (dir != -1){
			player_direction = dir;
			return action::slide(dir);
		}
		return action();
	}
	void updateWeight(board& cur, float error) {
		for(int i=0; i<4; i++) {
			for(int j=0; j<8; j++) {
				net[i][encode(cur, i, j)] += error;
			}
		}
	}
	float predictValue(board& cur) {
		float val = 0.0;
		for(int i=0; i<4; i++) {
			for(int j=0; j<8; j++) {
				val += net[i][encode(cur, i, j)];
			}
		}
		return val;
	}
	unsigned int encode(board& cur, int k, int l) {
		unsigned int encode = 0;
		for(int i=0, offset=1; i<6; i++) {
			encode += cur(six_tuple[k][l][i]) * offset;
			offset *= 15;
		}
		encode = encode * 4 + cur.info() - 1;
		return encode;
	}

protected:
	virtual void init_weights(const std::string& info) {
		net.emplace_back(11390625*4);  //15^6 * 11 (hint)
		net.emplace_back(11390625*4);  //15^6 * 11 (hint)
		net.emplace_back(11390625*4);  //15^6 * 11 (hint)
		net.emplace_back(11390625*4);  //15^6 * 11 (hint)

		for(unsigned i=0; i<net[0].size(); i++){
			net[0][i] = 0;
			net[1][i] = 0;
			net[2][i] = 0;		//6
			net[3][i] = 0;		//6
		}
	}
	virtual void load_weights(const std::string& path) {
		std::ifstream in(path, std::ios::in | std::ios::binary);
		if (!in.is_open()) std::exit(-1);
		uint32_t size;
		in.read(reinterpret_cast<char*>(&size), sizeof(size));
		net.resize(size);
		for (weight& w : net) in >> w;
		in.close();
	}
	virtual void save_weights(const std::string& path) {
		std::ofstream out(path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) std::exit(-1);
		uint32_t size = net.size();
		out.write(reinterpret_cast<char*>(&size), sizeof(size));
		for (weight& w : net) out << w;
		out.close();
	}

protected:
	std::vector<weight> net;
private:
	int player_direction;
	std::array<int, 4> opcode {0, 1, 2, 3};
	std::array<std::array<std::array<int, 6>, 8>, 4> six_tuple {
		 0,  1,  2,  3,  4,  5,
		 3,  7, 11, 15,  2,  6,
		15, 14, 13, 12, 11, 10,
		12,  8,  4,  0, 13,  9,
		 3,  2,  1,  0,  7,  6,
		 0,  4,  8, 12,  1,  5,
		12, 13, 14, 15,  8,  9,
		15, 11,  7,  3, 14, 10,

		 4,  5,  6,  7,  8,  9,
		 2,  6, 10, 14,  1,  5,
		11, 10,  9,  8,  7,  6,
		13,  9,  5,  1, 14, 10,
		 7,  6,  5,  4, 11, 10,
		 1,  5,  9, 13,  2,  6,
		 8,  9, 10, 11,  4,  5,
		14, 10,  6,  2, 13,  9,

		 0,  1,  2,  4,  5,  6,
		 3,  7, 11,  2,  6, 10,
		15, 14, 13, 11, 10,  9,
		12,  8,  4, 13,  9,  5,
		 3,  2,  1,  7,  6,  5,
		 0,  4,  8,  1,  5,  9,
		12, 13, 14,  8,  9, 10,
		15, 11,  7, 14, 10,  6,

		 4,  5,  6,  8,  9, 10,
		 2,  6, 10,  1,  5,  9,
		11, 10,  9,  7,  6,  5,
		13,  9,  5, 14, 10,  6,
		 7,  6,  5, 11, 10,  9,
		 1,  5,  9,  2,  6, 10,
		 8,  9, 10,  4,  5,  6,
		14, 10,  6, 13,  9,  5
	};
};

/**
 * base agent for agents with a learning rate
 */
class learning_agent : public agent {
public:
	learning_agent(const std::string& args = "") : agent(args), alpha(0.1f) {
		if (meta.find("alpha") != meta.end())
			alpha = float(meta["alpha"]);
	}
	virtual ~learning_agent() {}
	float get_alpha(){
		return alpha;
	}
	void update_alpha() {
		alpha *= 0.5;
	}
protected:
	float alpha;
};
/**
 * random environment
 * add a new random tile to an empty cell
 * 2-tile: 90%
 * 4-tile: 10%
 */
/*
class rndenv : public random_agent {
public:
	rndenv(const std::string& args = "") : random_agent("name=random role=environment " + args),
		space({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }), popup(0, 9) {}

	virtual action take_action(const board& after) {
		std::shuffle(space.begin(), space.end(), engine);
		for (int pos : space) {
			if (after(pos) != 0) continue;
			board::cell tile = popup(engine) ? 1 : 2;
			return action::place(pos, tile);
		}
		return action();
	}

private:
	std::array<int, 16> space;
	std::uniform_int_distribution<int> popup;
};
*/

class rndenv : public random_agent {
public:
	int slide_direction;
	rndenv(const std::string& args = "") : random_agent("name=random role=environment " + args), bonus_p(1, 21), bonus_cnt(0), total_tile(0) {
		tile_bag.clear();
		slide_direction = 5;
	}
	void get_user_direction(const int user_move) {
		slide_direction = user_move;
	}
	void initial(void) {
		slide_direction = 5;
		total_tile = 0;
		tile_bag.clear();
	}
	virtual action take_action(board& after) {
		bool is_bonus = 0;
		board::cell tile = 0;

		if(tile_bag.empty()) {
			tile_bag = {1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3};
		}

		std::shuffle(tile_bag.begin(), tile_bag.end(), engine);

		// position space accroding from direction, 5 : initial 9 tile
		switch (slide_direction) {
			case 5:
				initial_space = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
				std::shuffle(initial_space.begin(), initial_space.end(), engine);
				for(int pos : initial_space){
					if(after(pos) != 0) continue;
					tile = tile_bag.back();
					tile_bag.pop_back();
					total_tile++;
					if(total_tile == 9) { //last initial tile
						//  change the hint
						int hint = tile_bag.back();
						tile_bag.pop_back();
						after.info(hint);
					}
					return action::place(pos, tile);
				}
				break;
			case 0: 
				space = {12, 13, 14, 15};   //up
				break;
			case 1: 
				space = {0, 4, 8, 12};   //right
				break;
			case 2: 
				space = {0, 1, 2, 3};   //down
				break;
			case 3: 
				space = {3, 7, 11, 15};   //left
				break;
			default:
				break;
		}
		// decide bonus or not
		if((static_cast<double>(bonus_cnt + 1) / total_tile <= 1.0 / 21) && after.BonusOn()){
			if(bonus_p(engine) == 21){
				tile = 4;
				is_bonus = 1;
			}
		}
		std::shuffle(space.begin(), space.end(), engine);
		
		for(int pos : space){
			if (after(pos) != 0) continue;
			
			if(!is_bonus){
				tile = tile_bag.back();
				tile_bag.pop_back();
				total_tile++;
			}
			total_tile++;
			bonus_cnt += is_bonus;
			tile = after.info(tile); // get cur from previous hint and update hint
			if(tile == 4){
				std::uniform_int_distribution<int> bonus_num(4, after.MaxTile()-3);
				tile = bonus_num(engine);
			}
			
			return action::place(pos, tile);
		}
		return action();
	}
private:
	std::vector<int> tile_bag;
	std::array<int, 4> space;
	std::array<int, 16> initial_space;
	std::uniform_int_distribution<int> bonus_p;
	int bonus_cnt;
	int total_tile;
};
// -----
/**
 * dummy player
 * select a legal action randomly
 */
class player : public random_agent {
public:
	player(const std::string& args = "") : random_agent("name=dummy role=player " + args),
		opcode({ 0, 1, 2, 3 }) {}

	virtual action take_action(const board& before) {
		std::shuffle(opcode.begin(), opcode.end(), engine);
		for (int op : opcode) {
			board::reward reward = board(before).slide(op);
			if (reward != -1) return action::slide(op);
		}
		return action();
	}

private:
	std::array<int, 4> opcode;
};
