#pragma once

extern "C"
{

  /**
  Reads a file and mounts the bfs files listed therein.
  Does nothing if FO2_detail_mountFromFileList == nullptr.
  **/
  void FO2_mountFromFileList( const char* filename );

}
