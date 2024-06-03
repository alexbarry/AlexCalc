#include <string.h>
#include <jni.h>
#include <android/log.h>

#include<string.h>
#include<iostream>
#include<memory>

#include "calc_json.h"

extern "C" {

JNIEXPORT jlong JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniInit( JNIEnv* env, jobject thiz );

JNIEXPORT jstring JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniCalc( JNIEnv* env, jobject thiz, jlong calcData_ptr, jstring str_input_jstring );

JNIEXPORT void JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniDelete( JNIEnv* env, jobject thiz, jlong calcData_ptr );

JNIEXPORT jstring JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniToLatex( JNIEnv* env, jobject thiz, jlong calcData_ptr_long,
                                                         jstring str_input_jstring,
                                                         jboolean parse_wip,
                                                         jint  cursor_pos);

JNIEXPORT void JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniStateSet( JNIEnv* env, jobject thiz, jlong calcData_ptr_long,
                                                         jboolean polar,
                                                         jboolean degree);


JNIEXPORT jstring JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniGetCalcDataJsonStr( JNIEnv* env, jobject thiz, jlong calcData_ptr_long);

JNIEXPORT jstring JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniGetUnitInfo( JNIEnv* env, jclass cls);

JNIEXPORT jint JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniGetGroupCount( JNIEnv* env, jclass cls);
JNIEXPORT jint JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniGetItemCount( JNIEnv* env, jclass cls, jint group_idx);

JNIEXPORT jstring JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniGetUnitGroupName( JNIEnv* env, jclass cls, jint group_idx);

JNIEXPORT jstring JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniGetUnitItemInfoJson( JNIEnv* env, jclass cls, jint group_idx, jint item_idx);
}

void log(std::string str) {
	__android_log_write(ANDROID_LOG_INFO, "calc_android_jni.cpp", str.c_str());
}

JNIEXPORT jlong JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniInit( JNIEnv* env, jobject thiz )
{
	jlong calcData_ptr_long = (jlong)alexcalc_new_calcdata();
	log("calc instance created");
	return calcData_ptr_long;
}

JNIEXPORT void JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniDelete( JNIEnv* env, jobject thiz, jlong calcData_ptr )
{
	alexcalc_free_calcdata((void*)calcData_ptr);
	log("calc instance deleted");
}


JNIEXPORT jstring JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniCalc( JNIEnv* env, jobject thiz, jlong calcData_ptr_long, jstring str_input_jstring )
{
	void *calcData_ptr = (void*)calcData_ptr_long;
	if (calcData_ptr == nullptr) {
		return env->NewStringUTF("Calc not init");
	}


	log("calc getstring");
	const char *cstr = env->GetStringUTFChars(str_input_jstring, NULL);

	std::string str_input(cstr);
	log("got str_input: ");
	log(str_input);


	char str_output[1024*8];
	int rc = alexcalc_json_str_output(cstr, calcData_ptr, str_output, sizeof(str_output));

	if (rc == 0) {
		return env->NewStringUTF(str_output);
	} else {
		// TODO build string from return code
		return env->NewStringUTF("err");
	}
}

JNIEXPORT jstring JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniToLatex( JNIEnv* env, jobject thiz, jlong calcData_ptr_long,
                                                         jstring str_input_jstring,
                                                         jboolean parse_wip,
                                                         jint  cursor_pos) {
	void *calcData_ptr = (void*)calcData_ptr_long;
	if (calcData_ptr == nullptr) {
		return env->NewStringUTF("Calc not init");
	}


	log("calc getstring");
	const char *cstr = env->GetStringUTFChars(str_input_jstring, NULL);

	std::string str_input(cstr);
	log("got str_input: ");
	log(str_input);


	char str_output[1024*16];
	int rc = alexcalc_to_latex(cstr, calcData_ptr, str_output, sizeof(str_output), cursor_pos, parse_wip);

	if (rc == 0) {
		return env->NewStringUTF(str_output);
	} else {
		// TODO build string from return code
		return env->NewStringUTF("err");
	}
}

JNIEXPORT void JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniStateSet( JNIEnv* env, jobject thiz, jlong calcData_ptr_long,
                                                         jboolean polar,
                                                         jboolean degree) {
	void *calcData_ptr = (void*)calcData_ptr_long;
	alexcalc_data_state_set(calcData_ptr, polar, degree);
}

JNIEXPORT jstring JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniGetUnitInfo( JNIEnv* env, jclass cls) {
	char str_output[1024*32];
	alexcalc_get_unit_info_json(str_output, sizeof(str_output));
	return env->NewStringUTF(str_output);
}

JNIEXPORT jstring JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniGetCalcDataJsonStr( JNIEnv* env, jobject thiz, jlong calcData_ptr_long) {
	void *calcData_ptr = (void*)calcData_ptr_long;
	char str_output[32*1024];
	str_output[0] = '\0';
	if (calcData_ptr != nullptr) {
		alexcalc_calcdata_to_json(calcData_ptr, str_output, sizeof(str_output));
	}
	return env->NewStringUTF(str_output);
}

JNIEXPORT jint JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniGetGroupCount( JNIEnv* env, jclass cls) {
	return alexcalc_get_unit_info_group_count();
}

JNIEXPORT jint JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniGetItemCount( JNIEnv* env, jclass cls, jint group_idx) {
	return alexcalc_get_unit_info_group_item_count(group_idx);
}

JNIEXPORT jstring JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniGetUnitGroupName( JNIEnv* env, jclass cls, jint group_idx) {
	char str_output[1024*32];
	alexcalc_get_unit_info_json_group_name(str_output, sizeof(str_output), group_idx);
	return env->NewStringUTF(str_output);
}

JNIEXPORT jstring JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniGetUnitItemInfoJson( JNIEnv* env, jclass cls, jint group_idx, jint item_idx) {
	char str_output[1024*32];
	alexcalc_get_unit_info_json_item(str_output, sizeof(str_output), group_idx, item_idx);
	return env->NewStringUTF(str_output);
}

extern "C"
JNIEXPORT void JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniAddRecentlyUsedUnit(JNIEnv* env,
                                                                    jclass cls,
                                                                    jlong calcData_ptr_long,
                                                                    jstring unit_jstr) {
	void *calcData_ptr = (void*)calcData_ptr_long;
	const char *unit_cstr = env->GetStringUTFChars(unit_jstr, NULL);
	alexcalc_add_recently_used_unit(calcData_ptr, unit_cstr, strlen(unit_cstr));
}

extern "C"
JNIEXPORT jstring JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniGetRecentlyUsedUnitsJson(JNIEnv* env,
                                                                         jclass cls,
                                                                         jlong calcData_ptr_long) {
	void *calcData_ptr = (void*)calcData_ptr_long;
	char str_output[1024*64];

	alexcalc_get_recently_used_units_json(calcData_ptr, (char*)str_output, sizeof(str_output));
	return env->NewStringUTF(str_output);
}

extern "C"
JNIEXPORT void JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniDeleteRecentlyUsedUnits(JNIEnv* env,
                                                                        jclass cls,
                                                                        jlong calcData_ptr_long) {
	void *calcData_ptr = (void*)calcData_ptr_long;
	alexcalc_delete_recently_used_units(calcData_ptr);
}

extern "C"
JNIEXPORT void JNICALL
Java_net_alexbarry_calc_1android_CalcAndroid_jniDeleteVars(JNIEnv* env,
                                                           jclass cls,
                                                           jlong calcData_ptr_long) {
	void *calcData_ptr = (void*)calcData_ptr_long;
	alexcalc_delete_vars(calcData_ptr);
}
