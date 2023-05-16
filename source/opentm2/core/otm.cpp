
#include "otm.h"
#include "tm.h"

NewTM::NewTM(const std::string& tmName){
  TmBtree.fb.fileName = TMManager::GetTmdPath(tmName);
  InBtree.fb.fileName = TMManager::GetTmiPath(tmName);
}
