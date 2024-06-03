
#ifndef __CALC_CORE_TYPES_H__
#define __CALC_CORE_TYPES_H__

#include<cstdlib>
#include<cstdint>

typedef std::int8_t arg_count_t;
#define ARG_COUNT_UNLIMITED  ( (arg_count_t)0xff )

//#define USE_LONG_DOUBLES

#ifdef USE_LONG_DOUBLES
typedef long double calc_float_t;
#define CALC_FLOAT_FMT "L"
#define CALC_SIN sinl
#define CALC_COS cosl
#else
typedef double calc_float_t;
#define CALC_FLOAT_FMT ""
#define CALC_SIN sin
#define CALC_COS cos
#endif

typedef struct {
	int s;
	int m;
	int kg;
	int A;
	int K;
	int mol;
	int cd;
} unit_dim_t;

typedef struct {
	calc_float_t re;
	calc_float_t im;
	unit_dim_t   unit_dim;
} val_t;

typedef struct {
	calc_float_t  mag;
	unit_dim_t    dim;
} unit_t;

/* Structure used to break internal calc values to
 * values that can be easily string formatted and shown to the user
 *     val = base * (10^exponent);
 */
typedef struct {
	int          exponent;
	calc_float_t base;
	int          decimal_places;
} calc_flt_fmt_t;

enum exp_format {
	EXP_FMT_NORMAL,
	EXP_FMT_SCI,
	EXP_FMT_ENG,
};

struct calc_fmt_params {
	enum exp_format exp_format;
	/* if true, always show `max_decimal_places`.
	 * if false, only show now zero decimal places, up to `max_decimal_places` */
	bool            fixed_decimal_places;
	int             max_decimal_places;
};

typedef std::int8_t precedence_t;
// is it okay to make addition a lower precedence than the other nodes?
// 1-(2+3) needs brackets, but
// 1+(2-3) does not.
// The subtraction node must be seen as something that has to be evaluated before addition
//
// -(2-3) must be rendered with brackets too. So negation needs higher precedence than subtraction
#define PRECEDENCE_ADD          ( (precedence_t) 0 )
#define PRECEDENCE_SUB          ( (precedence_t) 1 )
#define PRECEDENCE_NEG          ( (precedence_t) 2 )
#define PRECEDENCE_MULT_DIV     ( (precedence_t) 3 )
#define PRECEDENCE_POW          ( (precedence_t) 4 )
#define PRECEDENCE_ANGLE        ( (precedence_t) 5 )

typedef val_t (*eval_func_t)(std::vector<val_t> vals);

typedef enum {
	OP_NONE,
	OP_ADD,
	OP_SUB,
	OP_MULT,
	OP_DIV,
	OP_NEG,
	OP_POW,
	OP_ANGLE,
} op_t;


typedef enum {
	NODE_OP,
	NODE_VAL,
} node_type;


#endif
