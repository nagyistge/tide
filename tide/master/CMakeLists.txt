# Copyright (c) 2013-2016, EPFL/Blue Brain Project
#                          Raphael Dumusc <raphael.dumusc@epfl.ch>

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(TIDEMASTER_LINK_LIBRARIES
  PUBLIC
    TideCore
    Qt5::Widgets
  PRIVATE
    Qt5::Concurrent
    DeflectQt
)

list(APPEND TIDEMASTER_PUBLIC_HEADERS
  ContentLoader.h
  control/AutomaticLayout.h
  control/CanvasNode.h
  control/CanvasTree.h
  control/ContentController.h
  control/ContentWindowController.h
  control/DisplayGroupController.h
  control/LayoutEngine.h
  control/LayoutPolicy.h
  control/PixelStreamController.h
  control/ZoomController.h
  FileInfoHelper.h
  localstreamer/HtmlSelectReplacer.h
  localstreamer/CommandLineOptions.h
  localstreamer/PixelStreamerLauncher.h
  localstreamer/PixelStreamerType.h
  localstreamer/ProcessForker.h
  localstreamer/QmlKeyInjector.h
  LoggingUtility.h
  MasterApplication.h
  MasterConfiguration.h
  MasterDisplayGroupRenderer.h
  multitouch/DoubleTapDetector.h
  multitouch/MathUtils.h
  multitouch/MultitouchArea.h
  multitouch/PanDetector.h
  multitouch/PinchDetector.h
  multitouch/SwipeDetector.h
  multitouch/TapAndHoldDetector.h
  multitouch/TapDetector.h
  network/MasterFromWallChannel.h
  network/MasterToForkerChannel.h
  network/MasterToWallChannel.h
  PixelStreamWindowManager.h
  QmlTypeRegistration.h
  ScreenshotAssembler.h
  State.h
  StateSerializationHelper.h
  ui/BackgroundWidget.h
  ui/ContentWindowListWidgetItem.h
  ui/DisplayGroupListWidget.h
  ui/MasterQuickView.h
  ui/BackgroundWidget.h
  ui/MasterWindow.h
)

list(APPEND TIDEMASTER_SOURCES
  ContentLoader.cpp
  control/AutomaticLayout.cpp
  control/CanvasNode.cpp
  control/CanvasTree.cpp
  control/ContentController.cpp
  control/ContentWindowController.cpp
  control/DisplayGroupController.cpp
  control/LayoutEngine.cpp
  control/LayoutPolicy.cpp
  control/PixelStreamController.cpp
  control/ZoomController.cpp
  localstreamer/HtmlSelectReplacer.cpp
  localstreamer/CommandLineOptions.cpp
  localstreamer/PixelStreamerLauncher.cpp
  localstreamer/PixelStreamerType.cpp
  localstreamer/ProcessForker.cpp
  localstreamer/QmlKeyInjector.cpp
  LoggingUtility.cpp
  MasterApplication.cpp
  MasterConfiguration.cpp
  MasterDisplayGroupRenderer.cpp
  multitouch/DoubleTapDetector.cpp
  multitouch/MathUtils.cpp
  multitouch/MultitouchArea.cpp
  multitouch/PanDetector.cpp
  multitouch/PinchDetector.cpp
  multitouch/SwipeDetector.cpp
  multitouch/TapAndHoldDetector.cpp
  multitouch/TapDetector.cpp
  network/MasterFromWallChannel.cpp
  network/MasterToForkerChannel.cpp
  network/MasterToWallChannel.cpp
  PixelStreamWindowManager.cpp
  ScreenshotAssembler.cpp
  State.cpp
  StateSerializationHelper.cpp
  resources/master.qrc
  ui/BackgroundWidget.cpp
  ui/ContentWindowListWidgetItem.cpp
  ui/DisplayGroupListWidget.cpp
  ui/MasterQuickView.cpp
  ui/MasterWindow.cpp
)

if(TARGET Qt5::WebEngine)
  list(APPEND TIDEMASTER_PUBLIC_HEADERS
    ui/WebbrowserWidget.h
  )
  list(APPEND TIDEMASTER_SOURCES
    ui/WebbrowserWidget.cpp
  )
elseif(TARGET Qt5::WebKitWidgets)
  list(APPEND TIDEMASTER_PUBLIC_HEADERS
    control/WebbrowserController.h
    localstreamer/PixelStreamerFactory.h
    localstreamer/PixelStreamer.h
    localstreamer/WebkitAuthenticationHelper.h
    localstreamer/WebkitHtmlSelectReplacer.h
    localstreamer/WebkitPixelStreamer.h
    ui/WebbrowserWidget.h
  )
  list(APPEND TIDEMASTER_SOURCES
    control/WebbrowserController.cpp
    localstreamer/PixelStreamerFactory.cpp
    localstreamer/WebkitAuthenticationHelper.cpp
    localstreamer/WebkitHtmlSelectReplacer.cpp
    localstreamer/WebkitPixelStreamer.cpp
    ui/WebbrowserWidget.cpp
  )
  list(APPEND TIDEMASTER_LINK_LIBRARIES PRIVATE Qt5::WebKitWidgets)
endif()

if(TIDE_ENABLE_PDF_SUPPORT)
  list(APPEND TIDEMASTER_PUBLIC_HEADERS
    control/PDFController.h
  )
  list(APPEND TIDEMASTER_SOURCES
    control/PDFController.cpp
  )
endif()

if(TIDE_ENABLE_TUIO_TOUCH_LISTENER)
  list(APPEND TIDEMASTER_HEADERS MultitouchListener.h)
  list(APPEND TIDEMASTER_SOURCES MultitouchListener.cpp)
  list(APPEND TIDEMASTER_LINK_LIBRARIES TUIO ${X11_LIBRARIES})
endif()

if(TIDE_ENABLE_PLANAR_CONTROLLER)
  list(APPEND TIDEMASTER_HEADERS PlanarController.h)
  list(APPEND TIDEMASTER_SOURCES PlanarController.cpp)
  list(APPEND TIDEMASTER_LINK_LIBRARIES PRIVATE Qt5::SerialPort)
endif()

set(TIDEMASTER_INCLUDE_NAME tide/master)

if(TIDE_ENABLE_REST_INTERFACE)
  list(APPEND TIDEMASTER_PUBLIC_HEADERS
    rest/AppController.h
    rest/FileBrowser.h
    rest/FileReceiver.h
    rest/HtmlContent.h
    rest/json.h
    rest/JsonRpc.h
    rest/jsonschema.h
    rest/RestInterface.h
    rest/RestServer.h
    rest/ThumbnailCache.h
    rest/SceneController.h
    rest/serialization.h
  )
  list(APPEND TIDEMASTER_SOURCES
    rest/AppController.cpp
    rest/FileBrowser.cpp
    rest/FileReceiver.cpp
    rest/HtmlContent.cpp
    rest/json.cpp
    rest/JsonRpc.cpp
    rest/jsonschema.cpp
    rest/RestInterface.cpp
    rest/RestServer.cpp
    rest/ThumbnailCache.cpp
    rest/SceneController.cpp
    rest/serialization.cpp
  )
  # Servus PUBLIC because servus::Serializable is a base class of public headers
  list(APPEND TIDEMASTER_LINK_LIBRARIES PUBLIC Servus PRIVATE ZeroEQHTTP)
endif()

common_library(TideMaster)
