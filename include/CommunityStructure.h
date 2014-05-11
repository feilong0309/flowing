/*Flowing is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  SCD is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef COMMUNITY_STRUCTURE_H
#define COMMUNITY_STRUCTURE_H

#include "Types.h"
#include "StreamGraph.h"

namespace flowing {
    class CommunityStructure {
        public:
            CommunityStructure( StreamGraph* graph );
            ~CommunityStructure();

        private:
            const StreamGraph*    m_Graph;    /**< @brief The graph to compute the community structure from.*/
    };
}

#endif

