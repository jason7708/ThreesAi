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
class weight_agent;
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
	virtual action take_action(board& b, const weight_agent& weight_table) { return action(); }
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
	weight_agent(const std::string& args = "") : agent(args) {
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

protected:
	virtual void init_weights(const std::string& info) {
		//net.emplace_back(65536); // create an empty weight table with size 65536
		//net.emplace_back(65536); // create an empty weight table with size 65536
		// now net.size() == 2; net[0].size() == 65536; net[1].size() == 65536
		//RRRRRRRRRRRRRRRRRRRRRR
		/*
		net.emplace_back(50625); //15^4
		net.emplace_back(50625); //15^4
		*/

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
		//RRRRRRRRRRRRRRRRRRRRRR
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
// RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
class rndenv : public random_agent {
public:
	int slide_direction;
	rndenv(const std::string& args = "") : random_agent("name=random role=environment " + args), bonus_p(1, 21), bonus(0), total_tile(0) {
		tile_bag.clear();
		slide_direction=5;
	}
	void get_user_direction(const int user_move) {
		slide_direction = user_move;
	}
	void clear_bag(void) {
		tile_bag.clear();
	}
	void initial(void) {
		slide_direction=5;
	}
	virtual action take_action(board& after, const weight_agent& weight_table){
		bool is_bonus = 0;
		board::cell tile = 0;
		
		if(tile_bag.empty()){
			tile_bag.push_back (1);
			tile_bag.push_back (1);
			tile_bag.push_back (1);
			tile_bag.push_back (1);
			tile_bag.push_back (2);
			tile_bag.push_back (2);
			tile_bag.push_back (2);
			tile_bag.push_back (2);
			tile_bag.push_back (3);
			tile_bag.push_back (3);
			tile_bag.push_back (3);
			tile_bag.push_back (3);
		}
		if(((float)(bonus+1)/total_tile <= 1.0/21 )&& after.BonusOn()){
			if(bonus_p(engine) == 21){
				//std::uniform_int_distribution<int> bonus_num(4, after.MaxTile()-3);
				//tile = bonus_num(engine);
				tile = 4;
				is_bonus = 1;
			}
		}

		std::shuffle(tile_bag.begin(), tile_bag.end(), engine);
		switch (slide_direction) {
			case 5:
				initial_space={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
				std::shuffle(initial_space.begin(), initial_space.end(),engine);
				for(int pos : initial_space){
					if(after(pos) != 0) continue;
					if(after.InitNine() == false){
						tile = tile_bag.back();
						tile_bag.pop_back();
						total_tile++;

						return action::place(pos, tile);
					}
					else{  //last initial tile
						tile = tile_bag.back();
						tile_bag.pop_back();
						total_tile++;
						
						//  change the hint
						int hint = tile_bag.back();
						tile_bag.pop_back();
						after.info(hint);
						//std::cout << "////" << hint << "////" << "\n";

						return action::place(pos, tile);
					}
					
				}
				break;
			case 0: 
				space={12, 13, 14, 15};   //up
				break;
			case 1: 
				space={0, 4, 8, 12};   //right
				break;
			case 2: 
				space={0, 1, 2, 3};   //down
				break;
			case 3: 
				space={3, 7, 11, 15};   //left
				break;
			default:
				break;
		}
		//std::cout << "87878787\n";
		std::shuffle(space.begin(), space.end(), engine);
		//std::cout << "evil_dir:"<<slide_direction<<'\n';
		
		for(int pos : space){
			if (after(pos) != 0) continue;
			
			if(!is_bonus){
				tile = tile_bag.back();
				tile_bag.pop_back();
				total_tile++;
				//std::cout << tile << "\n";
				tile = after.info(tile);
				//std::cout << tile << "\n";
				if(tile == 4){
					std::uniform_int_distribution<int> bonus_num(4, after.MaxTile()-3);
					tile = bonus_num(engine);
				}
			}
			else{
				bonus++;
				total_tile++;
				is_bonus = 0;
				tile = after.info(tile);
				if(tile == 4){
					std::uniform_int_distribution<int> bonus_num(4, after.MaxTile()-3);
					tile = bonus_num(engine);
				}
			}
			//std::cout << "tile:" << tile << '\n';
			//std::cout << "pos:"<<pos<<'\n';
			return action::place(pos, tile);
		}
		return action();
	}
private:
	std::vector<int> tile_bag;
	std::array<int, 4> space;
	std::array<int, 16> initial_space;
	std::uniform_int_distribution<int> bonus_p;
	int bonus;
	int total_tile;
};
// RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
/**
 * dummy player
 * select a legal action randomly
 */
class player : public random_agent {
public:
	player(const std::string& args = "") : random_agent("name=dummy role=player " + args),
		opcode({ 0, 1, 2, 3 }),/*RRRR*/ player_direction(0)/*RRRR*/ {}

	int player_choose(void){
		return player_direction;
	}	
	virtual action take_action(board& before, const weight_agent& weight_table) {
		
		int go = -1;
		float max = -1000000.f;
		int lose = -1;
		board a;

		for (int op : opcode) {
			a = before;
			board::reward reward = a.slide(op);
			if(reward == -1){
				continue;
			}
			if(reward != -1){
				lose = 0;
			}
			float value_next = 0;
			/* 4 tuple
			value_next += weight_table[0][a(0)+a(1)*15+a(2)*15*15+a(3)*15*15*15]; //a's value
			value_next += weight_table[1][a(4)+a(5)*15+a(6)*15*15+a(7)*15*15*15];
			value_next += weight_table[1][a(8)+a(9)*15+a(10)*15*15+a(11)*15*15*15];
			value_next += weight_table[0][a(12)+a(13)*15+a(14)*15*15+a(15)*15*15*15];

			value_next += weight_table[0][a(0)+a(4)*15+a(8)*15*15+a(12)*15*15*15];
			value_next += weight_table[1][a(1)+a(5)*15+a(9)*15*15+a(13)*15*15*15];
			value_next += weight_table[1][a(2)+a(6)*15+a(10)*15*15+a(14)*15*15*15];
			value_next += weight_table[0][a(3)+a(7)*15+a(11)*15*15+a(15)*15*15*15];
			*/
			//6 tuple
			value_next += weight_table[0][(a(0)+a(1)*15+a(2)*15*15+a(3)*15*15*15+a(4)*15*15*15*15+a(5)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[0][(a(3)+a(7)*15+a(11)*15*15+a(15)*15*15*15+a(2)*15*15*15*15+a(6)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[0][(a(15)+a(14)*15+a(13)*15*15+a(12)*15*15*15+a(11)*15*15*15*15+a(10)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[0][(a(12)+a(8)*15+a(4)*15*15+a(0)*15*15*15+a(13)*15*15*15*15+a(9)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[0][(a(3)+a(2)*15+a(1)*15*15+a(0)*15*15*15+a(7)*15*15*15*15+a(6)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[0][(a(0)+a(4)*15+a(8)*15*15+a(12)*15*15*15+a(1)*15*15*15*15+a(5)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[0][(a(12)+a(13)*15+a(14)*15*15+a(15)*15*15*15+a(8)*15*15*15*15+a(9)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[0][(a(15)+a(11)*15+a(7)*15*15+a(3)*15*15*15+a(14)*15*15*15*15+a(10)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[1][(a(4)+a(5)*15+a(6)*15*15+a(7)*15*15*15+a(8)*15*15*15*15+a(9)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[1][(a(2)+a(6)*15+a(10)*15*15+a(14)*15*15*15+a(1)*15*15*15*15+a(5)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[1][(a(11)+a(10)*15+a(9)*15*15+a(8)*15*15*15+a(7)*15*15*15*15+a(6)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[1][(a(13)+a(9)*15+a(5)*15*15+a(1)*15*15*15+a(14)*15*15*15*15+a(10)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[1][(a(7)+a(6)*15+a(5)*15*15+a(4)*15*15*15+a(11)*15*15*15*15+a(10)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[1][(a(1)+a(5)*15+a(9)*15*15+a(13)*15*15*15+a(2)*15*15*15*15+a(6)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[1][(a(8)+a(9)*15+a(10)*15*15+a(11)*15*15*15+a(4)*15*15*15*15+a(5)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[1][(a(14)+a(10)*15+a(6)*15*15+a(2)*15*15*15+a(13)*15*15*15*15+a(9)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[2][(a(0)+a(1)*15+a(2)*15*15+a(4)*15*15*15+a(5)*15*15*15*15+a(6)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[2][(a(3)+a(7)*15+a(11)*15*15+a(2)*15*15*15+a(6)*15*15*15*15+a(10)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[2][(a(15)+a(14)*15+a(13)*15*15+a(11)*15*15*15+a(10)*15*15*15*15+a(9)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[2][(a(12)+a(8)*15+a(4)*15*15+a(13)*15*15*15+a(9)*15*15*15*15+a(5)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[2][(a(3)+a(2)*15+a(1)*15*15+a(7)*15*15*15+a(6)*15*15*15*15+a(5)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[2][(a(0)+a(4)*15+a(8)*15*15+a(1)*15*15*15+a(5)*15*15*15*15+a(9)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[2][(a(12)+a(13)*15+a(14)*15*15+a(8)*15*15*15+a(9)*15*15*15*15+a(10)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[2][(a(15)+a(11)*15+a(7)*15*15+a(14)*15*15*15+a(10)*15*15*15*15+a(6)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[3][(a(4)+a(5)*15+a(6)*15*15+a(8)*15*15*15+a(9)*15*15*15*15+a(10)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[3][(a(2)+a(6)*15+a(10)*15*15+a(1)*15*15*15+a(5)*15*15*15*15+a(9)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[3][(a(11)+a(10)*15+a(9)*15*15+a(7)*15*15*15+a(6)*15*15*15*15+a(5)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[3][(a(13)+a(9)*15+a(5)*15*15+a(14)*15*15*15+a(10)*15*15*15*15+a(6)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[3][(a(7)+a(6)*15+a(5)*15*15+a(11)*15*15*15+a(10)*15*15*15*15+a(9)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[3][(a(1)+a(5)*15+a(9)*15*15+a(2)*15*15*15+a(6)*15*15*15*15+a(10)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[3][(a(8)+a(9)*15+a(10)*15*15+a(4)*15*15*15+a(5)*15*15*15*15+a(6)*15*15*15*15*15)*4+(a.info()-1)];
			value_next += weight_table[3][(a(14)+a(10)*15+a(6)*15*15+a(13)*15*15*15+a(9)*15*15*15*15+a(5)*15*15*15*15*15)*4+(a.info()-1)];
			if((reward+value_next)>=max){
				go = op;
				max = reward+value_next;
			}

		}
		if (lose != -1){
				//RR
				player_direction = go;

				//RR
				return action::slide(go);
		}
		else{

			return action();
		}
		
		/*
		int best = -1;
		float max = -1000000.f;
		int lose = -1;
		for(int op : opcode){
			board a = before;
			board::reward reward = a.slide(op);
			if(reward == -1){
				continue;
			}
			lose = 0;
			float tmp = expectimax(a, weight_table, 2, op);
			if(tmp > max){
				best = op;
				max = tmp;
			}
		}
		if(lose != -1){
			return action::slide(best);
		}
		else{
			return action();
		}
		*/
	}
	/*
	float expectimax(board b, const weight_agent& weight_table, int level, int last){
		if(level == 2){
			std::array<int, 4> p;
			p = {0, 0, 0, 0};
			switch(last){
				case 0: 
					p={12, 13, 14, 15};   //up
					break;
				case 1: 
					p={0, 4, 8, 12};   //right
					break;
				case 2: 
					p={0, 1, 2, 3};   //down
					break;
				case 3: 
					p={3, 7, 11, 15};   //left
					break;
				default:
					break;
			}
			float avg = 0.0;
			int n = 0;
			for(int i : p){
				if (b(i) != 0) continue;
				board next = b;
				next.place(i, next.info());
				float tmp = 0.0;
				tmp = expectimax(next, weight_table, 1, last);
				avg += tmp;
				n++;
			}
			avg /= n;
			return avg;
		}
		else if(level == 1){
			std::array<int, 4> d;
			d = {0, 1, 2, 3};
			float max = -1000000.f;
			int lose = -1;
			for(int i : d){
				board a = b;
				board::reward reward = a.slide(i);
				if(reward == -1){
					continue;
				}
				lose = 0;
				float value_next = 0.0;
				value_next += weight_table[0][a(0)+a(1)*15+a(2)*15*15+a(3)*15*15*15+a(4)*15*15*15*15+a(5)*15*15*15*15*15];
				value_next += weight_table[0][a(3)+a(7)*15+a(11)*15*15+a(15)*15*15*15+a(2)*15*15*15*15+a(6)*15*15*15*15*15];
				value_next += weight_table[0][a(15)+a(14)*15+a(13)*15*15+a(12)*15*15*15+a(11)*15*15*15*15+a(10)*15*15*15*15*15];
				value_next += weight_table[0][a(12)+a(8)*15+a(4)*15*15+a(0)*15*15*15+a(13)*15*15*15*15+a(9)*15*15*15*15*15];
				value_next += weight_table[0][a(3)+a(2)*15+a(1)*15*15+a(0)*15*15*15+a(7)*15*15*15*15+a(6)*15*15*15*15*15];
				value_next += weight_table[0][a(0)+a(4)*15+a(8)*15*15+a(12)*15*15*15+a(1)*15*15*15*15+a(5)*15*15*15*15*15];
				value_next += weight_table[0][a(12)+a(13)*15+a(14)*15*15+a(15)*15*15*15+a(8)*15*15*15*15+a(9)*15*15*15*15*15];
				value_next += weight_table[0][a(15)+a(11)*15+a(7)*15*15+a(3)*15*15*15+a(14)*15*15*15*15+a(10)*15*15*15*15*15];
				value_next += weight_table[1][a(4)+a(5)*15+a(6)*15*15+a(7)*15*15*15+a(8)*15*15*15*15+a(9)*15*15*15*15*15];
				value_next += weight_table[1][a(2)+a(6)*15+a(10)*15*15+a(14)*15*15*15+a(1)*15*15*15*15+a(5)*15*15*15*15*15];
				value_next += weight_table[1][a(11)+a(10)*15+a(9)*15*15+a(8)*15*15*15+a(7)*15*15*15*15+a(6)*15*15*15*15*15];
				value_next += weight_table[1][a(13)+a(9)*15+a(5)*15*15+a(1)*15*15*15+a(14)*15*15*15*15+a(10)*15*15*15*15*15];
				value_next += weight_table[1][a(7)+a(6)*15+a(5)*15*15+a(4)*15*15*15+a(11)*15*15*15*15+a(10)*15*15*15*15*15];
				value_next += weight_table[1][a(1)+a(5)*15+a(9)*15*15+a(13)*15*15*15+a(2)*15*15*15*15+a(6)*15*15*15*15*15];
				value_next += weight_table[1][a(8)+a(9)*15+a(10)*15*15+a(11)*15*15*15+a(4)*15*15*15*15+a(5)*15*15*15*15*15];
				value_next += weight_table[1][a(14)+a(10)*15+a(6)*15*15+a(2)*15*15*15+a(13)*15*15*15*15+a(9)*15*15*15*15*15];
				value_next += weight_table[2][a(0)+a(1)*15+a(2)*15*15+a(4)*15*15*15+a(5)*15*15*15*15+a(6)*15*15*15*15*15];
				value_next += weight_table[2][a(3)+a(7)*15+a(11)*15*15+a(2)*15*15*15+a(6)*15*15*15*15+a(10)*15*15*15*15*15];
				value_next += weight_table[2][a(15)+a(14)*15+a(13)*15*15+a(11)*15*15*15+a(10)*15*15*15*15+a(9)*15*15*15*15*15];
				value_next += weight_table[2][a(12)+a(8)*15+a(4)*15*15+a(13)*15*15*15+a(9)*15*15*15*15+a(5)*15*15*15*15*15];
				value_next += weight_table[2][a(3)+a(2)*15+a(1)*15*15+a(7)*15*15*15+a(6)*15*15*15*15+a(5)*15*15*15*15*15];
				value_next += weight_table[2][a(0)+a(4)*15+a(8)*15*15+a(1)*15*15*15+a(5)*15*15*15*15+a(9)*15*15*15*15*15];
				value_next += weight_table[2][a(12)+a(13)*15+a(14)*15*15+a(8)*15*15*15+a(9)*15*15*15*15+a(10)*15*15*15*15*15];
				value_next += weight_table[2][a(15)+a(11)*15+a(7)*15*15+a(14)*15*15*15+a(10)*15*15*15*15+a(6)*15*15*15*15*15];
				value_next += weight_table[3][a(4)+a(5)*15+a(6)*15*15+a(8)*15*15*15+a(9)*15*15*15*15+a(10)*15*15*15*15*15];
				value_next += weight_table[3][a(2)+a(6)*15+a(10)*15*15+a(1)*15*15*15+a(5)*15*15*15*15+a(9)*15*15*15*15*15];
				value_next += weight_table[3][a(11)+a(10)*15+a(9)*15*15+a(7)*15*15*15+a(6)*15*15*15*15+a(5)*15*15*15*15*15];
				value_next += weight_table[3][a(13)+a(9)*15+a(5)*15*15+a(14)*15*15*15+a(10)*15*15*15*15+a(6)*15*15*15*15*15];
				value_next += weight_table[3][a(7)+a(6)*15+a(5)*15*15+a(11)*15*15*15+a(10)*15*15*15*15+a(9)*15*15*15*15*15];
				value_next += weight_table[3][a(1)+a(5)*15+a(9)*15*15+a(2)*15*15*15+a(6)*15*15*15*15+a(10)*15*15*15*15*15];
				value_next += weight_table[3][a(8)+a(9)*15+a(10)*15*15+a(4)*15*15*15+a(5)*15*15*15*15+a(6)*15*15*15*15*15];
				value_next += weight_table[3][a(14)+a(10)*15+a(6)*15*15+a(13)*15*15*15+a(9)*15*15*15*15+a(5)*15*15*15*15*15];

				if((reward+value_next)>=max){
					max = reward+value_next;
				}
			}
			if(lose != -1){
				return max;
			}
			else{
				float value_ = 0.0;
				board a = b;
				value_ += weight_table[0][a(0)+a(1)*15+a(2)*15*15+a(3)*15*15*15+a(4)*15*15*15*15+a(5)*15*15*15*15*15];
				value_ += weight_table[0][a(3)+a(7)*15+a(11)*15*15+a(15)*15*15*15+a(2)*15*15*15*15+a(6)*15*15*15*15*15];
				value_ += weight_table[0][a(15)+a(14)*15+a(13)*15*15+a(12)*15*15*15+a(11)*15*15*15*15+a(10)*15*15*15*15*15];
				value_ += weight_table[0][a(12)+a(8)*15+a(4)*15*15+a(0)*15*15*15+a(13)*15*15*15*15+a(9)*15*15*15*15*15];
				value_ += weight_table[0][a(3)+a(2)*15+a(1)*15*15+a(0)*15*15*15+a(7)*15*15*15*15+a(6)*15*15*15*15*15];
				value_ += weight_table[0][a(0)+a(4)*15+a(8)*15*15+a(12)*15*15*15+a(1)*15*15*15*15+a(5)*15*15*15*15*15];
				value_ += weight_table[0][a(12)+a(13)*15+a(14)*15*15+a(15)*15*15*15+a(8)*15*15*15*15+a(9)*15*15*15*15*15];
				value_ += weight_table[0][a(15)+a(11)*15+a(7)*15*15+a(3)*15*15*15+a(14)*15*15*15*15+a(10)*15*15*15*15*15];
				value_ += weight_table[1][a(4)+a(5)*15+a(6)*15*15+a(7)*15*15*15+a(8)*15*15*15*15+a(9)*15*15*15*15*15];
				value_ += weight_table[1][a(2)+a(6)*15+a(10)*15*15+a(14)*15*15*15+a(1)*15*15*15*15+a(5)*15*15*15*15*15];
				value_ += weight_table[1][a(11)+a(10)*15+a(9)*15*15+a(8)*15*15*15+a(7)*15*15*15*15+a(6)*15*15*15*15*15];
				value_ += weight_table[1][a(13)+a(9)*15+a(5)*15*15+a(1)*15*15*15+a(14)*15*15*15*15+a(10)*15*15*15*15*15];
				value_ += weight_table[1][a(7)+a(6)*15+a(5)*15*15+a(4)*15*15*15+a(11)*15*15*15*15+a(10)*15*15*15*15*15];
				value_ += weight_table[1][a(1)+a(5)*15+a(9)*15*15+a(13)*15*15*15+a(2)*15*15*15*15+a(6)*15*15*15*15*15];
				value_ += weight_table[1][a(8)+a(9)*15+a(10)*15*15+a(11)*15*15*15+a(4)*15*15*15*15+a(5)*15*15*15*15*15];
				value_ += weight_table[1][a(14)+a(10)*15+a(6)*15*15+a(2)*15*15*15+a(13)*15*15*15*15+a(9)*15*15*15*15*15];
				value_ += weight_table[2][a(0)+a(1)*15+a(2)*15*15+a(4)*15*15*15+a(5)*15*15*15*15+a(6)*15*15*15*15*15];
				value_ += weight_table[2][a(3)+a(7)*15+a(11)*15*15+a(2)*15*15*15+a(6)*15*15*15*15+a(10)*15*15*15*15*15];
				value_ += weight_table[2][a(15)+a(14)*15+a(13)*15*15+a(11)*15*15*15+a(10)*15*15*15*15+a(9)*15*15*15*15*15];
				value_ += weight_table[2][a(12)+a(8)*15+a(4)*15*15+a(13)*15*15*15+a(9)*15*15*15*15+a(5)*15*15*15*15*15];
				value_ += weight_table[2][a(3)+a(2)*15+a(1)*15*15+a(7)*15*15*15+a(6)*15*15*15*15+a(5)*15*15*15*15*15];
				value_ += weight_table[2][a(0)+a(4)*15+a(8)*15*15+a(1)*15*15*15+a(5)*15*15*15*15+a(9)*15*15*15*15*15];
				value_ += weight_table[2][a(12)+a(13)*15+a(14)*15*15+a(8)*15*15*15+a(9)*15*15*15*15+a(10)*15*15*15*15*15];
				value_ += weight_table[2][a(15)+a(11)*15+a(7)*15*15+a(14)*15*15*15+a(10)*15*15*15*15+a(6)*15*15*15*15*15];
				value_ += weight_table[3][a(4)+a(5)*15+a(6)*15*15+a(8)*15*15*15+a(9)*15*15*15*15+a(10)*15*15*15*15*15];
				value_ += weight_table[3][a(2)+a(6)*15+a(10)*15*15+a(1)*15*15*15+a(5)*15*15*15*15+a(9)*15*15*15*15*15];
				value_ += weight_table[3][a(11)+a(10)*15+a(9)*15*15+a(7)*15*15*15+a(6)*15*15*15*15+a(5)*15*15*15*15*15];
				value_ += weight_table[3][a(13)+a(9)*15+a(5)*15*15+a(14)*15*15*15+a(10)*15*15*15*15+a(6)*15*15*15*15*15];
				value_ += weight_table[3][a(7)+a(6)*15+a(5)*15*15+a(11)*15*15*15+a(10)*15*15*15*15+a(9)*15*15*15*15*15];
				value_ += weight_table[3][a(1)+a(5)*15+a(9)*15*15+a(2)*15*15*15+a(6)*15*15*15*15+a(10)*15*15*15*15*15];
				value_ += weight_table[3][a(8)+a(9)*15+a(10)*15*15+a(4)*15*15*15+a(5)*15*15*15*15+a(6)*15*15*15*15*15];
				value_ += weight_table[3][a(14)+a(10)*15+a(6)*15*15+a(13)*15*15*15+a(9)*15*15*15*15+a(5)*15*15*15*15*15];
				return value_;
			}
		}
		return -1;
	}
	*/
private:
	std::array<int, 4> opcode;
	int player_direction; //RRRRRRRRRRRRRR
};
