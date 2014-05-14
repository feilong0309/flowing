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

#include "Types.h"
#include "StreamGraph.h"
#include <cstdlib>
#include <assert.h>

namespace flowing {


    /// ADJACENCY PAGE METHODS

    StreamGraph::AdjacencyPage* StreamGraph::AllocateAdjacencyPage( void* buffer, const int size ) {
        AdjacencyPage* page = (AdjacencyPage*)malloc(sizeof(AdjacencyPage));
        if( page == NULL ) return NULL;
        page->m_Buffer = (unsigned int*)buffer; 
        page->m_NumAdjacencies = 0;
        page->m_MaxAdjacencies = size / sizeof(unsigned int);
        page->m_Next = NULL;
        page->m_Previous = NULL;
        page->m_NodeId = -1;
        return page;
    }

    void StreamGraph::FreeAdjacencyPage( AdjacencyPage* page ) {
        if( page->m_Next != NULL ) {
            page->m_Next->m_Previous = NULL;
        }
        if( page->m_Previous != NULL ) {
            page->m_Previous->m_Next = NULL;
        }
        free(page);
    }

    /// ADJACENCY LIST METHODS
    
    StreamGraph::AdjacencyList* StreamGraph::AllocateAdjacencyList() {
        AdjacencyList* list = (AdjacencyList*)malloc(sizeof(AdjacencyList));
        if( list == NULL ) return NULL;
        list->m_First = NULL;
        list->m_Last = NULL;
        list->m_Degree = 0;  
        return list;
    }

    void StreamGraph::FreeAdjacencyList( StreamGraph::AdjacencyList* adjacencyList ) {
        free(adjacencyList);
    }

    /// ADJACENCY ITERATOR METHODS

    StreamGraph::AdjacencyIterator::AdjacencyIterator( const AdjacencyList* adjacencyList ) :
            m_AdjacencyList( adjacencyList ) {
            m_CurrentPage = m_AdjacencyList != NULL ? m_AdjacencyList->m_First : NULL;
            m_CurrentIndex = 0;
    }

    StreamGraph::AdjacencyIterator::~AdjacencyIterator() {
        
    }

    bool StreamGraph::AdjacencyIterator::HasNext() {
       return   (m_AdjacencyList != NULL) &&
                (m_CurrentPage != NULL) &&
                (m_CurrentIndex < m_CurrentPage->m_NumAdjacencies);

    }

    unsigned int StreamGraph::AdjacencyIterator::Next() {
        unsigned int retValue = m_CurrentPage->m_Buffer[m_CurrentIndex++];
        if( m_CurrentIndex >= m_CurrentPage->m_NumAdjacencies ) {
            m_CurrentIndex = 0;
            m_CurrentPage = m_CurrentPage->m_Next;
        }
        return retValue;
    }


    /// STREAM GRAPH METHODS 

    StreamGraph::StreamGraph(   const EdgeMode mode, 
                                void (*processor)( StreamGraph* graph, Edge*,int),
                                void* (*nodeDataAllocate)(  StreamGraph* graph, unsigned int ),
                                void (*nodeDataFree)( StreamGraph* graph, unsigned int, void* ),
                                int batchSize ) :
        m_BufferPool( FLOWING_NUM_PAGES, FLOWING_PAGE_SIZE ) {
        m_Mode = mode;
        m_NextId = 0;
        m_NumPushedEdges = 0;
        m_Processor = processor;
        m_NodeDataAllocate = nodeDataAllocate;
        m_NodeDataFree = nodeDataFree;
        m_BatchSize = batchSize > 0 ? batchSize : 1;
        m_Batch = NULL;
        m_NumInBatch = 0;
    }

    StreamGraph::~StreamGraph() {

    }

    bool StreamGraph::Initialize() {
        m_Batch = (Edge*)malloc(sizeof(Edge)*m_BatchSize); 
        return m_BufferPool.Initialize();
    }

    void StreamGraph::Close() {
        if(m_NumInBatch > 0) {
          //  std::cout << "Processing batch ..." << std::endl;
            m_Processor( this, m_Batch, m_NumInBatch);
        }

        // FREE MEMORY
        free(m_Batch);
        for( std::list<AdjacencyPage*>::iterator it = m_Pages.begin(); it != m_Pages.end(); ++it ) {
            FreeAdjacencyPage(*it);
        }

        for( int i = 0; i < m_Adjacencies.size(); ++i ) {
            FreeAdjacencyList( m_Adjacencies[i] );
            m_NodeDataFree( this, i, m_NodeData[i] );
        }
        m_BufferPool.Close();
    }

    void StreamGraph::Push( std::istream& stream ) {
        unsigned int tail;
        while( stream >> tail ) {
            unsigned int head;
            stream >> head;
            Push(tail,head);
        }
    }

