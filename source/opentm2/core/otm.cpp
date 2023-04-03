
#include "otm.h"
#include "tm.h"

NewTM::NewTM(const std::string& tmName){
  TmBtree.fb.fileName = NewTMManager::GetTmdPath(tmName);
  InBtree.fb.fileName = NewTMManager::GetTmiPath(tmName);
}
