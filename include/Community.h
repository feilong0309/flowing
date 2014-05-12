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

#ifndef COMMUNITY_H
#define COMMUNITY_H

#include "Types.h"
#include <set>

namespace flowing {

    class Community {
        public:
            Community( unsigned int id );
            ~Community();

            /** @brief Checks if the node exists into the community.
             *  @param[in] id The node to check if exists.
             *  @return true if the node was already into the community. false otherwise.*/
            bool Exists( unsigned int id );

            /** @brief Inserts a node into the community.
             *  @param[in] id The node to insert.*/
            void Insert( unsigned int id );

            /** @brief Removes a node from the community.
             *  @param[in] id The node to remove.*/
            void Remove( unsigned int id );

        private:
            unsigned int m_CommunityId;
            std::set<unsigned int> m_Nodes; 
    };
}

#endif

