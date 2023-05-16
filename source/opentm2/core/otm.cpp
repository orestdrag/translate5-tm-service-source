
#include "otm.h"
#include "tm.h"

NewTM::NewTM(const std::string& tmName){
  TmBtree.fb.fileName = FilesystemHelper::GetTmdPath(tmName);
  InBtree.fb.fileName = FilesystemHelper::GetTmiPath(tmName);
}
