package net.alexbarry.calc_android;

import android.util.JsonReader;
import android.util.JsonWriter;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.List;
import java.util.SortedMap;
import java.util.TreeMap;

/**
 * Reads and writes calculator {@link PersistentState} to/from a JSON file.
 *
 * <p><em>NOTE:</em>On an emulator, this seems to work fine. On my real phone, the
 * activity seems to get killed before it has a chance to flush the file. So I think
 * instead of using this class, I'll try {@link PersistentStateDbHelper}.
 */
public class PersistentStateFileHelper {

    private static final String TAG = "PersistentStateFileH";

    private static final String KEY_IS_DEGREE_BOOL = "is_degree";
    private static final String KEY_IS_POLAR_BOOL  = "is_polar";

    private static final String KEY_INPUT_TOKEN_VAL = "value";
    private static final String KEY_INPUT_TOKEN_TYPE = "type";
    private static final String KEY_INPUT_TOKEN_IS_UNIT = "is_unit";

    private static final String KEY_CURRENT_INPUT_TOKENS = "current_input_tokens";

    private static final String KEY_HISTORY = "history";
    private static final String KEY_HISTORY_INPUT_TOKENS = "input_tokens";
    private static final String KEY_HISTORY_TEX_INPUT = "tex_input";
    private static final String KEY_HISTORY_TEX_OUTPUT = "tex_output";

    private static final String KEY_VARS_ARRAY = "vars";
    private static final String KEY_VAR_NAME = "name";
    private static final String KEY_VAR_VALUE = "value";

    private static final String KEY_RECENTLY_USED_UNITS_ARRAY = "recently_used_units";


    public static PersistentState GetStateFromFile(InputStream in) throws IOException {
        Log.v(TAG, String.format("GetStateFromFile"));
        PersistentState state = new PersistentState();

        JsonReader reader = new JsonReader(new InputStreamReader(in));

        reader.beginObject();
        while (reader.hasNext()) {
            String keyName = reader.nextName();
            Log.v(TAG, String.format("reading key %s", keyName));
            switch (keyName) {
                case KEY_IS_POLAR_BOOL:  state.is_polar  = reader.nextBoolean(); break;
                case KEY_IS_DEGREE_BOOL: state.is_degree = reader.nextBoolean(); break;
                case KEY_CURRENT_INPUT_TOKENS: state.currentInput = getInputTokenAry(reader); break;
                case KEY_VARS_ARRAY:     state.vars      = getJsonVars(reader);  break;
                case KEY_RECENTLY_USED_UNITS_ARRAY: state.units = getStringAry(reader); break;
                case KEY_HISTORY:        state.inputs = getHistoryAry(reader); break;
                default:
                    Log.w(TAG, String.format("unhandled key name: %s", keyName));
                    reader.skipValue();
                    continue;
            }
        }
        reader.endObject();

        return state;
    }

    private static SortedMap<String, String> getJsonVars(JsonReader reader) throws IOException {
        SortedMap<String, String> ary = new TreeMap<>();
        reader.beginArray();
        while (reader.hasNext()) {
            while (reader.hasNext()) {
                reader.beginObject();
                String var_name = null;
                String var_value = null;
                while (reader.hasNext()) {
                    String keyName = reader.nextName();
                    switch (keyName) {
                        case KEY_VAR_NAME:
                            var_name = reader.nextString();
                            break;
                        case KEY_VAR_VALUE:
                            var_value = reader.nextString();
                            break;
                        default:
                            Log.w(TAG, String.format("unexpected key in getJsonVars: %s", keyName));
                            reader.skipValue();
                            reader.endObject();
                            continue;
                    }
                }

                if (var_name == null || var_value == null) {
                    Log.e(TAG, String.format("missing var name/val: name=%s, val=%s", var_name, var_value));
                    reader.endObject();
                    continue;
                }

                ary.put(var_name, var_value);
                reader.endObject();
            }
        }
        reader.endArray();
        return ary;
    }

    private static List<String> getStringAry(JsonReader reader) throws IOException {
        List<String> ary = new ArrayList<>();
        reader.beginArray();
        while (reader.hasNext()) {
            ary.add(reader.nextString());
        }
        reader.endArray();
        return ary;
    }

