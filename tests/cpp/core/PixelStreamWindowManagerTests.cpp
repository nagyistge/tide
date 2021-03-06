/*********************************************************************/
/* Copyright (c) 2014-2016, EPFL/Blue Brain Project                  */
/*                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>     */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/*   1. Redistributions of source code must retain the above         */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer.                                                  */
/*                                                                   */
/*   2. Redistributions in binary form must reproduce the above      */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer in the documentation and/or other materials       */
/*      provided with the distribution.                              */
/*                                                                   */
/*    THIS  SOFTWARE IS PROVIDED  BY THE  UNIVERSITY OF  TEXAS AT    */
/*    AUSTIN  ``AS IS''  AND ANY  EXPRESS OR  IMPLIED WARRANTIES,    */
/*    INCLUDING, BUT  NOT LIMITED  TO, THE IMPLIED  WARRANTIES OF    */
/*    MERCHANTABILITY  AND FITNESS FOR  A PARTICULAR  PURPOSE ARE    */
/*    DISCLAIMED.  IN  NO EVENT SHALL THE UNIVERSITY  OF TEXAS AT    */
/*    AUSTIN OR CONTRIBUTORS BE  LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL,  SPECIAL, EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES    */
/*    (INCLUDING, BUT  NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE    */
/*    GOODS  OR  SERVICES; LOSS  OF  USE,  DATA,  OR PROFITS;  OR    */
/*    BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON ANY THEORY OF    */
/*    LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR TORT    */
/*    (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY OUT    */
/*    OF  THE  USE OF  THIS  SOFTWARE,  EVEN  IF ADVISED  OF  THE    */
/*    POSSIBILITY OF SUCH DAMAGE.                                    */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#define BOOST_TEST_MODULE PixelStreamWindowManagerTests
#include <boost/test/unit_test.hpp>

#include <deflect/EventReceiver.h>
#include <deflect/Frame.h>

#include "PixelStreamWindowManager.h"
#include "control/PixelStreamController.h"
#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"
#include "scene/Options.h"
#include "scene/PixelStreamContent.h"

#include "MinimalGlobalQtApp.h"
BOOST_GLOBAL_FIXTURE(MinimalGlobalQtApp);

namespace
{
const QString CONTENT_URI("bla");
const QString PANEL_URI("Launcher");
const QSize wallSize(1000, 1000);
const QSize defaultPixelStreamWindowSize(640, 480);
const QSize testWindowSize(500, 400);
const QPointF testWindowPos(400.0, 300.0);
const QSize testFrameSize(600, 500);
const QSize testFrameSize2(700, 600);
}

deflect::FramePtr createTestFrame(const QSize& size)
{
    deflect::FramePtr frame(new deflect::Frame);
    frame->uri = CONTENT_URI;
    deflect::Segment segment;
    segment.parameters.width = size.width();
    segment.parameters.height = size.height();
    frame->segments.push_back(segment);
    return frame;
}

class DummyEventReceiver : public deflect::EventReceiver
{
public:
    DummyEventReceiver()
        : success(false)
    {
    }
    virtual void processEvent(deflect::Event /*event*/) { success = true; }
    bool success;
};

BOOST_AUTO_TEST_CASE(testNoStreamerWindowCreation)
{
    DisplayGroupPtr displayGroup(new DisplayGroup(wallSize));
    PixelStreamWindowManager windowManager(*displayGroup);

    const QString uri = CONTENT_URI;
    const QPointF pos(testWindowPos);
    const QSize size(testWindowSize);

    windowManager.openWindow(uri, pos, size);
    ContentWindowPtr window = windowManager.getWindow(uri);
    BOOST_REQUIRE(window);

    BOOST_CHECK_EQUAL(window, windowManager.getWindow(uri));

    const QRectF& coords = window->getCoordinates();
    BOOST_CHECK_EQUAL(coords.center(), pos);
    BOOST_CHECK_EQUAL(coords.size(), size);

    windowManager.handleStreamEnd(uri);
    BOOST_CHECK(!windowManager.getWindow(uri));
}

