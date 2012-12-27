#ifndef State_h
#define State_h

#include <vector>
#include <functional>
#include "rule.h"

template <typename Functor>
class State {			
private:
	bool enabled, do_enable, stay_enabled, has_functor;
	std::vector<Rule<Functor>> transition_rules;
	std::vector<State<Functor>*> epsilon_transitions;
	Functor functor;
	
public:
	State(){
		enabled = false;
		do_enable = false;
		stay_enabled = false;
		has_functor = false;
	}
	
	void add_functor(Functor _functor){
		functor = _functor;
		has_functor = true;
	}
	
	void add_rule(std::string unit, State<Functor> *target_state){
		transition_rules.push_back(Rule<Functor>(unit, target_state));
	}
	
	void add_epsilon(State<Functor> *target_state){
		epsilon_transitions.push_back(target_state);
	}
	
	template <typename... Inputs>
	void step(Inputs... inputs){
		if(enabled){
			for(int i = 0; i < transition_rules.size(); i++){
				transition_rules.at(i).step(inputs...);
			}
			enabled = false;
			if(stay_enabled){
				mark_enable();
			}
		}
	}
	
	void mark_enable(){
		do_enable = true;
		for(int i = 0; i < epsilon_transitions.size(); i++){
			epsilon_transitions.at(i)->mark_enable();
		}
	}
	
	void update_enabled(){
		if(do_enable){
			enabled = true;
			if(has_functor){
				functor();
			}
		}
		do_enable = false;
	}
	
	void set_stay_enabled(bool _stay_enabled){
		stay_enabled = _stay_enabled;
	}
	
	void disable(){
		do_enable = false;
		enabled = false;
	}
	
	bool is_active(){
		return enabled;
	}
	
	#ifdef PMATCH_DEBUG
	void debug_output_structure(){
		for(int i = 0; i < transition_rules.size(); ++i){
			transition_rules.at(i).debug_output_structure();
		}
		for(int i = 0; i < epsilon_transitions.size(); ++i){
			std::cout << "--- E ---> " << epsilon_transitions.at(i) << std::endl;
		}
		std::cout << "Has callback function: " << ((callback_function) ? "Yes" : "No") << std::endl;
		std::cout << std::endl;
	}
	#endif
};

#endif
