

#ifndef __CALC_CORE_H__
#define __CALC_CORE_H__

#include<string>
#include<vector>
// For debugging only
#include<unordered_map>

#include "calc_core_types.h"
#include "calc_units.h"
#include "calc_core_exceptions.h"

#define MAX_DECIMAL_PLACES (9)


extern const char VAR_NAME_ANS[];

class NodeOp;
class CalcData;
class UnitInfoInput;
class CursorPosInfo;
class OutputInfo;
class UnitInfoInputAry;

/*
 * Used for input only. Not known at this level what is a prefix and what
 * is the base unit
 *
 * Only for one base unit (optional prefix) and an exponent.
 * Any value can have an array of these as its "units"
 */
class UnitInfoInput {
	public:
	UnitInfoInput(std::string base, int pow);
	std::string to_string(void) const;
	std::string base;
	int         pow;

	// TODO this is kind of ugly... maybe split this class into something separate
	// this should only be set for wip unit tokens
	// if false, convert pow to string and display that
	// if true, ignore pow and use \square
	bool wip_pow = false;
	std::string pow_wip_str;
};


/*
 * Can contain any units attached to a single value, e.g. "km hr^-1"
 * TODO replace most std::vector<UnitInfoInput> with this
 */
class UnitInfoInputAry {
	public:
	std::vector<UnitInfoInput> units;
	std::string to_string(void) const;
};

class Node {

	public:
		static int nodes_allocated;
		static std::unordered_map<Node*, bool> nodes;
		static void print_allocated_nodes(std::ostream &out);

		Node(void);
		virtual ~Node(void);
		virtual std::string to_string(void) const;
		virtual val_t        eval(const CalcData *data);
		virtual node_type    get_node_type(void);
		virtual NodeOp*      promote_to_op(void);
		virtual bool         has_units(void) const;
		virtual UnitInfoInputAry get_units(void) const;
		virtual std::vector<Node*> get_children(void) const;
};
std::ostream& operator<<(std::ostream &out, const Node& n);


class NodeValue : public Node {

	public:
		NodeValue() {};
		virtual ~NodeValue(void) {};
		virtual std::string to_string(void) const = 0;
		val_t        eval(const CalcData *data);
		//val_t        eval(const CalcData *data);
		// special function for NodeValue only, since they can return their value without a full calcData,
		// unlike most nodes
		virtual val_t        get_val(const CalcData *calcData, bool degrees) const = 0;
		node_type    get_node_type(void);
		NodeOp*      promote_to_op(void);

		bool has_units(void) const;
		UnitInfoInputAry get_units(void) const;

		std::vector<UnitInfoInput> input_units;
};

class NodeValueRect : public NodeValue {
	public:
		NodeValueRect(std::string val_str, bool is_imag);
		virtual ~NodeValueRect(void) {};
		virtual std::string to_string(void) const;
		virtual val_t        get_val(const CalcData *calcData, bool degrees) const;


	//private:
		std::string val_str;
		bool        is_imag;
};

// TODO I don't think these can exist without literal angles enabled, right?
class NodeValuePolar : public NodeValue {
	public:
		NodeValuePolar(std::string mag_str, std::string angle_str);
		virtual ~NodeValuePolar(void) {};
		virtual std::string to_string(void) const;
		val_t get_val(const CalcData *calcData, bool degrees) const;
	//private:
		std::string mag_str;
		std::string angle_str;
};

class NodeVar : public Node {
	public:
		NodeVar(std::string var_name);
		~NodeVar(void);
		std::string to_string(void) const;
		val_t        eval(const CalcData *data);
		node_type    get_node_type(void);
		NodeOp*      promote_to_op(void);
	
		std::string var_name;
};

class NodeFunc : public Node {
	public:
		NodeFunc(std::string func_name, std::vector<Node*> args);
		NodeFunc(NodeFunc *arg);
		~NodeFunc(void);
		std::string to_string(void) const;
		val_t        eval(const CalcData *data);
		node_type    get_node_type(void);
		NodeOp*      promote_to_op(void);
	
		std::string func_name;
		std::vector<Node*> args;
};

class NodeWipToken : public Node {
	public:
		NodeWipToken(std::string token, std::string angle, std::vector<UnitInfoInput> wip_units);
		~NodeWipToken(void);
		std::string to_string(void) const;
		val_t        eval(const CalcData *data);
		node_type    get_node_type(void);
		NodeOp*      promote_to_op(void);
		bool has_units(void) const;
		UnitInfoInputAry get_units(void) const;
	
		std::string wip_token;
		std::string wip_angle;
		std::vector<UnitInfoInput> wip_units;
};

class NodeWipBrackets : public Node {
	public:
		NodeWipBrackets(Node* arg, bool right_brack_present);
		~NodeWipBrackets(void);
		std::string to_string(void) const;
		val_t        eval(const CalcData *data);
		node_type    get_node_type(void);
		NodeOp*      promote_to_op(void);
	
		Node* arg;
		bool right_brack_present;
};

class NodeWipFuncCall : public Node {
	public:
		NodeWipFuncCall(NodeFunc* arg, bool right_brack_present);
		~NodeWipFuncCall(void);
		std::string to_string(void) const;
		node_type get_node_type(void);
		NodeFunc* arg;
		bool right_brack_present;
};

class NodeOp : public Node {

