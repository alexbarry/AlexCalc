package net.alexbarry.calc_android;

import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.List;
import java.util.SortedMap;
import java.util.TreeMap;

public class CalcAndroid {

    private static final String TAG = "CalcAndroid";

    public static final String VAR_NAME_ANS = "ans";

    public static class CalcData {
		public boolean is_degree = false;
		public boolean is_polar = false;
		public SortedMap<String, String> vars;
		public List<String> recentlyUsedUnits;
	}

	public static class Value {
		public final double re;
		public final double im;
		public Value(double re, double im) {
			this.re = re;
			this.im = im;
		}
	}

	public static class UnitInfoFactor {
		public final String unit_base;
		public final int unit_pow;
		public UnitInfoFactor(String unit_base, int unit_pow) {
			this.unit_base = unit_base;
			this.unit_pow = unit_pow;
		}
		public String toString() {
			if (unit_pow == 1) {
				return this.unit_base;
			} else {
				return String.format("%s^%d", unit_base, unit_pow);
			}
		}
	}

	public static class UnitValue {
		private final List<UnitInfoFactor> unitInfoFactors;
		public UnitValue(List<UnitInfoFactor> unitInfoFactors) {
			this.unitInfoFactors = unitInfoFactors;
		}
		public List<UnitInfoFactor> getFactors() {
			return new ArrayList<>(this.unitInfoFactors);
		}

		public String toString() {
			StringBuilder stringBuilder = new StringBuilder();
			boolean first = true;
			for (UnitInfoFactor factor : unitInfoFactors) {
				if (!first) { stringBuilder.append(" "); }
				first = false;
				stringBuilder.append(factor.toString());
			}
			return stringBuilder.toString();
		}
	}

	public static class UnitInfo {
		public final String nice_name;
		private final List<UnitValue> unit_names;
		public final boolean is_si_prefixable;
		public UnitInfo(String nice_name, List<UnitValue> unit_names, boolean is_si_prefixable) {
			this.nice_name = nice_name;
			this.unit_names = unit_names;
			this.is_si_prefixable = is_si_prefixable;
		}

		public List<UnitValue> getUnitNames() {
			return new ArrayList<>(this.unit_names);
		}

		public String getUnitValuesString() {
			String toReturn = "";
			boolean first = true;
			for (UnitValue unitValue :  this.getUnitNames()) {
				if (!first) { toReturn += ", "; }
				first = false;
				toReturn += unitValue.toString();
			}
			return toReturn;
		}

		public String toString() {
			String toReturn = "{";
			toReturn += String.format(" nice_name: \"%s\", ", nice_name);
			toReturn += String.format( "unit_names: [");
			for (UnitValue unitName : unit_names) {
				toReturn += String.format("\"%s\", ", unitName);
			}
			toReturn += String.format("], ");
			toReturn += String.format("is_si_prefixable: %b", is_si_prefixable);
			toReturn += "}";
			return toReturn;
		}
	}

	public static class UnitGroup {
		public final String groupName;
		private final List<UnitInfo> units;
		public UnitGroup(String groupName, List<UnitInfo> units) {
			this.groupName = groupName;
			this.units = units;
		}

		public List<UnitInfo> getUnits() {
			return new ArrayList<>(this.units);
		}

		public String toString() {
			StringBuilder stringBuilder = new StringBuilder();
			stringBuilder.append(String.format("groupName: %s", groupName));
			for (UnitInfo unitInfo : units) {
				stringBuilder.append(unitInfo.toString());
			}
			return stringBuilder.toString();
		}

	}

	static {
		System.loadLibrary("calc_android_jni");
	}

	private boolean polar  = false;
	private boolean degree = false;

	/*
	public static String getUnitInfo() {
		return jniGetUnitInfo();
	}
	 */

