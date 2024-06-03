package net.alexbarry.calc_android;

import java.util.ArrayList;
import java.util.List;

public class CalcHistoryHelper {

    public enum EntryType {
        NORMAL,
        ERROR_MSG,
    }

    public static class HistoryEntry {
        List<CalcInputHelper.InputToken> inputTokens = new ArrayList<>();
        String tex_input = "";
        String tex_output = "";
        EntryType output_type = EntryType.NORMAL;
    }

    private List<HistoryEntry> inputHistory = new ArrayList<>();
    private HistoryEntry currentEntry = null;
    int historyPos = 0;

    public void addHistoryEntry(HistoryEntry historyEntry) {
        inputHistory.add(historyEntry);
        historyPos++;
    }

    public void setCurrentEntry(HistoryEntry entry) {
        if (entry != null) {
            this.currentEntry = entry;
        } else {
            this.currentEntry = new HistoryEntry();
        }
    }

    public void adjustHistoryPos(int delta) {
        historyPos += delta;
        if (historyPos <= 0) { historyPos = 0; }
        else if (historyPos > inputHistory.size() ) { historyPos = inputHistory.size(); }
    }

    public HistoryEntry getHistoryEntry() {
        if (historyPos == inputHistory.size()) {
            return currentEntry;
        } else if (historyPos < inputHistory.size()){
            return inputHistory.get(historyPos);
        } else {
            throw new RuntimeException(String.format("Unexpected history pos %d for history size %d", historyPos, inputHistory.size()));
        }
    }

    public boolean isTraversingHistory() {
        return historyPos < inputHistory.size();
    }

    public void historyPosReset() {
        historyPos = inputHistory.size();
    }

    public void clearHistory() {
        this.inputHistory.clear();
        this.historyPos = 0;
        this.currentEntry = null;
    }

    public void loadState(List<HistoryEntry> inputs) {
        clearHistory();
        for (HistoryEntry input : inputs) {
            addHistoryEntry(input);
        }
    }

    public List<HistoryEntry> getState() {
        return inputHistory;
    }
}
