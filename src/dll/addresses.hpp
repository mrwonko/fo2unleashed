#pragma once

extern "C"
{
  // location of mountFromFileList function in FO2
  extern const void* FO2_detail_mountFromFileList;
  // address after hook point in mounting function
  extern const void* FO2_detail_mountReturn;
}
