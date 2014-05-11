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

#include "BufferPool.h"
#include <cstdlib>
#include <cstring>

namespace flowing {

    BufferPool::BufferPool( const int numBuffers, const int bufferSize) {
        m_NumBuffers = numBuffers;
        m_BufferSize = bufferSize;
        m_Buffers = NULL;
        m_Next = 0;
    }

    BufferPool::~BufferPool() {

    }

    bool BufferPool::Initialize() {
//       posix_memalign( &m_Buffers, m_NumBuffers*m_BufferSize, m_BufferSize ) == 0;
       m_Buffers = (void*)malloc(m_NumBuffers*m_BufferSize);
       if(!m_Buffers) return false;
       memset(m_Buffers, 0, m_NumBuffers*m_BufferSize );
       return true;
    }

    void BufferPool::Close() {
        if( m_Buffers ) free(m_Buffers);        
    }

    void* BufferPool::NextBuffer() {
        return m_Next < m_NumBuffers ? (unsigned char*)(m_Buffers) + (m_Next++)*m_BufferSize : NULL;
    }

    int BufferPool::MaxNumBuffers() {
        return m_NumBuffers;
    }

    int BufferPool::NumFreeBuffers() {
        return m_NumBuffers - m_Next;
    }
}
