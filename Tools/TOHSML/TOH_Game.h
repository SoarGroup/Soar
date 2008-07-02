#ifndef TOH_GAME_H
#define TOH_GAME_H

#include "Soar_Kernel.h"
#include "Soar_Agent.h"
#include <vector>

class TOH_Tower;

class TOH_Game {
  TOH_Game(const TOH_Game &);
  TOH_Game & operator=(const TOH_Game &);

public:
  TOH_Game();
  ~TOH_Game();

  int run();
  inline void update(sml::Kernel &kernel);

private:
  inline void move_disk(const char &source_peg_index, const char &destination_peg_index);

  //static std::string to_string(const char * const &c_str) {if(c_str) return c_str; else return "";}

  Soar_Kernel m_kernel;
  Soar_Agent m_agent;

  std::vector<TOH_Tower *> m_towers;
};

#endif
