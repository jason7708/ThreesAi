#pragma once
#include <list>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <chrono>
#include <numeric>
#include "board.h"
#include "action.h"
#include "agent.h"
#include <cmath>

class statistic;

class episode {
friend class statistic;
public:
	episode() : ep_state(initial_state()), ep_score(0), ep_time(0) { ep_moves.reserve(10000); }
//RRRRRRRRRRRRRRRRRRRRRRRRR
public:
	void train_update(weight_agent& weight_table, learning_agent& _alpha){
		for(unsigned i = ep_moves.size()-1; i>=9; i-=2){
			float error = 0;
			float reward_st_1 = 0, value_st_1 = 0, value_st = 0;
			if(i == ep_moves.size()-1){ //last step is evil
				i--;
			}
			if(i == ep_moves.size()-2){
				reward_st_1 = 0;
				value_st_1 = 0;
			}
			else{
				reward_st_1 = ep_moves[i+2].reward;
				const board &x = ep_moves[i+2].board_now;
				/*  4 tuple
				value_st_1 += weight_table[0][x(0)+x(1)*15+x(2)*15*15+x(3)*15*15*15];
				value_st_1 += weight_table[1][x(4)+x(5)*15+x(6)*15*15+x(7)*15*15*15];
				value_st_1 += weight_table[1][x(8)+x(9)*15+x(10)*15*15+x(11)*15*15*15];
				value_st_1 += weight_table[0][x(12)+x(13)*15+x(14)*15*15+x(15)*15*15*15];

				value_st_1 += weight_table[0][x(0)+x(4)*15+x(8)*15*15+x(12)*15*15*15];
				value_st_1 += weight_table[1][x(1)+x(5)*15+x(9)*15*15+x(13)*15*15*15];
				value_st_1 += weight_table[1][x(2)+x(6)*15+x(10)*15*15+x(14)*15*15*15];
				value_st_1 += weight_table[0][x(3)+x(7)*15+x(11)*15*15+x(15)*15*15*15];
				*/
				//6 tuple
				value_st_1 += weight_table[0][(x(0)+x(1)*15+x(2)*15*15+x(3)*15*15*15+x(4)*15*15*15*15+x(5)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[0][(x(3)+x(7)*15+x(11)*15*15+x(15)*15*15*15+x(2)*15*15*15*15+x(6)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[0][(x(15)+x(14)*15+x(13)*15*15+x(12)*15*15*15+x(11)*15*15*15*15+x(10)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[0][(x(12)+x(8)*15+x(4)*15*15+x(0)*15*15*15+x(13)*15*15*15*15+x(9)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[0][(x(3)+x(2)*15+x(1)*15*15+x(0)*15*15*15+x(7)*15*15*15*15+x(6)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[0][(x(0)+x(4)*15+x(8)*15*15+x(12)*15*15*15+x(1)*15*15*15*15+x(5)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[0][(x(12)+x(13)*15+x(14)*15*15+x(15)*15*15*15+x(8)*15*15*15*15+x(9)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[0][(x(15)+x(11)*15+x(7)*15*15+x(3)*15*15*15+x(14)*15*15*15*15+x(10)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[1][(x(4)+x(5)*15+x(6)*15*15+x(7)*15*15*15+x(8)*15*15*15*15+x(9)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[1][(x(2)+x(6)*15+x(10)*15*15+x(14)*15*15*15+x(1)*15*15*15*15+x(5)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[1][(x(11)+x(10)*15+x(9)*15*15+x(8)*15*15*15+x(7)*15*15*15*15+x(6)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[1][(x(13)+x(9)*15+x(5)*15*15+x(1)*15*15*15+x(14)*15*15*15*15+x(10)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[1][(x(7)+x(6)*15+x(5)*15*15+x(4)*15*15*15+x(11)*15*15*15*15+x(10)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[1][(x(1)+x(5)*15+x(9)*15*15+x(13)*15*15*15+x(2)*15*15*15*15+x(6)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[1][(x(8)+x(9)*15+x(10)*15*15+x(11)*15*15*15+x(4)*15*15*15*15+x(5)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[1][(x(14)+x(10)*15+x(6)*15*15+x(2)*15*15*15+x(13)*15*15*15*15+x(9)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[2][(x(0)+x(1)*15+x(2)*15*15+x(4)*15*15*15+x(5)*15*15*15*15+x(6)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[2][(x(3)+x(7)*15+x(11)*15*15+x(2)*15*15*15+x(6)*15*15*15*15+x(10)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[2][(x(15)+x(14)*15+x(13)*15*15+x(11)*15*15*15+x(10)*15*15*15*15+x(9)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[2][(x(12)+x(8)*15+x(4)*15*15+x(13)*15*15*15+x(9)*15*15*15*15+x(5)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[2][(x(3)+x(2)*15+x(1)*15*15+x(7)*15*15*15+x(6)*15*15*15*15+x(5)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[2][(x(0)+x(4)*15+x(8)*15*15+x(1)*15*15*15+x(5)*15*15*15*15+x(9)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[2][(x(12)+x(13)*15+x(14)*15*15+x(8)*15*15*15+x(9)*15*15*15*15+x(10)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[2][(x(15)+x(11)*15+x(7)*15*15+x(14)*15*15*15+x(10)*15*15*15*15+x(6)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[3][(x(4)+x(5)*15+x(6)*15*15+x(8)*15*15*15+x(9)*15*15*15*15+x(10)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[3][(x(2)+x(6)*15+x(10)*15*15+x(1)*15*15*15+x(5)*15*15*15*15+x(9)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[3][(x(11)+x(10)*15+x(9)*15*15+x(7)*15*15*15+x(6)*15*15*15*15+x(5)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[3][(x(13)+x(9)*15+x(5)*15*15+x(14)*15*15*15+x(10)*15*15*15*15+x(6)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[3][(x(7)+x(6)*15+x(5)*15*15+x(11)*15*15*15+x(10)*15*15*15*15+x(9)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[3][(x(1)+x(5)*15+x(9)*15*15+x(2)*15*15*15+x(6)*15*15*15*15+x(10)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[3][(x(8)+x(9)*15+x(10)*15*15+x(4)*15*15*15+x(5)*15*15*15*15+x(6)*15*15*15*15*15)*4+(x.info()-1)];
				value_st_1 += weight_table[3][(x(14)+x(10)*15+x(6)*15*15+x(13)*15*15*15+x(9)*15*15*15*15+x(5)*15*15*15*15*15)*4+(x.info()-1)];
				
			}
			

			const board &y = ep_moves[i].board_now;
			/*
			value_st += weight_table[0][y(0)+y(1)*15+y(2)*15*15+y(3)*15*15*15];
			value_st += weight_table[1][y(4)+y(5)*15+y(6)*15*15+y(7)*15*15*15];
			value_st += weight_table[1][y(8)+y(9)*15+y(10)*15*15+y(11)*15*15*15];
			value_st += weight_table[0][y(12)+y(13)*15+y(14)*15*15+y(15)*15*15*15];

			value_st += weight_table[0][y(0)+y(4)*15+y(8)*15*15+y(12)*15*15*15];
			value_st += weight_table[1][y(1)+y(5)*15+y(9)*15*15+y(13)*15*15*15];
			value_st += weight_table[1][y(2)+y(6)*15+y(10)*15*15+y(14)*15*15*15];
			value_st += weight_table[0][y(3)+y(7)*15+y(11)*15*15+y(15)*15*15*15];
			*/
			// 6 tuple
			value_st += weight_table[0][(y(0)+y(1)*15+y(2)*15*15+y(3)*15*15*15+y(4)*15*15*15*15+y(5)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[0][(y(3)+y(7)*15+y(11)*15*15+y(15)*15*15*15+y(2)*15*15*15*15+y(6)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[0][(y(15)+y(14)*15+y(13)*15*15+y(12)*15*15*15+y(11)*15*15*15*15+y(10)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[0][(y(12)+y(8)*15+y(4)*15*15+y(0)*15*15*15+y(13)*15*15*15*15+y(9)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[0][(y(3)+y(2)*15+y(1)*15*15+y(0)*15*15*15+y(7)*15*15*15*15+y(6)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[0][(y(0)+y(4)*15+y(8)*15*15+y(12)*15*15*15+y(1)*15*15*15*15+y(5)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[0][(y(12)+y(13)*15+y(14)*15*15+y(15)*15*15*15+y(8)*15*15*15*15+y(9)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[0][(y(15)+y(11)*15+y(7)*15*15+y(3)*15*15*15+y(14)*15*15*15*15+y(10)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[1][(y(4)+y(5)*15+y(6)*15*15+y(7)*15*15*15+y(8)*15*15*15*15+y(9)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[1][(y(2)+y(6)*15+y(10)*15*15+y(14)*15*15*15+y(1)*15*15*15*15+y(5)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[1][(y(11)+y(10)*15+y(9)*15*15+y(8)*15*15*15+y(7)*15*15*15*15+y(6)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[1][(y(13)+y(9)*15+y(5)*15*15+y(1)*15*15*15+y(14)*15*15*15*15+y(10)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[1][(y(7)+y(6)*15+y(5)*15*15+y(4)*15*15*15+y(11)*15*15*15*15+y(10)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[1][(y(1)+y(5)*15+y(9)*15*15+y(13)*15*15*15+y(2)*15*15*15*15+y(6)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[1][(y(8)+y(9)*15+y(10)*15*15+y(11)*15*15*15+y(4)*15*15*15*15+y(5)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[1][(y(14)+y(10)*15+y(6)*15*15+y(2)*15*15*15+y(13)*15*15*15*15+y(9)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[2][(y(0)+y(1)*15+y(2)*15*15+y(4)*15*15*15+y(5)*15*15*15*15+y(6)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[2][(y(3)+y(7)*15+y(11)*15*15+y(2)*15*15*15+y(6)*15*15*15*15+y(10)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[2][(y(15)+y(14)*15+y(13)*15*15+y(11)*15*15*15+y(10)*15*15*15*15+y(9)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[2][(y(12)+y(8)*15+y(4)*15*15+y(13)*15*15*15+y(9)*15*15*15*15+y(5)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[2][(y(3)+y(2)*15+y(1)*15*15+y(7)*15*15*15+y(6)*15*15*15*15+y(5)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[2][(y(0)+y(4)*15+y(8)*15*15+y(1)*15*15*15+y(5)*15*15*15*15+y(9)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[2][(y(12)+y(13)*15+y(14)*15*15+y(8)*15*15*15+y(9)*15*15*15*15+y(10)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[2][(y(15)+y(11)*15+y(7)*15*15+y(14)*15*15*15+y(10)*15*15*15*15+y(6)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[3][(y(4)+y(5)*15+y(6)*15*15+y(8)*15*15*15+y(9)*15*15*15*15+y(10)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[3][(y(2)+y(6)*15+y(10)*15*15+y(1)*15*15*15+y(5)*15*15*15*15+y(9)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[3][(y(11)+y(10)*15+y(9)*15*15+y(7)*15*15*15+y(6)*15*15*15*15+y(5)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[3][(y(13)+y(9)*15+y(5)*15*15+y(14)*15*15*15+y(10)*15*15*15*15+y(6)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[3][(y(7)+y(6)*15+y(5)*15*15+y(11)*15*15*15+y(10)*15*15*15*15+y(9)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[3][(y(1)+y(5)*15+y(9)*15*15+y(2)*15*15*15+y(6)*15*15*15*15+y(10)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[3][(y(8)+y(9)*15+y(10)*15*15+y(4)*15*15*15+y(5)*15*15*15*15+y(6)*15*15*15*15*15)*4+(y.info()-1)];
			value_st += weight_table[3][(y(14)+y(10)*15+y(6)*15*15+y(13)*15*15*15+y(9)*15*15*15*15+y(5)*15*15*15*15*15)*4+(y.info()-1)];


			error = (value_st_1 - value_st + reward_st_1) * _alpha.get_alpha() / 192/*32*/;
			/* 4 tuple
			weight_table[0][y(0)+y(1)*15+y(2)*15*15+y(3)*15*15*15] += error;
			weight_table[1][y(4)+y(5)*15+y(6)*15*15+y(7)*15*15*15] += error;
			weight_table[1][y(8)+y(9)*15+y(10)*15*15+y(11)*15*15*15] += error;
			weight_table[0][y(12)+y(13)*15+y(14)*15*15+y(15)*15*15*15] += error;

			weight_table[0][y(0)+y(4)*15+y(8)*15*15+y(12)*15*15*15] += error;
			weight_table[1][y(1)+y(5)*15+y(9)*15*15+y(13)*15*15*15] += error;
			weight_table[1][y(2)+y(6)*15+y(10)*15*15+y(14)*15*15*15] += error;
			weight_table[0][y(3)+y(7)*15+y(11)*15*15+y(15)*15*15*15] += error;
			*/

			weight_table[0][(y(0)+y(1)*15+y(2)*15*15+y(3)*15*15*15+y(4)*15*15*15*15+y(5)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[0][(y(3)+y(7)*15+y(11)*15*15+y(15)*15*15*15+y(2)*15*15*15*15+y(6)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[0][(y(15)+y(14)*15+y(13)*15*15+y(12)*15*15*15+y(11)*15*15*15*15+y(10)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[0][(y(12)+y(8)*15+y(4)*15*15+y(0)*15*15*15+y(13)*15*15*15*15+y(9)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[0][(y(3)+y(2)*15+y(1)*15*15+y(0)*15*15*15+y(7)*15*15*15*15+y(6)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[0][(y(0)+y(4)*15+y(8)*15*15+y(12)*15*15*15+y(1)*15*15*15*15+y(5)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[0][(y(12)+y(13)*15+y(14)*15*15+y(15)*15*15*15+y(8)*15*15*15*15+y(9)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[0][(y(15)+y(11)*15+y(7)*15*15+y(3)*15*15*15+y(14)*15*15*15*15+y(10)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[1][(y(4)+y(5)*15+y(6)*15*15+y(7)*15*15*15+y(8)*15*15*15*15+y(9)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[1][(y(2)+y(6)*15+y(10)*15*15+y(14)*15*15*15+y(1)*15*15*15*15+y(5)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[1][(y(11)+y(10)*15+y(9)*15*15+y(8)*15*15*15+y(7)*15*15*15*15+y(6)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[1][(y(13)+y(9)*15+y(5)*15*15+y(1)*15*15*15+y(14)*15*15*15*15+y(10)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[1][(y(7)+y(6)*15+y(5)*15*15+y(4)*15*15*15+y(11)*15*15*15*15+y(10)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[1][(y(1)+y(5)*15+y(9)*15*15+y(13)*15*15*15+y(2)*15*15*15*15+y(6)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[1][(y(8)+y(9)*15+y(10)*15*15+y(11)*15*15*15+y(4)*15*15*15*15+y(5)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[1][(y(14)+y(10)*15+y(6)*15*15+y(2)*15*15*15+y(13)*15*15*15*15+y(9)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[2][(y(0)+y(1)*15+y(2)*15*15+y(4)*15*15*15+y(5)*15*15*15*15+y(6)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[2][(y(3)+y(7)*15+y(11)*15*15+y(2)*15*15*15+y(6)*15*15*15*15+y(10)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[2][(y(15)+y(14)*15+y(13)*15*15+y(11)*15*15*15+y(10)*15*15*15*15+y(9)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[2][(y(12)+y(8)*15+y(4)*15*15+y(13)*15*15*15+y(9)*15*15*15*15+y(5)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[2][(y(3)+y(2)*15+y(1)*15*15+y(7)*15*15*15+y(6)*15*15*15*15+y(5)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[2][(y(0)+y(4)*15+y(8)*15*15+y(1)*15*15*15+y(5)*15*15*15*15+y(9)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[2][(y(12)+y(13)*15+y(14)*15*15+y(8)*15*15*15+y(9)*15*15*15*15+y(10)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[2][(y(15)+y(11)*15+y(7)*15*15+y(14)*15*15*15+y(10)*15*15*15*15+y(6)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[3][(y(4)+y(5)*15+y(6)*15*15+y(8)*15*15*15+y(9)*15*15*15*15+y(10)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[3][(y(2)+y(6)*15+y(10)*15*15+y(1)*15*15*15+y(5)*15*15*15*15+y(9)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[3][(y(11)+y(10)*15+y(9)*15*15+y(7)*15*15*15+y(6)*15*15*15*15+y(5)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[3][(y(13)+y(9)*15+y(5)*15*15+y(14)*15*15*15+y(10)*15*15*15*15+y(6)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[3][(y(7)+y(6)*15+y(5)*15*15+y(11)*15*15*15+y(10)*15*15*15*15+y(9)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[3][(y(1)+y(5)*15+y(9)*15*15+y(2)*15*15*15+y(6)*15*15*15*15+y(10)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[3][(y(8)+y(9)*15+y(10)*15*15+y(4)*15*15*15+y(5)*15*15*15*15+y(6)*15*15*15*15*15)*4+(y.info()-1)] += error;
			weight_table[3][(y(14)+y(10)*15+y(6)*15*15+y(13)*15*15*15+y(9)*15*15*15*15+y(5)*15*15*15*15*15)*4+(y.info()-1)] += error;
		}
	}
//RRRRRRRRRRRRRRRRRRRRRRRRR
public:
	board& state() { return ep_state; }
	const board& state() const { return ep_state; }
	board::reward score() const {
		return ep_score; 
	}

	void open_episode(const std::string& tag) {
		ep_open = { tag, millisec() };
	}
	void close_episode(const std::string& tag) {
		ep_close = { tag, millisec() };
	}
	bool apply_action(action move) {
		board::reward reward = move.apply(state());
		if (reward == -1) return false;
		ep_moves.emplace_back(move, reward, millisec() - ep_time, ep_state);
		ep_score += reward;
		return true;
	}
	agent& take_turns(agent& play, agent& evil) {
		ep_time = millisec();
		//if(std::max(step() + 1, size_t(9)) % 2)std::cout<<"evil"<<'\n';
		//else std::cout<<"play"<<'\n';
		//return (std::max(step() + 1, size_t(2)) % 2) ? play : evil; //decide who moves
		return (std::max(step() + 1, size_t(9)) % 2) ? evil : play;  //RRRRR
	}
	agent& last_turns(agent& play, agent& evil) {
		return take_turns(evil, play);
	}

public:
	size_t step(unsigned who = -1u) const {
		int size = ep_moves.size(); // 'int' is important for handling 0
		switch (who) {
		case action::slide::type: return (size - 1) / 2;
		case action::place::type: return (size - (size - 1) / 2);
		default:                  return size;
		}
	}

	time_t time(unsigned who = -1u) const {
		time_t time = 0;
		size_t i = 2;
		switch (who) {
		case action::place::type:
			if (ep_moves.size()) time += ep_moves[0].time, i = 1;
			// no break;
		case action::slide::type:
			while (i < ep_moves.size()) time += ep_moves[i].time, i += 2;
			break;
		default:
			time = ep_close.when - ep_open.when;
			break;
		}
		return time;
	}

	std::vector<action> actions(unsigned who = -1u) const {
		std::vector<action> res;
		size_t i = 2;
		switch (who) {
		case action::place::type:
			if (ep_moves.size()) res.push_back(ep_moves[0]), i = 1;
			// no break;
		case action::slide::type:
			while (i < ep_moves.size()) res.push_back(ep_moves[i]), i += 2;
			break;
		default:
			res.assign(ep_moves.begin(), ep_moves.end());
			break;
		}
		return res;
	}

public:

	friend std::ostream& operator <<(std::ostream& out, const episode& ep) {
		out << ep.ep_open << '|';
		for (const move& mv : ep.ep_moves) out << mv;
		out << '|' << ep.ep_close;
		return out;
	}
	friend std::istream& operator >>(std::istream& in, episode& ep) {
		ep = {};
		std::string token;
		std::getline(in, token, '|');
		std::stringstream(token) >> ep.ep_open;
		std::getline(in, token, '|');
		for (std::stringstream moves(token); !moves.eof(); moves.peek()) {
			ep.ep_moves.emplace_back();
			moves >> ep.ep_moves.back();
			ep.ep_score += action(ep.ep_moves.back()).apply(ep.ep_state);
		}
		std::getline(in, token, '|');
		std::stringstream(token) >> ep.ep_close;
		return in;
	}

protected:

	struct move {
		action code;
		board::reward reward;
		time_t time;
		board board_now;
		move(action code = {}, board::reward reward = 0, time_t time = 0, board a = {}) : code(code), reward(reward), time(time), board_now(a) {
		}

		operator action() const { return code; }
		friend std::ostream& operator <<(std::ostream& out, const move& m) {
			out << m.code;
			if (m.reward) out << '[' << std::dec << m.reward << ']';
			if (m.time) out << '(' << std::dec << m.time << ')';
			return out;
		}
		friend std::istream& operator >>(std::istream& in, move& m) {
			in >> m.code;
			m.reward = 0;
			m.time = 0;
			if (in.peek() == '[') {
				in.ignore(1);
				in >> std::dec >> m.reward;
				in.ignore(1);
			}
			if (in.peek() == '(') {
				in.ignore(1);
				in >> std::dec >> m.time;
				in.ignore(1);
			}
			return in;
		}
		//RRRRRRRRRRRRRRR
		
	};

	struct meta {
		std::string tag;
		time_t when;
		meta(const std::string& tag = "N/A", time_t when = 0) : tag(tag), when(when) {}

		friend std::ostream& operator <<(std::ostream& out, const meta& m) {
			return out << m.tag << "@" << std::dec << m.when;
		}
		friend std::istream& operator >>(std::istream& in, meta& m) {
			return std::getline(in, m.tag, '@') >> std::dec >> m.when;
		}
	};

	static board initial_state() {
		return {};
	}
	static time_t millisec() {
		auto now = std::chrono::system_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
	}

private:
	board ep_state;
	board::reward ep_score;
	std::vector<move> ep_moves;
	time_t ep_time;

	meta ep_open;
	meta ep_close;
};
