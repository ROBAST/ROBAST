#include <iostream>
#include "Rtypes.h"
#include "AGeoUtil.h"

#if !defined(R__ALPHA) && !defined(R__SOLARIS) && !defined(R__ACC) && !defined(R__FBSD)
NamespaceImp(AGeoUtil)
#endif

void AGeoUtil::test(){
  std::cout << "test\n";
}

