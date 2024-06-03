package net.alexbarry.calc_android;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.provider.BaseColumns;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.SortedMap;
import java.util.TreeMap;

public class PersistentStateDbHelper {

    private static final String TAG = "PersistentStateDbHelper";

    private static final int MAX_HISTORY_ENTRIES = 100;

    private static final String TRUE_STR = "true";
    private static final String FALSE_STR = "false";

    /** Number of history entries read from the database. */
    private static final int NUM_HISTORY_ENTRIES = 10;
    private static final int NUM_RECENT_UNITS = 20;

    static final class DbContract {
        private DbContract() {
        }

        public static class PrefEntry implements BaseColumns {
            public static final String TABLE_NAME = "prefs";
            public static final String COLUMN_NAME_PREF_NAME = "pref";
            public static final String COLUMN_NAME_PREF_VAL = "value";

            public static final String PREF_DEGREE = "is_degree";
            public static final String PREF_POLAR  = "is_polar";
        }

        public static class HistoryEntry implements BaseColumns {
            public static final String TABLE_NAME = "history";

            //public static final String COLUMN_NAME_IDX = "idx";
            // This column will have to be input tokens, also with two extra fields to indicate
            // their type (digit, var, bracket, etc) and if they're a unit or not.
            // It is probably fine to simply store arrays of input tokens as JSON
            public static final String COLUMN_NAME_TXT_INPUT = "txt_input";
            public static final String COLUMN_NAME_TEX_INPUT = "tex_input";
            public static final String COLUMN_NAME_TEX_OUTPUT = "tex_output";
            public static final String COLUMN_NAME_OUTPUT_TYPE = "output_type";
        }

        public static class VarEntry implements BaseColumns {
            public static final String TABLE_NAME = "vars";
            //public static final String COLUMN_NAME_IDX = "idx";
            public static final String COLUMN_NAME_VAR_NAME = "name";
            public static final String COLUMN_NAME_VAR_VAL = "value";
        }

        public static class RecentlyUsedUnitsEntry implements BaseColumns {
            public static final String TABLE_NAME = "recently_used_units";
            //public static final String COLUMN_NAME_IDX = "idx";
            public static final String COLUMN_NAME_UNIT = "unit";
        }

        private static final String SQL_CREATE_PREF_TABLE =
                "CREATE TABLE " + PrefEntry.TABLE_NAME + " (" +
                        PrefEntry._ID + " INTERGER PRIMARY KEY, " +
                        PrefEntry.COLUMN_NAME_PREF_NAME + " TEXT," +
                        PrefEntry.COLUMN_NAME_PREF_VAL + " TEXT)";

        private static final String SQL_CREATE_HISTORY_TABLE =
                "CREATE TABLE " + HistoryEntry.TABLE_NAME + " (" +
                        HistoryEntry._ID + " INTEGER PRIMARY KEY, " +
                        HistoryEntry.COLUMN_NAME_TXT_INPUT + " TEXT," +
                        HistoryEntry.COLUMN_NAME_TEX_INPUT + " TEXT," +
                        HistoryEntry.COLUMN_NAME_TEX_OUTPUT + " TEXT," +
                        HistoryEntry.COLUMN_NAME_OUTPUT_TYPE + " TEXT)";

        private static final String SQL_CREATE_VAR_TABLE =
                "CREATE TABLE " + VarEntry.TABLE_NAME + " (" +
                        VarEntry._ID + " INTEGER PRIMARY KEY, " +
                        VarEntry.COLUMN_NAME_VAR_NAME + " TEXT," +
                        VarEntry.COLUMN_NAME_VAR_VAL + " TEXT)";

        private static final String SQL_CREATE_RECENT_UNITS_TABLE =
                "CREATE TABLE " + RecentlyUsedUnitsEntry.TABLE_NAME + "( " +
                        RecentlyUsedUnitsEntry._ID + " INTEGER PRIMARY KEY, " +
                        RecentlyUsedUnitsEntry.COLUMN_NAME_UNIT + " TEXT)";
    }

    private class DbHelper extends SQLiteOpenHelper {
        public static final int DATABASE_VERSION = 1;
        public static final String DATABASE_NAME = "calc_state.db";

