/*
 * Copyright (C) 2012 Canonical Ltd.
 *
 * Authors:
 *  Ugo Riboni <ugo.riboni@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cameraapplication.h"

#include <QtCore/QDebug>
#include <QtCore/QLibrary>
#include <QQmlContext>
#include <QQmlEngine>
#include <QScreen>
#include <QtGui/QGuiApplication>
#include <QZXing.h>

#include "config.h"

static void printUsage(const QStringList& arguments)
{
    qDebug() << "usage:"
             << arguments.at(0).toUtf8().constData()
             << "[-testability]";
}

CameraApplication::CameraApplication(int &argc, char **argv)
    : QGuiApplication(argc, argv),
      m_mode(CameraMode::NORMAL)
{

    // The testability driver is only loaded by QApplication but not by QGuiApplication.
    // However, QApplication depends on QWidget which would add some unneeded overhead => Let's load the testability driver on our own.
    if (arguments().contains(QLatin1String("-testability")) ||
        qgetenv("QT_LOAD_TESTABILITY") == "1") {
        QLibrary testLib(QLatin1String("qttestability"));
        if (testLib.load()) {
            typedef void (*TasInitialize)(void);
            TasInitialize initFunction = (TasInitialize)testLib.resolve("qt_testability_init");
            if (initFunction) {
                initFunction();
            } else {
                qCritical("Library qttestability resolve failed!");
            }
        } else {
            qCritical("Library qttestability load failed!");
        }
    }
    if (arguments().contains(QLatin1String("--mode=barcode-reader"))) {
        m_mode = CameraMode::BARCODE_READER;
    }
}

CameraApplication::~CameraApplication()
{
}

bool CameraApplication::isDesktopMode() const
{
  // Platform is not a good way to determine this. Mainline devices currently
  // uses Wayland platform and it's expected that Hybris devices will follow
  // suite sometime in the future.
  // The best way would be calling to libdeviceinfo. However, currently
  // libdeviceinfo doesn't work for Click packages. So, for now, this.
#ifdef CLICK_MODE
  // Assumes that desktop users won't be able to install Click packages.
  return false;
#else
  // Assume that platformName (QtUbuntu) with ubuntu
  // in name means it's running on device
  // TODO: replace this check with SDK call for formfactor
  QString platform = QGuiApplication::platformName();
  return !((platform == "ubuntu") || (platform == "ubuntumirclient"));
#endif
}


bool CameraApplication::setup()
{
    QGuiApplication::primaryScreen()->setOrientationUpdateMask(Qt::PortraitOrientation |
                Qt::LandscapeOrientation |
                Qt::InvertedPortraitOrientation |
                Qt::InvertedLandscapeOrientation);

    QZXing::registerQMLTypes();

    m_engine.reset(new QQmlApplicationEngine());
    m_engine->rootContext()->setContextProperty("application", this);
    m_engine->rootContext()->setContextProperty("i18nDirectory", cameraAppLocaleDirectory());
    m_engine->setBaseUrl(QUrl::fromLocalFile(cameraAppDirectory()));
    m_engine->addImportPath(cameraAppImportDirectory());
    qDebug() << "Import path added" << cameraAppImportDirectory();
    qDebug() << "Camera app directory" << cameraAppDirectory();
    m_engine->load(QUrl::fromLocalFile(sourceQml(m_mode)));

    return true;
}
