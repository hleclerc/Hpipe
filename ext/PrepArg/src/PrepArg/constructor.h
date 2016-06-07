/*
 Copyright 2012 Structure Computation  www.structure-computation.com
 Copyright 2012 Hugo Leclerc

 This file is part of PrepArg.

 PrepArg is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 PrepArg is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with PrepArg. If not, see <http://www.gnu.org/licenses/>.
*/


// load STRCMP, ... if not done
#include "default_values.h"

// decl
#define SARG( SHORT, VAR, HELP, DEFAULT_VALUE ) VAR = DEFAULT_VALUE
#define BARG( SHORT, VAR, HELP, DEFAULT_VALUE ) VAR = DEFAULT_VALUE
#define IARG( SHORT, VAR, HELP, DEFAULT_VALUE ) VAR = DEFAULT_VALUE
#define EARG( VAR, HELP ) VAR = -1
#define DESCRIPTION( TXT )

// code
#include PREPARG_FILE
#include "undefs.h"
