// This file is part of Hermes2D.
//
// Hermes2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Hermes2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hermes2D.  If not, see <http://www.gnu.org/licenses/>.

#include "common.h"
#include "hermes2d.h"


void hermes2d_initialize(int* argc, char* argv[])
{
  warn("W this function is deprecated.");
}


void hermes2d_finalize(bool force_quit)
{
  warn("W this function is deprecated.");
  //free_ortho_base();
  // TODO: free matrices in curved.cpp
  // TODO: free bases in H1OrthoHp, ...
}
