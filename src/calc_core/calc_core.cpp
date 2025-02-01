# include<assert.h>

#include<iostream>
#include<string>
#include<list>
#include<sstream>
#include<cstdlib>
#include<unordered_map>
#include<vector>
#include<cmath>
//#include<complex>

#include "calc_core.h"
#include "calc_units.h"
#include "calc_core_exceptions.h"


// If sin/cos/tan return values less in magnitude than this,
// then round them to zero.
const calc_float_t TRIG_NEGLIGIBLE_VAL = 1e-15;

typedef val_t (*calc_func_t)(val_t, bool degree);

// TODO split into separate file, "unit_input"?
unit_t eval_units(const CalcData *calcData, std::vector<UnitInfoInput> input_units);

#define ALEXCALC_HELLO_INFO ("\nAlexCalc created by Alex Barry (alexbarry.dev2@gmail.com). See https://github.com/alexbarry/AlexCalc . " ALEXCALC_BUILD_INFO)

const char alexcalc_hello_info[] = ALEXCALC_HELLO_INFO;
const char alexcalc_build_info[] = ALEXCALC_BUILD_INFO;

extern "C"
const char *alexcalc_info_func(void) {
	(void)alexcalc_hello_info;
	return alexcalc_build_info;
}

const char VAR_NAME_ANS[] = "ans";

#if 0
calc_func_t builtin_log10;
calc_func_t builtin_ln;
calc_func_t builtin_sqrt;
calc_func_t builtin_sin;
calc_func_t builtin_cos;
calc_func_t builtin_tan;
calc_func_t builtin_asin;
calc_func_t builtin_acos;
calc_func_t builtin_atan;
#endif

#if 1
val_t builtin_log10(val_t, bool);
val_t builtin_ln(val_t, bool);
val_t builtin_sqrt(val_t, bool);
val_t builtin_sin(val_t, bool);
val_t builtin_cos(val_t, bool);
val_t builtin_tan(val_t, bool);
val_t builtin_asin(val_t, bool);
val_t builtin_acos(val_t, bool);
val_t builtin_atan(val_t, bool);
val_t builtin_get_real(val_t, bool);
val_t builtin_get_imag(val_t, bool);
val_t builtin_get_abs(val_t, bool);
val_t builtin_get_angle(val_t, bool);
#endif

static const std::unordered_map<std::string, val_t> CONSTANTS = std::unordered_map<std::string, val_t> {
	//{"pi", { .re = M_PI, .im = 0.0} },
	{"pi", { .re = 3.1415926535897932384626433L, .im = 0.0} },
	//{"pi", { .re = std::numbers::pi, .im = 0.0} },
	//{"e" , { .re = M_E,  .im = 0.0} },
	{"e" , { .re = 2.71828182845904523536028747135266249775724709369995L,  .im = 0.0} },
	//{"e" , { .re = std::numbers::e,  .im = 0.0} },
	{"i" , { .re = 0,    .im = 1.0} },
	{"j" , { .re = 0,    .im = 1.0} },
};


struct optional_constant_info {
	val_t val;
	const char *description;
};


#if 0
// These shouldn't be defined by default, since the chocie of letters can varry and
// mean other things sometimes.
// If the user references one of these and it's not defined already,
// maybe search this list and allow them to select one of these options, or define their own
static const std::unordered_map<std::string, std::list<struct optional_constant_info>> OPTIONAL_CONSTANTS =
	std::unordered_map<std::string, std::list<struct optional_constant_info>> {
	{"k", /* std::list<struct optional_constant_info>() */ {
		{ .val = { .re = 8.9875517923e9L, .im = 0.0 }, .description = "Coulomb's constant kg m^3 s^-2 C^-2"},
	}},
	{"G", /* std::list<struct optional_constant_info>() */ {
		{ .val = { .re = 6.67430e-11, .im = 0.0 }, .description = "Gravitational constant (m^3 kg^-1 s^-2)",}
	}},
	{"R", /* std::list<struct optional_constant_info>() */ {
		{ .val = { .re = 8.31446261815324, .im = 0.0 }, .description = "Gas constant (J K^-1 mol^-1) = (L kPa K^-1 mol^-1)",}
	}},
	{"c", {
		{ .val = { .re = 299792458, .im = 0.0 }, .description = "Speed of light in vacuum (m/s)",},
	}},
	{"q_e", {
		{ .val = { .re = 1.602e-19, .im = 0.0 }, .description = "Elementary charge (C)",},
	}},
	{"h", {
		{ .val = { .re = 6.626e-34, .im = 0.0 }, .description = "Planck constant (J s)",},
		{ .val = { .re = 4.135667e-15, .im = 0.0 }, .description = "Planck constant (eV s)",},
	}},
	// TODO add other important ones:
	// - mu_o magnetic constant
	// - epsilon_o electric constant
	// - Z_o characteristic impedance of vacuum
};
#endif

static const std::unordered_map<std::string, calc_func_t> CONSTANT_FUNCS = std::unordered_map<std::string, calc_func_t> {
	{"log",   builtin_log10},
	{"ln",    builtin_ln},
	{"sqrt",  builtin_sqrt},
	{"sin",   builtin_sin},
	{"cos",   builtin_cos},
	{"tan",   builtin_tan},
	{"asin",   builtin_asin},
	{"acos",   builtin_acos},
	{"atan",   builtin_atan},
	{"real",   builtin_get_real},
	{"re",     builtin_get_real},
	{"imag",   builtin_get_imag},
	{"im",     builtin_get_imag},
	{"abs",    builtin_get_abs},
	{"mag",    builtin_get_abs},
	// dammit. Calling this just "angle" would conflict with the argument
	// called "angle".
	{"getangle", builtin_get_angle},
	// TODO add hyperbolic, arcsin/cos/tan, and arcsinh
};


int Node::nodes_allocated = 0;
std::unordered_map<Node*, bool> Node::nodes;


// The "degree" arugment should only be used for trig functions, I think
// it's not meant to convert all angles from imaginary numbers to degrees
// Imaginary numbers' angles will be converted to degrees at the last step
// before displaying, if the user has selected degrees (and polar)

void rect_to_polar(val_t val, calc_float_t *mag_out, calc_float_t *angle_out) {
	*mag_out = std::sqrt(std::pow(val.re,2) + std::pow(val.im,2));
	*angle_out = std::atan2(val.im, val.re);
}

void polar_to_rect(calc_float_t mag, calc_float_t angle, val_t *val_out) {
	val_out->re = mag*std::cos(angle);
	val_out->im = mag*std::sin(angle);
}

static calc_float_t round_to_zero_if_small(calc_float_t arg) {
	if (std::abs(arg) < TRIG_NEGLIGIBLE_VAL) {
		return 0.0;
	} else {
		return arg;
	}
}

