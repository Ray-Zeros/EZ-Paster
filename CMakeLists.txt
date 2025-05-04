cmake_minimum_required(VERSION 3.16)
project(ez-paster VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 设置Qt6路径
set(CMAKE_PREFIX_PATH "D:/Qt/6.5.3/msvc2019_64")

# 查找Qt包
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
endif()

# 配置静态链接 C/C++ 运行时库 (仅限 MSVC)
if(MSVC)
    # 遍历所有相关的 CXX 标志变量
    foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        # 检查是否存在动态链接标志 /MD (非 Debug)
        if(${flag_var} MATCHES "/MD")
            # 将 /MD 替换为 /MT (静态链接)
            string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endif()
        # 检查是否存在动态链接标志 /MDd (Debug)
         if(${flag_var} MATCHES "/MDd")
            # 将 /MDd 替换为 /MTd (静态链接 Debug)
            string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
        endif()
    endforeach()
    # 清理变量，防止影响其他地方
    unset(flag_var)
endif()

# 添加源文件
set(SOURCES
    main.cpp
    mainwindow.cpp
    draftwidget.cpp
    resizablepixmapitem.cpp
)

# 添加头文件
set(HEADERS
    mainwindow.h
    draftwidget.h
    resizablepixmapitem.h
)

# Windows 特定源文件
if(WIN32)
    # 添加图标资源
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/app_icon.rc")
    list(APPEND SOURCES ${APP_ICON_RESOURCE_WINDOWS})
endif()

# 创建可执行文件 (使用 SOURCES 和 HEADERS)
add_executable(ez-paster ${SOURCES} ${HEADERS})

# 链接Qt库
target_link_libraries(ez-paster PRIVATE Qt::Core Qt::Gui Qt::Widgets)

# 设置Windows特定选项
if(WIN32)
    target_link_libraries(ez-paster PRIVATE user32)
    
    # 应用程序清单
    if(MSVC)
        set_target_properties(ez-paster PROPERTIES
            WIN32_EXECUTABLE TRUE
            VS_USER_PROPS "${CMAKE_CURRENT_SOURCE_DIR}/application.props"
        )
    endif()
endif()

# 安装
install(TARGETS ez-paster
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 复制Qt库 (Windows)
if(WIN32)
    include(InstallRequiredSystemLibraries)
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
    # 注释掉下面两行
    # include(DeployQt)
    # windeployqt(ez-paster)
endif() 