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


#include "Community.h"

namespace flowing {

    Community::Community( unsigned int id ) :
        m_CommunityId( id )  {
    }

    Community::~Community() {
    }

    bool Community::Exists( unsigned int id ) {
        return m_Nodes.find(id) != m_Nodes.end();
    }

    void Community::Insert( unsigned int id ) {
        m_Nodes.insert( id );
    }

    void Community::Remove( unsigned int id ) {
        m_Nodes.erase( id );
    }
}
