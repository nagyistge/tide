/*********************************************************************/
/* Copyright (c) 2016-2017, EPFL/Blue Brain Project                  */
/*                          Pawel Podhajski <pawel.podhajski@epfl.ch>*/
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
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

#include "ThumbnailCache.h"

#include "thumbnail/thumbnail.h"

#include <QBuffer>
#include <QtConcurrent>

using namespace zeroeq;

namespace
{
const QSize thumbnailSize{512, 512};
}

ThumbnailCache::ThumbnailCache(const DisplayGroup& displayGroup)
{
    QObject::connect(&displayGroup, &DisplayGroup::contentWindowAdded,
                     [this](ContentWindowPtr window) {
                         _cacheThumbnail(window);
                     });

    QObject::connect(&displayGroup, &DisplayGroup::contentWindowRemoved,
                     [this](ContentWindowPtr window) {
                         _thumbnailCache.remove(window->getID());
                     });
}

std::future<http::Response> ThumbnailCache::getThumbnail(
    const QUuid& uuid) const
{
    if (!_thumbnailCache.contains(uuid))
        return make_ready_response(http::Code::NOT_FOUND);

    const auto& image = _thumbnailCache[uuid];
    if (!image.isFinished())
        return make_ready_response(http::Code::NO_CONTENT);

    return make_ready_response(http::Code::OK, image.result(), "image/png");
}

void ThumbnailCache::_cacheThumbnail(ContentWindowPtr window)
{
    const auto content = window->getContent();

    _thumbnailCache[window->getID()] = QtConcurrent::run([content]() {
        const auto image = thumbnail::create(*content, thumbnailSize);
        QByteArray imageArray;
        QBuffer buffer(&imageArray);
        buffer.open(QIODevice::WriteOnly);
        if (!image.save(&buffer, "PNG"))
            return std::string();
        buffer.close();
        return "data:image/png;base64," + imageArray.toBase64().toStdString();
    });
}
