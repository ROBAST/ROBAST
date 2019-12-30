#ifndef ROBAST_INIT_H
#define ROBAST_INIT_H

#include "stdio.h"

#ifndef SKIP_ROBAST_WELCOME
__attribute__((constructor)) static void PrintReference() {
  // clang-format off
  printf(" ------------------------------------------------------------------------------\n"
         "| Welcome to ROBAST                                 https://robast.github.io/  |\n"
         "|   ROBAST is developed by Akira Okumura (oxon@mac.com)                        |\n"
         "|   For support & FAQ, please visit https://robast.github.io/support.html      |\n"
         "|                                                                              |\n"
         "| Please cite the following paper when you publish your ROBAST simulation.     |\n"
         "|   Akira Okumura, Koji Noda, Cameron Rulten (2016)                            |\n"
         "|   \"ROBAST: Development of a ROOT-Based Ray-Tracing Library for Cosmic-Ray    |\n"
         "|   Telescopes and its Applications in the Cherenkov Telescope Array\"          |\n"
         "|   \e[3mAstroparticle Physics\e[0m \e[1m76\e[0m 38-47                                             |\n"
         "|                                                                              |\n"
         "| If you use the AMultilayer class, it is recommended to cite the following    |\n"
	 "| paper as well, because the calculation algorithm was taken from it and the   |\n"
         "| software package tmm.py (see https://pypi.org/project/tmm/).                 |\n"
         "|   Steven J. Byrnes (2016) \"Multilayer optical calculations\" arXiv:1603.02720 |\n"
         "|                                                                              |\n"
         "| You can remove this welcome message by adding a make option.                 |\n"
         "|   $ make ROBASTFLAGS=-DSKIP_ROBAST_WELCOME                                   |\n"
         " ------------------------------------------------------------------------------\n");
  // clang-format on
}
#endif

#endif // ROBAST_INIT_H
