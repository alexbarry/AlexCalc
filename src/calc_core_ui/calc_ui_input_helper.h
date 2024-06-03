// TODO at some point I want to move the whole UI layer to C++.
// For now I won't even compile this file...

enum input_token_type {
	INPUT_TOKEN_TYPE_UNIT,
	INPUT_TOKEN_TYPE_OTHER,
};

class InputToken {
	std::string token;
	enum input_token_type type;
};

// TODO ... all I need is to get the last few inputted units.
// It seems like a pain to implement all these APIs just for that. But long term,
// as much of the UI logic as possible should move to the C++ layer.
//
// In the meantime I could simply send one of two events on a button press:
//    - new unit token added (already doing this), and
//    - another button was pressed
// Then I could keep track of unit tokens... ah no, that wouldn't even work,
// because if you press backspace or something then it modifies the current list.

class CalcInputHelper {
	public:
	void add_token(InputToken token);
	std::string get_input_str(void);
	void clear(void);
	void backspace(void);
	void del(void);
	void inc_pos(int inc);
	void jump_pos_to_back(void);
	void jump_pos_to_end(void);
	int get_input_pos(void);
	std::list<InputToken> get_input_tokens(void);
	void set_input_tokens(std::list<InputToken> input_tokens);
	void set_raw_input_text(std::string txt, int pos);

	private:
	std::list<InputToken> input_tokens;
	int pos = 0;
	// TODO I should probably use an iterator here instead of an integer
};