    private static List<CalcHistoryHelper.HistoryEntry> getHistoryAry(JsonReader reader) throws IOException {
        List<CalcHistoryHelper.HistoryEntry> historyEntries = new ArrayList<>();

        reader.beginArray();
        int i = 0;
        while (reader.hasNext()) {
            Log.v(TAG, String.format("Reading history elem %d", i++));
            CalcHistoryHelper.HistoryEntry historyEntry = new CalcHistoryHelper.HistoryEntry();
            reader.beginObject();
            while (reader.hasNext()) {
                String keyName = reader.nextName();
                Log.v(TAG, String.format("History elem key=%s", keyName));
                switch (keyName) {
                    case KEY_HISTORY_INPUT_TOKENS: historyEntry.inputTokens = getInputTokenAry(reader); break;
                    case KEY_HISTORY_TEX_INPUT: historyEntry.tex_input = reader.nextString(); break;
                    case KEY_HISTORY_TEX_OUTPUT: historyEntry.tex_output = reader.nextString(); break;
                    default:
                        Log.w(TAG, String.format("unexpected key in history entry \"%s\"", keyName));
                        reader.skipValue();
                        continue;
                }
            }
            Log.v(TAG, String.format("Read history elem %s", historyEntry.tex_input));
            historyEntries.add(historyEntry);
            reader.endObject();
        }
        reader.endArray();

        return historyEntries;
    }

    private static List<CalcInputHelper.InputToken> getInputTokenAry(JsonReader reader) throws IOException {
        List<CalcInputHelper.InputToken> inputTokens = new ArrayList<>();
        reader.beginArray();
        while (reader.hasNext()) {
            String token = null;
            TokenType type = null;
            boolean is_unit = false;
            boolean is_unit_set = false;
            reader.beginObject();
            while (reader.hasNext()) {
                String keyName = reader.nextName();
                switch (keyName) {
                    case KEY_INPUT_TOKEN_VAL: token = reader.nextString(); break;
                    case KEY_INPUT_TOKEN_TYPE: type = DeserializeTokenType(reader.nextString()); break;
                    case KEY_INPUT_TOKEN_IS_UNIT:
                        is_unit = reader.nextBoolean();
                        is_unit_set = true;
                        break;
                    default:
                        Log.w(TAG, String.format("unexpected field %s", keyName));
                        reader.skipValue();
                        continue;
                }
            }
            reader.endObject();
            if (token == null || type == null || !is_unit_set) {
                Log.e(TAG, String.format("missing fields token=%s, type=%s, is_unit_set=%b", token, type, is_unit_set));
                continue;
            }
            inputTokens.add(new CalcInputHelper.InputToken(token, type, is_unit));
        }
        reader.endArray();
        return inputTokens;
    }

    private static void WriteInputToken(CalcInputHelper.InputToken token, JsonWriter writer) throws IOException {
        writer.beginObject();
        Log.v(TAG, String.format("writing input token \"%s\"",token.token));
        writer.name(KEY_INPUT_TOKEN_VAL).value(token.token);
        writer.name(KEY_INPUT_TOKEN_TYPE).value(SerializeTokenType(token.type));
        writer.name(KEY_INPUT_TOKEN_IS_UNIT).value(token.is_unit);
        writer.endObject();
    }

    private static void WriteInputTokenAry(List<CalcInputHelper.InputToken> tokens, JsonWriter writer) throws IOException {
        writer.beginArray();
        for (CalcInputHelper.InputToken token : tokens) {
            WriteInputToken(token, writer);
        }
        writer.endArray();
    }

