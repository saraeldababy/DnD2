include("/Users/saraeldababy/DungeonRealm/build/Qt_6_10_2_for_macOS-Debug/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/DungeonRealm-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE "DungeonRealm.app"
)