	public static List<UnitGroup> getUnitGroups() {
		List<UnitGroup> unitGroups = new ArrayList<>();
		for (int group_idx=0; group_idx<jniGetGroupCount(); group_idx++) {
			String groupName = jniGetUnitGroupName(group_idx);
			List<UnitInfo> units = new ArrayList<UnitInfo>();
			for (int item_idx=0; item_idx<jniGetItemCount(group_idx); item_idx++) {
				String unit_info_json_str = jniGetUnitItemInfoJson(group_idx, item_idx);
				UnitInfo unitInfo;
				try {
					JSONObject unit_info_json = new JSONObject(unit_info_json_str);
					List<UnitValue> unit_names = new ArrayList<>();
					JSONArray unit_names_json_ary = unit_info_json.getJSONArray("unit_names");

					for (int i=0; i<unit_names_json_ary.length(); i++) {
						//unit_names.add(unit_names_json_ary.getString(i));
						JSONArray jsonFactors = unit_names_json_ary.getJSONArray(i);
						List<UnitInfoFactor> factors = new ArrayList<>();
						for (int j=0; j<jsonFactors.length(); j++) {
							String unitBase = jsonFactors.getJSONObject(j).getString("name");
							int pow = jsonFactors.getJSONObject(j).getInt("pow");
							factors.add(new UnitInfoFactor(unitBase, pow));
						}
						unit_names.add(new UnitValue(factors));
					}
					unitInfo = new CalcAndroid.UnitInfo(unit_info_json.getString("nice_name"),
							unit_names,
							unit_info_json.getBoolean("si_prefixable")
							);
				} catch (JSONException ex) {
					Log.e(TAG, String.format("JSONException in group %d item %d, str %s",
							group_idx, item_idx, unit_info_json_str),
							ex);
					continue;
				}
				units.add(unitInfo);
			}
			unitGroups.add(new CalcAndroid.UnitGroup(groupName, units));
		}
		return unitGroups;
	}

	private long calcData_ptr = 0;

	public void init() {
		this.calcData_ptr = this.jniInit();
	}

	public void delete() {
		if (this.calcData_ptr != 0) {
			this.jniDelete(this.calcData_ptr);
			this.calcData_ptr = 0;
		}
	}

	public JSONObject calc(String str_input) throws JSONException  {
		String txt_output = this.jniCalc(this.calcData_ptr, str_input);
		return new JSONObject(txt_output);
	}

	static class ValSpecialInfo {
		public final double val;
		public final boolean handled;
		public ValSpecialInfo(double val) {
			this.val = val;
			this.handled = true;
		}

		public ValSpecialInfo(double val, boolean handled) {
			this.val = val;
			this.handled = handled;
		}
	}

	public static ValSpecialInfo val_str_special_to_val(String val_str) {
		if (val_str.equals("inf")) {
			return new ValSpecialInfo(Double.POSITIVE_INFINITY);
		} else if (val_str.equals("-inf")) {
			return new ValSpecialInfo(Double.NEGATIVE_INFINITY);
		} else if (val_str.toLowerCase().equals("nan")) {
			return new ValSpecialInfo(Double.NaN);
		} else {
			Log.v(TAG, String.format("unhandled val_str \"%s\"", val_str));
			return new ValSpecialInfo(Double.NaN, false);
		}
	}

	private static double get_val_from_str(String val_str) {
		double val;
		ValSpecialInfo info = val_str_special_to_val(val_str);

		if (info.handled) {
			return info.val;
		} else {
			try {
				return Double.parseDouble(val_str);
			} catch (NumberFormatException ex) {
				Log.e(TAG, String.format("could not parse double \"%s\"", val_str));
				return Double.NaN;
			}
		}
	}

	public static Value getValueFromCalcOutput(JSONObject calcOutput) throws JSONException {
		String re_str = calcOutput.getString("re");
		String im_str = calcOutput.getString("im");

		double re = get_val_from_str(re_str);
		double im = get_val_from_str(im_str);
		return new Value(re, im);
	}

	public String to_latex(String str_input) {
		return this.to_latex(str_input, false, 0);
	}

	public String to_latex(String str_input, boolean parse_wip, int cursor_pos) {
		return this.jniToLatex(this.calcData_ptr, str_input, parse_wip, cursor_pos);
	}

	public String format_output(Value val) throws JSONException {
		// TODO add an API specifically for this instead
		JSONObject calcOutput = calc(String.format("%f + i*%f", val.re, val.im));
		return calcOutput.getString("val_str");
	}

	public void setPolar(boolean polar) {
		this.polar = polar;
		this.jniStateSet(calcData_ptr, this.polar, this.degree);
	}

	public void setDegree(boolean degree) {
		this.degree = degree;
		this.jniStateSet(calcData_ptr, this.polar, this.degree);
	}

