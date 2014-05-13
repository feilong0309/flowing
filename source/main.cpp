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

#include "Flowing.h"
#include "Community.h"
#include <iostream>
#include <fstream>

std::ofstream outputFile;

void* nodeDataAllocate( flowing::StreamGraph* graph, unsigned int nodeId ) {
    return static_cast<void*>(new flowing::Community( graph, nodeId ));
}

void nodeDataFree( flowing::StreamGraph* graph, unsigned int nodeId, void* nodeData ) {
   flowing::Community* community = (flowing::Community*) nodeData;
    if( community != NULL ) {
        if( (community->Size() > 0) ) {
            // print
            flowing::Community::CommunityIterator iterCom = community->Iterator();
            while( iterCom.HasNext() ) {
                unsigned int node = iterCom.Next();
                outputFile << graph->Remap( node ); 
                if( iterCom.HasNext() ) outputFile << " ";
                graph->SetNodeData( node, NULL );
            }
            outputFile << std::endl;
        } 
        delete community;
    }
}

void process( flowing::StreamGraph* graph, flowing::Edge* edges, int numEdges ) {
    for( int i = 0; i < numEdges; ++i ) {
        unsigned int tail = edges[i].m_Tail;
        unsigned int head = edges[i].m_Head;
        flowing::Community* tailCommunity = static_cast<flowing::Community*>(graph->GetNodeData( tail ));
        flowing::Community* headCommunity = static_cast<flowing::Community*>(graph->GetNodeData( head ));
        if( tailCommunity->Id() != headCommunity->Id() ) {
            double currentStore = tailCommunity->Score() + headCommunity->Score(); 
            double tailToHead = tailCommunity->TestRemove( tail ) + headCommunity->TestInsert( tail );
            double headToTail = tailCommunity->TestInsert( head ) + headCommunity->TestRemove( head );
            if( ( currentStore < headToTail ) || ( currentStore < tailToHead ) ) {
                if( tailToHead > headToTail ) {
                    tailCommunity->Remove( tail );
                    headCommunity->Insert( tail );
                    if( tailCommunity->Size() == 0)
                        nodeDataFree( graph, tail, tailCommunity );
                    graph->SetNodeData( tail, headCommunity );
                } else {
                    headCommunity->Remove( head );
                    tailCommunity->Insert( head );
                    if( headCommunity->Size() == 0)
                        nodeDataFree( graph, head, headCommunity );
                    graph->SetNodeData( head, tailCommunity );
                }
            }
        }
    }
}

int main( int argc, char** argv ) {

    flowing::StreamGraph graph( flowing::StreamGraph::UNDIRECTED, 
                                process, 
                                nodeDataAllocate,
                                nodeDataFree,
                                1024 );
    if(!graph.Initialize()) {
        std::cout << "ERROR: Unable to initialize the stream graph." << std::endl;
        return 1;
    }
    graph.Push(std::cin);
    outputFile.open("communities.dat");
/*    unsigned int numNodes = graph.NumNodes();
    for( unsigned int i = 0; i < numNodes; ++i ) {
        flowing::StreamGraph::AdjacencyIterator it = graph.Iterator(i);
        while( it.HasNext() ) {
            it.Next();
        }
        i % 10000 == 0 ? std::cout << "Iterated over " << i << " nodes" << std::endl : (void*)0;
    }
    */
    graph.Close();
    return 0;
}