BOOST_AUTO_TEST_CASE(testEventReceiver)
{
    DisplayGroupPtr displayGroup(new DisplayGroup(wallSize));
    PixelStreamWindowManager windowManager(*displayGroup);
    windowManager.openWindow(CONTENT_URI, testWindowPos, testWindowSize);
    ContentWindowPtr window = windowManager.getWindow(CONTENT_URI);
    BOOST_REQUIRE(window);

    auto content = dynamic_cast<PixelStreamContent*>(window->getContentPtr());
    BOOST_REQUIRE(content);
    BOOST_REQUIRE(!content->hasEventReceivers());
    BOOST_REQUIRE_EQUAL(content->getInteractionPolicy(),
                        Content::Interaction::OFF);

    DummyEventReceiver receiver;
    BOOST_REQUIRE(!receiver.success);

    auto controller = ContentController::create(*window);
    auto streamController =
        dynamic_cast<PixelStreamController*>(controller.get());
    BOOST_REQUIRE(streamController);

    streamController->notify(deflect::Event());
    BOOST_CHECK(!receiver.success);

    auto promise = std::make_shared<std::promise<bool>>();
    windowManager.registerEventReceiver(content->getURI(), false, &receiver,
                                        promise);
    BOOST_CHECK(promise->get_future().get());
    BOOST_CHECK(content->hasEventReceivers());
    BOOST_CHECK_EQUAL(content->getInteractionPolicy(),
                      Content::Interaction::ON);
    BOOST_CHECK(!receiver.success);

    streamController->notify(deflect::Event());
    BOOST_CHECK(receiver.success);
}

BOOST_AUTO_TEST_CASE(testExplicitWindowCreation)
{
    DisplayGroupPtr displayGroup(new DisplayGroup(wallSize));
    PixelStreamWindowManager windowManager(*displayGroup);

    const QString uri = CONTENT_URI;
    const QPointF pos(testWindowPos);
    const QSize size(testWindowSize);

    windowManager.openWindow(uri, pos, size);
    ContentWindowPtr window = windowManager.getWindow(uri);
    BOOST_REQUIRE(window);

    BOOST_CHECK_EQUAL(window, windowManager.getWindow(uri));
    BOOST_CHECK_EQUAL(window, displayGroup->getContentWindow(window->getID()));

    windowManager.handleStreamStart(uri);
    BOOST_CHECK_EQUAL(window, windowManager.getWindow(uri));

    ContentPtr content = window->getContent();
    BOOST_REQUIRE(content);
    BOOST_CHECK(content->getURI() == uri);
    BOOST_CHECK_EQUAL(content->getType(), CONTENT_TYPE_PIXEL_STREAM);

    const QRectF& coords = window->getCoordinates();
    BOOST_CHECK_EQUAL(coords.center(), pos);
    BOOST_CHECK_EQUAL(coords.size(), size);

    // Check that the window is NOT resized to the first frame dimensions
    windowManager.updateStreamDimensions(createTestFrame(testFrameSize));
    BOOST_CHECK_EQUAL(content->getDimensions(), testFrameSize);
    BOOST_CHECK_EQUAL(coords.center(), pos);
    BOOST_CHECK_EQUAL(coords.size(), size);

    windowManager.handleStreamEnd(uri);
    BOOST_CHECK(!windowManager.getWindow(uri));
    BOOST_CHECK(!displayGroup->getContentWindow(window->getID()));
}

BOOST_AUTO_TEST_CASE(testImplicitWindowCreation)
{
    DisplayGroupPtr displayGroup(new DisplayGroup(wallSize));
    PixelStreamWindowManager windowManager(*displayGroup);

    const QString uri = CONTENT_URI;
    // window will be positioned centerred
    const QPointF pos(wallSize.width() * 0.5, wallSize.height() * 0.5);
    const QSize size(defaultPixelStreamWindowSize);

    windowManager.handleStreamStart(uri);
    ContentWindowPtr window = windowManager.getWindow(uri);
    BOOST_REQUIRE(window);
    BOOST_CHECK_EQUAL(window, displayGroup->getContentWindow(window->getID()));

    ContentPtr content = window->getContent();
    BOOST_REQUIRE(content);
    BOOST_CHECK(content->getURI() == uri);
    BOOST_CHECK_EQUAL(content->getType(), CONTENT_TYPE_PIXEL_STREAM);
    BOOST_CHECK_EQUAL(content->getDimensions(), UNDEFINED_SIZE);

    const QRectF& coords = window->getCoordinates();
    BOOST_CHECK_EQUAL(coords.center(), pos);
    BOOST_CHECK_EQUAL(coords.size(), size);

    // Check that the window is resized to the first frame dimensions
    windowManager.updateStreamDimensions(createTestFrame(testFrameSize));
    BOOST_CHECK_EQUAL(content->getDimensions(), testFrameSize);
    BOOST_CHECK_EQUAL(coords.center(), pos);
    BOOST_CHECK_EQUAL(coords.size(), testFrameSize);

    // Check that the window is NOT resized to the next frame dimensions
    windowManager.updateStreamDimensions(createTestFrame(testFrameSize2));
    BOOST_CHECK_EQUAL(content->getDimensions(), testFrameSize2);
    BOOST_CHECK_EQUAL(coords.center(), pos);
    BOOST_CHECK_EQUAL(coords.size(), testFrameSize);

    windowManager.handleStreamEnd(uri);
    BOOST_CHECK(!windowManager.getWindow(uri));
    BOOST_CHECK(!displayGroup->getContentWindow(window->getID()));
}