	public:
		NodeOp( op_t op, std::vector<Node*> children );
		NodeOp( op_t op );
		~NodeOp(void);
		std::string  to_string(void) const;
		val_t        eval(const CalcData *data);
		node_type    get_node_type(void);
		void         add_child(Node * child_node);
		precedence_t get_precedence(void);
		bool         is_left_associative(void);
		op_t         get_op(void);
		NodeOp*      promote_to_op(void);
		bool         needs_args(void);
		std::vector<Node*> get_children(void) const;
	
		op_t  op;
		std::vector<Node*> children;
};

/**
 * For the case where a value (e.g. complex, rectangular number) is given
 * inside brackets, and the units are defined outside the brackets, e.g.:
 *
 *     (3 + 4i) m
 */
class NodeApplyUnits : public Node {
	public:
	NodeApplyUnits(Node *n, UnitInfoInputAry unit_info);
	virtual ~NodeApplyUnits(void);
	std::string  to_string(void) const;
	val_t        eval(const CalcData *data);
	node_type    get_node_type(void);

	Node *n;
	UnitInfoInputAry unit_info;
};

class CalcData {
	public:
		CalcData(void);
		virtual ~CalcData(void);
		bool  var_is_defined(std::string name) const;
		val_t get_var(std::string name) const;
		void  set_var(std::string name, val_t val);
		void  delete_vars(void);
		void print_vars(void) const;

		std::unordered_map<std::string, val_t>  vars;
		std::unordered_map<std::string, UnitInfoParsed> units;
		bool polar  = false;
		bool degree = false;
};

class InputInfo {
    public:
    Node *n = nullptr;
    bool has_sto = false;
	bool has_to_unit = false;
	Node *cursor_node = nullptr;

	// these two should eventually replace `cursor_offset`
	std::unique_ptr<CursorPosInfo> cursor_pos_info;
	int cursor_pos;

	// TODO remove this
	int  cursor_offset = 0;


    std::string sto_var_name;
	std::vector<UnitInfoInput> to_unit;
    OutputInfo eval(CalcData *calcData);
	const CalcData *calcData;


	bool polar(void) const;
	bool degree(void) const;
};

class OutputInfo {
	public:
	val_t val;
	unit_t unit;
	std::vector<UnitInfoInputAry> units_in_input;
	std::vector<UnitInfoInput> output_unit_str;
	std::string to_string(void) const;
};




std::string unit_info_input_vec_to_string(const std::vector<UnitInfoInput> *unit_input_vec);

class CursorPosInfo {
	public:
	CursorPosInfo(void) = default;
	virtual ~CursorPosInfo() {};
	//public:
	virtual bool contains_pos(int pos) const = 0;
	//virtual std::string to_string(void) const = 0;
};

class UnitCursorPosInfo : public CursorPosInfo {
	//
	//                     second^-1
	//                     ^    ^^^^
	// base_unit_start_pos-+    ||||
	// base_unit_end_pos--------+|||
	// pos_symb_pos--------------+||
	// pow_val_start_pos----------+|
	// pow_val_end_pos-------------+
	public:
	int base_unit_start_pos;
	int base_unit_end_pos;

	bool pow_symbol_present = false;
	int pow_symbol_pos;

	bool pow_val_present = false;
	int pow_val_start_pos;
	int pow_val_end_pos;

	bool contains_pos(int pos) const;
	std::string to_string(void) const;
	int get_last_pos(void) const;
};

class WipValVarTokenCursorPos : public CursorPosInfo {
	public:
	int base_val_start_pos;
	int base_val_end_pos;
	// if literal angles were enabled, this would have extra fields for the angle
	std::vector<UnitCursorPosInfo> unit_info;
	bool base_contains_pos(int pos) const;
	bool contains_pos(int pos) const;
	std::string to_string(void) const;
};



arg_count_t get_l_arg_count( op_t op );
arg_count_t get_r_arg_count( op_t op );

/**
 * Returns true if this operator requires putting its brackets in brackets
 * even if they are the same precedence.
 *
 * e.g. for a - (b + c), even though addition and subtraction have the same
 * precedence, brackets are needed on (b + c).
 */
bool bracks_needed_if_eq_precedence(op_t op);

void rect_to_polar(val_t val, calc_float_t *mag_out, calc_float_t *angle_out);

std::string op_to_string( op_t op );

struct calc_fmt_params get_default_params(void);
calc_flt_fmt_t format_calc_float_params(calc_float_t val, const struct calc_fmt_params &params);
calc_float_t round_to_zero_if_negligible(calc_float_t compared_to, calc_float_t val);

std::string val_to_string(const val_t *val_arg, const struct calc_fmt_params &params,
                          const unit_t *desired_unit = nullptr);
//std::string val_to_string(const val_t *val_arg, const struct calc_fmt_params &params);

/** For a scientific number (e.g. "123.456E-7"), gets the 
 *  magnitude only (e.g. "123.456").
 *  If no exponent (e.g. "9876540000" or "9.87"), returns the entire
 *  value string.
 *
 *  (basically just checks if there is an "E" or "e" and returns
 *  the string to the left of it.)
 */
std::string get_mag_no_exp_str(std::string val_str);

/** For a scientific number, returns the exponent of 10.
 *  If no exponent, returns zero.
 *  e.g. "123.456E-7" will return -7, but for 10000000000 will return 0.
 *
 *  (basically just checks if there is an "E" or "e" and returns the value
 *  to the right of it, converted to an integer)
 */
int get_pow_exp_str(std::string val_str);

#endif
