#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <tuple>
#include <stack>
#include <algorithm>
#include <unordered_map>
#include <limits>

struct checkers_tree_node;
typedef std::shared_ptr<checkers_tree_node> node_ref;
struct checkers_tree_node {	
	checkers_tree_node(char *src, int from = 0, int to = 0) : score(0), from(from), to(to) {
		memcpy(board, src, sizeof(board));
	}
	int from, to, score;
	char board[64];	
	std::vector<node_ref> children;
};

void draw_board(char *board) {
	static std::unordered_map<char, std::string> emoji_map (
    { {'!', "\033[0;43m  \033[0m"}, {' ', "  "}, {'x', "\033[0;31m\033[1;31mⓄ \033[0m"}, {'o', "\033[1;34mⓄ \033[0m"}, {'X', "\033[0;31m\033[1;31mⓍ \033[0m"}, {'O', "\033[1;34mⓍ \033[0m"}});
	std::cout << "0 ";
	for(int i=0; i < 64; i++) {
		if(i % 8 ==0 && i > 0) 
			std::cout << std::endl << (i/8) << " ";
		std::cout << emoji_map[board[i]];	
	}
	std::cout << std::endl << "  a b c d e f g h" << std::endl;
}

inline bool check_move(char *board, int move) {
	if(move >= 0 && move < 64) 
		if(board[move] == ' ') 
			return true;
	return false;
}

inline bool check_capture(char *board, int spot, int piece) {
	if(spot >= 0 && spot < 64)
		if(board[spot] != board[piece] && board[spot] != board[piece]-32 && board[spot] != board[piece]+32 && board[spot] != ' ')
			return true;
	return false;
}

std::vector<int> get_possible_moves(char *board, int i) {
	std::vector<int> ret;
	static int dirs[] = {7, 9, 14, 18};
	switch(board[i]) {
		case 'o':
			for(int m=0; m < 4; m++)
				if(check_move(board, i-dirs[m]) && (m > 1 ? check_capture(board, i-dirs[m-2], i) : true))
					ret.push_back(i-dirs[m]);
		break;
		case 'x':
			for(int m=0; m < 4; m++)
				if(check_move(board, i+dirs[m]) && (m > 1 ? check_capture(board, i+dirs[m-2], i) : true))
					ret.push_back(i+dirs[m]);
		break;
		case 'X':
		case 'O':
			for(int m=0; m < 4; m++)
				if(check_move(board, i-dirs[m]) && (m > 1 ? check_capture(board, i-dirs[m-2], i) : true)) 
					ret.push_back(i-dirs[m]);
			for(int m=0; m < 4; m++)		
				if(check_move(board, i+dirs[m]) && (m > 1 ? check_capture(board, i+dirs[m-2], i) : true))
					ret.push_back(i+dirs[m]);					
		break;			
	}
	return ret;
}

std::vector<std::pair<int, int>> get_possible_moves_for_side(char *board, char side) {
	std::vector<std::pair<int, int>> ret;
	for(int i=0; i < 64; i++) {
		if(board[i] == side || board[i] == side-32) {
			std::vector<int> moves = get_possible_moves(board, i);
			for(int m : moves) {
				ret.push_back(std::make_pair(i, m));
			}
		}
	}
	return ret;
}

bool check_valid_move(char *board, int from, int to, char side) {
	if(from < 0 || from > 63 || to < 0 || to > 63)
		return false;
	if(!(board[from] == side || board[from] == side-32 || board[from] == side+32))
		return false;
	std::vector<int> moves = get_possible_moves(board, from);
	if(std::find(moves.begin(), moves.end(), to) == moves.end())
		return false;
	return true;
}

bool move_piece(char *board, int from, int to, char side) {
	int diag = -1;
	if(to < from) {
		if(to % 8 < from % 8)
			diag = from -9;
		else
			diag = from -7;
	} else {
		if(to % 8 < from % 8)
			diag = from + 7;
		else
			diag = from  + 9;
	}
	if(board[diag] != ' ' && board[diag] != side && board[diag] != side-32 && board[diag] != side+32)
		board[diag] = ' ';
	board[to] = board[from];
	board[from] = ' ';
	if(board[to] == 'x' && to > 55)
		board[to] = 'X';
	if(board[to] == 'o' && to < 8)
		board[to] = 'O';
	return true;
}

std::string board_to_str(int from, int to) {
	std::string ret;
	char row = (from /8) + 0x30;
	char col = (from % 8) + 0x61;
	ret.push_back(col);	
	ret.push_back(row);	
	row = (to /8) + 0x30;
	col = (to % 8) + 0x61;
	ret.push_back(col);	
	ret.push_back(row);	
	return ret;
}
int str_to_board(const std::string &move) {
	return (int)(move[0]-0x61) + ((int)(move[1]-0x30) * 8);
}

