/*
 * C++ Towers of Hanoi
 *
 * Author : Mitchell Keith Bloch, Soar Group at U-M
 * Date   : June/July 2008
 *
 * Hopefully a "good" C++ version of the TOH_Towers of Hanoi example, 
 * taken primarily from the JavaTOH code, but some information
 * taken from SML Quick Start Guide.doc and JavaTOHTutorial.doc
 */

#include "TOH_Game.inl"

int main(int /*argc*/, char * /*argv*/ []) {
#ifdef WIN32
#ifdef _DEBUG
	//_crtBreakAlloc = 1441;
  _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
#endif

  TOH_Game::run_trials(3);

  return 0;
}
