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

#ifndef FILEOPERATIONS_H
#define FILEOPERATIONS_H

#include <QtCore/QObject>
#include <QtCore/QJsonObject>
#include <exiv2/exiv2.hpp>


class FileOperations : public QObject
{
    Q_OBJECT

public:
    explicit FileOperations(QObject *parent = 0);
    Q_INVOKABLE bool remove(const QString & fileName) const;
    Q_INVOKABLE QJsonObject getEXIFData(const QString & path) const;
    Q_INVOKABLE bool setEXIFData(const QString & path, double latitude, double longitude) const;
private:
    static Exiv2::URationalValue toDMS(double decimal);

};

#endif // FILEOPERATIONS_H