int calc_score(char *board) {
	static std::unordered_map<char, int> score_map ({{'x', 1}, {'X', 10}, {'o', -1}, {'O', -10}});
	int score = 0;
	for(int i=0; i < 64; i++) {
		score += score_map[board[i]];
	}
	return score;
}

int minimax(node_ref node, int depth, bool maximizingPlayer) {
	if(depth == 0) {
		return calc_score(node->board);
	}
	if(maximizingPlayer) {
		std::vector<std::pair<int, int>> moves = get_possible_moves_for_side(node->board, 'x');
		int maxEval = std::numeric_limits<int>::min();
		for(auto &m : moves) {
			node_ref child_node = std::make_shared<checkers_tree_node>(node->board, m.first, m.second);
			node->children.push_back(child_node);
			move_piece(child_node->board, m.first, m.second, 'x');
			child_node->score = minimax(child_node, depth-1, false);
			maxEval = std::max(maxEval, child_node->score);
		}		
		return maxEval;
	} else {
		std::vector<std::pair<int, int>> moves = get_possible_moves_for_side(node->board, 'o');
		int minEval = std::numeric_limits<int>::max();
		for(auto &m : moves) {
			node_ref child_node = std::make_shared<checkers_tree_node>(node->board, m.first, m.second);
			node->children.push_back(child_node);
			move_piece(child_node->board, m.first, m.second, 'o');
			child_node->score = minimax(child_node, depth-1, true);
			minEval = std::min(minEval, child_node->score);
		}
		return minEval;	
	}
}

void run_ai(char *board, char side) {
	node_ref root = std::make_shared<checkers_tree_node>(board, side);
	minimax(root, 6, side == 'x');	
	std::sort(root->children.begin(), root->children.end(), [] (node_ref const& a, node_ref const& b) { return a->score > b->score; });
	int move_from, move_to;
	if(side == 'x') {
		move_from = root->children[0]->from;
		move_to = root->children[0]->to;
	} else {
		move_from = root->children[root->children.size()-1]->from;
		move_to = root->children[root->children.size()-1]->to;
	}
	std::cout << "\nAI move:" << board_to_str(move_from, move_to) << std::endl;
	move_piece(board, move_from, move_to ,side);	
}

bool check_game_over(char *board) {
	std::vector<std::pair<int, int>> x_moves = get_possible_moves_for_side(board, 'x');
	std::vector<std::pair<int, int>> o_moves = get_possible_moves_for_side(board, 'o');
	if(x_moves.size() == 0 && o_moves.size() > 0) {
		std::cout << "\nGAME OVER: BLUE WINS!\n\n";
		return true;
	} else if(o_moves.size() == 0 && x_moves.size() > 0) {
		std::cout << "\nGAME OVER: RED WINS!\n\n";
		return true;
	} else if(o_moves.size() == 0 && x_moves.size() == 0) {
		std::cout << "\nGAME OVER: DRAW!\n\n";
		return true;
	}
	return false;
}

int main(int argc, char **argv) {
	std::cout << "\n\n CHECKERS \n\n";
	char board[64];
	int cnt=0;
	for(int i=0; i < 64; i++) {
		if(i % 8 ==0 && i > 0)
			cnt++;
		board[i] = (cnt % 2 == 0) ? '!' : ' ';
		cnt++;
	}
	for(int row=0; row < 3; row++) {
		for(int i= (row % 2 == 0 ? 0 : 1); i < 8; i += 2) {
			board[(row * 8)+(7-i)] = 'x';
			board[((row+5) * 8)+i] = 'o';
		}
	}
	draw_board(board);
	bool ai_turn = false;
	while(!check_game_over(board)) {		
		if(ai_turn) {
			run_ai(board, 'x');
			ai_turn = false;
		} else {
			run_ai(board, 'o');
			ai_turn = true;
			/*
			std::string move;
			std::cout << "\nBlue: Enter move (ex. a5b4) or 'exit'.\n>";
			std::getline(std::cin, move);
			if(move == "exit")
				return 0;
			int from = str_to_board(move.substr(0,2));
			int to = str_to_board(move.substr(2,2));
			if(!check_valid_move(board, from, to, 'o')) {
				std::cout << "Invalid move!\n\n";
			} else {
				move_piece(board, from, to ,'o');
				std::cout << "Player move: " << move << std::endl;
				ai_turn = true;
			}
			*/
		}
		draw_board(board);
	}
	return 0;
}