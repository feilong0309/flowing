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

#include <iostream>
#include "Flowing.h"

void process( flowing::Edge* edge, int numEdges ) {
}

int main( int argc, char** argv ) {

    flowing::StreamGraph graph( flowing::StreamGraph::UNDIRECTED, process, 1024 );
    if(!graph.Initialize()) {
        std::cout << "ERROR: Unable to initialize the stream graph." << std::endl;
        return 1;
    }
    graph.Push(std::cin);
    unsigned int numNodes = graph.NumNodes();
    for( unsigned int i = 0; i < numNodes; ++i ) {
        flowing::StreamGraph::AdjacencyIterator it = graph.Iterator(i);
        while( it.HasNext() ) {
            it.Next();
        }

        i % 10000 == 0 ? std::cout << "Iterated over " << i << " nodes" << std::endl : (void*)0;
    }
    graph.Close();
    return 0;
}



