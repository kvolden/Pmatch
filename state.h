#ifndef State_h
#define State_h

#include <vector>
#include <functional>
#include "rule.h"
#include "clone_monitor.h"

template <typename Functor>
class State {			
private:
	bool enabled, do_enable, stay_enabled, has_functor;
	std::vector<Rule<Functor>> transition_rules;
	std::vector<State<Functor>*> epsilon_transitions;
	Functor functor;
	Clone_monitor* clone_monitor;
	
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
				if(transition_rules.at(i).step(inputs...)){
				    clone_monitor->add_transition(this, transition_rules.at(i).get_target_state(), false);
				}
			}
			enabled = false;
			if(stay_enabled){
				mark_enable();
				clone_monitor->add_transition(this, this, false);
			}
		}
	}
	
	void mark_enable(){
		do_enable = true;
		for(int i = 0; i < epsilon_transitions.size(); i++){
			epsilon_transitions.at(i)->mark_enable();
			clone_monitor->add_transition(this, epsilon_transitions.at(i), true);
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
	
	void set_clone_monitor(Clone_monitor* _clone_monitor){
	    clone_monitor = _clone_monitor;
    }
	
	#ifdef PMATCH_DEBUG
	void debug_output_structure(){
		for(int i = 0; i < transition_rules.size(); ++i){
			transition_rules.at(i).debug_output_structure();
		}
		for(int i = 0; i < epsilon_transitions.size(); ++i){
			std::cout << "--- E ---> " << epsilon_transitions.at(i) << std::endl;
		}
		std::cout << "Has callback function: " << ((has_functor) ? "Yes" : "No") << std::endl;
		std::cout << std::endl;
	}
	#endif
};

#endif
