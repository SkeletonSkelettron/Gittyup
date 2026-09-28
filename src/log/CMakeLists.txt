add_library(loglib LogDelegate.cpp LogEntry.cpp LogModel.cpp LogView.cpp)

# LogDelegate draws badges with ui's Badge. The libraries depend on each
# other, which CMake resolves by repeating them on the link line.
target_link_libraries(loglib ui Qt6::Widgets)

set_target_properties(loglib PROPERTIES AUTOMOC ON)
