import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ColumnLayout {
    id: root
    property alias cfg_thickness: thickness.value
    RowLayout {
        Label { text: i18n("Strip thickness (pixels):") }
        SpinBox { id: thickness; from: 1; to: 64; editable: true }
    }
    Label {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        text: i18n("Height on a left or right panel; width on a top or bottom panel. Pixels follow desktop scaling.\nOrange: Meta locked. Blue: scrolling locked. Gray: inactive. Red: mouse or service unavailable.")
    }
    Item { Layout.fillHeight: true }
}
