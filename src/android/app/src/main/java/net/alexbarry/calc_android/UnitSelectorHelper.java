package net.alexbarry.calc_android;

import static net.alexbarry.calc_android.Utils.strAryToStr;

import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ExpandableListAdapter;
import android.widget.ExpandableListView;
import android.widget.SimpleExpandableListAdapter;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.view.ContextThemeWrapper;

import com.google.android.material.dialog.MaterialAlertDialogBuilder;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class UnitSelectorHelper {
    private static final String TAG = "UnitSelectorHelper";

    public interface EventListener {
        public void onInsertUnit(String token);
        public void onClearRecentlyUsedUnits();
    }

    enum WindowType {
        NEW_UNIT,
        RECENT_UNIT,
    }

    private ExpandableListView expandableListView;

    private UnitAdjusterHelper unitAdjusterHelper;
    private final EventListener listener;


    private List<CalcAndroid.UnitGroup> unitGroups;
    private List<String> recently_used_units;
    private Activity activity;
    private TableRow recentlyUsedUnitsSelectedTableRow = null;
    private Button recentlyUsedUnitsInsertBtn;
    private final Map<View, String> tableRowToUnitMap = new HashMap<>();




    private final Map<CalcAndroid.UnitGroup, Boolean> isGroupExpanded = new HashMap<>();
    private final Map<CalcAndroid.UnitGroup, Integer> visibleChildrenCount = new HashMap<>();
    private final List<CalcAndroid.UnitGroup> groupPosToUnitGroup = new ArrayList<>();
    private final List<List<CalcAndroid.UnitInfo>> listPosToUnitInfo = new ArrayList<>();

    private final ExpandableListView.OnGroupClickListener onGroupClickListener = new ExpandableListView.OnGroupClickListener() {
        @Override
        public boolean onGroupClick(ExpandableListView parent, View v, int groupPosition, long id) {
            CalcAndroid.UnitGroup unitGroup = groupPosToUnitGroup.get(groupPosition);
            boolean isExpanded = expandableListView.isGroupExpanded(groupPosition);
            isExpanded = !isExpanded;
            Log.i(TAG, String.format("User clicked on group %d (\"%s\") and it is now %s",
                    groupPosition, unitGroup.groupName, isExpanded ? "expanded" : "collapsed"));
            isGroupExpanded.put(unitGroup, isExpanded);

            return false;
        }
    };

    private final ExpandableListView.OnChildClickListener onChildClickListener = new ExpandableListView.OnChildClickListener() {
        @Override
        public boolean onChildClick(ExpandableListView parent, View v, int groupPosition, int childPosition, long id) {
            CalcAndroid.UnitInfo unitInfo = listPosToUnitInfo.get(groupPosition).get(childPosition);
            onUnitSelected(unitInfo);
            return false;
        }
    };

    private final TextWatcher unitFilterTextWatcher = new TextWatcher() {
        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {}
        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {}
        @Override
        public void afterTextChanged(Editable s) {
            applyUnitFilterText();
        }
    };

    private WindowType activeWindow = WindowType.NEW_UNIT;
    private View unitSelectorView;
    private View unitAdjusterView;
    private Context context;
    private AlertDialog dialog;
    private AlertDialog recently_used_dialog;
    private View recently_used_units_view;
    private EditText unitFilterEditText;
    private LayoutInflater inflater;

    /*
    ExpandableListAdapter adapter = new ExpandableListAdapter() {
        @Override
        public void registerDataSetObserver(DataSetObserver observer) {

        }

        @Override
        public void unregisterDataSetObserver(DataSetObserver observer) {

        }

        @Override
        public int getGroupCount() {
            return unitGroups.size();
        }

        @Override
        public int getChildrenCount(int groupPosition) {
            return unitGroups.get(groupPosition).getUnits().size();
        }

        @Override
        public Object getGroup(int groupPosition) {
            return String.format("group %d", groupPosition);
        }

        @Override
        public Object getChild(int groupPosition, int childPosition) {
            return String.format("group %d item %d", groupPosition, childPosition);
        }

        @Override
        public long getGroupId(int groupPosition) {
            return 0;
        }

        @Override
        public long getChildId(int groupPosition, int childPosition) {
            return 0;
        }

        @Override
        public boolean hasStableIds() {
            return false;
        }

        @Override
        public View getGroupView(int groupPosition, boolean isExpanded, View convertView, ViewGroup parent) {
            TextView tv = new TextView(context);
            tv.setText(unitGroups.get(groupPosition).groupName);
            return tv;
        }

        @Override
        public View getChildView(int groupPosition, int childPosition, boolean isLastChild, View convertView, ViewGroup parent) {
            TextView tv = new TextView(context);
            tv.setText(unitGroups.get(groupPosition).getUnits().get(childPosition).nice_name);
            return tv;
        }

        @Override
        public boolean isChildSelectable(int groupPosition, int childPosition) {
            return true;
        }

        @Override
        public boolean areAllItemsEnabled() {
            return false;
        }

        @Override
        public boolean isEmpty() {
            return false;
        }

        @Override
        public void onGroupExpanded(int groupPosition) {

        }

        @Override
        public void onGroupCollapsed(int groupPosition) {

        }

        @Override
        public long getCombinedChildId(long groupId, long childId) {
            return 0;
        }

        @Override
        public long getCombinedGroupId(long groupId) {
            return 0;
        }
    };
     */

    public UnitSelectorHelper(EventListener listener) {
        this.listener = listener;
    }

    public void init(Activity activity) {
        this.activity = activity;
        this.unitGroups = CalcAndroid.getUnitGroups();
        for (CalcAndroid.UnitGroup group : unitGroups) {
            this.isGroupExpanded.put(group, false);
        }
        this.context = activity.getApplicationContext();

        this.recently_used_dialog = init_recently_used_units(activity);

        //AlertDialog.Builder builder = new AlertDialog.Builder(activity, R.style.FullScreenDialogTheme);
        MaterialAlertDialogBuilder builder = new MaterialAlertDialogBuilder(activity, R.style.MyMaterialAlertDialogBackground);

        this.inflater = activity.getLayoutInflater();
        unitSelectorView = inflater.inflate(R.layout.unit_selector_fragment, null);
        //popupView.setLayoutParams(new ConstraintLayout.LayoutParams(
        //        ConstraintLayout.LayoutParams.MATCH_PARENT,
        //        ConstraintLayout.LayoutParams.MATCH_PARENT));

        unitSelectorView.findViewById(R.id.btn_unit_recently_used).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setActiveWindow(WindowType.RECENT_UNIT);
            }
        });

        this.unitFilterEditText = unitSelectorView.findViewById(R.id.unit_filter);
        this.unitFilterEditText.addTextChangedListener(unitFilterTextWatcher);

        expandableListView =
                unitSelectorView.findViewById(R.id.unit_expandable_list_view);
        expandableListView.setOnGroupClickListener(onGroupClickListener);
        expandableListView.setOnChildClickListener(onChildClickListener);
        expandableListView.setAdapter(createAdapter(activity));

        unitSelectorView.findViewById(R.id.unit_selector_cancel).setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                hide();
            }
        });


        UnitAdjusterHelper.EventListener unitAdjusterHelperEventListener =
                new UnitAdjusterHelper.EventListener() {
            @Override
            public void onBack() {
                setTab(TabType.UNIT_SELECTOR);
            }

            @Override
            public void onInsertUnitPressed(String token) {
                listener.onInsertUnit(token);
                UnitSelectorHelper.this.setTab(TabType.UNIT_SELECTOR);
                UnitSelectorHelper.this.hide();
            }
        };
        unitAdjusterHelper = new UnitAdjusterHelper();
        unitAdjusterView = unitAdjusterHelper.init(activity,
                activity.getLayoutInflater(), unitAdjusterHelperEventListener);


        builder.setView(unitSelectorView);
        this.dialog = builder.create();

    }

    private AlertDialog init_recently_used_units(Activity activity) {
        MaterialAlertDialogBuilder builder = new MaterialAlertDialogBuilder(activity, R.style.MyMaterialAlertDialogBackground);

        LayoutInflater inflater = activity.getLayoutInflater();
        this.recently_used_units_view = inflater.inflate(R.layout.recently_used_units_fragment, null);

        recently_used_units_view.findViewById(R.id.btn_back).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setActiveWindow(WindowType.NEW_UNIT);
            }
        });

        builder.setView(recently_used_units_view);

        recently_used_units_view.findViewById(R.id.btn_delete_recently_used_units).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showDeleteRecentlyUsedUnitsDialog();
            }
        });

        this.recentlyUsedUnitsInsertBtn = recently_used_units_view.findViewById(R.id.btn_insert_recently_used_unit);
        recentlyUsedUnitsInsertBtn.setEnabled(false);
        recentlyUsedUnitsInsertBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                insertSelectedRecentlyUsedUnit();
            }
        });
        return builder.create();
    }

    private void updateRecentlyUsedUnitsUi() {
        TableLayout table = recently_used_units_view.findViewById(R.id.recently_used_units_table);
        table.removeAllViewsInLayout();

        ContextThemeWrapper contextRow = new ContextThemeWrapper(table.getContext(), R.style.ExistingVarsRow);
        ContextThemeWrapper contextColName = new ContextThemeWrapper(table.getContext(), R.style.ExistingVarsColName);

        tableRowToUnitMap.clear();
        for (String unit : recently_used_units) {
            Log.d(TAG, "adding unit: " + unit);

            TableRow tr = new TableRow(contextRow);
            tableRowToUnitMap.put(tr, unit);
            TextView tv = new TextView(contextColName);
            tv.setText(unit);
            tr.addView(tv);
            tr.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    onRecentlyUsedUnitSelected((TableRow)v);
                }
            });
            table.addView(tr);
        }
        //table.invalidate();
        recently_used_units_view.invalidate();
    }

    private void onRecentlyUsedUnitSelected(TableRow tableRowSelected) {
        int unselectedColor = this.activity.getColor(R.color.windowBackground);
        int selectedColor = this.activity.getColor(R.color.btn_important);
        if (recentlyUsedUnitsSelectedTableRow != null) {
            recentlyUsedUnitsSelectedTableRow.setBackgroundColor(unselectedColor);
            recentlyUsedUnitsSelectedTableRow = null;
        }

        tableRowSelected.setBackgroundColor(selectedColor);
        recentlyUsedUnitsSelectedTableRow = tableRowSelected;
        recentlyUsedUnitsInsertBtn.setEnabled(true);
    }

    private void insertSelectedRecentlyUsedUnit() {
        if (recentlyUsedUnitsSelectedTableRow == null) {
            Log.w(TAG, "no row selected");
            return;
        }

        if (!tableRowToUnitMap.containsKey(recentlyUsedUnitsSelectedTableRow)) {
            Log.w(TAG, "view not found in map");
            return;
        }

        String unit = tableRowToUnitMap.get(recentlyUsedUnitsSelectedTableRow);
        listener.onInsertUnit(unit);
        hide();
    }

    private void setActiveWindow(WindowType type) {
        this.activeWindow = type;
        switch(type) {
            case NEW_UNIT:
                dialog.show();
                recently_used_dialog.hide();
                break;
            case RECENT_UNIT:
                recently_used_dialog.show();
                dialog.hide();
                break;
        }
    }

    public void show() {
        setActiveWindow(activeWindow);
    }

    public void hide() {
        dialog.hide();
        recently_used_dialog.hide();
    }

    private void applyUnitFilterText() {
        String filterText = this.unitFilterEditText.getText().toString();
        filterText = filterText.trim().toLowerCase();
        Log.i(TAG, String.format("updating unit filter to \"%s\"", filterText));
        this.expandableListView.setAdapter(createAdapter(activity, filterText));
        this.updateExpandedStatus();
    }

    private void updateExpandedStatus() {
        final int groupCount = expandableListView.getExpandableListAdapter().getGroupCount();
        for (int i=0; i<groupCount; i++) {

            CalcAndroid.UnitGroup unitGroup = groupPosToUnitGroup.get(i);
            if (unitGroup == null) {
                Log.e(TAG, String.format("null unit group for idx %d", i));
                continue;
            }
            boolean expandGroup = false;
            try {
                expandGroup = isGroupExpanded.get(unitGroup);
            } catch (NullPointerException ex) {
                Log.i(TAG, String.format("somehow received NPE for group %s", unitGroup.groupName));
            }

            if (visibleChildrenCount.get(unitGroup) == 1) {
                expandGroup = true;
            } else if (groupCount == 1) {
                expandGroup = true;
            }

            if (expandGroup) {
                expandableListView.expandGroup(i);
            } else {
                expandableListView.collapseGroup(i);
            }
        }
    }

    private boolean unitFilterMatchGroupName(CalcAndroid.UnitGroup unitGroup, String filterExpr) {
        if (filterExpr.length() == 0) { return true; }
        return anyWordStartsWith(unitGroup.groupName.toLowerCase(), filterExpr);
    }

    private boolean anyWordStartsWith(String str, String filterExpr) {
        String[] words = str.split("[()\\/ ]");
        for (String word : words) {
            if (word.startsWith(filterExpr)) { return true; }
        }
        return false;
    }

    private boolean unitFilterMatchUnitInfo(CalcAndroid.UnitInfo unitInfo, String filterExpr) {
        if (filterExpr.length() == 0) { return true; }
        if (anyWordStartsWith(unitInfo.nice_name.toLowerCase(), filterExpr)) {
            return true;
        }
        return false;
    }

    private ExpandableListAdapter createAdapter(Context context) {
        return createAdapter(context, "");
    }

    private ExpandableListAdapter createAdapter(Context context, String filterText) {

        List<Map<String, String>> groupData = new ArrayList<>();
        List<List<Map<String, String>>> childData = new ArrayList<>();
        List<Boolean> expandGroups = new ArrayList<>();

        groupPosToUnitGroup.clear();
        listPosToUnitInfo.clear();

        final String GROUP_NAME_ID = "groupName";
        final String UNIT_NAME_ID = "unitName";
        final String UNIT_VALUE_ID = "unitValue";

        for (int groupIdx = 0; groupIdx<unitGroups.size(); groupIdx++) {
            final CalcAndroid.UnitGroup unitGroup = unitGroups.get(groupIdx);
            Map<String, String> curGroupMap = new HashMap<>();
            curGroupMap.put(GROUP_NAME_ID, unitGroup.groupName);

            boolean showGroup = false;
            boolean showAllChildren = false;
            if (unitFilterMatchGroupName(unitGroup, filterText)) {
                showGroup = true;
                showAllChildren = true;
            } else {
                for (CalcAndroid.UnitInfo unitInfo : unitGroup.getUnits()) {
                    if (unitFilterMatchUnitInfo(unitInfo, filterText)) {
                        showGroup = true;
                        break;
                    }
                }
            }

            if (!showGroup) {
                continue;
            }

            groupPosToUnitGroup.add(unitGroup);
            List<CalcAndroid.UnitInfo> groupUnitInfoVisible = new ArrayList<>();
            listPosToUnitInfo.add(groupUnitInfoVisible);

            int visibleChildrenCount = 0;
            List<Map<String, String>> children = new ArrayList<>();
            for (int childIdx=0; childIdx<unitGroup.getUnits().size(); childIdx++) {
                final CalcAndroid.UnitInfo unitInfo = unitGroup.getUnits().get(childIdx);
                if (!showAllChildren && !unitFilterMatchUnitInfo(unitInfo, filterText)) {
                    continue;
                }
                groupUnitInfoVisible.add(unitInfo);
                visibleChildrenCount++;
                Map<String, String> curChildMap = new HashMap<>();
                children.add(curChildMap);
                curChildMap.put(UNIT_NAME_ID, unitInfo.nice_name);
                curChildMap.put(UNIT_VALUE_ID, unitInfo.getUnitValuesString());
            }
            this.visibleChildrenCount.put(unitGroup, visibleChildrenCount);


            groupData.add(curGroupMap);
            childData.add(children);
        }

        String[] groupFrom = {GROUP_NAME_ID};
        int[] groupTo = {R.id.unitGroupName};
        String[] childFrom = {UNIT_NAME_ID, UNIT_VALUE_ID};
        int[] childTo = {R.id.unitName, R.id.unitValues};


        int groupLayout = R.layout.unit_selector_group_fragment;
        int childLayout = R.layout.unit_selector_unit_item_fragment;
        return new SimpleExpandableListAdapter(
                context, groupData, groupLayout, groupFrom, groupTo,
                childData, childLayout, childFrom, childTo);
    }

    private void onUnitSelected(CalcAndroid.UnitInfo unitInfo) {
        Log.d(TAG, String.format("user selected unit %s", unitInfo));
        setTab(TabType.UNIT_ADJUSTOR);
        unitAdjusterHelper.setUnitInfo(unitInfo);
    }

    enum TabType {
        UNIT_SELECTOR,
        UNIT_ADJUSTOR,
    }

    private void setTab(TabType tabType) {
        {
            ViewGroup viewGroup = (ViewGroup) unitSelectorView.getParent();
            if (viewGroup != null) {
                viewGroup.removeView(unitSelectorView);
            }
        }

        {
            ViewGroup viewGroup = (ViewGroup) unitAdjusterView.getParent();
            if (viewGroup != null) {
                viewGroup.removeView(unitAdjusterView);
            }
        }

        switch(tabType) {
            case UNIT_ADJUSTOR: dialog.setContentView(unitAdjusterView); break;
            case UNIT_SELECTOR: dialog.setContentView(unitSelectorView); break;
        }
    }

    public void updateRecentlyUsedUnits(List<String> recentlyUsedUnits) {
        Log.d(TAG, String.format("updateRecentlyUsedUnits: %s", strAryToStr(recentlyUsedUnits)));
        Log.i(TAG, "updating recently used units...");
        this.recently_used_units = recentlyUsedUnits;
        // TODO it may be more efficient to do this only when the UI is shown
        updateRecentlyUsedUnitsUi();
    }

    private void showDeleteRecentlyUsedUnitsDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this.activity);
        builder.setTitle(activity.getString(R.string.delete_units_popup_title));
        builder.setMessage(activity.getString(R.string.delete_units_popup_msg));
        builder.setPositiveButton(R.string.yes, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                listener.onClearRecentlyUsedUnits();
            }
        });
        builder.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
            }
        });
        builder.show();
    }
}