val_t builtin_log10(val_t arg, bool degree) {
	static const std::string name = "log(x)";
	if (units_non_zero(arg.unit_dim)) {
		throw InvalidInputException(std::string("func " + name + " called with units in argument"), 0);
	}

	(void)degree;
	calc_float_t mag, angle;
	rect_to_polar(arg, &mag, &angle);
	/* TODO allow for a setting to enable this behavior or not
	if (mag == 0) {
		throw InvalidInputException("can not evaluate log(0)");
	}
	*/
	// TODO technically should the imginary part be converted
	// to degrees if degree is set?
	return { .re = std::log10(mag), .im = angle, .unit_dim = init_unit_dim() };
}

val_t builtin_ln(val_t arg, bool degree) {
	static const std::string name = "ln(x)";
	if (units_non_zero(arg.unit_dim)) {
		throw InvalidInputException(std::string("func " + name + " called with units in argument"), 0);
	}

	(void)degree;
	calc_float_t mag, angle;
	rect_to_polar(arg, &mag, &angle);
	/* TODO allow for a setting to enable this behavior or not
	if (mag == 0) {
		throw InvalidInputException("can not evaluate ln(0)");
	}
	*/
	return { .re = std::log(mag), .im = angle, .unit_dim = init_unit_dim() };
}

#if 0
std::string val_to_str(val_t val) {
	char buff[256];
	snprintf(buff, sizeof(buff), "{re: %" CALC_FLOAT_FMT "e, im: %" CALC_FLOAT_FMT "e}", val.re, val.im);
	return std::string(buff);
}
#endif

val_t builtin_sqrt(val_t arg, bool degree) {
	(void)degree;

	//return std::sqrt(arg);
	calc_float_t mag, angle;
	rect_to_polar(arg, &mag, &angle);

	mag = std::sqrt(mag);
	angle /= 2;

	// take the positive square root
	if (angle < -M_PI/2 || angle > M_PI/2) {
		angle += M_PI;
	}

	val_t result;
	polar_to_rect(mag, angle, &result);
	result.unit_dim = sqrt_unit(arg.unit_dim);
	return result;
}

val_t re_func(calc_float_t (*func)(calc_float_t), val_t arg, std::string name) {
	if (arg.im != 0) {
		throw InvalidInputException(std::string("func ") + name + " called with imaginary component", 0);
	}
	if (units_non_zero(arg.unit_dim)) {
		throw InvalidInputException(std::string("func " + name + " called with units in argument"), 0);
	}
	return { .re = func(arg.re), .im = 0, .unit_dim = init_unit_dim() };
}

val_t builtin_sin(val_t arg, bool degree) {
	if (degree) {
		arg.re *= M_PI/180;
	}
	val_t val = re_func(std::sin, arg, "sin(x)");
	val.re = round_to_zero_if_small(val.re);
	return val;
}

val_t builtin_cos(val_t arg, bool degree) {
	if (degree) {
		arg.re *= M_PI/180;
	}
	val_t val = re_func(std::cos, arg, "cos(x)");
	val.re = round_to_zero_if_small(val.re);
	return val;
}
val_t builtin_tan(val_t arg, bool degree) {
	if (degree) {
		arg.re *= M_PI/180;
	}
	val_t val = re_func(std::tan, arg, "tan(x)");
	val.re = round_to_zero_if_small(val.re);
	return val;
	
}

val_t builtin_asin(val_t arg, bool degree) {
	val_t output = re_func(std::asin, arg, "asin(x)");
	if (degree) {
		output.re *= 180/M_PI;
	}
	return output;
}

val_t builtin_acos(val_t arg, bool degree) {
	val_t output = re_func(std::acos, arg, "acos(x)");
	if (degree) {
		output.re *= 180/M_PI;
	}
	return output;
}

val_t builtin_atan(val_t arg, bool degree) {
	val_t output = re_func(std::atan, arg, "atan(x)");
	if (degree) {
		output.re *= 180/M_PI;
	}
	return output;
}

val_t add_vals( std::vector<val_t> vals ) {

	if( vals.size() == 0 ) {
		throw InvalidArgCountException( OP_ADD, vals.size() );
	}

	val_t result = {
		.re = 0.0,
		.im = 0.0,
		.unit_dim = vals[0].unit_dim,
	};

	for( uint32_t i=0; i<vals.size(); i++ )
	{
		if (!units_dim_eq(result.unit_dim, vals[i].unit_dim)) {
			throw UnitMismatchException(OP_ADD, result.unit_dim, vals[i].unit_dim);
		}
		result.re += vals[i].re;
		result.im += vals[i].im;
	}
	return result;
}

val_t sub_vals( std::vector<val_t> vals ) {

	if( vals.size() != 2 ) {
		throw InvalidArgCountException( OP_SUB, vals.size() );
	}

	if (!units_dim_eq(vals[0].unit_dim, vals[1].unit_dim)) {
			throw UnitMismatchException(OP_SUB, vals[0].unit_dim, vals[1].unit_dim);
	}

	val_t result = {
		.re = 0.0,
		.im = 0.0,
		.unit_dim = vals[0].unit_dim,
	};
	result.re = vals[0].re - vals[1].re;
	result.im = vals[0].im - vals[1].im;
	return result;
}


val_t mult_vals( std::vector<val_t> vals ) {

	if( vals.size() == 0 ) {
		throw InvalidArgCountException( OP_MULT, vals.size() );
	}

	// Optimize if only two args (only possible number right now), and
	// if both are purely real or purely imaginary.
	// Instead of converting to polar and back, just operate on the real/im parts
	// directly.
	if (vals.size() == 2) {
		// multiplying two purely real numbers results
		// in a purely real number 
		if (vals[0].im == 0 && vals[1].im == 0) {
			val_t result;
			result.re = vals[0].re * vals[1].re;
			result.im = 0;
			result.unit_dim = mult_units_dim(vals[0].unit_dim, vals[1].unit_dim);
			return result;
		// multiplying two purely imaginary numbers results
		// in a purely real number, equal to (i*i) * im(a) * im(b),
		// which is -im(a)*im(b)
		} else if (vals[0].re == 0 && vals[1].re == 0) {
			val_t result;
			result.re = -1 * vals[0].im * vals[1].im;
			result.im = 0;
			result.unit_dim = mult_units_dim(vals[0].unit_dim, vals[1].unit_dim);
			return result;
		}
		// otherwise, fall through
	}

	calc_float_t result_mag   = 1.0;
	calc_float_t result_angle = 0.0;
	unit_dim_t   result_unit_dim = init_unit_dim();
	for( uint32_t i=0; i<vals.size(); i++ )
	{
		calc_float_t this_mag, this_angle;
		rect_to_polar(vals[i], &this_mag, &this_angle);
		result_mag   *= this_mag;
		result_angle += this_angle;
		result_unit_dim = mult_units_dim(result_unit_dim, vals[i].unit_dim);
	}
	val_t result;
	polar_to_rect(result_mag, result_angle, &result);
	result.unit_dim = result_unit_dim;
	return result;
}

