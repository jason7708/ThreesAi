/**
 * Basic Environment for Game Threes
 * use 'clang++ -O3 -o threes threes.cpp' to compile the source
 */

#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include "board.h"
#include "action.h"
#include "agent.h"
#include "episode.h"
#include "statistic.h"

int main(int argc, const char* argv[]) {
	std::cout << "Threes-Demo: ";
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl << std::endl;

	size_t total = 1000, block = 0, limit = 0;
	std::string play_args, evil_args;
	std::string load, save;
	bool summary = false;
	for (int i = 1; i < argc; i++) {
		std::string para(argv[i]);
		if (para.find("--total=") == 0) {
			total = std::stoull(para.substr(para.find("=") + 1));
		} else if (para.find("--block=") == 0) {
			block = std::stoull(para.substr(para.find("=") + 1));
		} else if (para.find("--limit=") == 0) {
			limit = std::stoull(para.substr(para.find("=") + 1));
		} else if (para.find("--play=") == 0) {
			play_args = para.substr(para.find("=") + 1);
		} else if (para.find("--evil=") == 0) {
			evil_args = para.substr(para.find("=") + 1);
		} else if (para.find("--load=") == 0) {
			load = para.substr(para.find("=") + 1);
		} else if (para.find("--save=") == 0) {
			save = para.substr(para.find("=") + 1);
		} else if (para.find("--summary") == 0) {
			summary = true;
		}
	}

	statistic stat(total, block, limit);

	if (load.size()) {
		std::ifstream in(load, std::ios::in);
		in >> stat;
		in.close();
		summary |= stat.is_finished();
	}

	weight_agent play(play_args);
	rndenv evil(evil_args);
	learning_agent alpha(play_args);
	int epoch = 0;

	while (!stat.is_finished()) {

		play.open_episode("~:" + evil.name());
		evil.open_episode(play.name() + ":~");

		stat.open_episode(play.name() + ":" + evil.name());
		episode& game = stat.back();

		// init environment
		evil.initial();
		
		while (true) {
			agent& who = game.take_turns(play, evil);
			action&& move = who.take_action(game.state());

			if (game.apply_action(move) != true) {
				break;
			}
			
			// get player previous slide direction
			if(game.step() >= 9 && game.step() % 2 == 0) {
				evil.get_user_direction(play.player_choose());
			}
			if (who.check_for_win(game.state())) {
				break;
			}
			
		}
		
		agent& win = game.last_turns(play, evil);
		stat.close_episode(win.name());

		play.close_episode(win.name());
		evil.close_episode(win.name());

		game.train_update(play, alpha);
		if(epoch % 100000 == 0) {
			alpha.update_alpha();
		}
		epoch++;
	}

	if (summary) {
		stat.summary();
	}

	if (save.size()) {
		std::ofstream out(save, std::ios::out | std::ios::trunc);
		out << stat;
		out.close();
	}

	return 0;
}
