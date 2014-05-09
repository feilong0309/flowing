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

#include "StreamGraph.h"
#include <cstdlib>

namespace flowing {
    AdjacencyPage* StreamGraph::AllocateAdjacencyPage( void* buffer, const int size ) {
        AdjacencyPage* page = (AdjacencyPage*)malloc(sizeof(AdjacencyPage));
        if( page == NULL ) return NULL;
        page->m_Buffer = buffer; 
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

    AdjacencyList* StreamGraph::AllocateAdjacencList() {
        AdjacencyList* list = (AdjacencyList*)malloc(sizeof(AdjacencyList));
        if( list == NULL ) return NULL;
        list->m_First = NULL;
        list->m_Degree = NULL;  
        return list;
    }

    void StreamGraph::Free( AdjacencyList* adjacencyList ) {
        free(list);
    }

    StreamGraph::StreamGraph( const EdgeMode mode ) :
        m_BufferPool( FLOWING_NUM_PAGES, FLOWING_PAGE_SIZE ) {
        m_Mode = mode;
        m_NextId = 0;
    }

    StreamGraph::~StreamGraph() {

    }

    bool StreamGraph::Initialize() {
        return m_BufferPool.Initialize();
    }

    void StreamGraph::Close() {
        // FREE MEMORY
        for( std::list<AdjacencyPage*>::iterator it = m_Pages.begin(); it != m_Pages.end(); ++it ) {
            FreeAdjacencyPage(*it);
        }

        for( int i = 0; i < m_Adjacencies.size(); ++i ) {
            FreeAdjacencyList(m_Adjacencies[i]);
        }

        m_BufferPool.Close();
    }

    void StreamGraph::Push( std::ifstream& stream ) {
        unsigned int tail;
        if( m_Mode == UNDIRECTED ) {
            while( tail << stream ) {
                unsigned int head;
                head << stream;
                PushUndirected(tail,head);
            }
        } else if( m_Mode == DIRECTED ) {
            while( tail << stream ) {
                unsigned int head;
                head << stream;
                PushDirected(tail,head);
            }
        }
    }

    void StreamGraph::Push( const unsigned int tail, const unsigned int head, const double weight ) {
        if( m_Mode == UNDIRECTED ) {
            PushUndirected(tail,head);
        } else if( m_Mode == DIRECTED ) {
            PushDirected(tail,head);
        }
    }

    void StreamGraph::PushDirected( const unsigned int tail, const unsigned int head, const double weight ) {
        unsigned int internalTail = GetInternalId(tail);
        unsigned int internalHead = GetInternalId(head);
        InsertAdjacency(*itTail,*itHead);
    }

    void StreamGraph::PushUndirected( const unsigned int tail, const unsigned int head, const double weight ) {
        unsigned int internalTail = GetInternalId(tail);
        unsigned int internalHead = GetInternalId(head);
        InsertAdjacency(internalTail, internalHead);
        InsertAdjacency(internalHead, internalId);
    }

    AdjacencyPage* GetNewPage() {
        void* buffer = m_BufferPool.NextBuffer();
        if( buffer == NULL ) {
            AdjacencyPage* page = m_Pages.front();
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
        UMap::iterator it = m_Map.find(id);
        if( it == m_Map.end() ) {
            it = m_Map.insert(std::pair<unsigned int, unsigned int>( id, m_Next++ ));
            m_Remap.push_back(id);
            AdjacencyList* list = AllocateAdjacencyList();
            m_Adjacencies.push_back(list);            
        }
        return *it;
    }

    void StreamGraph::InsertAdjacency( const unsigned int tail, const unsigned int head ) {
        AdjacencyList* listTail = m_Adjacencies[tail];    
        if( listTail->m_First == NULL ) {
            listTail->m_First = GetNewPage(); 
            listTail->m_First->m_NodeId = tail;
            listTail->m_Last = listTail->m_First;
        }

        if( listTail->m_Last->m_NumAdjacencies == listTail->m_Last->m_MaxAdjacencies ) {
            AdjacencyPage* newPage  = GetNewPage(); // IMPORTANT: GetNewPage can modify the address pointed by listTail->m_Last.
            listTail->m_Last->m_Next = newPage;
            newPage->m_Previous = listTail->m_Last;
            listTail->m_Last = newPage;
            listTail->m_Last->m_NodeId = tail;
        }

        listTail->m_Last->m_Buffer[listTail->m_NumAdjacencies++] = head;
        listTail->m_Degree++;
    }
}