BOOST_AUTO_TEST_CASE(testSizeHints)
{
    DisplayGroupPtr displayGroup(new DisplayGroup(wallSize));
    PixelStreamWindowManager windowManager(*displayGroup);
    const QString uri = CONTENT_URI;
    windowManager.handleStreamStart(uri);
    ContentWindowPtr window = windowManager.getWindow(uri);
    ContentPtr content = window->getContent();

    BOOST_CHECK_EQUAL(content->getDimensions(), QSizeF());
    BOOST_CHECK_EQUAL(content->getMinDimensions(), QSizeF());
    BOOST_CHECK_EQUAL(content->getMaxDimensions(), QSizeF());
    BOOST_CHECK_EQUAL(content->getPreferredDimensions(), QSizeF());

    const QSize minSize(80, 100);
    const QSize maxSize(800, 1000);
    const QSize preferredSize(400, 500);
    deflect::SizeHints hints;
    hints.minWidth = minSize.width();
    hints.minHeight = minSize.height();
    hints.maxWidth = maxSize.width();
    hints.maxHeight = maxSize.height();
    hints.preferredWidth = preferredSize.width();
    hints.preferredHeight = preferredSize.height();
    windowManager.updateSizeHints(CONTENT_URI, hints);

    BOOST_CHECK_EQUAL(content->getDimensions(), preferredSize);
    BOOST_CHECK_EQUAL(content->getMinDimensions(), minSize);
    BOOST_CHECK_EQUAL(content->getMaxDimensions(), maxSize);
    BOOST_CHECK_EQUAL(content->getPreferredDimensions(), preferredSize);
}

BOOST_AUTO_TEST_CASE(hideAndShowWindow)
{
    DisplayGroupPtr displayGroup(new DisplayGroup(wallSize));
    PixelStreamWindowManager windowManager(*displayGroup);

    const QString uri = CONTENT_URI;
    windowManager.handleStreamStart(uri);
    ContentWindowPtr window = windowManager.getWindow(uri);

    BOOST_REQUIRE(window->isHidden());
    BOOST_REQUIRE(!window->isPanel());

    windowManager.showWindow(uri);
    BOOST_CHECK(!window->isHidden());

    windowManager.hideWindow(uri);
    BOOST_CHECK(window->isHidden());
}

BOOST_AUTO_TEST_CASE(hideAndShowPanel)
{
    DisplayGroupPtr displayGroup(new DisplayGroup(wallSize));
    PixelStreamWindowManager windowManager(*displayGroup);

    const QString uri = PANEL_URI;
    windowManager.handleStreamStart(uri);
    ContentWindowPtr panel = windowManager.getWindow(uri);

    BOOST_REQUIRE(panel->isPanel());
    BOOST_REQUIRE(!panel->isHidden());

    windowManager.hideWindow(uri);
    BOOST_CHECK(panel->isHidden());
    windowManager.showWindow(uri);
    BOOST_CHECK(!panel->isHidden());

    auto promise = std::make_shared<std::promise<bool>>();
    DummyEventReceiver receiver;
    windowManager.registerEventReceiver(uri, false, &receiver, promise);

    windowManager.hideWindow(uri);
    BOOST_CHECK(panel->isHidden());

    windowManager.showWindow(uri);
    BOOST_CHECK(!panel->isHidden());
}

BOOST_AUTO_TEST_CASE(check_external_stream_opening_after_sizehints)
{
    DisplayGroupPtr displayGroup(new DisplayGroup(wallSize));
    PixelStreamWindowManager windowManager(*displayGroup);

    bool externalStreamOpened = false;

    QObject::connect(&windowManager,
                     &PixelStreamWindowManager::externalStreamOpening,
                     [&externalStreamOpened]() {
                         externalStreamOpened = true;
                     });

    const QString uri = CONTENT_URI;
    windowManager.handleStreamStart(uri);
    ContentWindowPtr window = windowManager.getWindow(uri);

    // external streamer will be opened when provided with proper size
    BOOST_CHECK_EQUAL(externalStreamOpened, false);
    BOOST_CHECK(window->isHidden());

    const QSize expectedDimension{400, 300};

    deflect::SizeHints hints;
    hints.preferredWidth = expectedDimension.width();
    hints.preferredHeight = expectedDimension.height();
    windowManager.updateSizeHints(uri, hints);
    BOOST_CHECK_EQUAL(externalStreamOpened, true);

    BOOST_CHECK_EQUAL(
        windowManager.getWindow(uri)->getContent()->getDimensions(),
        expectedDimension);
}

