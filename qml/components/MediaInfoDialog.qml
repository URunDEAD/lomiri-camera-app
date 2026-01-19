/*
 * Copyright (C) 2025 UBports Foundation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
import QtQuick 2.12
import Lomiri.Components 1.3
import Lomiri.Components.Popups 1.3
import QtQuick.Layouts 1.1
import CameraApp 0.1

// For the thumbnail
import Lomiri.Components.Extras 0.2

import Lomiri.Content 1.3
import "MimeTypeMapper.js" as MimeTypeMapper

Dialog {
    id: root

    readonly property bool isVideo: MimeTypeMapper.mimeTypeToContentType(model.fileType) === ContentType.Videos
    readonly property bool isImage: MimeTypeMapper.mimeTypeToContentType(model.fileType) === ContentType.Pictures

    property url mediaUrl: null
    property var model: null
    property var exifData: mediaUrl != undefined ? fileOperations.getEXIFData(mediaUrl) : undefined

    property var infoKeys : [
              { "key": "Exif.Photo.PixelYDimension" , "title" :i18n.tr( "Width:"), "valueFunction": root.pixelFormat},
              { "key": "Exif.Photo.PixelXDimension" , "title" :i18n.tr( "Height:"), "valueFunction": root.pixelFormat},
              { "key": "Exif.Photo.DateTimeOriginal" , "title" :i18n.tr( "Date:"), "valueFunction": root.dateTimeFormat},
              { "key": "Exif.Image.Model" , "title" :i18n.tr( "Camera Model:"), "valueFunction": null},
              { "key": "Exif.Image.Copyright" , "title" :i18n.tr( "Copyright:"), "valueFunction": null},
              { "key": "Exif.Image.ExposureTime" , "title" :i18n.tr( "Exposure Time:"), "valueFunction": root.timeFormat},
	      { "key": "Exif.Image.FNumber" , "title" :i18n.tr( "F. Number:"), "valueFunction": null},
	      { "key": "Exif.Image.NewSubfileType" , "title" :i18n.tr( "Sub-File type:"), "valueFunction": null},
	      { "key": "Exif.Photo.Flash" , "title" :i18n.tr( "With Flash:"), "valueFunction": root.parseFlashData},
	      { "key": "Exif.GPSInfo.GPSLatitude" , "title" :i18n.tr( "GPS Latitude:"), "valueFunction": root.parseLatitudeData},
	      { "key": "Exif.GPSInfo.GPSLongitude" , "title" :i18n.tr( "GPS Longitude:"), "valueFunction": root.parseLongitudeData},
	      { "key": "Exif.GPSInfo.GPSAltitude" , "title" :i18n.tr( "GPS Altitude:"), "valueFunction": root.parseAltitudeData},
	    ];

    FileOperations {
        id: fileOperations
    }

    function dateTimeFormat(dateTime) {
        return new Date(dateTime).toLocaleString(Qt.locale());
    }

    function dateFormat(dateTime) {
        return new Date(dateTime).toLocaleDateString(Qt.locale());
    }

    function timeFormat(dateTime) {
        return new Date(dateTime).toLocaleTimeString(Qt.locale());
    }

    function pixelFormat(value) {
        return i18n.tr("%1 pixel", "%1 pixels", value).arg(value)
    }

    function parseFlashData(value) {
        const _flashTag = parseInt(value, /* radix */ 10);

        /*
         * From https://www.awaresystems.be/imaging/tiff/tifftags/privateifd/exif/flash.html:
         *
         * Exif TIFF Tag Flash, code 37385 (0x9209)
         *
         * Indicates the status of flash when the image was shot.
         *
         * Bit 0 indicates the flash firing status, bits 1 and 2
         * indicate the flash return status, bits 3 and 4 indicate
         * the flash mode, bit 5 indicates whether the flash
         * function is present, and bit 6 indicates "red eye" mode.
         */
        const _flashDidFire = (_flashTag & (1 << 0));

	return _flashDidFire ? i18n.tr("Yes") : i18n.tr("No");
    }

    function parseLatitudeData(value) {

	if (root.exifData["Exif.GPSInfo.GPSLatitudeRef"] == "N") {
	    //TRANSLATORS: DMS coordinate and coordinate latitude direction N for north
	    return i18n.tr("%1 N").arg(value)
	} else if (root.exifData["Exif.GPSInfo.GPSLatitudeRef"] == "S") {
	    //TRANSLATORS: DMS coordinate and latitude direction S for south
	    return i18n.tr("%1 S").arg(value)
	}

	return value + root.exifData["Exif.GPSInfo.GPSLatitudeRef"]
    }

    function parseLongitudeData(value) {
	if (root.exifData["Exif.GPSInfo.GPSLongitudeRef"] == "E") {
	    //TRANSLATORS: DMS coordinate and longitude direction E for east
	    return i18n.tr("%1 E").arg(value)
	} else if (root.exifData["Exif.GPSInfo.GPSLongitudeRef"] == "W") {
	    //TRANSLATORS: DMS coordinate and longitude direction W for west
	    return i18n.tr("%1 W").arg(value)
	}
	return value + root.exifData["Exif.GPSInfo.GPSLongitudeRef"]
    }

    function parseAltitudeData(value) {
	return value.replace("/1","m");
    }

    __closeOnDismissAreaPress: true

    Component.onCompleted: {
        __foreground.itemSpacing = units.gu(0)
    }

    RowLayout {
        Icon {
            Layout.preferredWidth: units.gu(6)
            Layout.preferredHeight: width
            name: {
                if (root.isImage) return "stock_image"
                if (root.isVideo) return "stock_video"
                return "empty-symbolic"
            }
            visible: !image.visible
        }

        Image {
            id: image
            Layout.preferredWidth: units.gu(6)
            Layout.preferredHeight: width
            sourceSize: Qt.size(units.gu(6), units.gu(6))
            visible: status == Image.Ready
            autoTransform: true

            source: "image://thumbnailer/" + root.mediaUrl
            fillMode: Image.PreserveAspectFit
            asynchronous: true
        }

        ListItemLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            title.text: root.model.fileName
            title.wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            title.maximumLineCount: 3
            subtitle.text: {
                if (root.isImage) return i18n.tr("Image")
                if (root.isVideo) return i18n.tr("Video")
                return root.model.fileType
            }
            summary.text: root.model.fileSizeLabel
        }
    }

    Rectangle {
        anchors { left: parent.left; right: parent.right }
        height: units.dp(1)
        color: theme.palette.normal.base
    }

    ListItem {
        divider.visible: false
        height: layout.height
        anchors { left: parent.left; right: parent.right }
        anchors.leftMargin: units.gu(-2)
        anchors.rightMargin: units.gu(-2)

        onClicked: Qt.openUrlExternally(root.model.parentUrl)

        ListItemLayout {
            id: layout

            subtitle.text: i18n.tr("Where:")
            summary.maximumLineCount: Number.MAX_VALUE
            summary.wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            summary.text: root.model.parentUrl.toString().replace(/^file:\/\//, i18n.tr("My Device")).replace(/\//g, " > ")

            Icon {
                name: "external-link"
                SlotsLayout.position: SlotsLayout.Trailing;
                anchors.verticalCenter: parent.verticalCenter
                SlotsLayout.overrideVerticalPositioning: true
                color: theme.palette.normal.foregroundText
                width: units.gu(2)
                height: width
            }
        }
    }

    Rectangle {
        anchors { left: parent.left; right: parent.right }
        height: units.dp(1)
        color: theme.palette.normal.base
    }

    Item {
        height: units.gu(2)
    }

    ListItem {
        divider.visible: false
        height: dateGrid.height + units.gu(4)

        ColumnLayout {
            id: dateGrid

            anchors { left: parent.left; right: parent.right }

            //Print stright forward EXIF data
            Repeater {
                model: root.infoKeys

                RowLayout {
                    Layout.fillWidth: true
                    spacing: units.dp(2)
                    visible: undefined !== root.exifData && undefined !== root.exifData[modelData.key];

                    Label {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: units.gu(12)
                        textSize: Label.Small
                        wrapMode: Text.WordWrap
                        color: theme.palette.normal.backgroundSecondaryText
                        text: visible ? modelData.title : ""
                    }

                    Label {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop
                        textSize: Label.Small
                        wrapMode: Text.WordWrap
                        color: theme.palette.normal.backgroundTertiaryText
                        text: {
                            const _keyValue = root.exifData[modelData.key];

                            // Using modelData or model doesn't work for functions
                            const _valueFunction = root.infoKeys[index].valueFunction;

                            if (!visible || _keyValue == null)
                                return ""

                            if (_valueFunction !== null) {
                                return _valueFunction(_keyValue)
                            } else {
                                return _keyValue
                            }
                        }
                    }
                }
            }
        }
    }

    Button {
        id: closeButton
        text: i18n.tr("Close")
        onClicked: {
            PopupUtils.close(root)
        }
    }
}
