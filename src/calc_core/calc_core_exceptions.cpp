#include "calc_core_exceptions.h"
#include "calc_core.h"

BaseCalcException::BaseCalcException( std::string msg ) {
	this->msg = msg;
}

InvalidInputException::InvalidInputException( std::string msg, int index ) 
	: BaseCalcException( msg ) {
	this->index= index;
}

NoClosingBracketsException::NoClosingBracketsException(void) 
	: InvalidInputException( "no closing brackets", 0 ) {
}

VariableNotDefinedException::VariableNotDefinedException(std::string var_name)
	: InvalidInputException( "variable '" + var_name + "' not defined", 0) {
	this->var_name = var_name;
}

FunctionNotDefinedException::FunctionNotDefinedException(std::string func_name)
	: InvalidInputException( "function '" + func_name + "' not defined", 0) {
	this->func_name = func_name;
}

OpNotFoundException::OpNotFoundException( op_t op ) 
	: BaseCalcException( "operator not found" ) {
	this->op = op;
}

op_t OpNotFoundException::get_op() { return this->op; }


InvalidArgCountException::InvalidArgCountException( op_t op, int count )
	: BaseCalcException( "invalid arg count" ) {
	this->op = op;
	this->count = count;
}

BaseNodeRefException::BaseNodeRefException( std::string msg )
	: BaseCalcException( msg ) {
}

std::string BaseNodeRefException::get_message(void) {
	return this->msg;
}


UnitMismatchException::UnitMismatchException(op_t op, const unit_dim_t &left, const unit_dim_t &right)
	: InvalidInputException( "unit dim " + unit_dim_to_string(left) + " and " +
	                         unit_dim_to_string(right) + " can not have " + 
	                         "op " + op_to_string(op) + " applied to them", 0) {
}

UnitInvalidOperationException::UnitInvalidOperationException(std::string msg) :
	InvalidInputException(msg, 0) { }


UnitNotDefinedException::UnitNotDefinedException(std::string unit_name)
	: InvalidInputException( "unit \"" + unit_name + "\" not defined", 0) {
	
}
