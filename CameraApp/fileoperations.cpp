/*
 * Copyright (C) 2014 Canonical Ltd.
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

#include <exiv2/exiv2.hpp>
#include "fileoperations.h"
#include <QtCore/QJsonObject>
#include <QtCore/QFile>
#include <QDebug>
#include <cmath>

FileOperations::FileOperations(QObject *parent) :
    QObject(parent)
{
}

bool FileOperations::remove(const QString & fileName) const
{
    return QFile::remove(fileName);
}

QJsonObject FileOperations::getEXIFData(const QString & path) const
{
    const std::string& exifPath = path.toStdString();

#if EXIV2_TEST_VERSION(0,28,0)
    Exiv2::Image::UniquePtr exifImageFile;
#else
    Exiv2::Image::AutoPtr exifImageFile;
#endif

    QJsonObject retJson;

    try {
	exifImageFile = Exiv2::ImageFactory::open(exifPath);
	exifImageFile->readMetadata();
	Exiv2::ExifData &exifData = exifImageFile->exifData();

	for(Exiv2::ExifMetadata::iterator iter = exifData.begin(); iter != exifData.end(); iter++) {
	    QString key = QString::fromStdString(iter->key());

	    // parsing of GPS-coordinates
	    if ((key == "Exif.GPSInfo.GPSLatitude" || key == "Exif.GPSInfo.GPSLongitude") && (iter->count() == 3)) {
		try {
		    Exiv2::Rational deg = iter->toRational(0);
		    Exiv2::Rational min = iter->toRational(1);
		    Exiv2::Rational sec = iter->toRational(2);

		    double degValue = static_cast<double>(deg.first) / deg.second;
		    double minValue = static_cast<double>(min.first) / min.second;
		    double secValue = static_cast<double>(sec.first) / sec.second;

		    QString formatted = QString("%1° %2' %3\"")
			.arg(static_cast<int>(degValue))
			.arg(static_cast<int>(minValue))
			.arg(secValue, 0, 'f', 2);

		    retJson[key] = formatted;
		} catch (...) {
		    retJson[key] = QString::fromStdString(iter->value().toString());
		}
	    } else {
		retJson[key] = QString::fromStdString(iter->value().toString());
	    }
	}
    } catch (const std::exception& e) {
	qDebug() << "Failed when reading EXIF data : " <<  e.what();
    }

    return retJson;
}

//! converts decimal coordinate do dms. For example 49.2021232 to [49/1, 12/1, 764/100]
Exiv2::URationalValue FileOperations::toDMS(double decimal)
{
    double absVal = std::abs(decimal),
           degRound, degFrac = std::modf(absVal, &degRound),
           minRound, minFrac = std::modf(degFrac * 60.0, &minRound),
           secVal = minFrac * 60.0;

    Exiv2::URationalValue dms;
    dms.value_.push_back(Exiv2::URational(static_cast<uint32_t>(degRound), 1));
    dms.value_.push_back(Exiv2::URational(static_cast<uint32_t>(minRound), 1));
    dms.value_.push_back(Exiv2::URational(static_cast<uint32_t>(std::round(secVal * 100.0)), 100));

    return dms;
}
//! Converts latitude and longitude from decimal to sexagesimal format and stores them in EXIF data at the given path.
bool FileOperations::setEXIFData(const QString & path, double latitude, double longitude) const
{
    const std::string& exifPath = path.toStdString();

#if EXIV2_TEST_VERSION(0,28,0)
    Exiv2::Image::UniquePtr exifImageFile;
#else
    Exiv2::Image::AutoPtr exifImageFile;
#endif

    try {
	exifImageFile = Exiv2::ImageFactory::open(exifPath);
	exifImageFile->readMetadata();
	Exiv2::ExifData &exifData = exifImageFile->exifData();

	Exiv2::URationalValue latValue = toDMS(latitude);
	exifData["Exif.GPSInfo.GPSLatitude"].setValue(&latValue);

	Exiv2::URationalValue lonValue = toDMS(longitude);
	exifData["Exif.GPSInfo.GPSLongitude"].setValue(&lonValue);

	// set GPS Ref (N/S/E/W)
	exifData["Exif.GPSInfo.GPSLatitudeRef"] = latitude >= 0 ? "N" : "S";
	exifData["Exif.GPSInfo.GPSLongitudeRef"] = longitude >= 0 ? "E" : "W";

	exifImageFile->writeMetadata();
	return true;
    } catch (const std::exception& e) {
	qDebug() << "Failed when writing EXIF data : " <<  e.what();
    }

    return false;
}
