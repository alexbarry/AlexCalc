package net.alexbarry.calc_android;

import android.util.Log;

import java.util.ArrayList;
import java.util.List;



public class CalcInputHelper {

    private static final String TAG = "CalcInputHelper";

    static class InputToken {
        public final String token;
        // TODO replace "is_unit" with "TokenType"
        public final boolean is_unit;
        public final TokenType type;
        public InputToken(String token, TokenType type, boolean is_unit) {
            this.token = token;
            this.type = type;
            this.is_unit = is_unit;
        }
    }


    private List<InputToken> input_tokens = new ArrayList<>();
    int pos = 0;

    private InputToken get_prev_token() {
        int prev_pos = this.pos - 1;
        if (0 <= prev_pos && prev_pos < input_tokens.size()) {
            return input_tokens.get(prev_pos);
        } else {
            return null;
        }
    }

    public void add_token(TokenType type, String token, boolean is_unit) {
        boolean add_mult = false;
        {
            InputToken prev_token = get_prev_token();
            if (prev_token != null) {
                if (implicit_multiplication(prev_token.type, type)) {
                    add_mult = true;
                }
            }
        }

        // TODO include a setting to choose between one token containing the mult symbol,
        // or two separate tokens.
        if (add_mult) {
            this.input_tokens.add(this.pos, new InputToken("*", TokenType.OP, false));
            this.pos++;
        }
        InputToken inputToken = new InputToken(token, type, is_unit);
        this.input_tokens.add(this.pos, inputToken);
        this.pos++;
    }

    public String get_input_str() {
        StringBuilder builder = new StringBuilder();
        for (InputToken token : input_tokens) {
            builder.append(token.token);
        }
        return builder.toString();
    }

    public void clear() {
        this.input_tokens.clear();
        this.pos = 0;
    }

    /** Deletes token before your cursor */
    public void backspace() {
        if (this.input_tokens.size() == 0) { return; }
        if (this.pos == 0) { return; }
        this.input_tokens.remove(this.pos-1);
        this.pos--;
    }

    /** Like "delete" on a keyboard, it deletes the token after your cursor */
    public void del() {
        if (this.input_tokens.size() == 0) { return; }
        if (this.pos >= this.input_tokens.size()) { return; }
        this.input_tokens.remove(this.pos);
    }

    public void inc_pos(int inc) {
        this.pos += inc;
        if (this.pos <= 0) { this.pos = 0; }
        if (this.pos >= this.input_tokens.size()) {
            this.pos = this.input_tokens.size();
        }
    }

    public void jump_pos_to_back() {
        this.pos = 0;
    }

    public void jump_pos_to_front() {
        this.pos = this.input_tokens.size();
    }

    /** Calculates position of cursor in string output
     * <p>(Since this is internally stored as a list of tokens and an index of that list)
     */
    public int get_input_pos() {
        int string_pos = 0;
        int list_index = 0;
        for (InputToken inputToken : this.input_tokens) {
            if (list_index >= this.pos) {
                break;
            }
            list_index++;
            string_pos += inputToken.token.length();
        }
        return string_pos;
    }

    public List<InputToken> getCurrentInputTokens() {
        return new ArrayList<>(this.input_tokens);
    }

    public void setInputTokens(List<InputToken> inputHistoryEntry) {
        this.input_tokens = new ArrayList<>(inputHistoryEntry);
        this.pos = input_tokens.size();
    }

    public void setRawInputText(String txt, int pos) {
        List<InputToken> inputTokens = new ArrayList<>();
        for (char c : txt.toCharArray()) {
            String charStr = Character.toString(c);
            InputToken inputToken = new InputToken(charStr, TokenType.OTHER,false);
            inputTokens.add(inputToken);
        }
        setInputTokens(inputTokens);
        this.pos = pos;
    }

    // See notes/auto_insert_mult.md
    private static boolean implicit_multiplication(TokenType prev_token_type,
                                                   TokenType new_token_type) {
        Log.v(TAG, String.format("implicit_multiplication check prev=%s, new=%s",
                prev_token_type.name(), new_token_type.name()));
        if (prev_token_type == TokenType.OP || new_token_type == TokenType.OP ||
            prev_token_type == TokenType.OTHER || new_token_type == TokenType.OTHER) {
            Log.d(TAG, "one token is op or other, no implicit mult");
            return false;
        }

        if (prev_token_type == TokenType.PAREN_OPEN ||
            prev_token_type == TokenType.FUNC_CALL ||
            new_token_type == TokenType.PAREN_CLOSE) {
            Log.d(TAG, "prev token is paren open/func call, or new token is close paren, " +
                    "no implicit mult");
            return false;
        }

        if (prev_token_type == TokenType.DIGIT && new_token_type == TokenType.DIGIT) {
            Log.d(TAG, "prev and new token are digits, no mult");
            return false;
        }

        if ( (prev_token_type == TokenType.DIGIT ||
		      prev_token_type == TokenType.VAR ||
		      prev_token_type == TokenType.PAREN_CLOSE) &&
		     (new_token_type == TokenType.DIGIT ||
		      new_token_type == TokenType.FUNC_CALL ||
		      new_token_type == TokenType.VAR ||
		      new_token_type == TokenType.PAREN_OPEN)) {
            Log.d(TAG, "prev token is digit/var/paren close, and new token is " +
                    "digit/func/var, adding mult");
			return true;
		}

        Log.d(TAG, "no match, not adding mult");
		return false;
    }
}