BOOST_AUTO_TEST_CASE(check_external_stream_opening_after_firstframe)
{
    DisplayGroupPtr displayGroup(new DisplayGroup(wallSize));
    PixelStreamWindowManager windowManager(*displayGroup);

    bool externalStreamOpened = false;

    QObject::connect(&windowManager,
                     &PixelStreamWindowManager::externalStreamOpening,
                     [&externalStreamOpened]() {
                         externalStreamOpened = true;
                     });

    const QString uri = CONTENT_URI;
    windowManager.handleStreamStart(uri);
    ContentWindowPtr window = windowManager.getWindow(uri);

    // external streamer will be opened when provided with proper size
    BOOST_CHECK_EQUAL(externalStreamOpened, false);
    BOOST_CHECK(window->isHidden());

    const QSize expectedDimension{400, 300};

    deflect::Segment segment;
    segment.parameters.width = expectedDimension.width();
    segment.parameters.height = expectedDimension.height();

    deflect::FramePtr frame(new deflect::Frame);
    frame->uri = uri;
    frame->segments.push_back(segment);
    windowManager.updateStreamDimensions(frame);
    BOOST_CHECK_EQUAL(externalStreamOpened, true);
    BOOST_CHECK(window->isHidden());

    BOOST_CHECK_EQUAL(
        windowManager.getWindow(uri)->getContent()->getDimensions(),
        expectedDimension);
}

BOOST_AUTO_TEST_CASE(resize_external_stream)
{
    DisplayGroupPtr displayGroup(new DisplayGroup(wallSize));
    PixelStreamWindowManager windowManager(*displayGroup);

    bool externalStreamOpened = false;

    const QString uri = CONTENT_URI;
    QObject::connect(&windowManager,
                     &PixelStreamWindowManager::externalStreamOpening,
                     [&externalStreamOpened, &windowManager, uri]() {
                         externalStreamOpened = true;
                         windowManager.showWindow(uri);
                     });

    windowManager.handleStreamStart(uri);
    ContentWindowPtr window = windowManager.getWindow(uri);

    // external streamer will be opened when provided with proper size
    BOOST_CHECK_EQUAL(externalStreamOpened, false);
    BOOST_CHECK(window->isHidden());

    const QSize expectedDimension{400, 300};

    deflect::SizeHints hints;
    hints.preferredWidth = expectedDimension.width();
    hints.preferredHeight = expectedDimension.height();
    windowManager.updateSizeHints(uri, hints);
    BOOST_CHECK_EQUAL(externalStreamOpened, true);
    BOOST_CHECK(!window->isHidden());

    BOOST_CHECK_EQUAL(
        windowManager.getWindow(uri)->getContent()->getDimensions(),
        expectedDimension);

    const QSize newDimension{200, 200};
    deflect::Segment segment;
    segment.parameters.width = newDimension.width();
    segment.parameters.height = newDimension.height();

    deflect::FramePtr frame(new deflect::Frame);
    frame->uri = uri;
    frame->segments.push_back(segment);
    windowManager.updateStreamDimensions(frame);

    BOOST_CHECK_EQUAL(
        windowManager.getWindow(uri)->getContent()->getDimensions(),
        newDimension);
}

BOOST_AUTO_TEST_CASE(check_local_stream_opening)
{
    DisplayGroupPtr displayGroup(new DisplayGroup(wallSize));
    PixelStreamWindowManager windowManager(*displayGroup);

    bool externalStreamOpened = false;

    QObject::connect(&windowManager,
                     &PixelStreamWindowManager::externalStreamOpening,
                     [&externalStreamOpened]() {
                         externalStreamOpened = true;
                     });

    const QString uri = PANEL_URI;
    windowManager.handleStreamStart(uri);

    ContentWindowPtr window = windowManager.getWindow(uri);

    BOOST_CHECK_EQUAL(externalStreamOpened, false);
    BOOST_CHECK(!window->isHidden());
}
