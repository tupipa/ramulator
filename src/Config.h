#ifndef __CONFIG_H
#define __CONFIG_H

#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <iostream>
#include <cassert>

namespace ramulator
{

class Config {

private:
    std::map<std::string, std::string> options;
    int channels;
    int ranks;
    int subarrays;
    int cpu_tick;
    int mem_tick;
	int cores_count;

public:
    Config() {}
    Config(const std::string& fname);
    void parse(const std::string& fname);
    std::string operator [] (const std::string& name) const {
      if (options.find(name) != options.end()) {
        return (options.find(name))->second;
      } else {
        return "";
      }
    }

    bool contains(const std::string& name) const {
      if (options.find(name) != options.end()) {
        return true;
      } else {
        return false;
      }
    }

    void add (const std::string& name, const std::string& value) {
      if (!contains(name)) {
        options.insert(make_pair(name, value));
      } else {
        printf("ramulator::Config::add options[%s] already set.\n", name.c_str());
      }
    }

    int get_channels() const {return channels;}
    int get_subarrays() const {return subarrays;}
    int get_ranks() const {return ranks;}
    int get_cpu_tick() const {return cpu_tick;}
	int get_cores_count() const {return cores_count;} 
	//TODO: each core_tick are currently equal with cpu_tick;
    int get_core_tick(int id) const {return cpu_tick;}
    int get_mem_tick() const {return mem_tick;}
    bool has_l3_cache() const {
      if (options.find("cache") != options.end()) {
        const std::string& cache_option = (options.find("cache"))->second;
        return (cache_option == "all") || (cache_option == "L3");
      } else {
        return false;
      }
    }
    bool has_core_caches() const {
      if (options.find("cache") != options.end()) {
        const std::string& cache_option = (options.find("cache"))->second;
          // TODO also includes L1&L2 caches only mode.
            return (cache_option == "all");
      } else {
        return false;
      }
    }
    bool is_early_exit() const {
      // the default value is true
      if (options.find("early_exit") != options.end()) {
        if ((options.find("early_exit"))->second == "off") {
          return false;
        }
        return true;
      }
      return true;
    }
    bool record_cmd_trace() const {
      // the default value is false
      if (options.find("record_cmd_trace") != options.end()) {
        if ((options.find("record_cmd_trace"))->second == "on") {
          return true;
        }
        return false;
      }
      return false;
    }
    bool print_cmd_trace() const {
      // the default value is false
      if (options.find("print_cmd_trace") != options.end()) {
        if ((options.find("print_cmd_trace"))->second == "on") {
          return true;
        }
        return false;
      }
      return false;
    }
};


} /* namespace ramulator */

#endif /* _CONFIG_H */

