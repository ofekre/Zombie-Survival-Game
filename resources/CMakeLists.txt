# Copy the new folder structure and Board.txt
file(COPY 
    ${CMAKE_CURRENT_SOURCE_DIR}/Board.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/textures
    ${CMAKE_CURRENT_SOURCE_DIR}/sounds
    ${CMAKE_CURRENT_SOURCE_DIR}/fonts
    DESTINATION ${CMAKE_BINARY_DIR}/resources)