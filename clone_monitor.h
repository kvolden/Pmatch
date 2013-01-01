#ifndef Clone_monitor_h
#define Clone_monitor_h

#include <systemc.h>
#include "state.h"
#include <iostream>
#include <vector>
#include <map>

class Clone_monitor {															// Monitors number of active clones
private:
    struct Transition{
        sc_time time;
        void* from_state;
        void* to_state;
        bool epsilon;
    };
    int step;
    std::vector<std::vector<Transition>> transitions_timeline;
    std::vector<std::map<void*, int>> clone_number_timeline;                // void* = state-ptr, int = number of FSMs in this state

    void calculate_active_fsms(){
        std::map<void*, int> new_clone_map, *last_clone_map;
        std::vector<Transition> *last_t_vec = &transitions_timeline.back();
        
        last_clone_map = (step > 0 ? &clone_number_timeline.back() : NULL);
        
        for(int i = 0; i < last_t_vec->size(); ++i){                        // Loop through all transitions from most recent step process
            if(!last_t_vec->at(i).epsilon){
                new_clone_map[last_t_vec->at(i).to_state] += (step > 0 ? (*last_clone_map)[last_t_vec->at(i).from_state] : 1);
                int j = i + 1;
                while(last_t_vec->size() > j && last_t_vec->at(j).epsilon){ // Add all epsilon transitions originating from the previous state entry
                    new_clone_map[last_t_vec->at(j).to_state] += (step > 0 ? (*last_clone_map)[last_t_vec->at(i).from_state] : 1);
                    ++j;
                }
            }
        }
        clone_number_timeline.push_back(new_clone_map);
    }
    
public:
    Clone_monitor(){
        step = 0;
    }
    
    void step_complete(){
        calculate_active_fsms();
        step++;
    }
    
    void add_transition(void* from_state, void* to_state, bool epsilon){
        Transition transition = {sc_time_stamp(), from_state, to_state, epsilon};
        if(transitions_timeline.size() < step+1){                                   // If first transition in step
            transitions_timeline.push_back(std::vector<Transition> {transition});   // Add new transition vector
        }
        else{                                                                       // If not first transition in step
            transitions_timeline.back().push_back(transition);                      // Push transition to already existing vector
        }
    }
    
    void restart(){
        transitions_timeline.clear();
        clone_number_timeline.clear();
        step = 0;
    }
    
    std::vector<int> get_clones_evolution(){
        std::vector<int> evolution;
        std::map<void*, int>::iterator j;
        
        for(int i = 0; i < clone_number_timeline.size(); ++i){
            evolution.push_back(0);
            for(j = clone_number_timeline.at(i).begin(); j != clone_number_timeline.at(i).end(); ++j){
                evolution.back() += j->second;
            }
        }
        return evolution;
    }
    
    int get_num_of_clones(){
        int clones = 0;
        std::map<void*, int>::iterator i;
        
        if(clone_number_timeline.size() > 0){
            for(i = clone_number_timeline.back().begin(); i != clone_number_timeline.back().end(); ++i){
                clones += i->second;
            }
        }
        return clones;
    }
};

#endif
