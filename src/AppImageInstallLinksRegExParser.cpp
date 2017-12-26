//
// Created by alexis on 22/12/17.
//

#include "AppImageInstallLinksRegExParser.h"
#include <QNetworkReply>

AppImageInstallLinksRegExParser::AppImageInstallLinksRegExParser(const QString &url,
                                                                 DownloadManager *downloadManager,
                                                                 QObject *parent)
        : QObject(parent),
          url(url),
          downloadManager(downloadManager) {
}

void AppImageInstallLinksRegExParser::parse() {
    if (job == nullptr) {
        job = downloadManager->downloadToMemory(url);
        QObject::connect(job, &DownloadJob::finished, this,
                         &AppImageInstallLinksRegExParser::handleDownloadFinished);

        QObject::connect(job, &DownloadJob::error, this,
                         &AppImageInstallLinksRegExParser::error);

        job->execute();
    }

}

void AppImageInstallLinksRegExParser::handleDownloadFinished() {
    const QByteArray &data = job->getData();

    QStringList list = parseAppimageLinks(data);
    QStringList absoluteLinks = resolveRelativeLinks(list);

    for (QString link: absoluteLinks)
            emit newLink(link);

    emit finished();

    job->deleteLater();
    job = nullptr;
}

QStringList AppImageInstallLinksRegExParser::resolveRelativeLinks(const QStringList &list) const {
    QStringList absoluteLinks;
    for (QString link: list) {
        if (!link.startsWith("http")) {
            QUrl qUrl(url);
            qUrl.setPath(link);
            absoluteLinks << qUrl.toString();
        } else
            absoluteLinks << link;
    }
    return absoluteLinks;
}

QStringList AppImageInstallLinksRegExParser::parseAppimageLinks(const QString &str) const {
    QStringList list;

    QString rxString = "href=\"([^\"]*%1[^\"]*AppImage)\"";
    rxString = rxString.arg(QSysInfo::currentCpuArchitecture());
    QRegExp rx(rxString);

    int pos = 0;
    while ((pos = rx.indexIn(str, pos)) != -1) {
        QString link = rx.cap(1);
        pos += rx.matchedLength();

        list << link;
    }
    return list;
}