val_t div_vals( std::vector<val_t> vals ) {

	if( vals.size() != 2 ) {
		throw InvalidArgCountException( OP_DIV, vals.size() );
	}

	val_t result;

	if (vals[0].im == 0 && vals[1].im == 0) {
		result.re = vals[0].re / vals[1].re;
		result.im = 0;
	} else if (vals[0].re == 0 && vals[1].re == 0) {
		result.re = vals[0].im / vals[1].im;
		result.im = 0;
	} else {
		calc_float_t result_mag;
		calc_float_t result_angle;
		rect_to_polar(vals[0], &result_mag, &result_angle);

		calc_float_t r_mag, r_angle;
		rect_to_polar(vals[1], &r_mag, &r_angle);

		result_mag   /= r_mag;
		result_angle -= r_angle;
		polar_to_rect(result_mag, result_angle, &result);
	}

	result.unit_dim = div_units_dim(vals[0].unit_dim, vals[1].unit_dim);
	return result;

}

val_t pow_vals( std::vector<val_t> vals ) {

	if( vals.size() != 2 ) {
		throw InvalidArgCountException( OP_POW, vals.size() );
	}

	// base: A exp(j*theta)
	// pow : a + j*b

	calc_float_t A, theta;
	rect_to_polar(vals[0], &A, &theta);
	unit_dim_t base_unit_dim = vals[0].unit_dim;

	calc_float_t arg_mag, arg_angle;
	rect_to_polar(vals[1], &arg_mag, &arg_angle);
	unit_dim_t arg_pow_unit_dim = vals[1].unit_dim;

	calc_float_t a = vals[1].re;
	calc_float_t b = vals[1].im;

#if 0
	std::complex<calc_float_t> base( vals[0].re, vals[0].im);
	std::complex<calc_float_t> power(vals[1].re, vals[1].im);

	std::complex<calc_float_t> result = std::pow(base, power);
	
	return { .re = result.real(), .im = result.imag() };
#endif

	val_t result;
	result.unit_dim = init_unit_dim();
	bool ignore_units = false;

	if (A == 0.0) {
		if (arg_mag == 0) {
        	// TODO: 0^0 == 1 warning
			result.re= 1.0;
			result.im= 0.0;
			ignore_units = true;
		} else {
			result.re= 0.0;
			result.im= 0.0;
			ignore_units = true;
		}
	} else if (theta == 0.0 && b == 0.0) {
		result.re = std::pow(A,a);
		result.im = 0.0;
	} else {
            calc_float_t mag = std::pow(M_E, -theta*b + a * std::log(A) );
            calc_float_t ang = b*std::log(A) + a*theta;
			val_t tmp_result;
			polar_to_rect(mag, ang, &tmp_result);
			result.re = tmp_result.re;
			result.im = tmp_result.im;
	}

	if (!ignore_units) {
		if (units_non_zero(arg_pow_unit_dim)) {
			throw UnitInvalidOperationException("can not use a value with units as an exponent");
		} else if (!units_non_zero(base_unit_dim)) {
			// then do nothing
		} else if (vals[1].im != 0.0) {
			throw UnitInvalidOperationException("can not raise value with units to an exponent with an imaginary component");
			
		} else {
			int pow_int = (int)vals[1].re;
			if (pow_int != vals[1].re) {
				// TODO I guess you can... but only square root, cube root...?
				throw UnitInvalidOperationException("can not raise value with units to a non integer exponent");
			}
			result.unit_dim = pow_unit_dim(base_unit_dim, pow_int);
		}
	}

	return result;
}

val_t angle_op_func_deg_arg(std::vector<val_t> vals, bool degrees) {

	if (vals.size() != 2) {
		throw InvalidArgCountException( OP_ANGLE, vals.size() );
	}

	if (units_non_zero(vals[1].unit_dim)) {
		throw UnitInvalidOperationException("can not use a value with units as an angle (for a complex number)");
	}

	val_t base_cmplx  = vals.at(0);
	val_t angle_cmplx = vals.at(1);

	// TODO round this down...
#warning "need to round this down or will be really annoying if anyone actually tries to use this"
	if (base_cmplx.im != 0 || angle_cmplx.im != 0) {
		throw InvalidInputException("base and angle must have no imaginary component", 0);
	}

	calc_float_t mag   = base_cmplx.re;
	calc_float_t angle = angle_cmplx.re;

	if (degrees) {
		angle *= M_PI/180;
	}

	val_t result;
	result.re = mag * std::cos(angle);
	result.im = mag * std::sin(angle);
	result.unit_dim = base_cmplx.unit_dim;

	return result;
}

val_t angle_op_func_rad(std::vector<val_t> vals)    { return angle_op_func_deg_arg(vals, false); }
val_t angle_op_func_degree(std::vector<val_t> vals) { return angle_op_func_deg_arg(vals, true);  }

val_t neg_vals( std::vector<val_t> vals ) {

	if( vals.size() != 1 ) {
		throw InvalidArgCountException( OP_NEG, vals.size() );
	}

	if (vals[0].im == 0) {
		val_t result;
		result.re = -vals[0].re;
		result.im = 0;
		result.unit_dim = vals[0].unit_dim;
		return result;
	} else if (vals[0].re == 0) {
		val_t result;
		result.re = 0;
		result.im = -vals[0].im;
		result.unit_dim = vals[0].unit_dim;
		return result;
	} else {
		calc_float_t result_mag;
		calc_float_t result_angle;
		rect_to_polar(vals[0], &result_mag, &result_angle);
		result_mag = -result_mag;
	
		val_t result;
		polar_to_rect(result_mag, result_angle, &result);
		result.unit_dim = vals[0].unit_dim;
		return result;
	}

}


val_t builtin_get_real(val_t arg, bool degrees) {
	(void)degrees;
	return { .re = arg.re, .im = 0, .unit_dim = arg.unit_dim };
}

val_t builtin_get_imag(val_t arg, bool degrees) {
	(void)degrees;
	return { .re = arg.im, .im = 0, .unit_dim = arg.unit_dim };
}

val_t builtin_get_abs(val_t arg, bool degrees)  {
	(void)degrees;
	calc_float_t result_mag = 0;
	calc_float_t result_angle = 0;
	rect_to_polar(arg, &result_mag, &result_angle);
	return { .re = result_mag, .im = 0, .unit_dim = arg.unit_dim };
}