    void StreamGraph::Push( const unsigned int tail, const unsigned int head, const double weight ) {
        unsigned int internalTail = GetInternalId(tail);
        unsigned int internalHead = GetInternalId(head);
        if( m_Mode == UNDIRECTED ) {
            PushUndirected(internalTail,internalHead);
        } else if( m_Mode == DIRECTED ) {
            PushDirected(internalTail,internalHead);
        }

        if( m_NumInBatch < m_BatchSize ) {
            m_Batch[m_NumInBatch].m_Tail = internalTail;
            m_Batch[m_NumInBatch].m_Head = internalHead;
            m_NumInBatch++;
        } else {
//            std::cout << "Processing batch ..." << std::endl;
            m_Processor( this, m_Batch, m_NumInBatch ); 
            m_NumInBatch = 0;
        }

        m_NumPushedEdges++;
        if( m_NumPushedEdges % 10000  == 0 ) {
            std::cout << "Number of edges read: " << m_NumPushedEdges << std::endl;
            std::cout << "\t " << m_BufferPool.NumFreeBuffers() << "/" << m_BufferPool.MaxNumBuffers() << std::endl;
        }
    }

    StreamGraph::AdjacencyIterator StreamGraph::Iterator( const unsigned int nodeId ) const {
       AdjacencyList* list = NULL;
       if( nodeId < m_NextId ) {
           list = m_Adjacencies[nodeId];
       }

       AdjacencyIterator iterator( list );
       return iterator;
    }

    unsigned int StreamGraph::NumNodes() const {
        return m_NextId;
    }

    void* StreamGraph::GetNodeData( unsigned int id ) {
        return m_NodeData[id];
    }

    void StreamGraph::SetNodeData( unsigned int id, void* nodeData ) {
        m_NodeData[id] = nodeData;
    }

    unsigned int StreamGraph::Remap( unsigned int id ) {
        return m_Remap[id];
    }

    void StreamGraph::PushDirected( const unsigned int tail, const unsigned int head, const double weight ) {
        InsertAdjacency(tail, head);
    }

    void StreamGraph::PushUndirected( const unsigned int tail, const unsigned int head, const double weight ) {
        InsertAdjacency(tail, head);
        InsertAdjacency(tail, head);
    }

    StreamGraph::AdjacencyPage* StreamGraph::GetNewPage() {
        void* buffer = m_BufferPool.NextBuffer();
        if( buffer == NULL ) {
            StreamGraph::AdjacencyPage* page = m_Pages.front();
            m_Pages.pop_front();
            unsigned int id = page->m_NodeId;
            if( m_Adjacencies[id]->m_First == page ) {
                m_Adjacencies[id]->m_First = page->m_Next;
                if( m_Adjacencies[id]->m_First != NULL ) {
                    m_Adjacencies[id]->m_First->m_Previous = NULL;
                }
                page->m_Next = NULL;
            }

            if( m_Adjacencies[id]->m_Last == page ) {
                m_Adjacencies[id]->m_Last = page->m_Previous;
                if( m_Adjacencies[id]->m_Last != NULL ) {
                    m_Adjacencies[id]->m_Last->m_Next = NULL;
                }
                page->m_Previous = NULL;
            }

            if( page->m_Next != NULL ) {
                page->m_Next->m_Previous = page->m_Previous;
            }

            if( page->m_Previous != NULL ) {
                page->m_Previous->m_Next = page->m_Next;
            }
            buffer = page->m_Buffer;
            FreeAdjacencyPage(page);
        }
        AdjacencyPage* page =  AllocateAdjacencyPage( buffer, FLOWING_PAGE_SIZE );
        m_Pages.push_back(page);
        return page;
    }

    
    unsigned int StreamGraph::GetInternalId( const unsigned int id ) {
        UUMap::iterator it = m_Map.find(id);
        if( it == m_Map.end() ) {                                                           // If this is a new node, assign an internal id and initialize its adjacency list.
            it = m_Map.insert(std::pair<unsigned int, unsigned int>( id, m_NextId )).first;;
            m_Remap.push_back(id);
            AdjacencyList* list = AllocateAdjacencyList();
            m_Adjacencies.push_back(list);            
            m_NodeData.push_back( m_NodeDataAllocate( this, m_NextId ) );
            m_NextId++;
        }
        return (*it).second;
    }

    void StreamGraph::InsertAdjacency( const unsigned int tail, const unsigned int head ) {
        AdjacencyList* listTail = m_Adjacencies[tail];    
        if( listTail->m_First == NULL ) {                                                   // Check if the list do not have any page.
            listTail->m_First = GetNewPage(); 
            listTail->m_First->m_NodeId = tail;
            listTail->m_Last = listTail->m_First;
        }

        if( listTail->m_Last->m_NumAdjacencies == listTail->m_Last->m_MaxAdjacencies ) {    // Check if we need a new page for the list.
            AdjacencyPage* newPage  = GetNewPage();                                         // IMPORTANT: GetNewPage can modify the address pointed by listTail->m_Last.
            if( listTail->m_Last == NULL ) {
                assert(listTail->m_First == NULL);
                listTail->m_First = listTail->m_Last = newPage;
                listTail->m_First->m_Next = listTail->m_First->m_Previous = NULL;
            } else {
                listTail->m_Last->m_Next = newPage;
                newPage->m_Previous = listTail->m_Last;
                listTail->m_Last = newPage;
            }
            listTail->m_Last->m_NodeId = tail;
       }
        listTail->m_Last->m_Buffer[listTail->m_Last->m_NumAdjacencies++] = head;
        listTail->m_Degree++;
    }
}

