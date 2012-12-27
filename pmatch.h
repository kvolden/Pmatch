#ifndef PMatch_h
#define PMatch_h

#include <string>
#include <deque>
#include <vector>
#include <functional>
#include <systemc.h>
#include "state.h"

#ifdef PMATCH_DEBUG
#include <iostream>
#endif

template <class Functor=std::function<void()>>
class PMatch : public sc_module{
private:
	std::deque<State<Functor>> states;
	
	int max(int a, int b){
		return (a < b) ? b : a;
	}
	
	int get_highest_callback_index(std::string reg_exp){
		int highest_index = 0, i = 0;

		while(i < reg_exp.length()){
			switch(reg_exp.at(i)){
				case '\\':
					i += 2;
					break;
				case '@':
					highest_index = max(highest_index, atoi(reg_exp.substr(++i).c_str()));
					break;
				default:
					i++;
					break;
			}
		}
		return highest_index;
	}
	
	std::string extract_subexpression(std::string expression, int start_pos){	// Returns subexpression, takes expression and start position of subexpression
		int i = start_pos + 1;
		int open_parentheses = 1;												// Used to count nested parentheses
		
		while(open_parentheses > 0){
			switch(expression.at(i)){
				case '\\':														// If character is an escaping backslash, skip next character
					++i;
					break;
				case '(':
					++open_parentheses;
					break;
				case ')':
					--open_parentheses;
					break;
			}
			++i;
		}
		return expression.substr(start_pos + 1, i - start_pos - 2);				// Return subexpression without surrounding parentheses
	}
	
	std::string extract_unit(std::string expression, int start_pos){
		int i = start_pos, end_pos = 0;
		
		while(end_pos == 0){
			++i;
			if(expression.at(i) == '\\'){
				++i;
			}
			else if(expression.at(i) == '>'){
				end_pos = i+1;
			}
		}
		return expression.substr(start_pos, end_pos-start_pos);
	}
	
	std::string build_quantified_subexpression(std::string expression, int min, int max){
		std::string built_expression = "";
		
		for(int i = min; i <= max; ++i){
			if(i > min){
				built_expression += "|";
			}
			for(int j = 0; j < i; j++){
				built_expression += expression;
			}
		}
		return built_expression;	
	}
	
