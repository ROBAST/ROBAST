#include "stdio.h"

#ifndef SKIP_ROBAST_WELCOME
__attribute__((constructor)) static void PrintReference() {
  // clang-format off
  printf(" ----------------------------------------------------------------------------\n"
         "| Welcome to ROBAST                               https://robast.github.io/  |\n"
         "|                                                                            |\n"
         "| Please cite the following paper when you publish your ROBAST simulation.   |\n"
         "|                                                                            |\n"
         "| Akira Okumura, Koji Noda, Cameron Rulten (2016)                            |\n"
         "| \"ROBAST: Development of a ROOT-Based Ray-Tracing Library for Cosmic-Ray    |\n"
         "| Telescopes and its Applications in the Cherenkov Telescope Array\"          |\n"
         "| \e[3mAstroparticle Physics\e[0m \e[1m76\e[0m 38-47                                             |\n"
         "|                                                                            |\n"
         "| For support & FAQ, please visit https://robast.github.io/support.html      |\n"
         "|                                                                            |\n"
         "|                       ROBAST is developed by Akira Okumura (oxon@mac.com)  |\n"
         "|                                                                            |\n"
         "|  * You can remove this welcome message by adding a make option *           |\n"
         "|  $ make ROBASTFLAGS=-DSKIP_ROBAST_WELCOME                                  |\n"
         " ----------------------------------------------------------------------------\n");
  // clang-format on
}
#endif
