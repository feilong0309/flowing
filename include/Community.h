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
#include "StreamGraph.h"
#include <set>

namespace flowing {

    class Community {
        public:
            class CommunityIterator {
                public:
                    ~CommunityIterator();

                    /** @brief Tells if there is another member to iterate to.
                     *  @return true if there are remaining elements. false otherwise.*/
                    bool HasNext();

                    /** @brief Gets the next node in the community.
                     *  @return  The next node in the community.*/
                    unsigned int Next();

                private:
                    friend class Community;

                     /** @param community The community this iterator belongs to.*/
                    CommunityIterator( const Community* community );
                    const Community* const m_Community; 
                    std::set< unsigned int >::iterator m_Iterator;
            };

            /** param[in] graph The graph this community belongs to.
             *  param[in] id The identifier of the community.*/
            Community( StreamGraph* graph, unsigned int id );
            ~Community();

            /** @brief Checks if the node exists into the community.
             *  @param[in] id The node to check if exists.
             *  @return true if the node was already into the community. false otherwise.*/
            bool Exists( unsigned int id ) const;

            /** @brief Inserts a node into the community.
             *  @param[in] id The node to insert.*/
            void Insert( unsigned int id );

            /** @brief Removes a node from the community.
             *  @param[in] id The node to remove.*/
            void Remove( unsigned int id );

            /** @brief Gets the size of the community.
             *  @return The size of the community.*/
            int Size() const;

            /** @brief Tests the score of the community if a node is inserted.
             *  @param[in] nodeId The node to insert.
             *  @return The score of the community if a node was inserted.*/
            double TestInsert( unsigned int nodeId ) const;

            /** @brief Tests the score of the community if a node is removed.
             *  @param[in] nodeId The node to remove.
             *  @return The score of the community if a node was removed.*/
            double TestRemove( unsigned int nodeId ) const ;

            /** @brief Gets the score of the community.
             *  @return The score of the community.*/
            double Score() const ;

            /** @brief Gets the id of the community.
             *  @return The id of the community.*/
            unsigned int Id() const ;

            /** @brief Obtains an iterator of the community.
             * @return An iterator of the community.*/ 
            CommunityIterator Iterator() const;

            void SignalInsertInternalEdge();
            void SignalInsertExternalEdge();
            void SignalRemoveInternalEdge();
            void SignalRemoveExternalEdge();

        private:

            /** @brief Tests the score of the community if a node is inserted.
             *  @param[in] nodeId The node to insert.
             *  @param[out] newKin The new Kin of the community.
             *  @param[out] newKout The new Kout of the community.
             *  @return The score of the community if a node was inserted.*/
            double TestInsert( unsigned int nodeId, unsigned int& newKin, unsigned int& newKout ) const;

            /** @brief Tests the score of the community if a node is removed.
             *  @param[in] nodeId The node to remove.
             *  @param[out] newKin The new Kin of the community.
             *  @param[out] newKout The new Kout of the community.
             *  @return The score of the community if a node was removed.*/
            double TestRemove( unsigned int nodeId, unsigned int& newKin, unsigned int& newKout ) const ;

            unsigned int            m_CommunityId;  /**< @brief The id of the community.*/
            std::set<unsigned int>  m_Nodes;        /**< @brief The set of nodes in the community.*/   
            StreamGraph* const      m_Graph;        /**< @brief The graph this community belongs to.*/
            int                     m_Kin;          /**< @brief Internal degree of the community.*/
            int                     m_Kout;         /**< @brief External degree of the community.*/
    };
}

#endif