	void build_from_expression(std::string reg_exp, State<Functor> *start_state, State<Functor> *end_state, std::vector<Functor>& functions){
		enum {ONE, ZEROORMORE, ONEORMORE, ZEROORONE, OTHER} quantifier;
		int quant_min, quant_max, callback_n, current_pos = 0;
		State<Functor> *new_state, *from_state = start_state;
		std::string unit, subexpression;
		std::vector<State<Functor>*> loose_ends;

		while(current_pos < reg_exp.length()){
			switch(reg_exp[current_pos]){
				case '<':														// Marks start of unit
				case '(':														// Marks start of subexpression
					if(reg_exp[current_pos] == '<'){							// If it's a unit
						unit = extract_unit(reg_exp, current_pos);
						current_pos += unit.length();
						subexpression.clear();
					}
					else{														// If it's a subexpression
						subexpression = extract_subexpression(reg_exp, current_pos);
						current_pos += subexpression.length() + 2;				// +2 for parantheses stripped from subexpression
						unit.clear();
					}

					states.push_back(State<Functor>());
					new_state = &states.back();
					
					switch(reg_exp[current_pos]){
						case '*':
							quantifier = ZEROORMORE;
							++current_pos;
							break;
						case '+':
							quantifier = ONEORMORE;
							++current_pos;	
							break;
						case '?':
							quantifier = ZEROORONE;
							++current_pos;
							break;
						case '{':
							quantifier = OTHER;
							++current_pos;
							
							quant_min = atoi(reg_exp.substr(current_pos).c_str());
							while(reg_exp[current_pos++] != ',');
							quant_max = atoi(reg_exp.substr(current_pos).c_str());
							while(reg_exp[current_pos++] != '}');
						
							if(!subexpression.empty()){							// If it's a subexpression
								subexpression = build_quantified_subexpression("(" + subexpression + ")", quant_min, quant_max);
							}
							else if(!unit.empty()){								// If it's a unit
								subexpression = build_quantified_subexpression(unit, quant_min, quant_max);	// Build a subexpression
								unit.erase();									// No longer a unit, but a subexpression
							}
							break;
						default:
							quantifier = ONE;
					}
					
					if(quantifier == ONE || quantifier == ONEORMORE || quantifier == ZEROORONE || quantifier == OTHER){	// Add rules or build subexpression
						if(!unit.empty()){										// If it's a unit
							from_state->add_rule(unit, new_state);				// Add rule
						}
						else if(!subexpression.empty()){						// If it's a subexpression
							build_from_expression(subexpression, from_state, new_state, functions);	// Run recursively
						}
					}
					
					if(quantifier == ONEORMORE || quantifier == ZEROORMORE){
						if(!unit.empty()){										// If it's a unit
							new_state->add_rule(unit, new_state);				// Add rule
						}
						else if(!subexpression.empty()){						// If it's a subexpression
							build_from_expression(subexpression, new_state, new_state, functions);	// Run recursively
						}
					}
					
					if(quantifier == ZEROORONE || quantifier == ZEROORMORE){	// Add epsilons
						from_state->add_epsilon(new_state);
					}
					

					if(reg_exp[current_pos] == '@'){							// If unit or subexpression has callback
						callback_n = atoi(reg_exp.substr(++current_pos).c_str());
						new_state->add_functor(functions.at(callback_n));
					}
					
					from_state = new_state;
					break;
					
				case '|':
					loose_ends.push_back(&states.back());
					from_state = start_state;
					++current_pos;
					break;
					
				default:
					++current_pos;
			}

		}
		
		if(end_state != NULL){													// Connect all loose threads to end state
			for(int i = 0; i < loose_ends.size(); ++i){
				loose_ends.at(i)->add_epsilon(end_state);
			}
			from_state->add_epsilon(end_state);
		}
	}
	

	
public:
	PMatch(sc_module_name name) : sc_module(name){ }
	
	void initialize(std::string reg_exp, Functor *functor, bool continuous = true){
		initialize(reg_exp, std::vector<Functor>(functor, functor + get_highest_callback_index(reg_exp) + 1), continuous);
	}

	void initialize(std::string reg_exp, Functor functor, bool continuous = true){
		initialize(reg_exp, std::vector<Functor>(1, functor), continuous);
	}
	
	void initialize(std::string reg_exp, std::vector<Functor> functions, bool continuous = true){
		int callback_n;
		
		states.clear();
		states.push_back(State<Functor>());
		
		if(reg_exp.at(0) == '@'){
			callback_n = atoi(reg_exp.substr(1).c_str());
			states.front().add_functor(functions.at(callback_n));;
		}
		
		build_from_expression(reg_exp, &states.front(), NULL, functions);
		states.front().set_stay_enabled(continuous);
		
		states.front().mark_enable();
		
		for(int i = 0; i < states.size(); ++i){									// Update-enabled all to get all epsilon transitions from S0 too
			states.at(i).update_enabled();
		}
	}

	template <typename... Inputs>
	void step(Inputs... inputs){
		for(int i = 0; i < states.size(); ++i){
			states.at(i).step(inputs...);
		}
		
		for(int i = 0; i < states.size(); ++i){
			states.at(i).update_enabled();
		}
	}
	
	void restart(){
		for(int i = 0; i < states.size(); ++i){
			states.at(i).disable();
		}
		states.front().mark_enable();
		for(int i = 0; i < states.size(); ++i){									// Update-enabled all to get all epsilon transitions from S0 too
			states.at(i).update_enabled();
		}
	}
	
	bool is_empty(){
		bool contains_active_states = false;
		for(int i = 0; i < states.size(); ++i){
			contains_active_states = contains_active_states || states.at(i).is_active();
		}
		return !contains_active_states;
	}
	
	#ifdef PMATCH_DEBUG
	void debug_output_structure(){
		for(int i = 0; i < states.size(); ++i){
			std::cout << "S" << i << " (" << &states.at(i) << "):\n";
			states.at(i).debug_output_structure();
		}
	}
	#endif
};

#endif
