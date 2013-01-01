#ifndef Rule_h
#define Rule_h

#include <string>
#include <vector>
#include "sub_rule.h"

template <typename Functor>														// Prototype for State pointers
class State;

template <typename Functor>
class Rule{
private:
	std::vector<Sub_rule> sub_rules;											// One sub_rule per input
	std::vector<char> bool_rels;												// Boolean relationships between the sub_rules
	State<Functor> *target_state;												// Pointer to transition's target state

	std::string unescape(std::string escape_string){							// Remove escaping backslashes
		for(int i = 0; i < escape_string.length(); i++){
			if(escape_string.at(i) == '\\'){									// Is escaped character
				escape_string.erase(i, 1);										// Delete escaping backslash, keep/skip escaped char
			}
		}
		return escape_string;
	}

	std::vector<std::string> split_fields_to_vector(std::string field_set){
		std::vector<std::string> split_fields;
		int start_of_field = 0;
		
		for(int i = 0; i < field_set.length(); ++i){							// Split the comma separated fields into a string-vector of fields
			if(field_set.at(i) == '\\'){
				++i;
			}
			else if(field_set.at(i) == ','){
				split_fields.push_back(field_set.substr(start_of_field, i - start_of_field));
				start_of_field = i + 1;
			}
		}
		split_fields.push_back(field_set.substr(start_of_field));				// Last field

		return split_fields;
	}
	
	std::vector<std::string> split_if_range(std::string field){
		std::vector<std::string> range_from_to;

		for(int i = 0; i < field.length(); ++i){								// Search for not-escaped dash
			if(field.at(i) == '\\'){
				++i;
			}
			else if(field.at(i) == '-'){
				range_from_to.push_back(field.substr(0, i));
				range_from_to.push_back(field.substr(i + 1));
			}
		}
		
		return range_from_to;
	}
	
	std::vector<bool> recursive_compare(int input_number){						// Empty function to end recursive function below
		return {};
	}

	template <typename First, typename... Rest>
	std::vector<bool> recursive_compare(int input_number, First first, Rest... rest){
		std::vector<bool> results, append;
		
		results.push_back(sub_rules.at(input_number).match(first));
		append = recursive_compare(++input_number, rest...);
		results.insert(results.end(), append.begin(), append.end());
		return results;
	}
	
	void create_sub_rule_from_string(std::string sub_unit){						// Create the sub rule from the string representation
		std::vector<std::string> values, ranges, split_fields, new_range;
		bool invert = false;
		
		if(sub_unit == "."){													// Wildcard
			sub_rules.push_back(Sub_rule());
		}
		
		else if(sub_unit.at(0) == '['){											// Several values/ranges in square brackets
			sub_unit = sub_unit.substr(1, sub_unit.length()-2);					// Trim square brackets
			if(sub_unit.at(0) == '^'){											// If NOT-condition
				invert = true;
				sub_unit.erase(0, 1);
			}
			
			split_fields = split_fields_to_vector(sub_unit);					// Divide the string of fields into a vector of fields
			
			for(int i = 0; i < split_fields.size(); ++i){
				new_range = split_if_range(split_fields.at(i));
				if(new_range.size()){
					ranges.push_back(unescape(new_range.at(0)));
					ranges.push_back(unescape(new_range.at(1)));
				}
				else{
					values.push_back(unescape(split_fields.at(i)));
				}
			}
			
			sub_rules.push_back(Sub_rule(invert, values, ranges));
		}
		
		else{																	// Single value
			values.push_back(unescape(sub_unit));
			sub_rules.push_back(Sub_rule(invert, values, ranges));
		}

	}
	
public:
	Rule(std::string regexp_unit, State<Functor> *_target_state){
		int sub_unit_start = 0;
		target_state = _target_state;
		std::string stripped_unit = regexp_unit.substr(1, regexp_unit.length()-2);
		
		for(int i = 0; i < stripped_unit.length(); i++){						// Separate the parts (inputs/sub units) of the unit
			if(stripped_unit.at(i) == '\\'){									// If next character is escaped, skip it (can't be boolean operator)
				i++;
			}
			else if(stripped_unit.at(i) == '&' || stripped_unit.at(i) == '|'){
				create_sub_rule_from_string(stripped_unit.substr(sub_unit_start, i-sub_unit_start));
				bool_rels.push_back(stripped_unit.at(i));
				sub_unit_start = i+1;
			}
		}
		create_sub_rule_from_string(stripped_unit.substr(sub_unit_start));		// Last part of the string
	}
	
	template <typename... Inputs>												// Variadic templates = C++0x
	bool step(Inputs... inputs){												// Steps the rule, enables target if conditions are met
		bool condition_met = false;
		std::vector<bool> sub_results = recursive_compare(0, inputs...);
		std::vector<bool> anded(1, sub_results.front());						// Push first sub result onto "anded" vector
		
		for(int i = 0; i < bool_rels.size(); i++){								// Do all the AND-ing and save temporary results in "anded" vector
			if(bool_rels.at(i) == '|'){
				anded.push_back(sub_results.at(i+1));
			}
			else{
				anded.back() = anded.back() && sub_results.at(i+1);
			}
		}
		
		for(int i = 0; i < anded.size(); i++){									// Do the OR-ing between the AND-ed values
			condition_met = condition_met || anded.at(i);
		}
		
		if(condition_met){
			target_state->mark_enable();
		}
		
		return condition_met;
	}
	
	State<Functor>* get_target_state(){
	    return target_state;
    }
	
	#ifdef PMATCH_DEBUG
	void debug_output_structure(){
		std::cout << "--- <";
		for(int i = 0; i < sub_rules.size(); ++i){
			sub_rules.at(i).debug_output_structure();
			if(i < bool_rels.size()){
				std::cout << bool_rels.at(i);
			}
		}
		std::cout << "> ---> " << target_state << std::endl;
	}
	#endif
};

#endif
