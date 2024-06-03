
#ifndef __CALC_CORE_EXCEPTIONS_H__
#define __CALC_CORE_EXCEPTIONS_H__

#include<string>
#include<vector>


#include "calc_core_types.h"

class BaseCalcException {
	public:
		BaseCalcException( std::string msg );
		virtual ~BaseCalcException(void) {};
		std::string msg;
};


class InvalidInputException : public BaseCalcException {
	public:
		InvalidInputException( std::string msg, int index );
		virtual ~InvalidInputException(void) {};
		int index;
};

class NoClosingBracketsException : public InvalidInputException {
	public:
		NoClosingBracketsException();
};

class VariableNotDefinedException : public InvalidInputException {
	public:
		VariableNotDefinedException(std::string var_name);
		virtual ~VariableNotDefinedException(void) {};
		std::string var_name;
};

class FunctionNotDefinedException : public InvalidInputException {
	public:
		FunctionNotDefinedException(std::string func_name);
		virtual ~FunctionNotDefinedException(void) {};
		std::string func_name;
};

class UnitMismatchException : public InvalidInputException {
	public:
		UnitMismatchException(op_t op, const unit_dim_t &left, const unit_dim_t &right);
		virtual ~UnitMismatchException(void) {};
};

class UnitNotDefinedException : public InvalidInputException {
	public:
		UnitNotDefinedException(std::string unit_name);
		virtual ~UnitNotDefinedException(void) {};
};

class UnitInvalidOperationException : public InvalidInputException {
	public:
		UnitInvalidOperationException(std::string msg);
		virtual ~UnitInvalidOperationException(void) { };
};


class OpNotFoundException : public BaseCalcException {
	public:
		OpNotFoundException( op_t op );
		op_t get_op();

	private:
		op_t op;
};

class BaseNodeRefException : public BaseCalcException {
	public:
		BaseNodeRefException( std::string msg );
		std::string get_message(void);
		std::string msg;
};

class InvalidArgCountException : public BaseCalcException {
	public:
		InvalidArgCountException( op_t op, int count );
		op_t op;
		int  count;
};

#endif