val_t builtin_get_angle(val_t arg, bool degrees) {
	calc_float_t result_mag = 0;
	calc_float_t result_angle = 0;
	rect_to_polar(arg, &result_mag, &result_angle);
	if (degrees) {
		result_angle *= 180/M_PI;
	}
	return { .re = result_angle, .im = 0, .unit_dim = init_unit_dim() };
}


std::string op_to_string( op_t op )
{
	switch(op)
	{
		case OP_ADD:  return "+";
		case OP_SUB:  return "-";
		case OP_MULT: return "*";
		case OP_DIV:  return "/";
		case OP_NEG:  return "neg";
		case OP_POW:  return "^";
		case OP_ANGLE: return "angle";

		default:
			return "unknown";
	}
}

precedence_t op_to_precedence( op_t op )
{
	switch( op )
	{
		case OP_ADD: return PRECEDENCE_ADD;

		case OP_SUB: return PRECEDENCE_SUB;

		case OP_NEG: return PRECEDENCE_NEG;

		case OP_MULT:
		case OP_DIV: return PRECEDENCE_MULT_DIV;

		case OP_POW: return PRECEDENCE_POW;

		case OP_ANGLE: return PRECEDENCE_ANGLE;

		default:
			throw OpNotFoundException( op );
	}
}

bool bracks_needed_if_eq_precedence(op_t op) {
	switch(op) {
		case OP_ADD:
		case OP_MULT:
		case OP_DIV:
		case OP_NEG:
		case OP_ANGLE:
		case OP_POW: // TODO this one might need it because it's right associative?
		case OP_NONE:
			return false;

		case OP_SUB:
			return true;
	}
	throw OpNotFoundException(op);
}

bool is_op_left_associative( op_t op ) {
	if( op != OP_POW ) {
		return true;
	} else {
		return false;
	}
}

arg_count_t get_arg_count( op_t op, bool left ) {
	switch( op ) {
	case OP_ADD:
	case OP_SUB:
	case OP_MULT:
	case OP_DIV:
	case OP_POW:
	case OP_ANGLE:
		return left ? 1 : 1;
	case OP_NEG:
		return left ? 0 : 1;

	default:
		throw OpNotFoundException( op );
	}
}

arg_count_t get_l_arg_count( op_t op ) { return get_arg_count(op,true ); }
arg_count_t get_r_arg_count( op_t op ) { return get_arg_count(op,false); }

bool Node::has_units(void) const {
	return false;
}

bool NodeValue::has_units(void) const {
	return this->input_units.size() > 0;
}

UnitInfoInputAry NodeValue::get_units(void) const {
	UnitInfoInputAry unit_ary;
	unit_ary.units = this->input_units;
	return unit_ary;
}
		
UnitInfoInputAry Node::get_units(void) const {
	throw BaseCalcException("Node::get_units called");
}


std::vector<Node*> Node::get_children(void) const {
	std::vector<Node*> empty_vec;
	return empty_vec;
}

NodeOp::NodeOp( op_t op )
{
	this->op       = op;
}

NodeOp::NodeOp( op_t op, std::vector<Node*> children )
{
	this->op       = op;
	this->children = children;
}

NodeValueRect::NodeValueRect(std::string val_str, bool is_imag)
{
	this->val_str = val_str;
	this->is_imag = is_imag;
}

NodeValuePolar::NodeValuePolar(std::string mag_str, std::string angle_str) {
	this->mag_str   = mag_str;
	this->angle_str = angle_str;
}


Node::Node() {
	Node::nodes_allocated++;
	Node::nodes[this] = true;
}

void Node::print_allocated_nodes(std::ostream &out) {
	out << "There are " << Node::nodes_allocated << " nodes allocated from prior calls" << std::endl;
	int i = 0;
	for (auto iter = Node::nodes.begin(); iter != Node::nodes.end(); iter++) {
		Node *n = iter->first;
		out << i++ << ": " << n->to_string() << std::endl;
	}
}

Node::~Node() {
	Node::nodes_allocated--;
	Node::nodes.erase(this);
}

NodeOp::~NodeOp() {
	for( uint32_t i=0; i<this->children.size(); i++ ) {
		delete this->children[i];
	}
}

std::vector<Node*> NodeOp::get_children(void) const {
	return this->children;
}



std::string Node::to_string(void) const
{
	throw BaseNodeRefException( "called to_string on base node" );
	//return "to_string not implemented on base node";
}

val_t Node::eval(const CalcData *data)
{
	(void)data;
	throw BaseNodeRefException( "called eval on base node" );
}

node_type Node::get_node_type(void)
{
	throw BaseNodeRefException( "called get_node_type on base node" );
}


std::ostream& operator<<(std::ostream &out, const Node& n) {
	return out << n.to_string();
}

std::string input_units_to_string(std::vector<UnitInfoInput> input_units) {
	std::string output = "";
	bool first = true;
	for (const UnitInfoInput &unit_info : input_units) {
		if (!first) {
			output += " ";
		}
		first = false;
		output += unit_info.to_string();
	}
	return output;
}


std::string NodeValueRect::to_string(void) const
{
	char buff[256];
	snprintf(buff, sizeof(buff), "{val_rect %s%s, unit:%s}",
	         (this->is_imag ? "j" : ""),
	         this->val_str.c_str(),
	         input_units_to_string(this->input_units).c_str());
	return std::string(buff);
}

std::string NodeValuePolar::to_string(void) const
{
	char buff[256];
	snprintf(buff, sizeof(buff), "{val_polar %s, %s, unit:%s}",
	         this->mag_str.c_str(),
	         this->angle_str.c_str(),
	         input_units_to_string(this->input_units).c_str());
	return std::string(buff);
}


val_t NodeValue::eval(const CalcData *data) {
	val_t val = this->get_val(data, data->degree);
	//unit_t new_unit = eval_units(data, this->input_units);
	//val.unit_dim = new_unit.dim;
	//val.re *= new_unit.mag;
	//val.im *= new_unit.mag;
	return val;
}

#warning TODO why was anyone else calling this besides NodeValue::eval???
// I think that my original intention was for this to return a unitless
// value... but I was calling it in node_to_latex for some reason.
val_t NodeValueRect::get_val(const CalcData *calcData, bool degrees) const {
	(void)degrees;
	val_t val;
	unit_t unit = eval_units(calcData, this->input_units);
	calc_float_t mag = std::atof(this->val_str.c_str());
	mag *= unit.mag;
	if (!this->is_imag) {
		val.re = mag;
		val.im = 0;
	} else {
		val.re = 0;
		val.im = mag;
	}
	val.unit_dim = unit.dim;
	return val;
}