    public static void WriteStateToFile(PersistentState state, OutputStream outputStream) throws IOException {
        JsonWriter writer = new JsonWriter(new OutputStreamWriter(outputStream));

        writer.beginObject();
        writer.name(KEY_IS_DEGREE_BOOL).value(state.is_degree);
        writer.name(KEY_IS_POLAR_BOOL) .value(state.is_polar);

        writer.name(KEY_CURRENT_INPUT_TOKENS);
        WriteInputTokenAry(state.currentInput, writer);

        Log.v(TAG, "writing history entries");
        {
            writer.name(KEY_HISTORY);
            writer.beginArray();
            for (CalcHistoryHelper.HistoryEntry historyEntry : state.inputs) {
                writer.beginObject();
                Log.v(TAG, "writing history entry: " + historyEntry.tex_input);
                writer.name(KEY_HISTORY_INPUT_TOKENS);
                WriteInputTokenAry(historyEntry.inputTokens, writer);
                writer.name(KEY_HISTORY_TEX_INPUT).value(historyEntry.tex_input);
                writer.name(KEY_HISTORY_TEX_OUTPUT).value(historyEntry.tex_output);
                writer.endObject();
            }
            writer.endArray();
        }

        Log.v(TAG, "writing var entries");
        {
            writer.name(KEY_VARS_ARRAY);
            writer.beginArray();
            for (String varName : state.vars.keySet()) {
                writer.beginObject();
                String varValue = state.vars.get(varName);
                writer.name(KEY_VAR_NAME).value(varName);
                writer.name(KEY_VAR_VALUE).value(varValue);
                writer.endObject();
            }
            writer.endArray();
        }

        Log.v(TAG, "writing recently used units entries");
        {
            writer.name(KEY_RECENTLY_USED_UNITS_ARRAY);
            writer.beginArray();
            for (String unit : state.units) {
                writer.value(unit);
            }
            writer.endArray();
        }

        Log.d(TAG, "finished writing to JSON writer, calling last endObject");
        writer.endObject();
        Log.d(TAG, "closing JSON writer");
        writer.close();
        Log.i(TAG, "successfully closed JSON writer when saving state to file");
    }

    private static TokenType DeserializeTokenType(String val) {
        try {
            return TokenType.valueOf(val);
        } catch (IllegalArgumentException ex) {
            Log.w(TAG, String.format("IllegalArgumentException when converting %s to TokenType enum", val), ex);
            return TokenType.OTHER;
        }
        /*
        switch (val) {
            case 1: return TokenType.DIGIT;
            case 2: return TokenType.VAR;
            case 3: return TokenType.FUNC_CALL;
            case 4: return TokenType.PAREN_OPEN;
            case 5: return TokenType.PAREN_CLOSE;
            case 6: return TokenType.OP;
            case 7:
			default: return TokenType.OTHER;
        }*/
    }

    private static String SerializeTokenType(TokenType type) {
        return type.name();
        /*
        switch (type) {
			case DIGIT:       return 1;
			case VAR:         return 2;
			case FUNC_CALL:   return 3;
			case PAREN_OPEN:  return 4;
			case PAREN_CLOSE: return 5;
			case OP:          return 6;
			case OTHER:       return 7;
        }
        return 0;
         */
    }

    public static JSONObject tokenToJsonObject(CalcInputHelper.InputToken token_info) throws JSONException {
        JSONObject elem = new JSONObject();
        elem.put("val", token_info.token);
        elem.put("type", PersistentStateFileHelper.SerializeTokenType(token_info.type));
        elem.put("is_unit", token_info.is_unit);
        return elem;
    }

    public static CalcInputHelper.InputToken jsonObjectToToken(JSONObject obj) throws JSONException {
        String val = obj.getString("val");
        TokenType token_type = PersistentStateFileHelper.DeserializeTokenType(obj.getString("type"));
        boolean is_unit = obj.getBoolean("is_unit");
        return new CalcInputHelper.InputToken(val, token_type, is_unit);
    }

    public static String inputTokensToJsonStr(List<CalcInputHelper.InputToken> tokens) throws JSONException {
        JSONArray inputTokens = new JSONArray();
        for (CalcInputHelper.InputToken token : tokens) {
            JSONObject tokenJson = PersistentStateFileHelper.tokenToJsonObject(token);
            inputTokens.put(tokenJson);
        }
        return inputTokens.toString();
    }

    public static List<CalcInputHelper.InputToken> inputTokensToJsonStr(String str) throws JSONException {
        JSONArray ary = new JSONArray(str);

        List<CalcInputHelper.InputToken> tokens = new ArrayList<>();

        for (int i=0; i<ary.length(); i++) {
            JSONObject obj = ary.getJSONObject(i);
            tokens.add(jsonObjectToToken(obj));
        }
        return tokens;
    }
}
