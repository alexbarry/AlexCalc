package net.alexbarry.calc_android;

import java.util.ArrayList;
import java.util.List;
import java.util.SortedMap;
import java.util.TreeMap;

public class PersistentState {
    public List<CalcHistoryHelper.HistoryEntry> inputs = new ArrayList<>();
    public List<CalcInputHelper.InputToken> currentInput = new ArrayList<>();
    public SortedMap<String, String> vars   = new TreeMap<>();
    public List<String>              units  = new ArrayList<>();
    public boolean is_polar = false;
    public boolean is_degree = false;

    // These aren't really needed to be stored over app close/opens, but
    // should maybe be saved when the app toggles between horizontal/vertical mode?
    public boolean is_alt = false;
    public boolean is_inv = false;
}
