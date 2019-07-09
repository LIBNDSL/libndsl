# 
# @file 
# @brief 
#
# @author zzt
# @emial 429834658@qq.com
#


set(SRC_V8 V8Test.cc)
ADD_EXECUTABLE(v8test ${SRC_V8})
add_dependencies(v8test ndsl)
target_link_libraries(v8test ndsl pthread v8)