val_t NodeValuePolar::get_val(const CalcData *calcData, bool degrees) const {
	val_t val;
	unit_t unit = eval_units(calcData, this->input_units);
	calc_float_t mag   = std::atof(this->mag_str.c_str());
	calc_float_t angle = std::atof(this->angle_str.c_str());
	if (degrees) {
		angle *= M_PI/180;
	}
	mag *= unit.mag;
	val.re = mag * std::cos(angle);
	val.im = mag * std::sin(angle);
	val.unit_dim = unit.dim;
	return val;
}

node_type NodeValue::get_node_type(void) { return NODE_VAL; }

std::string NodeOp::to_string(void) const
{
	std::stringstream sstr;
	
	sstr << "( ";
	sstr << op_to_string(this->op);
	sstr << " ";

	for( uint32_t i=0; i<this->children.size(); i++ )
	{
		sstr << this->children[i]->to_string();

		if( i < this->children.size() - 1 ) {
			sstr << ", ";
		}
	}

	sstr << " )";

	return sstr.str();
}

val_t NodeOp::eval(const CalcData *data)
{
	eval_func_t func;
	switch( this->op )
	{
		case OP_ADD:  func = add_vals;  break;
		case OP_SUB:  func = sub_vals;  break;
		case OP_MULT: func = mult_vals; break;
		case OP_DIV:  func = div_vals;  break;
		case OP_NEG:  func = neg_vals;  break;
		case OP_POW:  func = pow_vals;  break;
		case OP_ANGLE:func = data->degree ? angle_op_func_degree : angle_op_func_rad; break;
	
		default: throw OpNotFoundException( this->op );
	}

	std::vector<val_t> vals( this->children.size() );

	for( uint32_t i=0; i<this->children.size(); i++ )
	{
		vals[i] = this->children[i]->eval(data);
	}

	val_t result = func(vals);

	return result;
}

node_type NodeOp::get_node_type(void) { return NODE_OP; }

op_t      NodeOp::get_op(void) { return this->op; }

precedence_t NodeOp::get_precedence(void) { return op_to_precedence( this->op ); }


void NodeOp::add_child( Node* child_node )
{
	this->children.push_back( child_node );
}

bool NodeOp::is_left_associative(void)
{
	return is_op_left_associative( this->op );
}

NodeOp*      Node::promote_to_op(void) {
	throw BaseCalcException( "called promote_to_op on base node" );
}

NodeOp*      NodeValue::promote_to_op(void) {
	throw BaseCalcException( "called promote_to_op on value node" );
}

NodeOp*      NodeOp::promote_to_op(void) {
	return this;
}

bool         NodeOp::needs_args(void) {
	arg_count_t needed_args = 0;
	needed_args += get_l_arg_count( this->op );
	needed_args += get_r_arg_count( this->op );

	return this->children.size() < needed_args;

}



