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

#ifndef PAGE_POOL_H
#define PAGE_POOL_H

namespace flowing {

    class BufferPool {
        public:

            /** @param numPages The number of buffers to contain.
              @param bufferSize The size of the buffers in bytes.*/
            BufferPool( const int numBuffers, const int bufferSize);
            ~BufferPool();

            /** @brief Initializes the buffer pool.
              @param True if the initialization was successful*/
            bool Initialize();

            /** @brief Closes the buffer pool by freeing all the used resources.*/
            void Close();

            /** @brief Gets a new buffer.
              @return A pointer to the buffer. NULL if there are not remaining buffers.*/
            void* NextBuffer();

            /** @brief Gets the maximum number of buffers.
             *  @return The maximum number of buffers available.*/
            int MaxNumBuffers();

            /** @brief The number of free buffers.
             *  @return The number of free buffers.*/
            int NumFreeBuffers();

        public:
            int     m_NumBuffers; /**< @brief The number of buffers.*/
            int     m_BufferSize; /**< @brief The buffer size in bytes.*/
            int     m_Next;       /**< @brief The index to the next available buffer.*/
            void*   m_Buffers;     /**< @brief A pointer to the memory buffer.*/
    };

}

#endif 