        public DbHelper(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            db.execSQL(DbContract.SQL_CREATE_PREF_TABLE);
            db.execSQL(DbContract.SQL_CREATE_HISTORY_TABLE);
            db.execSQL(DbContract.SQL_CREATE_VAR_TABLE);
            db.execSQL(DbContract.SQL_CREATE_RECENT_UNITS_TABLE);
        }

        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            // TODO
        }

        public void onDowngrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            // TODO
        }
    }

    private final DbHelper dbHelper;


    public PersistentStateDbHelper(Context context) {
        this.dbHelper = new DbHelper(context);
    }



    public void addInput(CalcHistoryHelper.HistoryEntry historyEntry) {

        String inputTokenSerialized;
        try {
            inputTokenSerialized = PersistentStateFileHelper.inputTokensToJsonStr(historyEntry.inputTokens);
        } catch (JSONException ex) {
            Log.e(TAG, String.format("unable to convert to input tokens"), ex);
            return;
        }

        SQLiteDatabase db = dbHelper.getWritableDatabase();

        ContentValues values = new ContentValues();
        values.put(DbContract.HistoryEntry.COLUMN_NAME_TXT_INPUT, inputTokenSerialized);
        values.put(DbContract.HistoryEntry.COLUMN_NAME_TEX_INPUT, historyEntry.tex_input);
        values.put(DbContract.HistoryEntry.COLUMN_NAME_TEX_OUTPUT, historyEntry.tex_output);
        values.put(DbContract.HistoryEntry.COLUMN_NAME_OUTPUT_TYPE, serializeOutputType(historyEntry.output_type));

        long newRowId = db.insert(DbContract.HistoryEntry.TABLE_NAME, null, values);

        clearHistoryEntries(MAX_HISTORY_ENTRIES);
        // TODO should delete anything older than the last 100 entries or something like that
    }

    public void setVar(String var_name, String var_val) {
        SQLiteDatabase db = dbHelper.getWritableDatabase();

        ContentValues values = new ContentValues();
        values.put(DbContract.VarEntry.COLUMN_NAME_VAR_NAME, var_name);
        values.put(DbContract.VarEntry.COLUMN_NAME_VAR_VAL, var_val);

        long newRowId = db.insert(DbContract.VarEntry.TABLE_NAME, null, values);
    }

    public void setVars(SortedMap<String, String> vars) {
        SQLiteDatabase db = dbHelper.getWritableDatabase();

        for (String var_name : vars.keySet()) {
            String var_val = vars.get(var_name);
            ContentValues values = new ContentValues();
            values.put(DbContract.VarEntry.COLUMN_NAME_VAR_NAME, var_name);
            values.put(DbContract.VarEntry.COLUMN_NAME_VAR_VAL, var_val);

            long newRowId = db.insert(DbContract.VarEntry.TABLE_NAME, null, values);
        }
    }

    public void addRecentlyUsedUnit(String unit) {
        SQLiteDatabase db = dbHelper.getWritableDatabase();

        ContentValues values = new ContentValues();
        values.put(DbContract.RecentlyUsedUnitsEntry.COLUMN_NAME_UNIT, unit);

        long newRowId = db.insert(DbContract.RecentlyUsedUnitsEntry.TABLE_NAME, null, values);
    }

    public void setDegree(boolean val) {
        SQLiteDatabase db = dbHelper.getWritableDatabase();
        setPrefBoolean(db, DbContract.PrefEntry.PREF_DEGREE, val);
    }

    public void setPolar(boolean val) {
        SQLiteDatabase db = dbHelper.getWritableDatabase();
        setPrefBoolean(db, DbContract.PrefEntry.PREF_POLAR, val);
    }

    private List<CalcHistoryHelper.HistoryEntry> getInputs(SQLiteDatabase db, int limit) {

        List<CalcHistoryHelper.HistoryEntry> historyEntries = new ArrayList<>();

        String[] projection = {
            BaseColumns._ID,
            DbContract.HistoryEntry.COLUMN_NAME_TXT_INPUT,
            DbContract.HistoryEntry.COLUMN_NAME_TEX_INPUT,
            DbContract.HistoryEntry.COLUMN_NAME_TEX_OUTPUT,
            DbContract.HistoryEntry.COLUMN_NAME_OUTPUT_TYPE,
        };

        Cursor cursor = db.query(
            DbContract.HistoryEntry.TABLE_NAME,
            projection,
            null,
            null,
            null,
            null,
            DbContract.HistoryEntry._ID + " DESC",
            limit < 0 ? null : Integer.toString(limit));

        while (cursor.moveToNext()) {
            String txtInputStr = cursor.getString(1);
            String texInput = cursor.getString(2);
            String texOutput = cursor.getString(3);
            String outputTypeStr = cursor.getString(4);

            CalcHistoryHelper.HistoryEntry entry = new CalcHistoryHelper.HistoryEntry();

            try {
                entry.inputTokens = PersistentStateFileHelper.inputTokensToJsonStr(txtInputStr);
            } catch (JSONException ex) {
                Log.e(TAG, String.format("error parsing json token str \"%s\"", txtInputStr));
                continue;
            }

            entry.tex_input = texInput;
            entry.tex_output = texOutput;
            entry.output_type = deserializeOutputType(outputTypeStr);

            // insert new elements at the front of the array.
            // This is because we want to reverse them in the order they come out of the
            // database.
            // In the database we get sort by descending (newest first) to get the last 10.
            // But when presenting to the user, we want to show the oldest at the top.
            historyEntries.add(0, entry);
            //historyEntries.add(entry);
        }

        return historyEntries;
    }

    private SortedMap<String, String> getVars(SQLiteDatabase db) {
        SortedMap<String, String> vars = new TreeMap<>();

        String[] projection = {
                BaseColumns._ID,
                DbContract.VarEntry.COLUMN_NAME_VAR_NAME,
                DbContract.VarEntry.COLUMN_NAME_VAR_VAL,
        };

        Cursor cursor = db.query(
                DbContract.VarEntry.TABLE_NAME,
                projection,
                null,
                null,
                null,
                null,
                null,
                null);

        while (cursor.moveToNext()) {
            String varName = cursor.getString(1);
            String varValue = cursor.getString(2);

            vars.put(varName, varValue);
        }
        return vars;

    }

    private List<String> getRecentlyUsedUnits(SQLiteDatabase db, int limit) {
        List<String> recentlyUsedUnits = new ArrayList<>();

        String[] projection = {
                BaseColumns._ID,
                DbContract.RecentlyUsedUnitsEntry.COLUMN_NAME_UNIT,
        };

        Cursor cursor = db.query(
                DbContract.RecentlyUsedUnitsEntry.TABLE_NAME,
                projection,
                null,
                null,
                null,
                null,
                DbContract.RecentlyUsedUnitsEntry._ID + " DESC",
                "100");

        Map<String, Boolean> unitSeen = new HashMap<>();
        while (cursor.moveToNext()) {
            String unit = cursor.getString(1);

            if (!unitSeen.containsKey(unit)) {
                recentlyUsedUnits.add(unit);
            }
            unitSeen.put(unit, true);

            if (recentlyUsedUnits.size() >= limit) {
                break;
            }
        }
        return recentlyUsedUnits;
    }

    private void setPref(SQLiteDatabase db, String pref_name, String val) {

        ContentValues values = new ContentValues();
        //values.put(DbContract.PrefEntry.COLUMN_NAME_PREF_NAME, pref_name);
        values.put(DbContract.PrefEntry.COLUMN_NAME_PREF_VAL, val);

        String whereClause = DbContract.PrefEntry.COLUMN_NAME_PREF_NAME + " = ?";
        String[] whereArgs = { pref_name };

        long count = db.update(DbContract.PrefEntry.TABLE_NAME, values, whereClause, whereArgs);

        if (count == 0) {
            values.put(DbContract.PrefEntry.COLUMN_NAME_PREF_NAME, pref_name);
            db.insert(DbContract.PrefEntry.TABLE_NAME, null, values);
        }

    }

    private String getPref(SQLiteDatabase db, String pref_name, String default_val) {
        String[] projection = {
                BaseColumns._ID,
                DbContract.PrefEntry.COLUMN_NAME_PREF_NAME,
                DbContract.PrefEntry.COLUMN_NAME_PREF_VAL,
        };

        String selection = DbContract.PrefEntry.COLUMN_NAME_PREF_NAME + " = ?";
        String[] selectionArgs = { pref_name };

        Cursor cursor = db.query(
                DbContract.PrefEntry.TABLE_NAME,
                projection,
                selection,
                selectionArgs,
                null,
                null,
                null,
                null);

        if (cursor.moveToNext()) {
            String val = cursor.getString(2);
            return val;
        } else {
            return default_val;
        }

    }

    private boolean getPrefBoolean(SQLiteDatabase db, String pref_name, boolean default_val) {
        String val_str = getPref(db, pref_name, null);
        if (val_str == null) { return default_val; }
        else if (val_str.equals(TRUE_STR)) { return true; }
        else if (val_str.equals(FALSE_STR)) { return false; }
        else {
            Log.e(TAG, "unexpected val_str, expected boolean: " + val_str);
            return default_val;
        }
    }

    private void setPrefBoolean(SQLiteDatabase db, String pref_name, boolean val) {
        setPref(db, pref_name, val ? TRUE_STR : FALSE_STR);
    }

    public PersistentState getPersistentState() {
        PersistentState state = new PersistentState();

        SQLiteDatabase db = dbHelper.getReadableDatabase();

        state.inputs = getInputs(db, NUM_HISTORY_ENTRIES);
        state.vars = getVars(db);
        state.units = getRecentlyUsedUnits(db, NUM_RECENT_UNITS);
        state.is_polar = getPrefBoolean(db, DbContract.PrefEntry.PREF_POLAR, false);
        state.is_degree = getPrefBoolean(db, DbContract.PrefEntry.PREF_DEGREE, false);

        Log.d(TAG, String.format("loaded db state, polar:%b, degree:%b",
                state.is_polar, state.is_degree));

        return state;
    }

    private static String serializeOutputType(CalcHistoryHelper.EntryType entryType) {
        return entryType.name();
    }

    private static CalcHistoryHelper.EntryType deserializeOutputType(String entryTypeStr) {
        try {
            return CalcHistoryHelper.EntryType.valueOf(entryTypeStr);
        } catch (IllegalArgumentException ex) {
            Log.w(TAG, String.format("unhandled entryType str \"%s\"", entryTypeStr));
            return CalcHistoryHelper.EntryType.NORMAL;
        }
    }

    private static int getLastHistoryId(SQLiteDatabase db) {
        String[] projection = {
                BaseColumns._ID,
        };

        Cursor cursor = db.query(
                DbContract.HistoryEntry.TABLE_NAME,
                projection,
                null,
                null,
                null,
                null,
                DbContract.HistoryEntry._ID + " DESC",
                "1");

        if (!cursor.moveToNext()) {
            // DB empty
            return -1;
        } else {
            return cursor.getInt(0);
        }
    }

    public void clearHistoryEntries(int historyRowKeepCount) {
        SQLiteDatabase db = dbHelper.getWritableDatabase();

        int lastHistoryId = getLastHistoryId(db);
        int idToDeleteLimit = lastHistoryId - historyRowKeepCount;

        int deletedCount = db.delete(DbContract.HistoryEntry.TABLE_NAME,
                DbContract.HistoryEntry._ID + " < ?",
                new String[] { Integer.toString(idToDeleteLimit)});
        Log.d(TAG, String.format("clearHistoryEntries: lastHistoryId: %d, deleted %d rows",
                lastHistoryId, deletedCount));
    }

    public void deleteVars() {
        SQLiteDatabase db = dbHelper.getWritableDatabase();

        int deletedCount = db.delete(DbContract.VarEntry.TABLE_NAME,
                DbContract.VarEntry.COLUMN_NAME_VAR_NAME + " != ?",
                new String[] { CalcAndroid.VAR_NAME_ANS });

        Log.d(TAG, String.format("deleteVars: deleted %d rows", deletedCount));
    }

    public void deleteRecentlyUsedUnits() {
        SQLiteDatabase db = dbHelper.getWritableDatabase();

        int deletedCount = db.delete(DbContract.RecentlyUsedUnitsEntry.TABLE_NAME,
                null, null);
        Log.d(TAG, String.format("deleteRecentlyUsedUnits: deleted %d rows", deletedCount));
    }
}
