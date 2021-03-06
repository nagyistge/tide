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

#include "AppController.h"

#include "MasterConfiguration.h"
#include "config.h"
#include "serialization.h"

#include <QDir>

namespace
{
QString _makeAbsPath(const QString& baseDir, const QString& uri)
{
    return QDir::isRelativePath(uri) ? baseDir + "/" + uri : uri;
}
} // anonymous namespace

/** Wrapper function to call fromJson for all parameters. */
template <typename T>
bool from_json_object(T& params, const QJsonObject& object)
{
    return params.fromJson(object);
}

JsonRpc::Response makeJsonRpcResponse(const bool success)
{
    return success ? JsonRpc::Response{"OK"}
                   : JsonRpc::Response{"operation failed", -1};
}

struct Uri
{
    QString uri;

    bool fromJson(const QJsonObject& object)
    {
        uri = object["uri"].toString();
        return !uri.isNull();
    }
};

AppController::AppController(const MasterConfiguration& config)
{
    const auto contentDir = config.getContentDir();
    const auto sessionDir = config.getSessionsDir();

    _rpc.bindAsync<Uri>("open", [this,
                                 contentDir](Uri uri,
                                             JsonRpc::AsyncResponse callback) {
        auto boolCallback = [callback](const bool result) {
            callback(makeJsonRpcResponse(result));
        };
        emit open(_makeAbsPath(contentDir, uri.uri), QPointF(), boolCallback);
    });
    _rpc.bindAsync<Uri>("load", [this,
                                 sessionDir](Uri uri,
                                             JsonRpc::AsyncResponse callback) {
        auto boolCallback = [callback](const bool result) {
            callback(makeJsonRpcResponse(result));
        };
        emit load(_makeAbsPath(sessionDir, uri.uri), boolCallback);
    });
    _rpc.bindAsync<Uri>("save", [this,
                                 sessionDir](Uri uri,
                                             JsonRpc::AsyncResponse callback) {
        auto boolCallback = [callback](const bool result) {
            callback(makeJsonRpcResponse(result));
        };
        emit save(_makeAbsPath(sessionDir, uri.uri), boolCallback);
    });

    _rpc.notify<Uri>("browse", [this](Uri uri) { emit browse(uri.uri); });
    _rpc.notify<Uri>("screenshot",
                     [this](Uri uri) { emit takeScreenshot(uri.uri); });
    _rpc.notify("whiteboard", [this] { emit openWhiteboard(); });
    _rpc.notify("exit", [this] { emit exit(); });

#if TIDE_ENABLE_PLANAR_CONTROLLER
    _rpc.notify("poweroff", [this] { emit powerOff(); });
#endif
}

std::future<std::string> AppController::processJsonRpc(
    const std::string& request)
{
    return _rpc.processAsync(request);
}