#if 0
void print_node_tree( Node * n, int depth )
{
	for( int i=0; i<n->children.size(); i++ )
	{
		for( int tab_count=0; tab_count<depth; tab_count++ )
		{
			std::cout << "    ";
		}

		if( 

	}
}
#endif


#if 0
int main(void)
{
	/*  (10 - 3) ^ 2 + 5 */
	/*  ( a - b) ^ c + d */
	/*    a - b           is n1  */
	/*      n1   ^ c      is n2  */
	/*          n2   + d  is n3  */

	NodeValue a(10);
	NodeValue b(3);
	NodeValue c(2);
	NodeValue d(5);

	std::vector<Node*> n1_children = { &a, &b };

	NodeOp n1( OP_SUB, n1_children );

	std::vector<Node*> n2_children = { &n1, &c };

	NodeOp n2( OP_POW, n2_children );

	std::vector<Node*> n3_children = { &n2, &d };

	NodeOp n3( OP_ADD, n3_children );

	std::vector<Node*> tests = {
		&a,
		&b,
		&c,
		&n1,
		&n2,
		&n3
	};

	for( uint32_t i=0; i<tests.size(); i++ )
	{
		std::cout << i << ": " ;
		std::cout << tests[i]->to_string();
		std::cout << " " ;
		try {
			val_t result = tests[i]->eval();
			std::cout << "= " << result;
		} catch(const BaseCalcException &e ) {
			std::cout << "exception: " << e.msg;
		}
		std::cout << std::endl;
	}
	
	return 0;
}
#endif


NodeVar::NodeVar(std::string var_name) {
	this->var_name = var_name;
}
NodeVar::~NodeVar(void) { }

std::string NodeVar::to_string(void) const {
	std::string prefix("var('");
	std::string suffix("')");
	return prefix + this->var_name + suffix;
}

val_t NodeVar::eval(const CalcData *data) {
	if (CONSTANTS.find(this->var_name) != CONSTANTS.end()) {
		return CONSTANTS.at(this->var_name);
	}
	if (data->var_is_defined(this->var_name)) {
		return data->get_var(this->var_name);
	}
	throw VariableNotDefinedException(this->var_name);
}
node_type NodeVar::get_node_type(void) { return NODE_VAL; } // TODO I think this is okay...

NodeOp* NodeVar::promote_to_op(void) {
	throw BaseCalcException( "called promote_to_op on var node" );
}


NodeFunc::NodeFunc(std::string func_name, std::vector<Node*> args) {
	this->func_name = func_name;
	this->args      = args;
}

NodeFunc::~NodeFunc(void) {
 	for (Node *n : this->args) {
		delete n;
	}
}

std::string NodeFunc::to_string(void) const {
	return std::string("(func \"") +
	       this->func_name +
	       std::string("\": ") +
	       this->args[0]->to_string() +
	       std::string(")");
}

val_t NodeFunc::eval(const CalcData *data) {
	if (CONSTANT_FUNCS.find(this->func_name) != CONSTANT_FUNCS.end()) {
		calc_func_t func = CONSTANT_FUNCS.at(this->func_name);
		return func(this->args.at(0)->eval(data), data->degree);
	}
	throw FunctionNotDefinedException(this->func_name);
}
node_type NodeFunc::get_node_type(void) { return NODE_VAL; } // TODO I think this is okay...

NodeOp* NodeFunc::promote_to_op(void) {
	throw BaseCalcException( "called promote_to_op on func node" );
}

NodeWipToken::NodeWipToken(std::string wip_token, std::string wip_angle, std::vector<UnitInfoInput> wip_units) {
	this->wip_token = wip_token;
	this->wip_angle = wip_angle;
	this->wip_units = wip_units;
}

NodeWipToken::~NodeWipToken(void) { }

std::string NodeWipToken::to_string(void) const {
	return std::string("(wip \"") +
	       this->wip_token + ", "
	       "angle: " + this->wip_angle + ","
	       "units: " + input_units_to_string(this->wip_units) +
	       std::string("\")");
}

val_t NodeWipToken::eval(const CalcData *data) {
	throw BaseCalcException("tried to evaluate wip token");
}
node_type NodeWipToken::get_node_type(void) { return NODE_VAL; }

bool NodeWipToken::has_units(void) const {
	return this->wip_units.size() > 0;
}

UnitInfoInputAry NodeWipToken::get_units(void) const {
	UnitInfoInputAry unit_ary;
	unit_ary.units = this->wip_units;
	return unit_ary;
}

NodeOp* NodeWipToken::promote_to_op(void) {
	throw BaseCalcException( "called promote_to_op on wip token node" );
}

NodeWipBrackets::NodeWipBrackets(Node *arg, bool right_brack_present) {
	this->arg = arg;
	this->right_brack_present = right_brack_present;
}

NodeWipBrackets::~NodeWipBrackets(void) {
	delete this->arg;
}

std::string NodeWipBrackets::to_string(void) const {
	return std::string("(wip_brackets[r:") +
	       (this->right_brack_present ? std::string("1") : std::string("0")) +
	       "] " +
	       this->arg->to_string() +
	       std::string(")");
}

val_t NodeWipBrackets::eval(const CalcData *data) {
	(void)data;
	// could just evaluate child node, but these shouldn't be in anything that
	// gets evaluated
	throw BaseCalcException("tried to evaluate wip brackets");
}
node_type NodeWipBrackets::get_node_type(void) { return NODE_VAL; }

NodeOp* NodeWipBrackets::promote_to_op(void) {
	throw BaseCalcException( "called promote_to_op on wip bracket node" );
}


NodeWipFuncCall::NodeWipFuncCall(NodeFunc* arg, bool right_brack_present) {
	this->arg = arg;
	this->right_brack_present = right_brack_present;
}

NodeWipFuncCall::~NodeWipFuncCall(void) {
	delete this->arg;
}

std::string  NodeWipFuncCall::to_string(void) const {
	return std::string("(wip_func_call[r:") + 
	       (this->right_brack_present ? std::string("1") : std::string("0")) +
	       "]" +
			this->arg->to_string() +
			")";
}

node_type NodeWipFuncCall::get_node_type(void) { return NODE_VAL; }


struct calc_fmt_params get_default_params(void) {
	return {
		.exp_format         = EXP_FMT_NORMAL,
		.fixed_decimal_places = false,
		.max_decimal_places = MAX_DECIMAL_PLACES,
	};
}

NodeApplyUnits::NodeApplyUnits(Node *n, UnitInfoInputAry unit_info) {
	this->n = n;
	this->unit_info = unit_info;
}
NodeApplyUnits::~NodeApplyUnits(void) {
	delete n;
}

std::string NodeApplyUnits::to_string(void) const {
	std::stringstream ss;
	ss << "(apply_units: " << this->n->to_string()
	   << ", units: " << unit_info.to_string()
	   << ")";
	return ss.str();
}

val_t NodeApplyUnits::eval(const CalcData *data) {
	val_t val = this->n->eval(data);
	if (units_non_zero(val.unit_dim)) {
		// TODO I suppose you could, and just multiply them.
		// Maybe this should be a preference. I can't see why anyone
		// would want to do this though.
		throw InvalidInputException("can not apply units to value with units already set", 0);
	}

	unit_t units_to_apply = eval_units(data, unit_info.units);
	val = mult_units(val, units_to_apply);

	return val;
}

node_type NodeApplyUnits::get_node_type(void) { return NODE_VAL; }

#if 0
int count_decimal_places(calc_float_t val, int max) {
	// TODO maybe this should just use snprintf and count
	// how many nonzero decimal places there are at the end of the string?
	max = std::min(max, MAX_DECIMAL_PLACES);
	val -= (int)val;
	long long val_int = round(val*std::pow(10,max));
	std::cout << "count_decimal_places{ f:" << val << ", d:" << val_int << std::endl;
	int decimal_places = 0;
	while (val_int > 0) {
		//val_int -= std::pow(10, std::floor(std::log10(val_int)));
		long long pos_10 = std::pow(10, std::floor(std::log10(val_int)));
		long long diff = (val_int / pos_10) * pos_10;
		val_int -= diff;
		std::cout << "count_decimal_places{ diff:" << diff << ", new_val_d:" << val_int << std::endl;
		if (decimal_places >= max) { break; }
		decimal_places++;
	}
	return decimal_places;
}
#endif

int count_decimal_places(calc_float_t val, int max) {
	assert(max >= 0);
	assert(max <= MAX_DECIMAL_PLACES);

	char str_buff[256];
	int len = snprintf(str_buff, sizeof(str_buff), "%.*" CALC_FLOAT_FMT "f", max, val);
	if (len <= 0) { return max; }
	int decimal_places = 0;
	bool seen_non_zero_yet = false;
	//std::cout << "debug count_decimal_places: str_buff = \"" << str_buff << "\", len = " << len << ";" << std::endl;
	for (int i=len-1; i>=0; i--) {
		const char c = str_buff[i];
		//std::cout << "debug count_decimal_places: i = " << i << ", c = \'" << c << "\'" << std::endl;
		if (!seen_non_zero_yet && c == '0') {
			continue;
		} else if (c == '.') {
			break;
		}
		if (c != '0') {
			seen_non_zero_yet = true;
		}
		decimal_places++;
		if (decimal_places >= max) {
			return max;
		}
	}
	return decimal_places;
}

calc_flt_fmt_t format_calc_float_params(calc_float_t val, const struct calc_fmt_params &params) {
	calc_flt_fmt_t fmt;
	if (val == 0.0) {
		fmt.exponent = 0;
	} else {
		float exponent_f = std::log10(std::abs(val));
		// round down to nearest integer
		fmt.exponent = (int)exponent_f;
		if (exponent_f - fmt.exponent < 0) {
			fmt.exponent -= 1;
		}
	}
	switch(params.exp_format) {
		case EXP_FMT_NORMAL:
			if (-4 <= fmt.exponent && fmt.exponent <= 9) {
				fmt.exponent = 0;
			}
			break;
		case EXP_FMT_SCI:
			break;
		case EXP_FMT_ENG:
			if (fmt.exponent >= 0) {
				fmt.exponent = (fmt.exponent/3)*3;
			} else {
				fmt.exponent = std::floor(fmt.exponent*1.0/3.0)*3;
			}
			break;
	}
	fmt.base     = val/std::pow(10, fmt.exponent);
	if (params.fixed_decimal_places) {
		fmt.decimal_places = params.max_decimal_places;
	} else {
		fmt.decimal_places = count_decimal_places(fmt.base, params.max_decimal_places);
	}
	return fmt;
}

std::string calc_float_to_str(calc_float_t flt, const struct calc_fmt_params &params) {
	calc_flt_fmt_t val_fmt = format_calc_float_params(flt, params);
	char buff[256];
	if (std::isnan(flt)) {
		snprintf(buff, sizeof(buff), "NaN");
	} else if (std::isinf(flt)) {
		snprintf(buff, sizeof(buff), "%sInf", (flt < 0) ? "-" : "");
	} else if (val_fmt.exponent == 0) {
		snprintf(buff, sizeof(buff), "%.*" CALC_FLOAT_FMT "f", val_fmt.decimal_places, val_fmt.base);
	} else {
		//snprintf(buff, sizeof(buff), "%.*" CALC_FLOAT_FMT "f * 10^%d", val_fmt.decimal_places, val_fmt.base, val_fmt.exponent);
		snprintf(buff, sizeof(buff), "%.*" CALC_FLOAT_FMT "fe%d", val_fmt.decimal_places, val_fmt.base, val_fmt.exponent);
	}
	return std::string(buff);
}

calc_float_t round_to_zero_if_negligible(calc_float_t compared_to, calc_float_t val) {
	if (val == 0.0) { return 0.0; }
	if (compared_to == 0.0) { return val; } 
	
	// if this value is more than 11 orders of magnitude away from the 
	// value we're comparing it to (e.g. val is the real component
	// and compared_to is the imaginary component), then round it down to zero
	if ( std::log10(std::abs(val)) - std::log10(std::abs(compared_to)) < -11) {
		return 0.0;
	}
	return val;
}

// TODO give this an argument to lookup units?
std::string val_to_string(const val_t *val_arg, const struct calc_fmt_params &params,
                          const unit_t *desired_unit) {
	// TODO implement polar here too

	std::string re_str, im_str;
	val_t val = *val_arg;
	val.im = round_to_zero_if_negligible(val.re, val.im);
	val.re = round_to_zero_if_negligible(val.im, val.re);
	bool im_neg;

	if (desired_unit != nullptr) {
		val.re /= desired_unit->mag;
		val.im /= desired_unit->mag;
	}

	if (val.im < 0) {
		val.im = -val.im;
		im_neg = true;
	} else {
		#warning "FIXME why isn't this being reported??"
		// TODO I think this is a clang/gcc bug. I might try to report it

		// TODO I missed this case at one point and didn't realize it,
		// the variable was left uninitialized. Check to see if a warning was being generated,
		// and if I would have saved myself a bit of debugging if I had it error on warnings
		im_neg = false;

	}
	if (val.re != 0.0 || (val.re == 0 && val.im == 0)) {
		re_str = calc_float_to_str(val.re, params);
	}
	if (val.im != 0.0) {
		im_str = "j" + calc_float_to_str(val.im, params);
	}


	std::string output;

	if (re_str.size() > 0 && im_str.size() > 0) {
		// TODO should show polar too if desired
		output = re_str + (im_neg ? " - " : " + ") + im_str;
	} else if (re_str.size() > 0) {
		output = re_str;
	} else {
		output = (im_neg ? "-" : "") + im_str;
	}

	std::string unit_str;
	if (desired_unit == nullptr) {
		std::unique_ptr<UnitInfoParsed> unit_info = unit_dim_to_string_nice_lookup(val.unit_dim);
		if (unit_info != nullptr) {
			unit_str = unit_info_to_plaintext(*unit_info);
		} else {
			unit_str = unit_dim_to_string(val.unit_dim);
		}
	} else {
		unit_str = "";
	}

	//std::cerr << "output: \"" << output << "\", unit_str: \"" << unit_str << "\"" << std::endl;

	// TODO make sure this works for polar?
	if (unit_str.size() > 0) {
		if (re_str.size() > 0 && im_str.size() > 0) {
			output = "(" + output + ")";
		}
		output += " " + unit_str;
	}

	return output;
}


CalcData::CalcData(void) { }
CalcData::~CalcData(void) { }
bool  CalcData::var_is_defined(std::string name) const {
	return this->vars.find(name) != this->vars.end();
}
val_t CalcData::get_var(std::string name) const {
	return this->vars.at(name);
}
void  CalcData::set_var(std::string name, val_t val) {
	this->vars[name] = val;
}

void CalcData::print_vars(void) const {
	std::cout << "Printing " << this->vars.size() << " variables:" << std::endl;
	for (auto it = this->vars.begin(); it != this->vars.end(); it++) {
		const std::string name = it->first;
		const val_t       val  = it->second;
		//std::cout << "    " << name << ": {re: " << val.re << ", im: " << val.im << "}" << std::endl;
		std::cout << "    " << name << ": " << val_to_string(&val, get_default_params(), nullptr) << std::endl;
	}
}

void CalcData::delete_vars(void) {
	bool has_ans = false;
	val_t ans_val;
	if (this->vars.find(VAR_NAME_ANS) != this->vars.end()) {
		has_ans = true;
		ans_val = this->vars[VAR_NAME_ANS];
	}

	this->vars.clear();
	if (has_ans) {
		this->vars[VAR_NAME_ANS] = ans_val;
	}
}

void get_all_input_units(const Node *n, std::vector<UnitInfoInputAry> *all_input_units) {
	if (n->has_units()) {
		UnitInfoInputAry units = n->get_units();
		all_input_units->push_back(units);
	}

	// TODO remove duplicates

	for (const Node *n2 : n->get_children()) {
		get_all_input_units(n2, all_input_units);
	}

}


std::string UnitInfoInputAry::to_string(void) const {
	std::string output = "";
	bool first = true;
	for (const UnitInfoInput &unit_info : this->units) {
		if (!first) {
			output += " ";
		}
		first = false;
		output += unit_info.to_string();
	}
	return output;
}


OutputInfo InputInfo::eval(CalcData *calcData) {
	OutputInfo output;
	output.val = this->n->eval(calcData);
	get_all_input_units(this->n, &output.units_in_input);
	if (this->has_to_unit) {
		UnitInfoInputAry to_unit_obj;
		to_unit_obj.units = this->to_unit;
		output.units_in_input.push_back(to_unit_obj);
	}

	calcData->set_var(VAR_NAME_ANS, output.val);

	if (this->has_sto) {
		if (CONSTANTS.find(this->sto_var_name) != CONSTANTS.end()) {
			throw InvalidInputException(std::string("\"") +
			          this->sto_var_name + "\" is a constant and can not be overwritten", 0);
		}

		calcData->set_var(this->sto_var_name, output.val);
	}

	if (this->has_to_unit) {
		output.output_unit_str = this->to_unit;
		unit_t unit = eval_units(calcData, this->to_unit);
		output.unit = unit;

		if ( !units_dim_eq(output.val.unit_dim, unit.dim)) {
			throw InvalidInputException(std::string("output has units \"") + 
			                            unit_dim_to_string(output.unit.dim) +
			                            "\", can not convert to unit \"" +
			                            unit_dim_to_string(output.val.unit_dim), 0);
		}
	}

	return output;
}

bool InputInfo::polar(void) const {
	return this->calcData->polar;
}

bool InputInfo::degree(void) const {
	return this->calcData->degree;
}

std::string OutputInfo::to_string(void) const {
	std::stringstream ss;
	ss << "{re:" << this->val.re << ", im: " << this->val.im << ", "
	   << "to_units: " << unit_to_string(this->unit) << "}";
	return ss.str();
	
}

UnitInfoInput::UnitInfoInput(std::string base, int pow) {
	this->base = base;
	this->pow  = pow;
}
std::string UnitInfoInput::to_string(void) const {
	if (this->pow == 1) {
		return this->base;
	} else {
		std::string pow_str;
		if (this->wip_pow) {
			pow_str = this->pow_wip_str;
		} else {
			pow_str = "^" + std::to_string(this->pow);
		}
		return this->base + pow_str;
	}
}

unit_t eval_units(const CalcData *calcData, std::vector<UnitInfoInput> input_units) {
	unit_t output = init_unit();
	for (const auto &unit_info : input_units) {
		if (calcData->units.find(unit_info.base) == calcData->units.end()) {
			throw UnitNotDefinedException(unit_info.base);
		}
		unit_t unit = calcData->units.at(unit_info.base).unit;
		unit = pow_unit(unit, unit_info.pow);
		output = mult_units(output, unit);
	}
	return output;
}


bool UnitCursorPosInfo::contains_pos(int pos) const {
	int start_pos = base_unit_start_pos;
	int end_pos   = pow_val_end_pos;
	return start_pos <= pos && pos <= end_pos+1;
}


int UnitCursorPosInfo::get_last_pos(void) const {
	if (pow_val_present) {
		return pow_val_end_pos;
	} else if (pow_symbol_present) {
		return pow_symbol_pos;
	} else {
		return base_unit_end_pos;
	}
}

bool WipValVarTokenCursorPos::contains_pos(int pos) const {
	int start_pos = base_val_start_pos;
	int end_pos;
	if (unit_info.size() == 0) {
		end_pos = base_val_end_pos;
	} else {
		const int unit_info_len = unit_info.size();
		end_pos = unit_info.at(unit_info_len - 1).get_last_pos();
	}
	return (start_pos <= pos && pos <= end_pos)
#warning TODO not sure about this
		|| pos == end_pos+1; // include character after last?
}


bool WipValVarTokenCursorPos::base_contains_pos(int pos) const {
	return (base_val_start_pos <= pos && pos <= base_val_end_pos)
#warning TODO not sure about this
		|| pos == base_val_end_pos+1; // include character after last?
}

std::string UnitCursorPosInfo::to_string(void) const {
	std::string output = "{UnitCursorPosInfo";
	output += "base[" + std::to_string(this->base_unit_start_pos) + ","
	                  + std::to_string(this->base_unit_end_pos) + "]";
	output += ",";
	if (!pow_symbol_present) {
		output += "pow_symb:false";
	} else {
		output += "pow_symb:" + std::to_string(pow_symbol_pos);
		if (!pow_val_present) {
			output += "pow_val:false";
		} else {
			output += "pow_val[" + std::to_string(pow_val_start_pos) + "," +
			                       std::to_string(pow_val_end_pos) + "]";
		}
	}
	output += "}";
	return output;
}

std::string WipValVarTokenCursorPos::to_string(void) const {
	std::string output = "{WipValVarTokenCursorPos";
	output += "val[" + std::to_string(this->base_val_start_pos) + ","
	                 + std::to_string(this->base_val_end_pos) + "], ";
	output += "units{";
	bool first = true;
	for (const UnitCursorPosInfo &u : unit_info) {
		if (!first) {
			output += ",";
		}
		first = false;
		output += u.to_string();
	}
	output += "}";
	output += "}";
	return output;
}


std::string unit_info_input_vec_to_string(const std::vector<UnitInfoInput> *unit_input_vec) {
	std::string output = "";
	bool first = true;
	for (const UnitInfoInput &unit_info : *unit_input_vec) {
		if (!first) { output += " "; }
		first = false;
		output += unit_info.to_string();
	}
	return output;
}

static int get_exp_pos(std::string val_str) {
	bool has_exp = false;
	int exp_pos = 0;

	if (val_str.find("E") != std::string::npos) {
		has_exp = true;
		exp_pos = val_str.find("E");
	} else if (val_str.find("e") != std::string::npos) {
		has_exp = true;
		exp_pos = val_str.find("e");
	}

	if (has_exp) { return exp_pos; }
	else { return -1; }
}

std::string get_mag_no_exp_str(std::string val_str) {
	int pos = get_exp_pos(val_str);
	if (pos == -1) {
		return val_str;
	} else {
		return val_str.substr(0, pos);
	}
}

int get_pow_exp_str(std::string val_str) {
	int pos = get_exp_pos(val_str);
	if (pos == -1) {
		return 0;
	} else {
		std::string pow_str = val_str.substr(pos+1, val_str.size()-pos-1);
		return std::atoi(pow_str.c_str());
	}
}

// I originally thought I'd need this for the cursor, but I don't think it's useful anymore.
// Keeping it in here for now in case I change my mind again
#if 0
Node* calc_find_rightmost_deepest_node(Node *n) {
	int i = 0;
	while (true) {
		assert(i++ < 1000);
		if (dynamic_cast<NodeFunc*>(n) != nullptr) {
			NodeFunc *node_func = dynamic_cast<NodeFunc*>(n);
			if (node_func->args.size() == 0) { return node_func; }
			n = node_func->args.at(node_func->args.size()-1);
		} else if (dynamic_cast<NodeWipBrackets*>(n) != nullptr) {
			NodeWipBrackets* node_wip_brackets = dynamic_cast<NodeWipBrackets*>(n);
			if (node_wip_brackets->arg != nullptr) {
				n = node_wip_brackets->arg;
			} else {
				return node_wip_brackets;
			}
		} else if (dynamic_cast<NodeWipFuncCall*>(n) != nullptr) {
			NodeWipFuncCall* node_wip_func_call = dynamic_cast<NodeWipFuncCall*>(n);
			if (node_wip_func_call->arg != nullptr) { n = node_wip_func_call->arg; }
			else { return node_wip_func_call; }
		} else if (dynamic_cast<NodeOp*>(n) != nullptr) {
			NodeOp* node_op = dynamic_cast<NodeOp*>(n);
			if (node_op->children.size() == 0) { return n; }
			else { n = node_op->children.at(node_op->children.size() - 1); }
		} else {
			return n;
		}
	}
}
#endif
