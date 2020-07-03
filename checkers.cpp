#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <tuple>
#include <stack>
#include <algorithm>
#include <unordered_map>
#include <random>

struct checkers_tree_node;
typedef std::shared_ptr<checkers_tree_node> node_ref;
struct checkers_tree_node {	
	checkers_tree_node(char *src, char side, int from = 0, int to = 0) : score(0), side(side), visited(false), from(from), to(to), parent(nullptr) {
		memcpy(board, src, sizeof(board));
	}
	int from, to, score;
	char board[64];	
	char side;
	bool visited;
	node_ref parent;
	std::vector<node_ref> children;
};

void draw_board(char *board) {
	static std::unordered_map<char, std::string> emoji_map (
    { {'!', "⬜️"}, {' ', "⬛️"}, {'x', "🔴"}, {'o', "🔵"}, {'X', "🍎"}, {'O', "🦋"}});
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

int calc_score(char *board, char side) {
	int score = 0;
	for(int i=0; i < 64; i++) {
		if(side == 'x') {
			if(board[i] == 'x' || board[i] == 'X')
				score += 10;
			if(board[i] == 'o' || board[i] == 'O') 
				score -= 10;			
		} else {
			if(board[i] == 'x' || board[i] == 'X')
				score -= 10;
			if(board[i] == 'o' || board[i] == 'O')
				score += 10;
		}		
	}
	return score;
}

void make_moves(node_ref node, char side, int &moves_left, int levels) {
	if(levels < 0) {
		return;
	}
	std::vector<node_ref> child_nodes;
	for(int i=0; i < 64; i++) {
		if(node->board[i] == side || node->board[i] == side-32) {
			std::vector<int> moves = get_possible_moves(node->board, i);
			for(int m : moves) {
				node_ref child_node = std::make_shared<checkers_tree_node>(node->board, side, i, m);
				move_piece(child_node->board, i, m , side);
				child_node->score = calc_score(child_node->board, side);
				child_node->parent = node;
				node->children.push_back(child_node);
				child_nodes.push_back(child_node);
				moves_left--;				
				if(moves_left < 0) {
					return ;
				}
			}
		}
	}
	auto rng = std::default_random_engine {};
	std::shuffle(std::begin(child_nodes), std::end(child_nodes), rng);
	std::sort(child_nodes.begin(), child_nodes.end(), [] (node_ref const& a, node_ref const& b) { return a->score > b->score; });
	for(auto c : child_nodes)
		make_moves(c, side == 'x' ? 'o' : 'x', moves_left, levels-1);
}

void run_ai(char *board, char side) {
	node_ref root = std::make_shared<checkers_tree_node>(board, side);
	root->score = -999999;
	int moves = 100000;
	make_moves(root, side, moves, 10);
	int lowest_score = -999999;
	node_ref lowest_node = nullptr;
	std::stack <node_ref> node_stack; 
	node_stack.push(root);
	while(node_stack.size() > 0) {
		node_ref node = node_stack.top();
		node_stack.pop();
		if(!node->visited) {
			if(node->score > lowest_score && node->side == side) {				
				lowest_score = node->score;
				lowest_node = node;
			}
			node->visited = true;
			for(auto n : node->children)
				node_stack.push(n);
		}
	}
	int move_from = -1, move_to = -1;	
	while(lowest_node->parent) {
		move_from = lowest_node->from;
		move_to = lowest_node->to;
		lowest_node = lowest_node->parent;
	}
	std::cout << "AI move:" << board_to_str(move_from, move_to) << std::endl;
	move_piece(board, move_from, move_to ,side);
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
	bool done = false;
	bool ai_turn = false;
	while(!done) {		
		if(ai_turn) {
			run_ai(board, 'x');
			ai_turn = false;
		} else {
			std::string move;
			std::cout << "\nEnter move (ex. a5b4) or 'exit'.\n>";
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
		}
		draw_board(board);
	}
	return 0;
}