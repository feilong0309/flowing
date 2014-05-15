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
        page->m_Buffer = (Edge*)buffer; 
        page->m_NumEdges = 0;
        page->m_MaxEdges = size / sizeof(Edge);
        return page;
    }

    void StreamGraph::FreeAdjacencyPage( AdjacencyPage* page ) {
        assert(page);
        free(page);
    }

    /// ADJACENCY LIST NODE METHODS

    StreamGraph::AdjacencyListNode* StreamGraph::AllocateAdjacencyListNode() {
        AdjacencyListNode* node = (AdjacencyListNode*)malloc(sizeof(AdjacencyListNode));
        if( node == NULL ) return NULL;
        node->m_Next = NULL;
        node->m_Previous = NULL;
        node->m_Page = NULL;
        return node;
    }

    void StreamGraph::FreeAdjacencyListNode( AdjacencyListNode* adjacencyListNode ) {
        assert(adjacencyListNode);
    }

    /// ADJACENCY LIST METHODS
    
    StreamGraph::AdjacencyList* StreamGraph::AllocateAdjacencyList( unsigned int id ) {
        AdjacencyList* list = (AdjacencyList*)malloc(sizeof(AdjacencyList));
        if( list == NULL ) return NULL;
        list->m_Node = id;
        list->m_First = NULL;
        list->m_Last = NULL;
        return list;
    }

    void StreamGraph::FreeAdjacencyList( StreamGraph::AdjacencyList* adjacencyList ) {
        free(adjacencyList);
    }

    /// ADJACENCY ITERATOR METHODS

    StreamGraph::AdjacencyIterator::AdjacencyIterator( const AdjacencyList* adjacencyList, StreamGraph::EdgeMode mode ) :
            m_AdjacencyList( adjacencyList ),
            m_EdgeMode( mode ) {
            m_CurrentNode = m_AdjacencyList != NULL ? m_AdjacencyList->m_First : NULL;
            m_CurrentIndex = 0;
    }

    StreamGraph::AdjacencyIterator::~AdjacencyIterator() {
        
    }

    bool StreamGraph::AdjacencyIterator::HasNext() {
        if( (m_AdjacencyList == NULL) || (m_AdjacencyList->m_First == NULL) ) return false;
        while( m_CurrentNode != NULL ) {
            for( ; m_CurrentIndex < m_CurrentNode->m_Page->m_NumEdges; ++m_CurrentIndex ) {
                Edge* edge = &m_CurrentNode->m_Page->m_Buffer[m_CurrentIndex];
                if( (edge->m_Tail == m_AdjacencyList->m_Node) )  {
                    return true;
                }
                if( m_EdgeMode == UNDIRECTED && (edge->m_Head == m_AdjacencyList->m_Node) ) {
                    return true;
                }
            }
            m_CurrentNode = m_CurrentNode->m_Next;
            m_CurrentIndex = 0;
        }
        return false;
    }

    unsigned int StreamGraph::AdjacencyIterator::Next() {
        Edge* edge = &m_CurrentNode->m_Page->m_Buffer[m_CurrentIndex++];
        return edge->m_Tail == m_AdjacencyList->m_Node ? edge->m_Head : edge->m_Tail;
    }


    /// STREAM GRAPH METHODS 

    StreamGraph::StreamGraph(   const EdgeMode mode, 
                                void (*insert)( StreamGraph* graph, Edge*, int ),
                                void (*remove)( StreamGraph* graph, Edge*, int ),
                                void* (*nodeDataAllocate)(  StreamGraph* graph, unsigned int ),
                                void (*nodeDataFree)( StreamGraph* graph, unsigned int, void* ),
                                int batchSize ) :

        m_BufferPool( FLOWING_NUM_PAGES, FLOWING_PAGE_SIZE ) {
        m_EdgeMode = mode;
        m_NextId = 0;
        m_NumPushedEdges = 0;
        m_Insert = insert;
        m_Remove = remove;
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
            m_Insert( this, m_Batch, m_NumInBatch);
        }

        // FREE MEMORY
        free(m_Batch);
        for( std::list<AdjacencyPage*>::iterator it = m_Pages.begin(); it != m_Pages.end(); ++it ) {
            FreeAdjacencyPage(*it);
        }

        for( unsigned int i = 0; i < m_Adjacencies.size(); ++i ) {
            AdjacencyListNode* node = m_Adjacencies[i]->m_First;
            while( node != NULL ) {
                AdjacencyListNode* aux = node;
                node = node->m_Next;
                FreeAdjacencyListNode(aux);
            };
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
        InsertAdjacency( internalTail, internalHead );

        if( m_NumInBatch < m_BatchSize ) {
            m_Batch[m_NumInBatch].m_Tail = internalTail;
            m_Batch[m_NumInBatch].m_Head = internalHead;
            m_NumInBatch++;
        } 
        
        if( m_NumInBatch == m_BatchSize ) {
//            std::cout << "Processing batch ..." << std::endl;
            m_Insert( this, m_Batch, m_NumInBatch ); 
            m_NumInBatch = 0;
        }

        m_NumPushedEdges++;
        if( m_NumPushedEdges % 10000  == 0 ) {
            std::cout << "Number of edges read: " << m_NumPushedEdges << std::endl;
            std::cout << "\t " << m_BufferPool.NumFreeBuffers() << "/" << m_BufferPool.MaxNumBuffers() << std::endl;
        }
    }

    StreamGraph::AdjacencyIterator StreamGraph::Iterator( const unsigned int nodeId ) const {
        AdjacencyIterator iterator( m_Adjacencies[nodeId], m_EdgeMode );
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

    StreamGraph::AdjacencyPage* StreamGraph::GetNewPage() {
        void* buffer = m_BufferPool.NextBuffer();
        StreamGraph::AdjacencyPage* page = NULL;
        if( buffer == NULL ) {
            page = m_Pages.front();
            m_Pages.pop_front();
            m_Remove( this, page->m_Buffer, page->m_NumEdges );
            for( int i = 0; i < page->m_NumEdges; ++i ) {
                unsigned int tail = page->m_Buffer[i].m_Tail;
                unsigned int head = page->m_Buffer[i].m_Head;
                if( (m_Adjacencies[tail]->m_First != NULL) && (m_Adjacencies[tail]->m_First->m_Page == page) ) { 
                    AdjacencyListNode* aux = m_Adjacencies[tail]->m_First;
                    m_Adjacencies[tail]->m_First = aux->m_Next;
                    FreeAdjacencyListNode( aux );

                    if( m_Adjacencies[tail]->m_First == NULL ) {
                        m_Adjacencies[tail]->m_Last = NULL;
                    }
                }

                if( (m_Adjacencies[head]->m_First != NULL) && (m_Adjacencies[head]->m_First->m_Page == page) ) { 
                    AdjacencyListNode* aux = m_Adjacencies[head]->m_First;
                    m_Adjacencies[head]->m_First = aux->m_Next;
                    FreeAdjacencyListNode( aux );

                    if( m_Adjacencies[head]->m_First == NULL ) {
                        m_Adjacencies[head]->m_Last = NULL;
                    }
                }
            }
            page->m_NumEdges = 0;
        } else {
            page =  AllocateAdjacencyPage( buffer, FLOWING_PAGE_SIZE );
        }
        return page;
    }

    
    unsigned int StreamGraph::GetInternalId( const unsigned int id ) {
        UUMap::iterator it = m_Map.find(id);
        if( it == m_Map.end() ) {                                                           // If this is a new node, assign an internal id and initialize its adjacency list.
            it = m_Map.insert(std::pair<unsigned int, unsigned int>( id, m_NextId )).first;
            m_Remap.push_back(id);
            AdjacencyList* list = AllocateAdjacencyList( m_NextId );
            m_Adjacencies.push_back(list);            
            m_NodeData.push_back( m_NodeDataAllocate( this, m_NextId ) );
            m_NextId++;
        }
        return (*it).second;
    }

    void StreamGraph::InsertAdjacency( const unsigned int tail, const unsigned int head ) {
        AdjacencyPage* page = NULL;
        if( m_Pages.size() > 0 ) {
            page = m_Pages.back();  
        }
        if( page == NULL || page->m_NumEdges == page->m_MaxEdges ) {
            page = GetNewPage();
            m_Pages.push_back( page );
        }
        Edge* edge = &page->m_Buffer[page->m_NumEdges++];
        edge->m_Tail = tail;
        edge->m_Head = head;
        AdjacencyList* list = m_Adjacencies[tail]; 
        if( list->m_First == NULL ) {
            AdjacencyListNode* node = AllocateAdjacencyListNode();
            node->m_Page = page;
            node->m_Next = NULL;
            node->m_Previous = NULL;
            list->m_First = node;
            list->m_Last = node;
        }
        else if( list->m_Last->m_Page != page ) {
            AdjacencyListNode* node = AllocateAdjacencyListNode();
            node->m_Page = page;
            node->m_Next = NULL;
            node->m_Previous = NULL;
            list->m_Last->m_Next = node;
            node->m_Previous = list->m_Last->m_Next;
            list->m_Last = node;
        }

        if( m_EdgeMode == UNDIRECTED ) {
            AdjacencyList* list = m_Adjacencies[head]; 
            if( list->m_First == NULL ) {
                AdjacencyListNode* node = AllocateAdjacencyListNode();
                node->m_Page = page;
                node->m_Next = NULL;
                node->m_Previous = NULL;
                list->m_First = node;
                list->m_Last = node;
            }
            else if( list->m_Last->m_Page != page ) {
                AdjacencyListNode* node = AllocateAdjacencyListNode();
                node->m_Page = page;
                node->m_Next = NULL;
                node->m_Previous = NULL;
                list->m_Last->m_Next = node;
                node->m_Previous = list->m_Last->m_Next;
                list->m_Last = node;
            }
        }
    }
}

