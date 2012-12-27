#ifndef Sub_rule_h
#define Sub_rule_h

#include <vector>
#include <string>
#include "pcast.h"

class Sub_rule {																// Sub rule for testing one single input
private:
	bool invert;																// Whether to invert final answer (not-transition)
	bool wildcard;
	std::vector<std::string> single_values;
	std::vector<std::string> ranges;

public:
	Sub_rule(bool _invert, std::vector<std::string> _single_values, std::vector<std::string> _ranges){
		wildcard = false;
		invert = _invert;
		single_values = _single_values;
		ranges = _ranges;
	}
	
	Sub_rule(){																	// Constructor without arguments means wildcard
		wildcard = true;
		invert = false;
	}

	template <typename Input>
	bool match(Input input){
		bool match = wildcard;													// Ends up returning "true" (short circuits) if wildcard, depends on input if not
		
		for(int i = 0; i < single_values.size(); i++){							// Match any single value
			match = match || (input == pcast::cast<Input>(single_values.at(i)));
		}
		
		for(int i = 0; i < ranges.size(); i += 2){								// Match any range
			match = match || (input >= pcast::cast<Input>(ranges.at(i)) && input <= pcast::cast<Input>(ranges.at(i+1)));
		}
		
		return (invert ? !match : match);										// Return and invert if appropriate
	}
	
	#ifdef PMATCH_DEBUG
	std::string debug_escape_characters(std::string chars){
		for(int i = 0; i < chars.length(); ++i){
			switch(chars.at(i)){
				case '-':
				case ',':
				case '\\':
				case '^':
					chars.insert(i, "\\");
					++i;
					break;
			}
		}
		return chars;
	}
	
	void debug_output_structure(){
		if(wildcard){
			std::cout << ".";
		}
		else{
			std::cout << "[" << (invert ? "^" : "" );
			for(int i = 0; i < single_values.size(); ++i){
				std::cout << debug_escape_characters(single_values.at(i));
				if(i != single_values.size()-1){
					std::cout << ",";
				}
			}
			if(single_values.size() && ranges.size()){
				std::cout << ",";
			}

			for(int i = 0; i < ranges.size(); i += 2){
				std::cout << debug_escape_characters(ranges.at(i)) << "-" << debug_escape_characters(ranges.at(i+1));
				if(i != ranges.size()-2){
					std::cout << ",";
				}
			}
			std::cout << "]";
		}
	}
	#endif

};

#endif