	private JSONObject getCalcdataJson() throws JSONException {
		String jsonStr = this.jniGetCalcDataJsonStr(calcData_ptr);
		return new JSONObject(jsonStr);
	}

	public CalcData getCalcdata() {
		CalcData calcData = new CalcData();
		try {
			JSONObject calcDataJson = getCalcdataJson();
			calcData.vars = getVarsFromCalcdataJson(calcDataJson);
			calcData.recentlyUsedUnits = getRecentlyUsedUnits();
			calcData.is_polar = calcDataJson.getBoolean("polar");
			calcData.is_degree = calcDataJson.getBoolean("degree");
		} catch (JSONException ex) {
			Log.e(TAG, "JSONException in getCalcData", ex);
		}
		return calcData;
	}

	private static SortedMap<String, String> getVarsFromCalcdataJson(JSONObject calcDataJson) throws JSONException {
		SortedMap<String, String> vars = new TreeMap<>();
		JSONArray varsArray = calcDataJson.getJSONArray("vars");
		for (int i=0; i<varsArray.length(); i++) {
			JSONArray varNameValTuple = varsArray.getJSONArray(i);
			vars.put(varNameValTuple.getString(0), varNameValTuple.getString(1));
		}
		return vars;
	}

	public SortedMap<String, String> getVars() {
		CalcData calcData = getCalcdata();
		return calcData.vars;
	}

	public void addRecentlyUsedUnit(String unit_str) {
		jniAddRecentlyUsedUnit(calcData_ptr, unit_str);
	}

	public List<String> getRecentlyUsedUnits() {
		List<String> units = new ArrayList<>();

		String json_str = jniGetRecentlyUsedUnitsJson(calcData_ptr);
		try {
			JSONObject unitsData = new JSONObject(json_str);
			JSONArray jsonUnitsArray = unitsData.getJSONArray("units");
			for (int i=0; i<jsonUnitsArray.length(); i++) {
				units.add(jsonUnitsArray.getString(i));
			}
			return units;
		} catch (JSONException ex) {
			Log.e(TAG, "JSONException in getRecentlyUsedUnits", ex);
		}
		return null;
	}


	public void setVars(SortedMap<String, String> vars) {
		for (String var_name : vars.keySet()) {
			String var_value = vars.get(var_name);
			String calc_input_str = String.format("%s -> %s", var_value, var_name);
			try {
				JSONObject output = this.calc(calc_input_str);
				int rc = output.getInt("rc");
				if (rc != 0) {
					String err_msg = output.getString("msg");
					Log.e(TAG, String.format("err rc=%d when evaluating %s in calc.setVars, " +
							"msg: %s", rc, calc_input_str, err_msg));
					continue;
				}
			} catch (JSONException e) {
				Log.e(TAG, String.format("JSONException when evaluating %s in calc.setVars", calc_input_str));
				continue;
			}
		}
	}


	public void setRecentlyUsedUnits(List<String> units) {
		// TODO clear any existing recently used units
		for (String unit : units) {
			addRecentlyUsedUnit(unit);
		}
	}

	public void deleteRecentlyUsedUnits() {
		jniDeleteRecentlyUsedUnits(calcData_ptr);
	}

	public void deleteVars() {
		jniDeleteVars(calcData_ptr);
	}

	private native long jniInit();
	private native void jniDelete(long calcData_ptr);
	private native String jniCalc(long calcData_ptr, String str_input);
	private native String jniToLatex(long calcData_ptr, String str_input, boolean parse_wip, int cursor_pos);
	private native String jniStateSet(long calcData_ptr, boolean polar, boolean degree);
	private native String jniGetCalcDataJsonStr(long calcData_ptr);
	private native void jniAddRecentlyUsedUnit(long calcData_ptr, String unit_str);
	private native String jniGetRecentlyUsedUnitsJson(long calcData_ptr);
	private native void jniDeleteRecentlyUsedUnits(long calcData_ptr);
	private native void jniDeleteVars(long calcData_ptr);

	// returning all the units as a big string seems wasteful,
	// allocating such a giant string all at once.
	// instead I made more APIs to get each unit info
	private static native String jniGetUnitInfo();

	private static native int jniGetGroupCount();
	private static native int jniGetItemCount(int group_idx);
	private static native String jniGetUnitGroupName(int group_idx);
	private static native String jniGetUnitItemInfoJson(int group_idx, int item_idx);
}
