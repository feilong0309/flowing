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
#include <assert.h>

namespace flowing {

    // COMMUNITY ITERATOR METHODS

    Community::CommunityIterator::CommunityIterator( const Community* community ) :
            m_Community(community)
    {
        m_Iterator = community->m_Nodes.begin();
    }

    Community::CommunityIterator::~CommunityIterator() {

    }

    bool Community::CommunityIterator::HasNext() {
        return m_Iterator != m_Community->m_Nodes.end();
    }

    unsigned int Community::CommunityIterator::Next() {
        unsigned int node = *m_Iterator;
        ++m_Iterator;
        return node;
    }

    // COMMUNITY METHODS

    Community::Community( StreamGraph* graph, unsigned int id ) :
        m_CommunityId( id ), 
        m_Graph( graph ),
        m_Kin( 0 ), 
        m_Kout( 0 ) {
    }

    Community::~Community() {
    }

    bool Community::Exists( unsigned int id ) const {
        return m_Nodes.find(id) != m_Nodes.end();
    }

    void Community::Insert( unsigned int id ) {
        assert( m_Nodes.find(id) == m_Nodes.end());
        unsigned int newKin;
        unsigned int newKout;
        TestInsert( id, newKin, newKout );
        m_Nodes.insert( id );
        m_Kin = newKin;
        m_Kout = newKout;
    }

    void Community::Remove( unsigned int id ) {
        assert( m_Nodes.find(id) != m_Nodes.end());
        unsigned int newKin;
        unsigned int newKout;
        TestRemove( id, newKin, newKout );
        m_Nodes.erase( id );
        m_Kin = newKin;
        m_Kout = newKout;
    }

    int Community::Size() const {
        return m_Nodes.size();
    }

    double Community::TestInsert( unsigned int nodeId, unsigned int& newKin, unsigned int& newKout ) const {
        assert( m_Nodes.find(nodeId) == m_Nodes.end() );
        int nodeKin = 0;
        int nodeKout = 0;
        StreamGraph::AdjacencyIterator iterNode = m_Graph->Iterator( nodeId );
        while( iterNode.HasNext() ) {
            unsigned int neighbor = iterNode.Next();
            if( Exists( neighbor ) ) ++nodeKin;
            else ++nodeKout;
        }
        // New score
        newKin = m_Kin + 2*nodeKin;
        newKout = m_Kout - nodeKin;
        newKout += nodeKout;
        int denom = newKin + newKout + (this->Size()+1)*(this->Size()) - newKin;
        return denom > 0 ? newKin / (double)denom : 0;
    }

    double Community::TestRemove( unsigned int nodeId, unsigned int& newKin, unsigned int& newKout ) const {
        assert( m_Nodes.find(nodeId) != m_Nodes.end() );
        int nodeKin = 0;
        int nodeKout = 0;
        StreamGraph::AdjacencyIterator iterNode = m_Graph->Iterator( nodeId );
        while( iterNode.HasNext() ) {
            unsigned int neighbor = iterNode.Next();
            if( Exists( neighbor ) ) ++nodeKin;
            else ++nodeKout;
        }
        // New score
        newKin = m_Kin - 2*nodeKin;
        newKout = m_Kout + nodeKin;
        newKout -= nodeKout;
        int denom = newKin + newKout + (this->Size()+1)*(this->Size()) - newKin;
        return denom > 0 ? newKin / (double)denom : 0;
    }

    double Community::TestInsert( unsigned int nodeId ) const {
        unsigned int kin;
        unsigned int kout;
        return TestInsert( nodeId, kin, kout );
    }

    double Community::TestRemove( unsigned int nodeId ) const {
        unsigned int kin;
        unsigned int kout;
        return TestRemove( nodeId, kin, kout );
    }

    double Community::Score() const {
        int denom = m_Kin + m_Kout + (Size()+1)*(Size()) - m_Kin;
        double score = denom > 0 ? m_Kin / (double)denom : 0;
//        assert((score <= 1.0) && (score >= 0.0));
        return score;
    }

    unsigned int Community::Id() const {
        return m_CommunityId;
    }

    Community::CommunityIterator Community::Iterator() const {
        return CommunityIterator( this );
    }
}
