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

#ifndef STREAM_GRAPH_H
#define STREAM_GRAPH_H

#include "BufferPool.h"
#include "Types.h"
#include <iostream>
#include <vector>
#include <map>
#include <list>


namespace flowing {

#define FLOWING_NUM_PAGES 1024*1024
#define FLOWING_PAGE_SIZE 8*sizeof(unsigned int)

typedef std::map<unsigned int, unsigned int> UUMap;
typedef std::vector<unsigned int> UVector;


    /** @brief  This class represents a graph, where the edges are being inserted as 
                a stream. The amount of memory available to store the edges is limited.*/
    class StreamGraph {

        private:

            /** @brief Represents a page of adjacencies.*/
            struct AdjacencyPage {
                unsigned int*       m_Buffer;          /**< @brief A pointer to the buffer holding the adjacencies.*/
                int                 m_NumAdjacencies;  /**< @brief The number of adjacencies that are in the buffer.*/
                int                 m_MaxAdjacencies;  /**< @brief The maximum number of adjacencies that can fit into the buffer.*/
                AdjacencyPage*      m_Next;            /**< @brief The next page into the page list.*/
                AdjacencyPage*      m_Previous;        /**< @brief The previous page into the page list.*/
                unsigned int        m_NodeId;          /**< @brief The id of the node this page belongs to.*/
            };

            /** @brief Allocates an AdjacencyPage using the given buffer.
                @param[in] buffer The buffer that will hold the adjacency data.
                @param[in] size The size of the buffer in bytes.
                @return The allocated AdjacencyPage. */
            AdjacencyPage* AllocateAdjacencyPage( void* buffer, const int size );

            /** @brief Frees an AdjacencyPage.
                @param[in] The page to free.*/
            void             FreeAdjacencyPage( AdjacencyPage* page );

            /** @brief Represents a list of adjacencies.*/
            struct AdjacencyList {
                AdjacencyPage* m_First;     /**< @brief The first page of the list.*/
                AdjacencyPage* m_Last;      /**< @brief The last page of the list.*/
                int            m_Degree;  /**< @brief The number of adjacencies in the list.*/  
            };

            /** @brief Allocated an AdjacencyList.
                @return The allocated AdjacencyList.*/
            AdjacencyList* AllocateAdjacencyList();

            /** @brief Frees an AdjacencyPage.
                @param[in] adjacencyList The AdjacencyList to free*/
            void             FreeAdjacencyList( AdjacencyList* adjacencyList );

        public:

            class AdjacencyIterator {
                public:
                    ~AdjacencyIterator();

                    /** @brief Tells if there are more adjacencies to look at.
                     *  @return true if there are more adjacencies. false otherwise.*/
                    bool HasNext();

                    /** @brief Get the next adjacency of this iterator.
                     *  @param The next adjacency.*/
                    unsigned int Next();

                private:
                    friend class StreamGraph;
                    AdjacencyIterator( const AdjacencyList* adjacencyList );

                    const AdjacencyList* const m_AdjacencyList;   /**< @brief The adjacency list to iterate.*/
                    const AdjacencyPage* m_CurrentPage;           /**< @brief The current page in the adjacency list being iterated.*/
                    int m_CurrentIndex;                           /**< @brief The current index into the page being iterated.*/
                    
            };


            enum EdgeMode {
                UNDIRECTED,
                DIRECTED
            };

            StreamGraph( const EdgeMode mode, void (*function)(Edge*,int), int batchSize );
            ~StreamGraph();

            /** @brief Initializes the stream graph.
                @param[in] True if the initialization was successful*/
            bool Initialize();

            /** @brief Closes the stream graph by freeing all the used resources.*/
            void Close();

            /** @brief Pushes all the edges (tail,head) pairs that arrive from an input stream.
                @param[in] stream The stream to read from. */
            void Push( std::istream& stream );

            /** @brief Pushes an edge.
                @param[in] tail The tail of the edge.
                @param[in] head The head of the edge.
                @param[in] weight The weight of the edge.*/
            void Push( const unsigned int tail, const unsigned int head, const double weight = 1.0 );

            /** @brief Gets the adjacency iterator of a given node.
             *  @param[in] The node to get the adjacency iterator.
             *  @return The adjacency iterator.*/
            AdjacencyIterator Iterator( const unsigned int nodeId );

            /** @brief Gets the number of nodes in the graph.
             *  @return The number of nodes.*/
            unsigned int NumNodes();


        private:


            /** @brief Pushes a directed edge.
                @param[in] tail The tail of the edge.
                @param[in] head The head of the edge.
                @param[in] weight The weight of the edge.*/
            void PushDirected( const unsigned int tail, const unsigned int head, const double weight = 1.0 );

            /** @brief Pushes an undirected edge.
                @param[in] tail The tail of the edge.
                @param[in] head The head of the edge.
                @param[in] weight The weight of the edge.*/
            void PushUndirected( const unsigned int tail, const unsigned int head, const double weight = 1.0 );


            /** @brief Inserts an adjacency.
                @param[in] tail The tail of the edge.
                @param[in] head The head of the edge.*/
            void InsertAdjacency( const unsigned int tail, const unsigned int head );

            /** @brief Gets a new page to use in an adjacency list.*/
            AdjacencyPage*  GetNewPage();

            /** @brief Gets the internal id corresponding to the given one.
                @param[in] id The id to retrieve.
                @return The internal id.*/
            unsigned int GetInternalId( const unsigned int id );

            int                                     m_NumPushedEdges;   /**< @brief The number of pushed edges into the graph.*/
            int                                     m_NextId;           /**< @brief The next new identifier to assign.*/
            EdgeMode                                m_Mode;             /**< @brief The mode of the graph (DIRECTED or UNDIRECTED).*/
            BufferPool                              m_BufferPool;       /**< @brief The buffer pool.*/
            std::vector<AdjacencyList*>             m_Adjacencies;      /**< @brief The graph adjacencies.*/
            std::list<AdjacencyPage*>               m_Pages;            /**< @brief A list of pages in LRU to decide which to remove.*/
            UUMap                                   m_Map;              /**< @brief The old to new identifier map.*/
            UVector                                 m_Remap;            /**< @brief The new to old identifier map.*/
            void (*m_Processor)( Edge*, int );                          /**< @brief Function pointer to the function used to process the edges.*/
            int                                     m_BatchSize;        /**< @brief The size of the batch to process.*/ 
            int                                     m_NumInBatch;       /**< @brief The number of elements in the batch.*/
            Edge*                                   m_Batch;
    };

}
#endif

